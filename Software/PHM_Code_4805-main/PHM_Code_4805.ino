/*
 * PHM Project 4806
 * Kourosh T. Khoie, Corwin Warner
 *
 * Simple data sensing kit. Writes data to a .bin and .csv file.
 * Hopefully SD card reader doesn't suffer from any jank (and it's not one of the ones
 * where you have to avoid using a resistor)
 */
// every include minus FreeRTOS stuff.



//This line makes or breaks the code.
//Optimizer settings for the compiler 
// - https://www.elektormagazine.com/labs/arduino-compiler-optimizations-for-faster-smaller-code
// - https://ucexperiment.wordpress.com/2017/05/20/modify-arduino-optimization-levels-on-the-fly/
// - https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
#pragma GCC optimize ("-O1")


#include <Arduino.h>
#include <avr/wdt.h>
// Include ALL of the designed libraries

// we prob won't need these anymore, but i left them in anyway
//#include <Arduino_FreeRTOS.h>
//#include <semphr.h>
#include "definitions.hpp"
#include "ESC_Controller.hpp"
#include "pitches.h"
#include "Datapack.h"
#include "Sensors.hpp"
#include "Log.hpp"

//------------------------------ EXTERNAL ----------------------------------------
// For cycle counting
#include <EEPROM.h>
// For the I2C devices
#include <Wire.h>
// For the SD Card
#include <SPI.h>
//#include <SD.h>

// For the thermocouple
//#include <OneWire.h>
#include <DallasTemperature.h>
#include <NonBlockingDallas.h>

// For the inertia sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>

// For the VCur sensor (unused)
#include <Adafruit_INA219.h>

// For the humidity sensor
#include <Adafruit_SHT4x.h>

// For the i2c screen
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// For the Real-Time-Clock
#include <DS3231.h>

// For the thermistor(s)
#include <Thermistor.h>
#include <NTC_Thermistor.h>
#include <AverageThermistor.h>

// this was included already...
//#include <SD.h>

// using sdfat to boost performance
// the setup seems to lean towards the smaller boards, but I think it should help.
//  #include <BufferedPrint.h>
//  #include <FreeStack.h>
//  #include <MinimumSerial.h>
//  #include <RingBuf.h>
//  #include <SdFat.h>
//  #include <SdFatConfig.h>
//  #include <sdios.h>
//#include <BufferedPrint.h>
//#include <FreeStack.h>
//#include <MinimumSerial.h>
//#include <RingBuf.h>
//#include <SdFatConfig.h>
// don't forget the BOY (should also include the next three if you need em)
//#include "utility\SdFatUtil.h" //sd includes an old version of sdfat, but for some reason the freeram in the new version doesn't work for me ;-;
//#include "FreeStack.h"
//#include "ExFatLogger.h"
//#include <MemoryFree.h>;
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

// The I2C ADC Extension
//#include <ADS1115-Driver.h>

//================================ THE  SETUP ========================================

// assuming buttons aren't getting switched up:
#define BUTTON_PRIME 50
#define BUTTON_HALT 51
#define BUTTON_X 52
#define SAFETY_PIN 4
#define ESTOP_PIN 3

// the pins! obviously, change these to whatever you actually use.
// pinout:
// const uint8_t PIN_LIST[] = { 0, 1, 2, 3, 4 };

volatile ESC_Controller *esc;
// LOG *logger;
// Sensors *sensors;
// SENSOR_CONTAINER *hold;


#define USE_INJECTS 0

#if USE_INJECTS
#define INJECT_IM_ALIVE()   Serial_printf("Survived line %d, file %s", __LINE__, __FILE__)
#else
#define INJECT_IM_ALIVE()
#endif





//----------------------------- THE ACTUAL SETUP -------------------------------------

//void setup() __attribute__((optimize("-O1")));
//void loop() __attribute__((optimize("-O1")));




void setup()
{
  // DISABLE THE WATCHDOG ON BOOT JUST IN CASE
  wdt_disable();

  // Serial debug control
  Serial.begin(SERIAL_0_BAUDRATE);
  Wire.begin();
  SPI.begin();
  Serial.println(F("PASS: Serial, I2C, and SPI are started!"));

  



  //esc = new ESC_Controller(ESC_PIN, TARGET_MOSFET_TEMP_LOW_C, TARGET_MOSFET_TEMP_HIGH_C, 0.25f, 2.5f);
  esc = new ESC_Controller(ESC_PIN, COOLING_FAN_PIN, TARGET_MOSFET_TEMP_LOW_C, TARGET_MOSFET_TEMP_HIGH_C, 0.25f, 2.5f);
  esc->runOFF();
  //Serial.print("SIZE OF ESC IS: ");
  //Serial.println((unsigned long) sizeof(*esc));

  //esc->runCalibrate();
  //Serial.println(F("PASS: Initialized ESC."));
  //acceptTone();



  //pinMode(10, OUTPUT); //prevent being forced into slave mode???
  //digitalWrite(10, HIGH);
  delay(10);
  if (!Serial)
  {
    while (true)
    {
      initFailTone();
    }
  }
  //Serial.println(F("PASS: Initialized Serial."));
  acceptTone();
  // init speaker pin, unsure if you actually need to do this?
  // pinMode(SPKR_PIN, OUTPUT);


  //Setup the pins
  pinMode(SAFETY_PIN, INPUT_PULLUP);
  pinMode(ESTOP_PIN, INPUT_PULLUP);
  pinMode(IMU_INT_PIN, INPUT_PULLUP);
  delay(10);
  //===================================================================
  // EEPROM CHECK FOR FAILURE
  {
    _EEPROM_FAULT type = NO_FAULT;
    EEPROM.get(EEPROM_WATCHDOG_CONTROL_ADDRESS, type);
    if (type != NO_FAULT)
    {

      if(digitalRead(SAFETY_PIN) == LOW && digitalRead(ESTOP_PIN) == LOW)
      {
        Serial.println(F("WARN: --- OVERRIDE SAFETY! --- RESETTING EEPROM!"));
        EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, NO_FAULT);
        goto goto_override;
      }



      Serial.print(F("ERROR: EEPROM CHECK FAILURE :: "));
      Serial.println(EEPROM.read(EEPROM_WATCHDOG_CONTROL_ADDRESS));

      //Force the ESC servo lib to jump low to kill ESC. 
      //pinMode(ESC_PIN, OUTPUT);
      //digitalWrite(ESC_PIN, LOW);


      initFailTone();

      while (true)
      {
        faultDetected();
      }
    }
  }



  Serial.println(F("PASS: Initialized EEPROM."));
  acceptTone();

  //====================================================================
  // Set up control pins
  //pinMode(SAFETY_PIN, INPUT_PULLUP);
  if (digitalRead(SAFETY_PIN) != LOW)
  {
    // EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, SAFETY_FAULT);
    Serial.println(F("ERROR: SAFETY BUTTON NOT PRESSED!"));
    while (true)
    {
      initFailTone();
    }
  }
  //pinMode(ESTOP_PIN, INPUT_PULLUP);




  //skip the safety pin initialization, but enable interrupt initialization. 
  goto_override: 


  //===========================================================================
  //Enable the ESTOP interrupt asap
  attachInterrupt(digitalPinToInterrupt(ESTOP_PIN), estop_interrupt, FALLING);














  //=============================================================================
  //=============================================================================
  // THIS IS THE REAL START OF THE CODE AFTER WATCHDOG IS ENABLED
  // ENABLE THE WATCHDOG TIMER
  // wdt_enable(WDTO_2S);
  cli();
  WDTCSR = (1 << WDCE) | (1 << WDE);
  //// WDTCSR = (1<<WDIE)| (1<<WDP2) | (1<<WDP1) | (1<<WDP0); //2.0S
  WDTCSR = (1 << WDIE) | (1 << WDP3) | (0 << WDP2) | (0 << WDP1) | (1 << WDP0); // 8.0S maybe?
  sei();
  Serial.println(F("PASS: Initialized watchdog."));
  acceptTone();

  // tone(SPKR_PIN, 550, 5000);
  // delay(5000);
  // noTone(SPKR_PIN);
  //  test all the buzzer tones
  // acceptTone();
  wdt_reset();
  // acceptTone();
  // faultDetected();
  // initFailTone();
  // acceptTone();
  // acceptTone();
  // clearTone();
  // acceptTone();

  // clearTone();

  //===================================================================
  // initialize the ESC as soon as possible to prime the actual ESC programming controls.
  // ESC_Controller esc(ESC_PIN, TARGET_MOSFET_TEMP_LOW_C, TARGET_MOSFET_TEMP_HIGH_C, 1.0f, 2.5f);
  //esc = new ESC_Controller(ESC_PIN, TARGET_MOSFET_TEMP_LOW_C, TARGET_MOSFET_TEMP_HIGH_C, 0.25f, 2.5f);

  //Serial.print("SIZE OF ESC IS: ");
  //Serial.println((unsigned long) sizeof(*esc));

  esc->runCalibrate();
  Serial.println(F("PASS: Initialized ESC."));
  acceptTone();
  wdt_reset();





  //===================================================================
  // initialize the logger
  // LOG log;
  // logger = new LOG();
  LOG logger;
  Serial.println(F("PASS: Initialized LOGGER."));
  acceptTone();
  wdt_reset();
  //Serial.print(F("====FREE RAM:: "));
  //Serial.println(freeMemory());

  //===================================================================
  // start i2C in fast mode
  // Wire.setClock(400000);
  //Wire.begin();
  wdt_reset();





  //===================================================================
  // initialize the sensors
  // Sensors sensors;
  // sensors = new Sensors();
  Sensors sensors;
  Serial.println(F("PASS: Initialized Sensors."));
  //Serial.print(F("====FREE RAM:: "));
  //Serial.println(freeMemory());
  acceptTone();
  wdt_reset();
  Serial.flush();
  delay(1000);




  //====================================================================
  //now attach the interrupts
  pinMode(IMU_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(IMU_INT_PIN), imu_interrupt, FALLING);
  Serial.println(F("PASS: Initialized safety and estop interrupt/pins."));
  acceptTone();



  //===================================================================
  for (auto i = 0; i < 5; i++)
  {
    //Serial.println(F("Poll init sensors..."));
    sensors.poll_sensors();
    wdt_reset();
    INJECT_IM_ALIVE();
  }
  Serial.println(F("Update average sensors..."));
  sensors.update_avg();
  INJECT_IM_ALIVE();
  // SENSOR_CONTAINER hold = sensors->get_avg();
  Serial.println(F("Get average sensors..."));
  //*hold = sensors->get_avg();
  INJECT_IM_ALIVE();
  SENSOR_CONTAINER test = sensors.get_avg();
  INJECT_IM_ALIVE();
  wdt_reset();
  INJECT_IM_ALIVE();

  Serial.println(F("PASS: first container catch successful."));
  Serial.print(F("====FREE RAM:: "));
  Serial.println(freeMemory());
  acceptTone();
  //wdt_reset();










  //===================================================================

  //****************************************************//
  //========DO NOT REMOVE===============================//
  // We will NOT enter the loop function. instead we loop
  //   forever here for logging
  // while (true)
  // {
  //   Serial.println(F("kek"));
  //   //Serial.flush();
  //   delay(2000);
  //   wdt_reset();
  // }

  while (true)
  {
    // get sensor data
    sensors.poll_sensors();
    INJECT_IM_ALIVE();
    sensors.update_avg();
    INJECT_IM_ALIVE();
    // save sensor data to placeholder
    //*hold = sensors->get_avg();
    test = sensors.get_avg();
    INJECT_IM_ALIVE();
    // save placeholder data to esc class
    //esc->update(*hold);
    esc->update(test);
    INJECT_IM_ALIVE();

    wdt_reset();



    // Serial.println(F("cnt, T0, T1, T2, T3, T4, T5, T6, T7, CURR0, CURR1, CURR2, CURR3, TEMPC, RH%"));
    // Serial.print(esc->getCycleCountEEPROM());
    // Serial.print(F(","));
    // Serial.print(test.ntc_thermistor_temp_c[0]);
    // Serial.print(F(","));
    // Serial.print(test.ntc_thermistor_temp_c[1]);
    // Serial.print(F(","));
    // Serial.print(test.ntc_thermistor_temp_c[2]);
    // Serial.print(F(","));
    // Serial.print(test.ntc_thermistor_temp_c[3]);
    // Serial.print(F(","));
    // Serial.print(test.ntc_thermistor_temp_c[4]);
    // Serial.print(F(","));
    // Serial.print(test.ntc_thermistor_temp_c[5]);
    // Serial.print(F(","));
    // Serial.print(test.ntc_thermistor_temp_c[6]);
    // Serial.print(F(","));
    // Serial.print(test.ntc_thermistor_temp_c[7]);
    // Serial.print(F(","));
    // Serial.print(test.currents[0]);
    // Serial.print(F(","));
    // Serial.print(test.currents[1]);
    // Serial.print(F(","));
    // Serial.print(test.currents[2]);
    // Serial.print(F(","));
    // Serial.print(test.currents[3]);
    // Serial.print(F(","));
    // Serial.print(test.sht40_temp);
    // Serial.print(F(","));
    // Serial.print(test.sht40_hum);
    // Serial.println();


    // save data to file (if possible)
    if (!logger.logData(esc->getCycleCountEEPROM(), test))
    {
      INJECT_IM_ALIVE();
      EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, FILE_FAULT);
      faultDetected();
      esc->lock_off();
      esc->update(test);

      while (true)
      {
        faultDetected();

        resetFunc();
      }
    }






    //Serial.flush();
    (delay(250));
    wdt_reset();
    INJECT_IM_ALIVE();
  }
  //========DO NOT REMOVE===============================//
  //****************************************************//
}

//=================================== THE  LOOP ========================================
// csv file design hopefully, HOPEFULLY keeps our data nice and easy to spreadsheetenize
void loop()
{
  // if (!lg.logData())
  // { // this will run logdata every time if it's true, right?
  //   Serial.println("couldnt log anything aaaaAAAAAAA");
  //   Serial.println("//ETERNAL LOOP ENGAGED//");
  //   while (1)
  //   { /*get infinite looped on lmao*/
  //   }
  // }

  // // THIS IS THE END
  // if (!Serial.available())
  //   return;
  // // logfile.close();
  // Serial.println("done, hopefully");
  // while (1)
  //   ;
}

// Watchdog timer interrupt vector routine
ISR(WDT_vect)
{
  EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, WATCHDOG_FAULT);
  Serial.println(F("ERROR: WATCHDOG INTERRUPT!!!"));

  digitalWrite(COOLING_FAN_PIN, LOW);

  // MCUSR = MCUSR & 11110111; //clear the reset flag (bit 3) of MCUSR.

  resetFunc(); // call a reset
  // wdt_disable();
}

// Estop interrupt triggered on red button push
void estop_interrupt()
{
  static bool firstRun = true;


  digitalWrite(COOLING_FAN_PIN, LOW);

  if (firstRun)
  {
    Serial.println(F("ESTOP INTERRUPT!"));
    EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, ESTOP_FAULT);
    esc->lock_off();
    // esc->update
    firstRun = false;

    resetFunc();
  }
}

// IMU interrupt triggered on G's
void imu_interrupt()
{
  static bool firstRun = true;

  digitalWrite(COOLING_FAN_PIN, LOW);
  
  if (firstRun)
  {
    Serial.println(F("IMU INTERRUPT!"));
    EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, INERTIA_FAULT);
    esc->lock_off();

    firstRun = false;

    resetFunc();
  }
}

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char *sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif // __arm__

int freeMemory()
{
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char *>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif // __arm__
}