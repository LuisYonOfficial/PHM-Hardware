#include "ESC_Controller.hpp"

// snipey constructor. need to know a little about the system @ setup time, so no default constructor
ESC_Controller::ESC_Controller(uint8_t _escPin, uint8_t _coolPin, uint16_t _lowTempThresh, uint16_t _highTempThresh, float _calibTimeMin, float _cycleTimeMin)
{
    tdir = d_UP;
    run = r_CALIBRATE;

    // the cooling fan attached to the ESC
    coolingFanPin = _coolPin;
    pinMode(coolingFanPin, OUTPUT);
    digitalWrite(coolingFanPin, LOW);
    // myPin = _pin;
    myESC.attach(_escPin);
    lowTempThreshold = _lowTempThresh;
    highTempThreshold = _highTempThresh;

    calibrationTime = (uint32_t)MINUTE_TO_MS(_calibTimeMin);
    cycleTime = (uint32_t)MINUTE_TO_MS(_cycleTimeMin);
}

// updator-- puts sensor data wherever directed.
void ESC_Controller::update(const SENSOR_CONTAINER &_cont)
{
    // Serial.println(F("ESC UPDATE"));
    switch (run)
    {
    case (r_FAULT_OFF):
        myESC.write(0);
        while (true)
        {
            // call the alarm
            faultDetected();

            // force off
            myESC.write(0);
        }
        return;
        break;
    case (r_CALIBRATE):
        runCalibrate();
        break;
    case (r_OFF):
        runOFF();
        break;
    case (r_ON):
        runHIGH();
        break;
    }

    // for controlling the EEPROM cycle counting and what state we are in.
    thresholdControl(_cont);
}

// lock-off (uses FSM run state to stop you from turning it back on in case of a problem)
void ESC_Controller::lock_off()
{
    run = r_FAULT_OFF;
    runOFF();
}

//============PRIVATE====================
void ESC_Controller::runCalibrate()
{
    // Serial.println(F("ESC Run Calibrate"));
    //  special init code because AHHHHHHHHHHHHHHHHH
    static bool firstRun = true;

    digitalWrite(coolingFanPin, HIGH);

    if (firstRun)
    {
        runHIGH(); // max speed for calib

        wdt_reset();
        delay(1000);
        wdt_reset();
        delay(1000);
        wdt_reset();
        delay(1000);
        wdt_reset();
        delay(1000);
        wdt_reset();
        delay(1000);
        wdt_reset();
        // delay(5000);

        runOFF();

        wdt_reset();
        delay(1000);
        wdt_reset();
        delay(1000);
        wdt_reset();
        delay(1000);
        wdt_reset();
        delay(1000);
        wdt_reset();
        delay(1000);
        wdt_reset();
        firstRun = false;
    }

    wdt_reset();
    // time-based sanity check
    if (millis() < calibrationTime)
    {
        // turn cooling fan on
        digitalWrite(coolingFanPin, HIGH);
        // break;
        return;
    }




    // change state to ON
    run = r_ON;
    digitalWrite(coolingFanPin, LOW); // just force OFF the cooling fan currently
    
    // this is a fast ramp up to prevent locking of the motor on start
    for (int i = SERVO_MIN_RANGE_CALIB; i < SERVO_MAX_RANGE_CALIB; i++)
    {
        myESC.write(i);
        delay(10);
        wdt_reset();
        myESC.write(SERVO_MIN_RANGE_CALIB);
    }

    
}
void ESC_Controller::runHIGH()
{
    // cooling fan OFF while running the ESC
    digitalWrite(coolingFanPin, LOW);
    // Serial.println(F("ESC Run HIGH"));
    myESC.write(SERVO_MAX_RANGE_CALIB);
}
void ESC_Controller::runOFF()
{
    // cooling fan ON while ESC is OFF.
    digitalWrite(coolingFanPin, HIGH);
    // Serial.println(F("ESC Run OFF"));
    myESC.write(SERVO_MIN_RANGE_CALIB);
}
void ESC_Controller::thresholdControl(const SENSOR_CONTAINER &_cont)
{
    static bool dir_lock = T_LOW_TO_HIGH; // false for LOW to HIGH, true for HIGH to LOW

    float avg_temp_of_thermistors = 0.0f;
    float min_temp_of_thermistors = 150.0f;
    float max_temp_of_thermistors = 0.0f;

    for (auto &i : _cont.ntc_thermistor_temp_c)
    {
        avg_temp_of_thermistors += i;
        min_temp_of_thermistors = min(i, min_temp_of_thermistors);
        max_temp_of_thermistors = max(i, max_temp_of_thermistors);
    }
    avg_temp_of_thermistors = avg_temp_of_thermistors / THERMISTORS_SIZE;

    if (max_temp_of_thermistors >= MAX_MOSFET_TEMP_C)
    {
        EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, THERMISTOR_FAULT_HIGH);
        Serial.println(F("ERROR: THERMISTOR TEMP TOO HIGH!"));

        while (true)
        {
            digitalWrite(coolingFanPin, LOW);
            faultDetected();

            resetFunc();
        }
    }
    if (min_temp_of_thermistors <= MIN_MOSFET_TEMP_C)
    {
        EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, THERMISTOR_FAULT_LOW);
        Serial.println(F("ERROR: THERMISTOR TEMP TOO LOW!"));

        while (true)
        {
            digitalWrite(coolingFanPin, LOW);
            faultDetected();

            resetFunc();
        }
    }

    // The actual FSM control for temp cycling
    switch (dir_lock)
    {
    case (T_LOW_TO_HIGH):
        if (avg_temp_of_thermistors > highTempThreshold)
        {
            Serial.println(F(":: ESC OFF"));
            dir_lock = T_HIGH_TO_LOW;
            run = r_OFF;
        }
        break;
    case (T_HIGH_TO_LOW):
        if (avg_temp_of_thermistors < lowTempThreshold)
        {
            Serial.println(F(":: ESC ON"));
            dir_lock = T_LOW_TO_HIGH;
            increaseCycleCountEEPROM();
            run = r_ON;

            // this is a fast ramp up to prevent locking of the motor on start
            for (int i = SERVO_MIN_RANGE_CALIB; i < SERVO_MAX_RANGE_CALIB; i++)
            {
                myESC.write(i);
                delay(10);
                wdt_reset();
            }
        }
        break;
    default:
        Serial.println(F("ERROR: ESC FAULT!"));
        EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, ESC_FAULT);

        while (true)
        {
            faultDetected();

            delay(20);
            resetFunc();
        }
        break;
    }

    Serial.print(F("Avg Therms: "));
    Serial.print(avg_temp_of_thermistors);
    Serial.print(F(" -- Amb Temp: "));
    Serial.println(_cont.sht40_temp);
}

//==========EEPROM==================
// cycle count getter. needs some memory to save the variable.
uint32_t ESC_Controller::getCycleCountEEPROM()
{
    uint32_t cycleCount = 0;
    EEPROM.get(EEPROM_CYCLE_COUNT_ADDRESS, cycleCount);
    return (cycleCount);
}
// cycle count incrementer. Again, needs some memory for the variable.
void ESC_Controller::increaseCycleCountEEPROM()
{
    uint32_t cycleCount = getCycleCountEEPROM();
    // EEPROM.get(EEPROM_CYCLE_COUNT_ADDRESS, cycleCount);
    cycleCount += 1; // shortened for legibility
    EEPROM.put(EEPROM_CYCLE_COUNT_ADDRESS, cycleCount);
}