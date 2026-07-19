

#ifndef _SENSORS_HPP
#define _SENSORS_HPP

#include "Arduino.h"
#include "pins_arduino.h"
#include <EEPROM.h>
#include <avr/wdt.h>

// main definitions file
#include "definitions.hpp"

// The tones
#include "pitches.h"

// Digital DS18B20 Thermocouples
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NonBlockingDallas.h>

// NTC_Thermistor
#include <Thermistor.h>
#include <NTC_Thermistor.h>

// I2C
#include <Wire.h>
// SPI
#include <SPI.h>
// Voltage
#include <Adafruit_INA219.h>
#include <Adafruit_ADS1X15.h>
// Humidity
#include <Adafruit_SHT4x.h>
// IMU
#include <Adafruit_MPU6050.h>
// RTC
#include <DS3231.h>

// maximum permissible temperatures
#define MAX_MOSFET_TEMP_C 125
#define MAX_AMBIENT_TEMP_C 45
// minimum permissible temperatures
#define MIN_MOSFET_TEMP_C 20
#define MIN_AMBIENT_TEMP_C 20
// target values to confirm cycle
#define TARGET_MOSFET_TEMP_HIGH_C 45
#define TARGET_MOSFET_TEMP_LOW_C 35

// pins for the ADC thermistors ========
#define THERMISTOR_MOSFET_0_PIN A0
#define THERMISTOR_MOSFET_1_PIN A1
#define THERMISTOR_MOSFET_2_PIN A2
#define THERMISTOR_MOSFET_3_PIN A3
#define THERMISTOR_MOSFET_4_PIN A4
#define THERMISTOR_MOSFET_5_PIN A5
#define THERMISTOR_MOSFET_6_PIN A6
#define THERMISTOR_MOSFET_7_PIN A7
//--------------------------------------
#define REFERENCE_RESISTANCE 100000
#define NOMINAL_RESISTANCE 100000
#define NOMINAL_TEMPERATURE 25
#define B_VALUE 3950
//======================================

// pins for the ADC current sensors
#define MAIN_CURRENT_0_PIN 8
#define ESC_PHASE_1_PIN 9
#define ESC_PHASE_2_PIN 10
#define ESC_PHASE_3_PIN 11

//
//#define THERMOCOUPLE_AMBIENT_PIN 99
//#define THERMOCOUPLE_ESC_PIN 2

#include <stdarg.h>
static void Serial_printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    static char buf[128];
    memset(buf, 0, sizeof(buf));
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);
    Serial.println(buf);
}

//
//#define GATE_MOSFET_0_PIN 0

//

// I2C
// may need to change these back (A1, A0)
// const uint8_t INA219_MOSFET_0_ADDR PROGMEM = 0x40; //(GND, GND)
// const uint8_t INA219_MOSFET_1_ADDR PROGMEM = 0x41; //(GND, VDD)
// const uint8_t INA219_MOSFET_2_ADDR PROGMEM = 0x46; //(VDD, SDA)
// const uint8_t INA219_MOSFET_3_ADDR PROGMEM = 0x47; //(VDD, SCL)
// const uint8_t INA219_MOSFET_4_ADDR PROGMEM = 0x48; //(SDA, GND)
// const uint8_t INA219_MOSFET_5_ADDR PROGMEM = 0x49; //(SDA, VDD)
#define MOS_ADC_02_ADDR 0x49
#define MOS_ADC_35_ADDR 0x48

// Humidity sensor
#define SHT40_ADDR 0x44
// IMU
#define MPU6050_ADDR 0x69
// RTC
#define DS3231_ADDR 0x68

// IMU Interrupt pin
#define IMU_INT_PIN 2

//
#define THERMISTORS_SIZE 8
#define CURRENTS_SIZE 4
//#define INA219_SIZE 6
#define MOSFET_QUANTITY 6 // used for the VDS measurement
#define HOLDING_CONTAINER_SIZE 5

class DankThermistor
{
public:
    int pin = 0;
    double referenceResistance = 0;
    double nominalResistance = 0;
    double nominalTemperatureCelsius = 0;
    double bValue = 0;
    int adcResolution = 0;

    DankThermistor() = default;
    DankThermistor(int _pin, double _referenceResistance, double _nominalResistance, double _nominalTemperatureCelsius, double _bValue, int _adcResolution)
    {
        pin = _pin;
        referenceResistance = _referenceResistance;
        nominalResistance = _nominalResistance;
        nominalTemperatureCelsius = _nominalTemperatureCelsius;
        bValue = _bValue;
        adcResolution = _adcResolution;
    }

    float getTempC()
    {
        double voltage = analogRead(this->pin);
        //Serial.println("Voltage: " + String(voltage));
        double resistance =  (this->referenceResistance / (this->adcResolution / voltage - 1));
        //Serial.println("Resistance: " + String(resistance));
        double nominalTemperature = nominalTemperatureCelsius + 273.15;
        double inverseKelvin = (1.0 / nominalTemperature) + log(resistance / this->nominalResistance) / this->bValue;
        //Serial_printf("Inverse Kelvin: nominalTemp=%f, nominalResistance=%f, bValue=%f", nominalTemperature, this->nominalResistance, this->bValue);
        //Serial.print("Inverse Kelvin: nominalTemp" + String(nominalTemperature));
        //Serial.print(" nominalResistance: " + String(this->nominalResistance));
        //Serial.println(" bValue: " + String(this->bValue));
        
        
        double kelvin = 0;
        if (inverseKelvin != NAN)
        {
            kelvin = 1.0 / inverseKelvin;
        }
        else
        {
            Serial.println("KELVIN DIVISION BY ZERO");
            Serial.println("HERE IS THE KELVIN VALUE?? :: " + String(inverseKelvin));
        }

        //Serial.println("Here is the kelvin value: " + String(kelvin));
        double celsius = kelvin - 273.15;
        return celsius;
    }
};

struct _SENSOR_CONTAINER
{
    DateTime time;

    // uint16_t thermistors_ADC[THERMISTORS_SIZE];
    //  NTC_Thermistor ntc_thermistors[THERMISTORS_SIZE];
    float ntc_thermistor_temp_c[THERMISTORS_SIZE];

    // uint16_t currents_ADC[CURRENTS_SIZE];
    float currents[CURRENTS_SIZE];

    // float thermocouple_ambient_c;
    // float thermocouple_esc_c;

    // float ina219_voltage[INA219_SIZE];
    float mosfet_vds[MOSFET_QUANTITY];

    float sht40_hum;
    float sht40_temp;
    // float mpu6050_inertia;
};
typedef struct _SENSOR_CONTAINER SENSOR_CONTAINER;

// it's a function handle that is registered with some vector handler
// void handle_DS_IntervalElapsed(float temperature, bool valid, int deviceIndex);

// static Thermistor *ntc_thermistors[THERMISTORS_SIZE];

class Sensors
{

public:
    Sensors();
    Sensors(const Sensors &s) = delete;            // no copy constructor
    Sensors &operator=(const Sensors &s) = delete; // no move assignment

    bool poll_sensors();
    bool update_avg();
    const SENSOR_CONTAINER get_avg();

    ~Sensors(); // make a destructor

private:
    // rolling buffer of held data
    SENSOR_CONTAINER hold[HOLDING_CONTAINER_SIZE];
    SENSOR_CONTAINER avg;

    // cursor for ^ array
    uint8_t hold_index;

    // clock
    RTClib myClock;

    // ADC modules for reading the mosfets
    Adafruit_ADS1115 mos_02;
    Adafruit_ADS1115 mos_35;

    // thermistors
    // Thermistor *ntc_thermistors[THERMISTORS_SIZE];
    DankThermistor dtherms[THERMISTORS_SIZE];

    // OneWire oneWire_ESC = OneWire(THERMOCOUPLE_ESC_PIN);
    // DallasTemperature dallasTemp_ESC = DallasTemperature(&oneWire_ESC);
    // NonBlockingDallas sensorDs18b20_ESC = NonBlockingDallas(&dallasTemp_ESC);

    // OneWire oneWire_AMBIENT = OneWire(THERMOCOUPLE_AMBIENT_PIN);
    // DallasTemperature dallasTemp_AMBIENT = DallasTemperature(&oneWire_AMBIENT);
    // NonBlockingDallas sensorDs18b20_AMBIENT = NonBlockingDallas(&dallasTemp_AMBIENT);

    // humidity sensor and temp sensor
    Adafruit_SHT4x sht4 = Adafruit_SHT4x();
    sensors_event_t sht4_hum, sht4_temp;
    // inertia measurement unit
    Adafruit_MPU6050 imu = Adafruit_MPU6050();

    // forcefully initializes the sensor container given to idle values
    void init_sensor_container(SENSOR_CONTAINER &item)
    {
        item.time = DateTime(); // save time (but not cputime or memory)

        // prepare placeholders for data (cycles the arrays)
        for (auto &t : item.ntc_thermistor_temp_c)
            t = 0.0;
        // for (auto &t : item.currents_ADC) t = 0;
        for (auto &t : item.currents)
            t = 0.0f;

        // item.thermocouple_ambient_c = 0.0f;
        // item.thermocouple_esc_c = 0.0f;

        for (auto &t : item.mosfet_vds)
            t = 0.0f;

        item.sht40_hum = 0.0f;
        item.sht40_temp = 0.0f;
    };

    // a linear map
    inline float f_map(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

protected:
};

#endif