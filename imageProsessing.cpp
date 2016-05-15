#include "ioko.hpp"
#include <cv.h>
#include <highgui.h>
#include <curl/curl.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <opencv/cxcore.h>
#include <unistd.h>

using namespace std;
using namespace cv;

extern void acquireCamImages(void);
extern void updateDataOnImage(IplImage* ImageOverall);
extern bool AlarmActive;
extern int pd_alarm;
extern char alarmText[];
extern char peopleText[];
extern int newImage;
extern int peopleDetectCam(int i);

int imageConut = 0;

void mergeImages(IplImage* ImageOverall){
	IplImage *Image;
	int ret, i;
	char camName[256];

	// Merge images to overall image
	for (i = 0 ; i < MaxCams ; i++) {
		// Open received image
		sprintf(camName, "%scam_%d.jpg", IMAGES_PATH, i);
		Image = cvLoadImage(camName);
		if (!Image)
			continue;
		// Merge image to overall image
		cvSetImageROI(Image, cvRect(0, 0, Image->width, Image->height));
		cvSetImageROI(ImageOverall, cvRect(((i % 3) * 640), ((i / 3) * 480), Image->width, Image->height));
		cvCopy(Image, ImageOverall, NULL);

		cvReleaseImage(&Image);
	}

	// Merge PV Output
	switch((int)((imageConut/10)%7)){
		case (0):
			sprintf(camName, "%soutTemp.png", IMAGES_PATH);
			break;
		case (1):
			sprintf(camName, "%soutWind.png", IMAGES_PATH);
			break;
		case (2):
			sprintf(camName, "%soutPressure.png", IMAGES_PATH);
			break;
		case (3):
			sprintf(camName, "%soutRain.png", IMAGES_PATH);
			break;
		case (4):
			sprintf(camName, "%soutLight.png", IMAGES_PATH);
			break;
		case (5):
			sprintf(camName, "%soutHum.png", IMAGES_PATH);
			break;
		case (6):
			sprintf(camName, "%skaco.png", IMAGES_PATH);
			break;
		default:
			sprintf(camName, "%skaco.png", IMAGES_PATH);
			break;
	}
	Image = cvLoadImage(camName);
	if (Image){
		cvSetImageROI(Image, cvRect(0, 0, Image->width, Image->height));
		cvSetImageROI(ImageOverall, cvRect((2 * 640), 480, Image->width, Image->height));
		cvCopy(Image, ImageOverall, NULL);
		cvReleaseImage(&Image);
	}else{
		printf("Error opening inverter data image!\n");
	}

	cvResetImageROI(ImageOverall);

	imageConut = imageConut + 1;
}

void mergeWeatherImages(IplImage* WeatherOverall){
	IplImage *Image;
	int ret, i;
	char camName[256];

	// Merge PV Output
	sprintf(camName, "%soutTemp.png", IMAGES_PATH);
	Image = cvLoadImage(camName);
	if (Image){
		cvSetImageROI(Image, cvRect(0, 0, Image->width, Image->height));
		cvSetImageROI(WeatherOverall, cvRect(0, 0, Image->width, Image->height));
		cvCopy(Image, WeatherOverall, NULL);
		cvReleaseImage(&Image);
	}else{
		printf("Error opening inverter data image!\n");
	}

	// Merge PV Output
	sprintf(camName, "%soutHum.png", IMAGES_PATH);
	Image = cvLoadImage(camName);
	if (Image){
		cvSetImageROI(Image, cvRect(0, 0, Image->width, Image->height));
		cvSetImageROI(WeatherOverall, cvRect(640, 0, Image->width, Image->height));
		cvCopy(Image, WeatherOverall, NULL);
		cvReleaseImage(&Image);
	}else{
		printf("Error opening inverter data image!\n");
	}

	// Merge PV Output
	sprintf(camName, "%soutWind.png", IMAGES_PATH);
	Image = cvLoadImage(camName);
	if (Image){
		cvSetImageROI(Image, cvRect(0, 0, Image->width, Image->height));
		cvSetImageROI(WeatherOverall, cvRect(0, 480, Image->width, Image->height));
		cvCopy(Image, WeatherOverall, NULL);
		cvReleaseImage(&Image);
	}else{
		printf("Error opening inverter data image!\n");
	}

	// Merge PV Output
	sprintf(camName, "%soutLight.png", IMAGES_PATH);
	Image = cvLoadImage(camName);
	if (Image){
		cvSetImageROI(Image, cvRect(0, 0, Image->width, Image->height));
		cvSetImageROI(WeatherOverall, cvRect(640, 480, Image->width, Image->height));
		cvCopy(Image, WeatherOverall, NULL);
		cvReleaseImage(&Image);
	}else{
		printf("Error opening inverter data image!\n");
	}

	// Merge PV Output
	sprintf(camName, "%soutRain.png", IMAGES_PATH);
	Image = cvLoadImage(camName);
	if (Image){
		cvSetImageROI(Image, cvRect(0, 0, Image->width, Image->height));
		cvSetImageROI(WeatherOverall, cvRect((2 * 640), 0, Image->width, Image->height));
		cvCopy(Image, WeatherOverall, NULL);
		cvReleaseImage(&Image);
	}else{
		printf("Error opening inverter data image!\n");
	}

	// Merge PV Output
	sprintf(camName, "%soutPressure.png", IMAGES_PATH);
	Image = cvLoadImage(camName);
	if (Image){
		cvSetImageROI(Image, cvRect(0, 0, Image->width, Image->height));
		cvSetImageROI(WeatherOverall, cvRect((2 * 640), 480, Image->width, Image->height));
		cvCopy(Image, WeatherOverall, NULL);
		cvReleaseImage(&Image);
	}else{
		printf("Error opening inverter data image!\n");
	}

	cvResetImageROI(WeatherOverall);

}

void *imagesToVideoThread(void* key){
	// create video
	time_t now;
	struct tm * fileTime;
	char cmd[256];

	printf("imagesToVideo for key %d starting...\n", key);

	time (&now);
	fileTime = localtime(&now);
	printf("video at %4.4d%2.2d%2.2d-%2.2d%2.2d\n", (fileTime->tm_year + 1900), (fileTime->tm_mon + 1), fileTime->tm_mday, fileTime->tm_hour, fileTime->tm_min);
	sprintf(cmd, "cd %s", IMAGES_PATH);
	system(cmd);

	sprintf(cmd,"mkdir /var/www/html/Cam/%4.4d", 
		(fileTime->tm_year + 1900));
	system(cmd); 

	sprintf(cmd,"mkdir /var/www/html/Cam/%4.4d/%4.4d%2.2d", 
		(fileTime->tm_year + 1900), 
		(fileTime->tm_year + 1900), (fileTime->tm_mon + 1));
	system(cmd); 

	sprintf(cmd,"mkdir /var/www/html/Cam/%4.4d/%4.4d%2.2d/%4.4d%2.2d%2.2d", 
		(fileTime->tm_year + 1900), 
		(fileTime->tm_year + 1900), (fileTime->tm_mon + 1),
		(fileTime->tm_year + 1900), (fileTime->tm_mon + 1), fileTime->tm_mday);
	system(cmd); 

	if (AlarmActive || pd_alarm){
		if (AlarmActive){
			sprintf(alarmText, "Last Alarm: %2.2d/%2.2d/%2.2d-%2.2d:%2.2d", 
				fileTime->tm_mday, (fileTime->tm_mon + 1),(fileTime->tm_year + 1900), 
				fileTime->tm_hour, fileTime->tm_min);
		}else{
			sprintf(peopleText, "Last People: %2.2d/%2.2d/%2.2d-%2.2d:%2.2d", 
				fileTime->tm_mday, (fileTime->tm_mon + 1),(fileTime->tm_year + 1900), 
				fileTime->tm_hour, fileTime->tm_min);
		}
		AlarmActive = false;
		pd_alarm = 0;
		
		sprintf(cmd, "ffmpeg -y -i /var/www/temp/overall_%d%%4d.jpg -preset veryfast -filter:v \"setpts=10.0*PTS\" -c:v libx264 -an /var/www/html/Cam/%4.4d/%4.4d%2.2d/%4.4d%2.2d%2.2d/%4.4d%2.2d%2.2d-%2.2d%2.2d_A.mp4",

			key, (fileTime->tm_year + 1900),
			(fileTime->tm_year + 1900), (fileTime->tm_mon + 1),
			(fileTime->tm_year + 1900), (fileTime->tm_mon + 1), fileTime->tm_mday,
			(fileTime->tm_year + 1900), (fileTime->tm_mon + 1), fileTime->tm_mday,
			 fileTime->tm_hour, fileTime->tm_min);
	}else{
		sprintf(cmd, "ffmpeg -y -i /var/www/temp/overall_%d%%4d.jpg -preset veryfast -filter:v \"setpts=10.0*PTS\" -c:v libx264 -an /var/www/html/Cam/%4.4d/%4.4d%2.2d/%4.4d%2.2d%2.2d/%4.4d%2.2d%2.2d-%2.2d%2.2d.mp4",
			key, (fileTime->tm_year + 1900),
			(fileTime->tm_year + 1900), (fileTime->tm_mon + 1),
			(fileTime->tm_year + 1900), (fileTime->tm_mon + 1), fileTime->tm_mday,
			(fileTime->tm_year + 1900), (fileTime->tm_mon + 1), fileTime->tm_mday,
			 fileTime->tm_hour, fileTime->tm_min);
	}
	system(cmd);

	sprintf(cmd, "rm -f /var/www/temp/overall_%d*.jpg", key);
	system(cmd);
	system("rm -f /var/www/temp/cam_*.jpg");

	printf("imagesToVideo for key %d finished.\n", key);
}

void *imagesPeopleDetectThread(void* i){

	while(1){
		if (newImage){
			newImage = 0;
			peopleDetectCam(1);
		}else{
			sleep(1);
		}
	}

}

void imagesToVideo(int key){
	pthread_t i2v_thread;
	int ret;
	
	ret = pthread_create( &i2v_thread, NULL, imagesToVideoThread, (void*)key);
	if(ret){
	    fprintf(stderr,"Error - pthread_create() return code: %d\n",ret);
	}
}

void imagesPeopleDetect(void){
	pthread_t pd_thread;
	int ret;
	
	ret = pthread_create( &pd_thread, NULL, imagesPeopleDetectThread, (void*)0);
	if(ret){
	    fprintf(stderr,"Error - pthread_create() return code: %d\n",ret);
	}
}

void processWeather(void){
	char fileName[256];
	char bgName[256];
	IplImage *Image;

	// Create a new overall image
	IplImage *WeatherOverall = cvCreateImage(cvSize(640 * 3, 480 * 2), IPL_DEPTH_8U, 3);
	//cvSet(ImageOverall, CV_RGB(0,0,0));

	// Merge Images
	mergeWeatherImages(WeatherOverall);

	// Store Overall Image
	sprintf(fileName, "%sweather.jpg", IMAGES_PATH);
	cvSaveImage( fileName, WeatherOverall);

	sprintf(fileName, "cp -f %sweather.jpg /var/www/html/weather.jpg", IMAGES_PATH);
	system(fileName);
	system("convert  -resize 40% /var/www/html/weather.jpg /var/www/html/weather_r.jpg");
	cvReleaseImage(&WeatherOverall);
}

void processImages(int key, int index){
	char fileName[256];
	char bgName[256];
	IplImage *Image;

	acquireCamImages();

	// Create a new overall image
	IplImage *ImageOverall = cvCreateImage(cvSize(1920, 1080), IPL_DEPTH_8U, 3);
	//cvSet(ImageOverall, CV_RGB(0,0,0));

	sprintf(bgName, "%sbg.png", IMAGES_PATH);
	Image = cvLoadImage(bgName);
	if (Image){
		cvCopy(Image, ImageOverall, NULL);
		cvReleaseImage(&Image);
	}else{
		printf("Error opening bg image!\n");
	}

	// Merge Images
	mergeImages(ImageOverall);

	// Display Status data on Image
	updateDataOnImage(ImageOverall);

	// Store Overall Image
	sprintf(fileName, "%soverall_%d%4.4d.jpg", IMAGES_PATH, key, index);
	cvSaveImage( fileName, ImageOverall);

	sprintf(fileName, "cp -f %soverall_%d%4.4d.jpg /var/www/html/overall.jpg", IMAGES_PATH, key, index);
	system(fileName);
	system("convert  -resize 35% /var/www/html/overall.jpg /var/www/html/overall_r.jpg");

	sprintf(fileName, "cp -f /var/www/html/overall.jpg /var/www/html/overall_%1.1d.jpg", index%10);
	system(fileName);
	sprintf(fileName, "cp -f /var/www/html/overall_r.jpg /var/www/html/overall_r_%1.1d.jpg", index%10);
	system(fileName);

	cvReleaseImage(&ImageOverall);
}
