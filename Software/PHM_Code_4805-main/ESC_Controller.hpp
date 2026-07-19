#ifndef _ESC_CONTROLLER_HPP
#define _ESC_CONTROLLER_HPP

#include "Arduino.h"
#include <avr/wdt.h>
#include "pins_arduino.h"
#include "pitches.h"
#include "Sensors.hpp"


#include <Servo.h>
#include <EEPROM.h>


#define MINUTE_TO_MS(x) (x * 60000)
#define CALIBRATION_DURATION(x) (MINUTE_TO_MS(x))

#define SERVO_MAX_RANGE_CALIB 135 //measured in Degrees, should represent the MEASURED MAX value of servo library output timing.
#define SERVO_MIN_RANGE_CALIB 45 //yes

class ESC_Controller
{

public:
    ESC_Controller() = delete; //no default constructor
    ESC_Controller(const ESC_Controller &e) = delete; //no copy constructor
    ESC_Controller(uint8_t _escPin, uint8_t _coolPin, uint16_t _lowTempThresh, uint16_t _highTempThresh, float _calibTimeMin, float _cycleTimeMin);
    ESC_Controller &operator=(const ESC_Controller &e) = delete; //no move assignment



    //normal operation of the ESC control schema
    void update(const SENSOR_CONTAINER &_cont);
    //force the ESC to shutdown and NOT restart
    void lock_off();
    //for the initial boot of the ESC system
    void runCalibrate();

    void runHIGH();
    void runOFF();

    //returns the current thermal cycle count from EEPROM
    uint32_t getCycleCountEEPROM();

private:

    //Threshold direction for counting thermal cycles
    enum _thresh_direction {d_DOWN, d_UP};
    typedef enum _thresh_direction thresh_direction;

    thresh_direction tdir;

    //FSM run state can either be a:
    //-fault_off condition
    //-calibrate start condition
    //-off, waiting to run
    //-on, waiting to off
    enum _run_state {r_FAULT_OFF, r_CALIBRATE, r_OFF, r_ON};
    typedef enum _run_state run_state;

    run_state run;



    //uint8_t myPin;
    uint16_t lowTempThreshold;
    uint16_t highTempThreshold;

    uint32_t calibrationTime;
    uint32_t cycleTime;

    //the actual ESC instance
    Servo myESC;

    uint8_t coolingFanPin;


    void thresholdControl(const SENSOR_CONTAINER &_cont);

    //increment EEPROM count on a thermal cycle
    void increaseCycleCountEEPROM();

protected:
};




#endif