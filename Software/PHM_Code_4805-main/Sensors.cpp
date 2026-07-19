#include "Sensors.hpp"

Sensors::Sensors()
{
    // initialize the holding containers
    for (auto &item : hold) init_sensor_container(item);
    // initialize the average container
    init_sensor_container(avg);

    //initialize the hold index
    hold_index = 0;

    //The MOSFET VDS ADC devices
    if(!mos_02.begin(MOS_ADC_02_ADDR)){
        EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, I2C_ADC_FAULT);
        Serial.print(F("ERROR: Can't find ADS1115 chip! "));
        Serial.println(MOS_ADC_02_ADDR, HEX);
        while (true){
            initFailTone();
            delay(100);
            resetFunc();
        }
        
    }

    if(!mos_35.begin(MOS_ADC_35_ADDR)){
        EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, I2C_ADC_FAULT);
        Serial.print(F("ERROR: Can't find ADS1115 chip! "));
        Serial.println(MOS_ADC_35_ADDR, HEX);
        while (true){
            initFailTone();
            delay(100);
            resetFunc();
        }
    }
    wdt_reset();









    //------------------------------------------------------------------
    // this is setting up the thermocouples
    //sensorDs18b20_ESC.begin(NonBlockingDallas::resolution_12, NonBlockingDallas::unit_C, 500);
    //sensorDs18b20_AMBIENT.begin(NonBlockingDallas::resolution_12, NonBlockingDallas::unit_C, 500);
    
    //sensorDs18b20_ESC.onIntervalElapsed(handle_DS_IntervalElapsed);
    //sensorDs18b20_AMBIENT.onIntervalElapsed(handle_DS_IntervalElapsed);

    //-------------------------------------------------------------------
    // this is setting up the humidity/temp sensor?
    if(!sht4.begin()){
        EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, I2C_HUMIDITY_FAULT);
        Serial.println(F("ERROR: Couldn't find SHT4X"));
        while(true){
            //Serial.println(F("...sht4x..."));
            initFailTone();
            delay(100);
            resetFunc();
        }
    }
    sht4.setPrecision(SHT4X_MED_PRECISION);
    sht4.setHeater(SHT4X_NO_HEATER);




    
   




    //---------------------------------------------------------------------
    //This is setting up the IMU for interrupt configuration?
    pinMode(IMU_INT_PIN, INPUT_PULLUP);
    if(!imu.begin(MPU6050_ADDR)){
        EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, I2C_IMU_FAULT);
        Serial.println(F("ERROR: Couldn't find IMU"));
        while(true){
            //Serial.println(F("...mpu6050..."));
            initFailTone();
            delay(100);
            resetFunc();
        }
    }
    wdt_reset();
    //imu.reset();


    //imu.setAccelerometerRange(MPU6050_RANGE_2_G);
    //imu.setGyroRange(MPU6050_RANGE_500_DEG);
    //imu.setFilterBandwidth(MPU6050_BAND_94_HZ);
    imu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
    imu.setMotionDetectionThreshold(50);
    imu.setMotionDetectionDuration(20);
    imu.setInterruptPinLatch(true); //50us pulse
    imu.setInterruptPinPolarity(true); //active low
    imu.setMotionInterrupt(true); //enable interrupt

    delay(100); //THIS DELAY IS NECESSARY DO NOT REMOVE OR MODIFY AT ALL!!!!!!!!!!!!
    
    sensors_event_t a, b, c;
    imu.getEvent(&a,&b,&c);
    //imu.
    wdt_reset();






    //while(true)
    //{
    //    Serial.println("HERE LMAO");
    //    delay(1000);
    //}






    //------------------------------------------------------------------
    // this is setting up the ADC in freerunning mode....hopefully
    // noInterrupts();
    // ADCSRB = 0;                                      // ADTS[0-2]=0 - Free Running Mode
    // ADCSRA = bit(ADEN)                               // turn ADC ON
    //          | bit(ADATE)                            // ADC auto trigger enable
    //          | bit(ADIE)                             // Enable Interrupt
    //          | bit(ADPS0) | bit(ADPS1) | bit(ADPS2); // prescaler of 128
    // // ADMUX = bit(REFS0)                               // AVCC
    // //         | ((adc_pin - 14) & 0x07);               // Arduino Uno to ADC pin
    // ADMUX = bit(REFS0)                          // AVCC
    //         | ((THERMISTOR_MOSFET_0_PIN)&0x07); // Arduino Uno to ADC pin
    // bitSet(ADCSRA, ADSC);
    // interrupts();

    //Activate the ntc thermistors.... yes it's allocation
    // ntc_thermistors[0] = new NTC_Thermistor(THERMISTOR_MOSFET_0_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE);
    // ntc_thermistors[1] = new NTC_Thermistor(THERMISTOR_MOSFET_1_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE);
    // ntc_thermistors[2] = new NTC_Thermistor(THERMISTOR_MOSFET_2_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE);
    // ntc_thermistors[3] = new NTC_Thermistor(THERMISTOR_MOSFET_3_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE);
    // ntc_thermistors[4] = new NTC_Thermistor(THERMISTOR_MOSFET_4_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE);
    // ntc_thermistors[5] = new NTC_Thermistor(THERMISTOR_MOSFET_5_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE);
    // ntc_thermistors[6] = new NTC_Thermistor(THERMISTOR_MOSFET_6_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE);
    // ntc_thermistors[7] = new NTC_Thermistor(THERMISTOR_MOSFET_7_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE);

    dtherms[1] = DankThermistor(THERMISTOR_MOSFET_1_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE, 1023);
    dtherms[2] = DankThermistor(THERMISTOR_MOSFET_2_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE, 1023);
    dtherms[3] = DankThermistor(THERMISTOR_MOSFET_3_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE, 1023);
    dtherms[4] = DankThermistor(THERMISTOR_MOSFET_4_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE, 1023);
    dtherms[5] = DankThermistor(THERMISTOR_MOSFET_5_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE, 1023);
    dtherms[6] = DankThermistor(THERMISTOR_MOSFET_6_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE, 1023);
    dtherms[7] = DankThermistor(THERMISTOR_MOSFET_7_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE, 1023);
    dtherms[0] = DankThermistor(THERMISTOR_MOSFET_0_PIN, REFERENCE_RESISTANCE, NOMINAL_RESISTANCE, NOMINAL_TEMPERATURE, B_VALUE, 1023);
    
    //Serial.print("SIZE OF NTC IS: ");
    //Serial.println((unsigned long) sizeof(NTC_Thermistor));



    wdt_reset();
    // int a = therm_mos_0;
}

// void handle_DS_IntervalElapsed(float temperature, bool valid, int deviceIndex){
//   //Serial.print("Sensor ");
//   //Serial.print(deviceIndex);
//   //Serial.print(" temperature: ");
//   //Serial.print(temperature);
//   //Serial.println(" °C");

//   /*
//    *  DO SOME AMAZING STUFF WITH THE TEMPERATURE
//    */

//   if(deviceIndex == THERMOCOUPLE_ESC_PIN) {
//     esc_temp_c = temperature; }
//   else if(deviceIndex == THERMOCOUPLE_AMBIENT_PIN){
//     ambient_temp_c = temperature;}
//   else{
//     Serial.print(F("Could not detect sensors for ds18B20?? "));
//     Serial.println(deviceIndex);}
// }

//destructor doesn't need to do anything extra currently. 
Sensors::~Sensors() { }

bool Sensors::poll_sensors()
{

    // clear the IMU nonsense?
    sensors_event_t a, b, c;
    imu.getEvent(&a, &b, &c);




    //Serial.println("HOLD_INDEX:::: " + hold_index);
    SENSOR_CONTAINER *temp = &hold[hold_index];

    temp->time = myClock.now();


    //temp->ntc_thermistor_temp_c[0] = ntc_thermistors[0].readCelsius();
    //for(uint8_t i=0;i<THERMISTORS_SIZE;i++) temp->ntc_thermistor_temp_c[i] = ntc_thermistors[i]->readCelsius();
     for(uint8_t i=0;i<THERMISTORS_SIZE;i++) temp->ntc_thermistor_temp_c[i] = dtherms[i].getTempC();

    // Currents
    //temp->currents_ADC[0] = main_curr_0; // analogRead(MAIN_CURRENT_0_PIN);
    //temp->currents_ADC[1] = esc_phase_1; // analogRead(ESC_PHASE_1_PIN);
    //temp->currents_ADC[2] = esc_phase_2; // analogRead(ESC_PHASE_2_PIN);
    //temp->currents_ADC[3] = esc_phase_3; // analogRead(ESC_PHASE_3_PIN);
    
    //temp->currents_ADC[0] = analogRead(MAIN_CURRENT_0_PIN);
    //temp->currents_ADC[1] = analogRead(ESC_PHASE_1_PIN);
    //temp->currents_ADC[2] = analogRead(ESC_PHASE_2_PIN);
    //temp->currents_ADC[3] = analogRead(ESC_PHASE_3_PIN);

    temp->currents[0] = f_map(analogRead(MAIN_CURRENT_0_PIN), 0, 1023, -30.0f, 30.0f);
    temp->currents[1] = f_map(analogRead(ESC_PHASE_1_PIN), 0, 1023, -30.0f, 30.0f);
    temp->currents[2] = f_map(analogRead(ESC_PHASE_2_PIN), 0, 1023, -30.0f, 30.0f);
    temp->currents[3] = f_map(analogRead(ESC_PHASE_3_PIN), 0, 1023, -30.0f, 30.0f);
    // do a non-blocking read here instead
    // temp->thermocouple_ambient_c = x;
    // temp->thermocouple_esc_c = x;

    // voltages of mosfets
    // temp->ina219_voltage[0] = ina219_0.getShuntVoltage_mV();
    // temp->ina219_voltage[1] = ina219_1.getShuntVoltage_mV();
    // temp->ina219_voltage[2] = ina219_2.getShuntVoltage_mV();
    // temp->ina219_voltage[3] = ina219_3.getShuntVoltage_mV();
    // temp->ina219_voltage[4] = ina219_4.getShuntVoltage_mV();
    // temp->ina219_voltage[5] = ina219_5.getShuntVoltage_mV();

    //one of these will be negated
    temp->mosfet_vds[0] = (float) mos_02.readADC_Differential_2_3() * 0.1875F * 2; //neg
    temp->mosfet_vds[1] = (float) mos_02.readADC_Differential_1_3() * 0.1875F * 2; //neg
    temp->mosfet_vds[2] = (float) mos_02.readADC_Differential_0_3() * 0.1875F * 2; //neg
    temp->mosfet_vds[3] = (float) mos_35.readADC_Differential_0_3() * 0.1875F * 2;
    temp->mosfet_vds[4] = (float) mos_35.readADC_Differential_1_3() * 0.1875F * 2;
    temp->mosfet_vds[5] = (float) mos_35.readADC_Differential_2_3() * 0.1875F * 2;

    // humidity
    //temp->sht40_hum = sht4.
    sht4.getEvent(&sht4_hum, &sht4_temp);
    temp->sht40_hum = sht4_hum.relative_humidity;
    temp->sht40_temp = sht4_temp.temperature;


    // inertia
    // temp->mpu6050_inertia = x;
    //imu.
    //Serial.println("Here 1");
    hold_index++;
    if(hold_index > (HOLDING_CONTAINER_SIZE - 1)) hold_index = 0;
    //Serial.println("Here 2");

    wdt_reset();
    return (true);
}

bool Sensors::update_avg()
{
    init_sensor_container(avg);
    //Serial.println("Here 3");


    avg.time = myClock.now(); //get the time again
    //avg.time = hold->time; //copy the time

    uint8_t i = 0;
    for (auto &tech : hold) {
        // sum thermistors
        for (i = 0; i < THERMISTORS_SIZE; i++) avg.ntc_thermistor_temp_c[i] += tech.ntc_thermistor_temp_c[i];

        //avg.thermistors_ADC[i] += tech.thermistors_ADC[i];
        // sum currents
        //for (i = 0; i < CURRENTS_SIZE; i++) avg.currents_ADC[i] += tech.currents_ADC[i];
        for(i=0;i<CURRENTS_SIZE;i++) avg.currents[i] += tech.currents[i];

        // sum thermocouples
        //avg.thermocouple_ambient_c += tech.thermocouple_ambient_c;
        //avg.thermocouple_esc_c += tech.thermocouple_esc_c;

        // sum ina219s
        // for (i = 0; i < INA219_SIZE; i++)
        // {
        //     avg.ina219_voltage[i] += tech.ina219_voltage[i];
        // }

        // sum the mosfet VDS
        for(i=0;i<MOSFET_QUANTITY;i++) avg.mosfet_vds[i] += tech.mosfet_vds[i];

        // sum humidity
        avg.sht40_hum += tech.sht40_hum;
        avg.sht40_temp += tech.sht40_temp;
        // sum inertia
        //avg.mpu6050_inertia += tech.mpu6050_inertia;
    }

    for (i = 0; i < THERMISTORS_SIZE; i++) avg.ntc_thermistor_temp_c[i] = avg.ntc_thermistor_temp_c[i] / HOLDING_CONTAINER_SIZE;
        //avg.thermistors_ADC[i] = avg.thermistors_ADC[i] / HOLDING_CONTAINER_SIZE;
       
    //for (i = 0; i < CURRENTS_SIZE; i++) avg.currents_ADC[i] = avg.currents_ADC[i] / HOLDING_CONTAINER_SIZE;
    for(i=0;i<CURRENTS_SIZE;i++) avg.currents[i] = avg.currents[i] / HOLDING_CONTAINER_SIZE;

    //avg.thermocouple_ambient_c = avg.thermocouple_ambient_c / HOLDING_CONTAINER_SIZE;
    //avg.thermocouple_esc_c = avg.thermocouple_esc_c / HOLDING_CONTAINER_SIZE;

    // for (i = 0; i < INA219_SIZE; i++)
    // {
    //     avg.ina219_voltage[i] = avg.ina219_voltage[i] / HOLDING_CONTAINER_SIZE;
    // }

    for(i=0;i<MOSFET_QUANTITY;i++) avg.mosfet_vds[i] = avg.mosfet_vds[i] / HOLDING_CONTAINER_SIZE;

    avg.sht40_hum = avg.sht40_hum / HOLDING_CONTAINER_SIZE;
    avg.sht40_temp = avg.sht40_temp / HOLDING_CONTAINER_SIZE;
    //avg.mpu6050_inertia = avg.mpu6050_inertia / HOLDING_CONTAINER_SIZE;

    //Serial.println("Here 4");
    return (true);
}

//avg getter. note: negligible memory saving possible by removing some getters like this one and using public data and looser memory mgmt
const SENSOR_CONTAINER Sensors::get_avg()
{
    //Serial.println("Here 5");
    return (avg);
}
