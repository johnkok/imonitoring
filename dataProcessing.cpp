#include "ioko.hpp"
#include <cv.h>
#include <opencv/cxcore.h>
#include <highgui.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

using namespace std;
using namespace cv;

extern int getGPIO(unsigned char);

extern float intTemp;
extern float extTemp;
extern float intHum;
extern float extHum;
extern float windSpeed;
extern float windDir;
extern float windGust;
extern float pressure;
extern float rain;
extern float light;
extern float uv;
extern float DayYield;

char alarmText[256] = "No alarms";
char peopleText[256] = "No people detected";

void updateDataOnImage(IplImage* ImageOverall){
	// Add Status information
	CvFont font;
	string line, line2;
	ifstream dataFile, dataFile2;
	char tempC[128];
	float temp;
	time_t now;
	struct tm * fileTime;

	cvInitFont(&font, CV_FONT_HERSHEY_DUPLEX, 1.1, 1.1, 0, 2, 8);

	/* Print Time */
	time (&now);
	fileTime = localtime(&now);
	cvPutText(ImageOverall, ctime(&now), cvPoint(700, 30), &font, cvScalar(180,50,220));

	/* Print Alarms status */
	if (getGPIO(0)){
		cvPutText(ImageOverall, "Ok", cvPoint(140, 990), &font, cvScalar(0,255,0));
	}else{
		cvPutText(ImageOverall, "Alm", cvPoint(140, 990), &font, cvScalar(20,20,255));
		sprintf(alarmText, "Last Alm1: %2.2d/%2.2d/%2.2d-%2.2d:%2.2d", 
				fileTime->tm_mday, (fileTime->tm_mon + 1),(fileTime->tm_year + 1900), 
				fileTime->tm_hour, fileTime->tm_min);
	}
	if (getGPIO(1)){
		cvPutText(ImageOverall, "Ok", cvPoint(140, 1030), &font, cvScalar(0,255,0));
	}else{
		cvPutText(ImageOverall, "Alm", cvPoint(140, 1030), &font, cvScalar(20,20,255));	
		sprintf(alarmText, "Last Alm2: %2.2d/%2.2d/%2.2d-%2.2d:%2.2d", 
				fileTime->tm_mday, (fileTime->tm_mon + 1),(fileTime->tm_year + 1900), 
				fileTime->tm_hour, fileTime->tm_min);
	}
	if (getGPIO(2)){
		cvPutText(ImageOverall, "Ok", cvPoint(140, 1070), &font, cvScalar(0,255,0));
	}else{
		cvPutText(ImageOverall, "Alm", cvPoint(140, 1070), &font, cvScalar(20,20,255));
		sprintf(alarmText, "Last Alm3: %2.2d/%2.2d/%2.2d-%2.2d:%2.2d", 
				fileTime->tm_mday, (fileTime->tm_mon + 1),(fileTime->tm_year + 1900), 
				fileTime->tm_hour, fileTime->tm_min);
	}

	/* Print server statistics */
	system("cp -f /sys/class/thermal/thermal_zone0/temp /tmp/cpu_temp");
	dataFile.open ("/tmp/cpu_temp", std::ifstream::in);
	getline (dataFile,line);
	temp = atof(line.c_str());
	sprintf(tempC, "%3.1f C", (temp / 1000.0));
	if (temp > 50000.0){
		cvPutText(ImageOverall, tempC, cvPoint(310, 990), &font, cvScalar(20,20,255));
	}else{
		cvPutText(ImageOverall, tempC, cvPoint(310, 990), &font, cvScalar(0,255,0));
	}
	dataFile.close();

	system("smartctl -d sat --all /dev/sda | grep Temperature_Celsius | awk \'{ gsub(\",\",\"\",$10); print $10 }\' > /tmp/hd_temp");
	dataFile.open ("/tmp/hd_temp", std::ifstream::in);
	getline (dataFile,line);
	temp = atof(line.c_str());
	sprintf(tempC, "%3.1f C", temp);
	if (temp > 40.0){
		cvPutText(ImageOverall, tempC, cvPoint(310, 1030), &font, cvScalar(20,20,255));
	}else{
		cvPutText(ImageOverall, tempC, cvPoint(310, 1030), &font, cvScalar(0,255,0));
	}
	dataFile.close();

	system("df | grep sda1 | awk \'{ gsub(\",\",\"\",$5); print $5 }\' > /tmp/hd_usage");
	dataFile.open ("/tmp/hd_usage", std::ifstream::in);
	getline (dataFile,line);

	system("df | grep sdc1 | awk \'{ gsub(\",\",\"\",$5); print $5 }\' > /tmp/sd_usage");
	dataFile2.open ("/tmp/sd_usage", std::ifstream::in);
	getline (dataFile2,line2);

	sprintf(tempC, "%s/%s", line.c_str(), line2.c_str());
	cvPutText(ImageOverall, tempC, cvPoint(310, 1070), &font, cvScalar(0,255,0));
	dataFile2.close();
	dataFile.close();

	/* Weather Externla temp/hum */
	sprintf(tempC, "%1.1f C", extTemp);
	cvPutText(ImageOverall, tempC, cvPoint(525, 990), &font, cvScalar(0,255,0));
	sprintf(tempC, "%1.0f %%", extHum);
	cvPutText(ImageOverall, tempC, cvPoint(525, 1030), &font, cvScalar(0,255,0));	
	sprintf(tempC, "%3.1f", pressure);
	cvPutText(ImageOverall, tempC, cvPoint(525, 1070), &font, cvScalar(0,255,0));

	/* Wind data */
	sprintf(tempC, "%1.1fkmh", windSpeed);
	cvPutText(ImageOverall, tempC, cvPoint(750, 990), &font, cvScalar(0,255,0));
	sprintf(tempC, "%1.1fkmh", windGust);
	cvPutText(ImageOverall, tempC, cvPoint(750, 1030), &font, cvScalar(0,255,0));	

	switch((int)(windDir/22.5)){
		case (0):
			cvPutText(ImageOverall, "B", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (1):
			cvPutText(ImageOverall, "BBA", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (2):
			cvPutText(ImageOverall, "BA", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (3):
			cvPutText(ImageOverall, "BAA", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (4):
			cvPutText(ImageOverall, "A", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (5):
			cvPutText(ImageOverall, "ANA", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (6):
			cvPutText(ImageOverall, "NA", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (7):
			cvPutText(ImageOverall, "NNA", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (8):
			cvPutText(ImageOverall, "N", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (9):
			cvPutText(ImageOverall, "NND", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (10):
			cvPutText(ImageOverall, "ND", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (11):
			cvPutText(ImageOverall, "DND", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (12):
			cvPutText(ImageOverall, "D", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (13):
			cvPutText(ImageOverall, "DBD", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (14):
			cvPutText(ImageOverall, "BD", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		case (15):
			cvPutText(ImageOverall, "BBD", cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
		default:
			sprintf(tempC, "%1.0f o", windDir);
			cvPutText(ImageOverall, tempC, cvPoint(750, 1070), &font, cvScalar(0,255,0));
			break;
	}

	/* Light - Rain */
	sprintf(tempC, "%1.1fklx", (light / 1000.0));
	cvPutText(ImageOverall, tempC, cvPoint(960, 990), &font, cvScalar(0,255,0));
	sprintf(tempC, "%1.0f", uv);
	cvPutText(ImageOverall, tempC, cvPoint(960, 1030), &font, cvScalar(0,255,0));
	sprintf(tempC, "%1.1fmm", rain);
	cvPutText(ImageOverall, tempC, cvPoint(960, 1070), &font, cvScalar(0,255,0));

	/* interanl temp */
	sprintf(tempC, "%1.1fC", intTemp);
	cvPutText(ImageOverall, tempC, cvPoint(1180, 990), &font, cvScalar(0,255,0));
	sprintf(tempC, "%1.1f%%", intHum);
	cvPutText(ImageOverall, tempC, cvPoint(1180, 1030), &font, cvScalar(0,255,0));
	sprintf(tempC, "%1.1fkWh", DayYield);
	cvPutText(ImageOverall, tempC, cvPoint(1180, 1070), &font, cvScalar(0,255,0));

	cvPutText(ImageOverall, peopleText, cvPoint(1330, 990), &font, cvScalar(0,255,0));
	cvPutText(ImageOverall, alarmText, cvPoint(1330, 1030), &font, cvScalar(0,255,0));

	cvInitFont(&font, CV_FONT_HERSHEY_DUPLEX, 0.8, 0.8, 0, 1, 8);
	cvPutText(ImageOverall, "(c) 2015 - Kokkoris Ioannis", cvPoint(1530, 20), &font, cvScalar(50,50,50));
}

