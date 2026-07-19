#ifndef _DEFINITIONS
#define _DEFINITIONS


//===============DEBUGGING============================================================
// baudrate for UART debugging
#define SERIAL_0_BAUDRATE 115200


//force reset function
static void(* resetFunc)(void) = 0; //reset function at address 0.
//====================================================================================



//=========================ESC CONTROL================================================
//PWM Servo pin for the ESC communication wire.
#define ESC_PIN 9

#define COOLING_FAN_PIN 11


#define EEPROM_WATCHDOG_CONTROL_ADDRESS 0
#define EEPROM_CYCLE_COUNT_ADDRESS 4


typedef enum _EEPROM_FAULT{NO_FAULT, ESTOP_FAULT, INIT_FAULT, FILE_FAULT, POWER_FAULT, CLOCK_FAULT,
 THERMISTOR_FAULT_LOW, THERMISTOR_FAULT_HIGH, CURRENT_FAULT, ESC_FAULT, I2C_ADC_FAULT, I2C_HUMIDITY_FAULT,
 I2C_IMU_FAULT, INERTIA_FAULT, WATCHDOG_FAULT};


//temperature control for EEPROM cycle increment dependent on dir_lock
 #define T_LOW_TO_HIGH 0
 #define T_HIGH_TO_LOW 1

//_EEPROM_FAULT test;
//====================================================================================





















//==========================SPI LOGGING=================================================
// SPI - SD Card
//#define MOSI 51
//#define MISO 50
//#define CS 53
//#define SCK 52




#endif