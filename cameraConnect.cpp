#include "ioko.hpp"
#include <cv.h>
#include <highgui.h>
#include <curl/curl.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <opencv/cxcore.h>
#include <string.h>

using namespace std;
using namespace cv;

int newImage = 0;

int getImageFromRTSP(string url, string destination, string username, string password){


     system("convert -resize 640x480 /var/www/temp/cam_2/00000001.jpg /var/www/temp/cam_2.jpg");

     system("mplayer -vo jpeg:outdir=/var/www/temp/cam_2 -ao null -x 640 -y 480 -frames 1 rtsp://192.168.1.10:554//user=admin_password=tlJwpbo6_channel=1_stream=0.sdp");


	return 0;

}

int getImageFromURL(string url, string destination, string username, string password){
	  CURL *curl;
	  CURLcode res;
	  FILE *imageFile;

	  curl = curl_easy_init();
	   if(curl) {
		 imageFile = fopen(destination.c_str(), "wb");
	     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	     curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	     curl_easy_setopt(curl, CURLOPT_WRITEDATA, imageFile);
	     curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
	     curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
	     curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);

	     res = curl_easy_perform(curl);
	     if(res != CURLE_OK){
	       fprintf(stderr, "curl_easy_perform() failed: %s\n",
	               curl_easy_strerror(res));
	       fclose(imageFile);
	       return -1;
	     }
	     curl_easy_cleanup(curl);
	     fclose(imageFile);
	   }else{
		   fprintf(stderr, "curl_easy_init() failed\n");
		   return -1;
	   }

	return 0;
}

void *getImageFromCamID(void* cam){
	char path[256];

	switch ((long int)cam){
		case 0:
			sprintf(path, "%s%s",IMAGES_PATH, "cam_0.jpg");
			getImageFromURL(Cam_A_URL, 	path, Cam_A_User, Cam_A_Pass);
			newImage = 1;
			break;
		case 1:
			sprintf(path, "%s%s",IMAGES_PATH, "cam_1.jpg");
			getImageFromURL(Cam_B_URL, 	path, Cam_B_User, Cam_B_Pass);
			break;
		case 2:
			sprintf(path, "%s%s",IMAGES_PATH, "cam_2.jpg");
//			getImageFromURL(Cam_C_URL, 	path, Cam_C_User, Cam_C_Pass);
			getImageFromRTSP(Cam_C_URL,      path, Cam_C_User, Cam_C_Pass);
			break;
		case 3:
			sprintf(path, "%s%s",IMAGES_PATH, "cam_3.jpg");
			getImageFromURL(Cam_D_URL, 	path, Cam_D_User, Cam_D_Pass);
			break;
		case 4:
			sprintf(path, "%s%s",IMAGES_PATH, "cam_4.jpg");
			getImageFromURL(Cam_E_URL, 	path, Cam_E_User, Cam_E_Pass);
			break;
		default:
			break;
	}
	pthread_exit(NULL);

}

void acquireCamImages(void){
	pthread_t cam_thread[MaxCams];
	char cmd[256];
	int ret, i;

	sprintf(cmd, "cd %s", IMAGES_PATH);
	system(cmd);
	system("rm -f cam_*.jpg");

	// get images
	for(i=0;i<MaxCams;i++){
		ret = pthread_create( &cam_thread[i], NULL, getImageFromCamID, (void*)i);
		if(ret){
		    fprintf(stderr,"Error - pthread_create() return code: %d\n",ret);
		}
	}

	// Wait for images to be downloaded
	for (i=0; i<MaxCams; i++) {
		pthread_join(cam_thread[i], NULL);
		pthread_detach(cam_thread[i]);
	}

}
