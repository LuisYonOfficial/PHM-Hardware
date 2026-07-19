

#ifndef _LOG_HPP
#define _LOG_HPP

#include "definitions.hpp"
#include "pitches.h"
#include "Sensors.hpp"

#include <SPI.h>
#include <BufferedPrint.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <RingBuf.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <sdios.h>


#include <DS3231.h>

#include <EEPROM.h>

class LOG
{
public:
    LOG();
    LOG(const LOG &l) = delete;            // no copy constructor
    LOG &operator=(const LOG &l) = delete; // no move assignment
    // bool logData(Sensors b); //added arguments to control arg count (since it doesn't carry over)
    bool logData(const uint32_t thermalCycleCount, const _SENSOR_CONTAINER &b); // as an alternative, you can do it with just the struct
private:
    // File testfile;
    SdFile root; // this and next variables are needed to build the file.
    SdFat sd;
    SdFile file;
    ofstream logfile;             // debating whether i need this or if we can just use file for everything...
    char fileName[13] = "xx.csv"; // not nsfw or anything, just a placeholder for our iterator below
    // intention is to allow for the creation of many numbered files autonomously. When testing ends, they can be put in a folder for a new batch to be made.
    int i = 0;
    char buf[80]; // so we can echo if we gotta, idk im not a big arduino chad

    bool buildFile();   // returns false if sd card is not connected, not 777 (or otherwise read-only), or too full to allow for a file to be created.
    bool writeHeader(); // returns false if the file can't be written, similar reasoning to ^
    // void readDataFile(); //pulls data from the file for serial printing. uses sscanf, which may not be the best way in terms of memory.


    //this is a bad way to pass the RTC through files but it should work.
    //RTClib rtc;
    //Function to timestamp the excel files.
    static void dateTime(uint16_t *date, uint16_t *time)
    {
        RTClib rtc;
        DateTime now = rtc.now();

        // return date using FAT_DATE macro to format fields
        *date = FAT_DATE(now.year(), now.month(), now.day());

        // return time using FAT_TIME macro to format fields
        *time = FAT_TIME(now.hour(), now.minute(), now.second());
    }

protected:
};

#endif
