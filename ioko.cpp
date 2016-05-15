/*
 * main.cpp
 *
 *  Created on: Feb 2, 2014
 *      Author: johnkok
 */

#include "ioko.hpp"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

extern void imagesToVideo(int key);
extern void processImages(int key, int index);
extern void updateInverter(void);
extern void modemCommunicationInit(void);
extern void weatherUpdate(void);
extern void peopleDetectInit(void);
extern void imagesPeopleDetect(void);

unsigned char imageRingKey = 0;

int main( int argc, char** argv )
{
	int i;

	system("ulimit -c unlimited");
	system("ulimit -s unlimited");
	system("hdparm -B 254 /dev/sda");

//	peopleDetectInit();

//	updateInverter();

	modemCommunicationInit();

	weatherUpdate();

//	imagesPeopleDetect();

	while (true){
		for (i=0 ; i < MAX_FRAMES ; i++){
			printf ("Image %d%4.4d\n", imageRingKey, i);
			processImages(imageRingKey, i);
			sleep(0.5);
		}

		imagesToVideo(imageRingKey);
		imageRingKey++;
		imageRingKey = imageRingKey%10;
	}

	printf ("Finished\n");

	return 0;
}



