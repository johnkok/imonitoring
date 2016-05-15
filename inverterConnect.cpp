#include "ioko.hpp"
#include <curl/curl.h>
#include <pthread.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <plplot/plstream.h>
#include <time.h>
#include <unistd.h>

using namespace std;

const int red0[]   = {230, 255,   0,   0,   0, 255,   0};
const int green0[] = {230,   0, 255,   0, 204, 251, 255};
const int blue0[]  = {230,   0,   0, 255,   0,   0, 238};

float DayYield = 0;

int getInverterData(string url, string destination){
	  CURL *curl;
	  CURLcode res;
	  FILE *dataFile;

	  curl = curl_easy_init();
	   if(curl) {
		 dataFile = fopen(destination.c_str(), "wb");
	     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	     curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	     curl_easy_setopt(curl, CURLOPT_WRITEDATA, dataFile);
	     curl_easy_setopt(curl, CURLOPT_TIMEOUT, 180);

	     res = curl_easy_perform(curl);
	     if(res != CURLE_OK){
	       fprintf(stderr, "curl_easy_perform() failed: %s\n",
	               curl_easy_strerror(res));
	       fclose(dataFile);
	       return -1;
	     }
	     curl_easy_cleanup(curl);
	     fclose(dataFile);
	   }else{
		   fprintf(stderr, "curl_easy_init() failed\n");
		   return -1;
	   }

	return 0;
}

void processInverterData(char* source){
	string line, data;
	ifstream dataFile;
	int plotIndex, index, pos,pos_stop;
	PLFLT Time[MaxInvPlotPoints], Temp[MaxInvPlotPoints], PowerAC[MaxInvPlotPoints];
	PLFLT PowerDC1[MaxInvPlotPoints], PowerDC2[MaxInvPlotPoints];
	char label[256];

	index = 0;
	plotIndex = 0;
	dataFile.open (source, std::ifstream::in);

	if(dataFile){
	    while(!dataFile.eof()){

	    	getline (dataFile,line, (char)0x0d);

	    	if (index < 3){	// First description lines
			if(index == 1){
		    		pos = 0;  // Inverter type
		    		pos_stop = line.find(';',pos);
		    		//data = line.substr(pos,pos_stop-pos);	
	    		
				pos = pos_stop+1;  // Serial number
	    			pos_stop = line.find(';',pos);
		    		//data = line.substr(pos,pos_stop-pos);
	
		    		pos = pos_stop+1;  // RS485 address
		    		pos_stop = line.find(';',pos);
		    		//data = line.substr(pos,pos_stop-pos);

		    		pos = pos_stop+1;  // IP address
		    		pos_stop = line.find(';',pos);
		    		//data = line.substr(pos,pos_stop-pos);

		    		pos = pos_stop+1;  // Yield[kWh]
		    		pos_stop = line.find(';',pos);
		    		data = line.substr(pos,pos_stop-pos);
				DayYield = atof(data.c_str());
			}
	    	}else{

	    		if (line.size() < 20){
	    			continue;
	    		}
	    		if (index == MaxInvPlotPoints){
	    			break;
	    		}

	    		pos = 0;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Time: " + data + "\n").c_str() );

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Udc[1]: " + data + "\n").c_str() );

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Idc[1]: " + data + "\n").c_str() );

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Pdc[1]: " + data + "\n").c_str() );
	    		PowerDC1[plotIndex] = atof(data.c_str()) / 1000;

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Udc[2]: " + data + "\n").c_str() );

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Idc[2]: " + data + "\n").c_str() );

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Pdc[2]: " + data + "\n").c_str() );
	    		PowerDC2[plotIndex] = atof(data.c_str()) / 1000;

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Uac[1]: " + data + "\n").c_str() );

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Iac[1]: " + data + "\n").c_str() );

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Uac[2]: " + data + "\n").c_str() );

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Iac[2]: " + data + "\n").c_str() );

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Uac[3]: " + data + "\n").c_str() );

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Iac[3]: " + data + "\n").c_str() );

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Pdc: " + data + "\n").c_str() );

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Pac: " + data + "\n").c_str() );
	    		PowerAC[plotIndex] = atof(data.c_str()) / 1000;

	    		pos = pos_stop+1;
	    		pos_stop = line.find(';',pos);
	    		data = line.substr(pos,pos_stop-pos);
//	    		printf( ("Temperature: " + data + "\n").c_str() );
	    		Temp[plotIndex] = atof(data.c_str());

	    		Time[plotIndex] = plotIndex ;
	    		plotIndex++;
	    	}
	    	index++;
	    	}
	    	dataFile.close();

	    	//Plot Data
	    	plstream *pls;
	    	pls = new plstream();

	    	pls->setopt("o","kaco.svg");
	    	pls->setopt("dev","svg");
	    	pls->init();

	    	pls->adv(0);
	    	pls->vpor( 0.1, 0.9, 0.1, 0.9 );

	    	pls->scmap0(red0,green0,blue0,7);

	    	pls->timefmt("%M:%S");

	    	sprintf(label, "PV Inverter Output - Daily Yield: %3.2f kWh", DayYield);
	    	pls->col0(0);
	    	pls->lab( "Time [hrs]", "", label);
	    	pls->wind( 0.0, plotIndex, 0.0, 10.0 );
	    	pls->box( "bnstdv", 60, 6, "", 0.0, 0 );

	    	pls->wind( 0.0, plotIndex, 0.0, 10.0 );
	    	pls->box( "0", 0, 0, "bmnstvg", 1.0, 0 );
	    	pls->col0(1);
	    	pls->lab( "", "AC Power [kW]", "" );
	    	pls->width(3);
	    	pls->line( plotIndex, Time, PowerAC);

		// Plot Inverter inputs
	    	pls->width(3);
	    	pls->col0(5);
	    	pls->line( plotIndex, Time, PowerDC1);
	    	pls->col0(6);
	    	pls->line( plotIndex, Time, PowerDC2);

		// Plot temperature
	    	pls->width(1);
	    	pls->col0(0);
	    	pls->wind( 0.0, plotIndex, 0.0, 100.0 );
	    	pls->box( "", 0, 0, "cmstv", 10.0, 0 );
	    	pls->col0(2);
	    	pls->width(3);
	    	pls->line( plotIndex, Time, Temp);
	    	pls->mtex( "r", 5.0, 0.5, 0.5, "Temperature [C]");

		// Distractor will create the graph image
	    	delete pls;

		// Convert SVG image format tp png (supported by OpenCV)
	    	system("rsvg-convert -f png -w 640 -h 480 kaco.svg -o /var/www/temp/kaco.png");
	}
}

void * updateInverter(void* id){
	char getURL[256];
	time_t now;
	struct tm * fileTime;

	time (&now);
	fileTime = localtime(&now);

	while (true){
		sprintf(getURL,"%s%4.4d%2.2d%2.2d.CSV",InverterURL, (fileTime->tm_year + 1900), (fileTime->tm_mon + 1), fileTime->tm_mday);
		if (getInverterData(getURL, "kaco.csv") == 0){
			processInverterData("kaco.csv");

		}else{
			printf("Error getting inverter data!\n");
		}

		sleep(400);
	}

	pthread_exit(NULL);
}

void updateInverter(void){
	int ret;
	pthread_t inv_thread;

	ret = pthread_create( &inv_thread, NULL, updateInverter, (void*)0);
	if(ret){
	    fprintf(stderr,"Error - pthread_create() return code: %d\n",ret);
	}
}
