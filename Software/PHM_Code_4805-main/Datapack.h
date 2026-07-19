#pragma once

#ifndef DATAPACK_H
#define DATAPACK_H

#include <DS3231.h>


#define FILE_HEADER "Time,"\ 
"AmbTemp_C,"\ 
"RH%,"\
"AccelX,"\
"AccelY,"\
"AccelZ,"\
"MOSFET1_C,"\ 
"MOSFET2_C,"\
"MOSFET3_C,"\
"MOSFET4_C,"\
"MOSFET5_C,"\
"MOSFET6_C,"\
"VDS_Phase1H,"\ 
"VDS_Phase1L,"\
"VDS_Phase2H,"\
"VDS_Phase2L,"\
"VDS_Phase3H,"\
"VDS_Phase3L,"


struct _DATAPACK
{
	DateTime time; //RTC
	float ambTempC; //DS18B20
	float rh; //SHT40
	float accX; //accelerometer
	float accY;
	float accZ;
	float m1_C; //thermistor
	float m2_C;
	float m3_C;
	float m4_C;
	float m5_C;
	float m6_C;
	float vds_1H; //ADC
	float vds_1L;
	float vds_2H;
	float vds_2L;
	float vds_3H;
	float vds_3L;
	float vgs_1H;
	float vgs_1L;
	float vgs_2H;
	float vgs_2L;
	float vgs_3H;
	float vgs_3L;
};
typedef struct _DATAPACK DATAPACK;


#endif