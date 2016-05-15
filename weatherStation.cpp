#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string>
#include "hidapi.h"
#include "ioko.hpp"
#include <plplot/plstream.h>
#include <time.h>
#include <unistd.h>

//#define DEBUG 1

using namespace std;

extern void processWeather(void);

const int red1[]   = {230, 255,   0,   0,   0, 255,   0};
const int green1[] = {230,   0, 255,   0, 204, 251, 255};
const int blue1[]  = {230,   0,   0, 255,   0,   0, 238};

float intTemp = 0;
float extTemp = 0;
float intHum = 0;
float extHum = 0;
float windSpeed = 0;
float windDir = 0;
float windGust = 0;
float pressure = 0;
float rain = 0;
float light = 0;
float uv = 0;

PLFLT intTempLog[MaxWeatherLogPoints];
PLFLT extTempLog[MaxWeatherLogPoints];
PLFLT intHumLog[MaxWeatherLogPoints];
PLFLT extHumLog[MaxWeatherLogPoints];
PLFLT windSpeedLog[MaxWeatherLogPoints];
PLFLT windDirLog[MaxWeatherLogPoints];
PLFLT windGustLog[MaxWeatherLogPoints];
PLFLT pressureLog[MaxWeatherLogPoints];
PLFLT rainLog[MaxWeatherLogPoints];
PLFLT lightLog[MaxWeatherLogPoints];
//PLFLT uvLog[MaxWeatherLogPoints];

void parseWeatherData(unsigned char* buffer){

	if (buffer[15] & 0x40){
		printf("Error communicating with weather station\n");		
		return;
	}

	intHum = buffer[1];
	intTemp = (short int)(buffer[2] + buffer[3] * 256) / 10.0;
	extHum = buffer[4];
	extTemp = (short int)(buffer[5] + buffer[6] * 256) / 10.0;
	pressure = (buffer[7] + buffer[8] * 256) / 10.0;
	windSpeed = buffer[9] / 10.0;
	windGust = buffer[10] / 10.0;
	windDir = buffer[12] * 22.5;
	rain = buffer[13]* 0.33;
	if (buffer[16] != 0xFF && buffer[17] != 0xFF && buffer[18] != 0xFF)
		light = (buffer[16] + buffer[17] * 256 + buffer[18] * 65536) / 10.0;
	else
		light = 0;
	uv = buffer[19];

	printf("intTemp %f, extTemp %f,intHum %f,extHum %f\nwindSpeed %f,windDir %f,windGust %f\npressure %f,rain %f,light %f\n",
		intTemp,extTemp,intHum,extHum,windSpeed,windDir,windGust,pressure,rain,light);
}

void parseWeatherDataLog(unsigned char* buffer, int index){

	if (index >= MaxWeatherLogPoints  || index < 0){
		return;
	}

	if (buffer[15] & 0x40){
		printf("Error communicating with weather station %d\n", index);	
		if (index < (MaxWeatherLogPoints-1) ){
			intHumLog[index] = intHumLog[index+1];
			intTempLog[index] = intTempLog[index+1];
			extHumLog[index] = extHumLog[index+1];
			extTempLog[index] = extTempLog[index+1];
			pressureLog[index] = pressureLog[index+1];
			windSpeedLog[index] = windSpeedLog[index+1];
			windGustLog[index] = windGustLog[index+1];
			windDirLog[index] = windDirLog[index+1];
			rainLog[index] = rainLog[index+1];
			lightLog[index] = lightLog[index+1];
		//	uvLog[index] = 0;

		}else{		
			intHumLog[index] = 0;
			intTempLog[index] = 0;
			extHumLog[index] = 0;
			extTempLog[index] = 0;
			pressureLog[index] = 0;
			windSpeedLog[index] = 0;
			windGustLog[index] = 0;
			windDirLog[index] = 0;
			rainLog[index] = 0;
			lightLog[index] = 0;
		//	uvLog[index] = 0;
		}
		return;
	}

	intHumLog[index] = buffer[1];
	intTempLog[index] = (short int)(buffer[2] + buffer[3] * 256) / 10.0;
	extHumLog[index] = buffer[4];
	extTempLog[index] = (short int)(buffer[5] + buffer[6] * 256) / 10.0;
	pressureLog[index] = (buffer[7] + buffer[8] * 256) / 10.0;
	windSpeedLog[index] = 3.6 * buffer[9] / 10.0;
	windGustLog[index] = 3.6 * buffer[10] / 10.0;
	windDirLog[index] = buffer[12] * 22.5;
	rainLog[index] = buffer[13] * 0.33;
	lightLog[index] = (buffer[16] + buffer[17] * 256 + buffer[18] * 65536) / 10000.0;
//	uvLog[index] = buffer[19];

}

void diffRain(int i){
int x;

	for (x = (i - 1); x > 1 ; x = (x-2)){
		if(rainLog[x-2] > rainLog[x]){
			rainLog[x] = rainLog[x] + ((255 * 0.33) - rainLog[x-2]);
		}else{
			rainLog[x] = rainLog[x] - rainLog[x-2];
		}
		rainLog[x - 1] = rainLog[x];
	}

	if (i > 2){
		rainLog[1] = rainLog[2];
		rainLog[0] = rainLog[1];
	}
	printf("rain %f %f %f \n", rainLog[0], rainLog[1], rainLog[2]);
}

void plotWeatherData(int index){
int plotIndex = MaxWeatherLogPoints;
int i;
PLFLT Time[MaxWeatherLogPoints];
plstream *pls;
time_t now;

		printf("Plot log graphs %d\n", index);
		now = time(NULL);

		for(i = (MaxWeatherLogPoints - 1) ; i >= 0 ; i--){
			Time[i] = now;
			now = now - (30 * 60); /* 30 minutes sampling*/
		}

		//Plot Temperature Data
	    	pls = new plstream();
	    	pls->setopt("o","outTemp.svg");
	    	pls->setopt("dev","svg");
	    	pls->init();
	    	pls->adv(0);
	    	pls->vpor( 0.1, 0.9, 0.1, 0.9 );
	    	pls->scmap0(red1,green1,blue1,7);
	    	pls->timefmt("%d/%m");
	    	pls->col0(0);
	    	pls->lab( "Last Week", "Temperature [C]", "Temperature Weekly Log");
	    	pls->wind(Time[0], Time[MaxWeatherLogPoints-1], -5.0, 40.0 );
	    	pls->box( "bdnst", 86400, 6, "bmnstvg", 5.0, 0);
	    	pls->col0(1);
	    	pls->width(3);
	    	pls->line( plotIndex, Time, extTempLog);
		// Distractor will create the graph image
	    	delete pls;
		// Convert SVG image format tp png (supported by OpenCV)
	    	system("rsvg-convert -f png -w 640 -h 480 outTemp.svg -o /var/www/temp/outTemp.png");


		//Plot Humidity Data
	    	pls = new plstream();
	    	pls->setopt("o","outHum.svg");
	    	pls->setopt("dev","svg");
	    	pls->init();
	    	pls->adv(0);
	    	pls->vpor( 0.1, 0.9, 0.1, 0.9 );
	    	pls->scmap0(red1,green1,blue1,7);
	    	pls->timefmt("%d/%m");
	    	pls->col0(0);
	    	pls->lab( "Last Week", "Humidity [%]", "Humidity Weekly Log");
	    	pls->wind(Time[0], Time[MaxWeatherLogPoints-1], 0.0, 100.0 );
	    	pls->box( "bdnst", 86400, 0, "bmnstvg", 10.0, 0 );
	    	pls->col0(1);
	    	pls->width(3);
	    	pls->line( plotIndex, Time, extHumLog);
		// Distractor will create the graph image
	    	delete pls;
		// Convert SVG image format tp png (supported by OpenCV)
	    	system("rsvg-convert -f png -w 640 -h 480 outHum.svg -o /var/www/temp/outHum.png");


		//Plot Wind Data
	    	pls = new plstream();
	    	pls->setopt("o","outWind.svg");
	    	pls->setopt("dev","svg");
	    	pls->init();
	    	pls->adv(0);
	    	pls->vpor( 0.1, 0.9, 0.1, 0.9 );
	    	pls->scmap0(red1,green1,blue1,7);
	    	pls->timefmt("%d/%m");
	    	pls->col0(0);
	    	pls->lab( "Last Week", "Speed [km/h]", "Wind Weekly Log");
	    	pls->wind( Time[0], Time[MaxWeatherLogPoints-1], 0.0, 100.0 );
	    	pls->box( "dbnstv", 86400, 0, "bnstv", 10.0, 0 );
	    	pls->col0(1);
	    	pls->width(3);
	    	pls->line( plotIndex, Time, windSpeedLog);
	    	pls->col0(2);
	    	pls->width(2);
	    	pls->line( plotIndex, Time, windGustLog);
	    	pls->width(1);
	    	pls->col0(0);
	    	pls->wind( Time[0], Time[MaxWeatherLogPoints-1], 0.0, 360.0 );
	    	pls->box( "", 0, 0, "cmstv", 45.0, 0 );
	    	pls->col0(3);
	    	pls->width(1);
	    	pls->line( plotIndex, Time, windDirLog);
	    	pls->mtex( "r", 16.0, 22.5, 22.5, "Direction [o]");
		// Distractor will create the graph image
	    	delete pls;
		// Convert SVG image format tp png (supported by OpenCV)
	    	system("rsvg-convert -f png -w 640 -h 480 outWind.svg -o /var/www/temp/outWind.png");


		//Plot Rain Data
	    	pls = new plstream();
	    	pls->setopt("o","outRain.svg");
	    	pls->setopt("dev","svg");
	    	pls->init();
	    	pls->adv(0);
	    	pls->vpor( 0.1, 0.9, 0.1, 0.9 );
	    	pls->scmap0(red1,green1,blue1,7);
		pls->timefmt("%d/%m");
	    	pls->col0(0);
	    	pls->lab( "Last Week", "Rain [mm/h]", "Rain Weekly Log");
	    	pls->wind( Time[0], Time[MaxWeatherLogPoints-1], 0.0, 20.0 );
	    	pls->box( "bdnst", 86400, 0, "bmnstvg", 5.0, 0);
	    	pls->col0(1);
	    	pls->width(3);
	    	pls->line( plotIndex, Time, rainLog);
		// Distractor will create the graph image
	    	delete pls;
		// Convert SVG image format tp png (supported by OpenCV)
	    	system("rsvg-convert -f png -w 640 -h 480 outRain.svg -o /var/www/temp/outRain.png");



		//Plot Light Data
	    	pls = new plstream();
	    	pls->setopt("o","outLight.svg");
	    	pls->setopt("dev","svg");
	    	pls->init();
	    	pls->adv(0);
	    	pls->vpor( 0.1, 0.9, 0.1, 0.9 );
	    	pls->scmap0(red1,green1,blue1,7);
		pls->timefmt("%d/%m");
	    	pls->col0(0);
	    	pls->lab( "Last Week", "Light [klux]", "Light Weekly Log");
	    	pls->wind( Time[0], Time[MaxWeatherLogPoints-1], 0.0, 140.0 );
	    	pls->box( "bdnst", 86400, 0, "bmnstvg", 10.0, 0);
	    	pls->col0(1);
	    	pls->width(3);
	    	pls->line( plotIndex, Time, lightLog);
		// Distractor will create the graph image
	    	delete pls;
		// Convert SVG image format tp png (supported by OpenCV)
	    	system("rsvg-convert -f png -w 640 -h 480 outLight.svg -o /var/www/temp/outLight.png");


		//Plot Pressure Data
	    	pls = new plstream();
	    	pls->setopt("o","outPressure.svg");
	    	pls->setopt("dev","svg");
	    	pls->init();
	    	pls->adv(0);
	    	pls->vpor( 0.1, 0.9, 0.1, 0.9 );
	    	pls->scmap0(red1,green1,blue1,7);
		pls->timefmt("%d/%m");
	    	pls->col0(0);
	    	pls->lab( "Last Week", "At. Pressure [hpa]", "Pressure Log");
	    	pls->wind( Time[0], Time[MaxWeatherLogPoints-1], 960.0, 1060.0);
	    	pls->box( "bdnst", 86400, 0, "bmnstvg", 10.0, 0);
	    	pls->col0(1);
	    	pls->width(3);
	    	pls->line( plotIndex, Time, pressureLog);
		// Distractor will create the graph image
	    	delete pls;
		// Convert SVG image format tp png (supported by OpenCV)
	    	system("rsvg-convert -f png -w 640 -h 480 outPressure.svg -o /var/www/temp/outPressure.png");
}

int readAddress(hid_device *handle, unsigned short int address, unsigned char* buffer){
int cnt = 0;

	buffer[0] = 0xA1;
	buffer[1] = address / 256;
	buffer[2] = (address % 256 );
	buffer[3] = 0x20;
	buffer[4] = 0xA1;
	buffer[5] = address / 256;
	buffer[6] = (address % 256 );
	buffer[7] = 0x20;
	buffer[8] = 0x00;

	if (hid_write(handle, buffer, 8) < 0){
		printf("Error writting hid\n");	
		return -1;
	}	

	cnt =+ hid_read_timeout(handle, &buffer[0], 8, 200);
	cnt =+ hid_read_timeout(handle, &buffer[8], 8, 200);
	cnt =+ hid_read_timeout(handle, &buffer[16], 8, 200);
	cnt =+ hid_read_timeout(handle, &buffer[24], 8, 200);
#ifdef DEBUG
	printf("%4.0d: %2.2X %2.2X %2.2X %2.2X - %2.2X %2.2X %2.2X %2.2X \n", 
		address, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
	printf("%4.0d: %2.2X %2.2X %2.2X %2.2X - %2.2X %2.2X %2.2X %2.2X \n", 
		address, buffer[8], buffer[9], buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15]);
	printf("%4.0d: %2.2X %2.2X %2.2X %2.2X - %2.2X %2.2X %2.2X %2.2X \n", 
		address, buffer[16], buffer[17], buffer[18], buffer[19], buffer[20], buffer[21], buffer[22], buffer[23]);
	printf("%4.0d: %2.2X %2.2X %2.2X %2.2X - %2.2X %2.2X %2.2X %2.2X \n", 
		address, buffer[24], buffer[25], buffer[26], buffer[27], buffer[28], buffer[29], buffer[30], buffer[31]);
#endif
	return cnt;
}

void *weatherUpdateThread(void *id){

hid_device *handle;
unsigned char buffer[64];
unsigned int offset, updCount, lastAddress;
int i;

	updCount = 0;

	while (1){
		handle = hid_open(0x1941, 0x8021, NULL);
		if (handle <= 0){
			printf("Error opening HID device\n");
			hid_close(handle);
			sleep(50);
			continue;
		}

		hid_set_nonblocking(handle, 0);
	
		if (readAddress(handle, 0, buffer) < 0){
			hid_close(handle);
			printf("Error reading HID device\n");
			sleep (50);
			continue;
		}
	
		if (buffer[0] != 0x55 || buffer[1] != 0xAA || buffer[2] != 0xFF || buffer[3] != 0xFF){
			printf("Error reading data header\n");
			sleep(5);
			continue;
		}
		
		lastAddress = buffer[31] * 256 + buffer[30];
		printf ("Weather address: %X\n", lastAddress);

		readAddress(handle, lastAddress, buffer);

		parseWeatherData(buffer);

		if (updCount == 0){ /* Update Logs */
			i = MaxWeatherLogPoints;
			while (lastAddress >= (256 + 20) && i >= 0){
				i--;
				lastAddress = lastAddress - 20;
				readAddress(handle, lastAddress, buffer);
				parseWeatherDataLog(buffer, i);
			}
			diffRain(MaxWeatherLogPoints);
			plotWeatherData(i);
			processWeather();
		}

		hid_close(handle);
		updCount = (updCount + 1) % 72;
		sleep(50);
	}

	return 0;
}


void weatherUpdate(){
	int ret;
	pthread_t weather_thread;

	ret = pthread_create( &weather_thread, NULL, weatherUpdateThread, (void*)0);
	if(ret){
	    fprintf(stderr,"Error - pthread_create() return code: %d\n",ret);
	}
}


