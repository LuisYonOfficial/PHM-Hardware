#include "Log.hpp"

LOG::LOG()
{
  if (!sd.begin(PIN_SPI_SS, SD_SCK_MHZ(SPI_FULL_SPEED)))
  {
    EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, INIT_FAULT);
    Serial.println(F("ERROR: Couldn't init SD Card!"));
    while (true)
    {
      initFailTone();
      resetFunc();
    }
  }
  Serial.println(F("PASS: Init SD Card."));

  if (!file.open("text.bin", O_WRITE | O_CREAT))
  {
    EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, FILE_FAULT);
    Serial.println(F("ERROR: SD can't generate test file!"));
    while (true)
    {
      initFailTone();
      resetFunc();
    }
  }
  Serial.println(F("PASS: test file SD card."));

  if (!buildFile())
  {
    EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, FILE_FAULT);
    Serial.println(F("ERROR: SD can't generate data file!"));
    while (true)
    {
      initFailTone();
      resetFunc();
    }
  }
  Serial.println(F("PASS: data file SD Card Generated."));

  if (!writeHeader())
  {
    EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, FILE_FAULT);
    Serial.println(F("ERROR: File can't generate header!"));
    while (true)
    {
      initFailTone();
      resetFunc();
    }
  }
  Serial.println(F("PASS: generate file headers."));
}

//----------------------------- FILE ITERATOR -------------------------------------
// basically, me likey to dispense a whole bucket of files, so that way we can have
// a lil pile of them, numbered, later without messing with too much, it's safer!
bool LOG::buildFile()
{
  char f_name[25];
  char *ret;
  char ext[5] = ".csv";
  byte LastFileNum = 0;
  byte FileNum = 0;

  root.open("/");
  SdFile::dateTimeCallback(dateTime);
  while (file.openNext(&root, O_RDONLY))
  {
    file.getName(f_name, 25);
    ret = strstr(f_name, ext);
    if (ret)
    {
      FileNum = atoi(&f_name[0]);
      if (FileNum > LastFileNum)
        LastFileNum = FileNum;
    }
    file.close();
  }
  root.close();
  FileNum = LastFileNum + 1;

  // if we need 100s of files instead of 10s, add another line here.
  fileName[0] = (int)((FileNum / 10) % 10) + '0';
  fileName[1] = (int)((FileNum % 10)) + '0';

  if (sd.exists(fileName))
  {
    Serial.print("but da file named ");
    Serial.print(fileName);
    Serial.println(" is already there??? bruh");
    return false;
  }
  return true;
}

//------------------------------ HEADER BUILDER -----------------------------------------
bool LOG::writeHeader()
{
  if (!file.isOpen())
  {
    if (!file.open(fileName, O_WRITE | O_CREAT))
    {
      Serial.println(F("ERROR: Unable to create header in file!"));
      return false;
    }
  }


  file.print(F("tj_cycle,"));
  file.print(F("unixtime,"));
  file.print(F("T0_C,"));
  file.print(F("T1_C,"));
  file.print(F("T2_C,"));
  file.print(F("T3_C,"));
  file.print(F("T4_C,"));
  file.print(F("T5_C,"));
  file.print(F("T6_C,"));
  file.print(F("T7_C,"));
  file.print(F("M0_V,"));
  file.print(F("M1_V,"));
  file.print(F("M2_V,"));
  file.print(F("M3_V,"));
  file.print(F("M4_V,"));
  file.print(F("M5_V,"));
  file.print(F("esc_curr_ADC,"));
  file.print(F("phase1_curr_ADC,"));
  file.print(F("phase2_curr_ADC,"));
  file.print(F("phase3_curr_ADC,"));
  file.print(F("AMB_TEMP_C,"));
  file.print(F("ESC_HUMID_RH,"));

  file.println();
  // file.println("header1,header2,header3,header4,header5,header6,test");
  //  voltage(volts)
  //file.flush();
  file.sync();
  // use your rtc to get a timestamp or something here
  //  like
  /*
#if USE_DS1307
  DateTime now = RTC.now();
  bout << ',' << now;
#endif  // USE_DS1307
*/
  //  or something
  // put in any other stuff you want to be generated here

  file.close();
  //delay(75);
  return true;
}

//------------------------------ DATA LOGGER -----------------------------------------
// sc == Sensor Count
/*
bool LOG::logData(Sensors b)
{
  if (!file.open(fileName, O_APPEND | O_WRITE))
  {
    Serial.println("couldnt add to the file aaaaAAAAAAA");
    Serial.println("//ETERNAL LOOP ENGAGED//");
    while (1)
    { //get infinite looped on lmao
    }
  }
  /*
  float n = 0;
for (uint8_t y = 0; y < sc; y++) {
 #if ADC_DELAY
     delay(ADC_DELAY);
 #endif  // ADC_DELAY
     n = analogRead(y);
     file.print(F(n));
     if(y<sc-1) file.print(F(",")); //you used print in your bit so I assume this'll work...
   }
   file.println(F(" ")); //end the line!

  // if ^ doesn't work, we will fall back on thine glorious work:
  // I had a bigger brain variant originally, but that didn't work, so here:
  if (b.poll_sensors()) // uses your bool as a failsafe
    if (b.update_avg())
    {                                    // uses the next one as a failsafe
      _SENSOR_CONTAINER n = b.get_avg(); // note: the ide got mad when I tried to check the get_avg values in the arduino ide...
      time_t tim = n.time.unixtime();
      const char *tims = ctime(&tim);
      file.print((tims));
      char *t = new char[100];
      for (auto e : n.thermistors_ADC)
      {
        t = e;
        file.print(t);
        file.print(F(","));
      }
      for (auto e : n.currents_ADC)
      {
        t = e;
        file.print(t);
        file.print(F(","));
      }
      file.print((n.thermocouple_ambient_c));
      file.print((n.thermocouple_esc_c));
      for (auto e : n.ina219_voltage)
      {
        file.print(e);
        file.print(F(","));
      }
      file.print((n.sht40_hum));
      file.print(F(","));
      file.print((n.thermocouple_esc_c));
      file.print(F(","));
      file.println((n.mpu6050_inertia)); // no comma, end the line
    }
  file.close();
#if ADC_DELAY
  delay(ADC_DELAY);
#endif // ADC_DELAY
  Serial.print("logged ");
  return true;
}
*/

bool LOG::logData(const uint32_t thermalCycleCount, const _SENSOR_CONTAINER &n)
{


  static uint16_t sync_counter = 0;
  // if (!file.open(fileName, O_APPEND | O_WRITE))
  // {
  //   Serial.println("couldnt add to the file aaaaAAAAAAA");
  //   Serial.println("//ETERNAL LOOP ENGAGED//");
  //   while (1)
  //   { /*get infinite looped on lmao*/
  //   }
  // }


  // if(!sd.exists(fileName))
  // {
  //   Serial.println(F("ERROR: Likely the SD Card was removed!"));
  //   faultDetected();

  //   EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, FILE_FAULT);


  //   resetFunc();
  // }

  // if(!file.isOpen())
  // {
  //   Serial.println(F("FILE IS NOT OPEN>>> CRASH???"));
  //   //faultDetected();

  //   //EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, FILE_FAULT);

  //   //resetFunc();

  //   return(false);
  // }

  if(sync_counter == 0)
  {
    //file.open(fileName, O_WRITE | O_APPEND);
    file.open(fileName, FILE_WRITE);
  }


  if(!file.isOpen())
  {
    Serial.println(F("ERROR: File closed/unavailable!"));

    EEPROM.put(EEPROM_WATCHDOG_CONTROL_ADDRESS, FILE_FAULT);

    resetFunc();
    return(false);
  }


  //thermal cycle count
  file.print(thermalCycleCount);
  file.print(F(","));


  //time
  auto tim = n.time.unixtime();
  //const char *tims = ctime(&tim);
  file.print(tim);
  file.print(F(","));
  // char *t = new char[100];
  

  //t0-t7 (c)
  //for (int i = 0; i < sizeof(b.ntc_thermistor_temp_c); i++) {
  //  file.print(b.ntc_thermistor_temp_c[i]);
  for (auto& i : n.ntc_thermistor_temp_c) {
	  file.print(i);
	  file.print(F(","));
  }
  //m0-m5 (mV)
  // for (auto& i : n.ina219_voltage) {
	//   file.print(i);
	//   file.print(F(","));
  // }
  for(auto& i : n.mosfet_vds)
  {
    file.print(i);
    file.print(F(","));
  }

  //esc curr adc'd (index 0)
  //phase 1-3 curr adc'd
  for (auto& i : n.currents) {
	  file.print(i);
	  file.print(F(","));
  }
  //amb temp (c)
  //file.print(n.thermocouple_ambient_c);
  //file.print(F(","));
  file.print((n.sht40_temp));
  file.print(F(","));
  // esc humid (rh)
  file.print((n.sht40_hum));
  file.print(F(","));
  //file.print((n.mpu6050_inertia)); // no comma, end the line


  file.println();
  //file.flush();
  file.sync();
  //file.close();
//#if ADC_DELAY
// delay(ADC_DELAY);
//#endif // ADC_DELAY
//  Serial.print(F("logged "));
//  Serial.print(thermalCycleCount);



  sync_counter++;

  if(sync_counter >= 10)
  {
    file.close();
    sync_counter = 0;
  }
  return true;
}

//------------------------------ DATA READER -----------------------------------------
// incomplete. doesn't do much, but we prob won't need it
// void LOG::readDataFile()
// {
//   char line[200];
//   char Field0[20];
//   char Field1[20];
//   char Field2[20];

//   int Point = 0;
//   int DataBits = 0;
//   unsigned long DataVolts = 0;
//   int LineLength = 0;

//   // we want something like
//   // sscanf(line, "%i,%20[^,],&IntData,CharData);

//   if (!file.open(fileName, O_READ))
//   {
//     Serial.println("couldnt read anything aaaaAAAAAAA");
//     return; // don't wanna loopenize here so just ret
//   }

//   LineLength = file.fgets(line, sizeof(line));
//   if (line[LineLength - 1] == '\n')
//   {
//     line[LineLength - 1] = 0;
//   }
//   if (line[LineLength - 1] == '\n')
//   {
//     line[LineLength - 1] = 0;
//   }

//   // THE ALL SEEING EYE
//   sscanf(line, "%20[^,],%20[^,],%20[^,]", Field0, Field1, Field2);
// }