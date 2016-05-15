#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions
#include <pthread.h>
#include <unistd.h>

int BUFFER_SIZE=256;

bool gpioStatus[10];
bool AlarmActive = false;


bool getGPIO(unsigned char gpio){

	return gpioStatus[gpio];
}


int writeComBytes(int fd, char* cmd){
	int n_written = 0, spot = 0;

	do {
    		n_written = write( fd, &cmd[spot], 1 );
    		spot += n_written;
	} while (cmd[spot-1] != '\r' && n_written > 0);

	return 0;
}

int readComBytes(int fd, char* response, int size){
	int n = 0;
	int spot = 0;
	char buf = '\0';
	struct timeval timeout;
	fd_set set;

	memset(response, 0 ,size);

  	FD_ZERO(&set); /* clear the set */
  	FD_SET(fd, &set); /* add our file descriptor to the set */

	timeout.tv_sec = 0;
	timeout.tv_usec = 500000;

	/* Whole response*/
	memset(response, '\0', sizeof response);

	do {
 		if(select(fd + 1, &set, NULL, NULL, &timeout) > 0){
   			n = read( fd, &buf, 1 );
   			sprintf( &response[spot], "%c", buf );
   			spot += n;
		}else{
			n=0;
		}
	} while( n > 0 && spot < 256);

	return spot;
}

int sendSMS(int fd){
	char temp[BUFFER_SIZE];

	writeComBytes(fd, "AT+CMGF=1\r");
	readComBytes(fd, temp, BUFFER_SIZE);

	writeComBytes(fd, "AT+CMGS=\"6946421660\"\x0d\"Test Message\"\x1a\r");
	readComBytes(fd, temp, BUFFER_SIZE);

	return 0;
}

	
void updateGPIO(int fd){
	char temp[BUFFER_SIZE];
	int i,x;


	writeComBytes(fd, "AT^SGIO=0\r");
	x = readComBytes(fd, temp, BUFFER_SIZE);

	for (i = 0 ; i < (x - 7) ; i++){
		if (!strncmp("SGIO: 0", &temp[i],7)){
			gpioStatus[0] = false;			
		}else if(!strncmp("SGIO: 1", &temp[i],7)){
			gpioStatus[0] = true;
		}
	}

	usleep(100000);

	writeComBytes(fd, "AT^SGIO=1\r");
	readComBytes(fd, temp, BUFFER_SIZE);

	for (i = 0 ; i < (x - 7) ; i++){
		if (!strncmp("SGIO: 0", &temp[i],7)){
			gpioStatus[1] = false;
		}else if(!strncmp("SGIO: 1", &temp[i],7)){
			gpioStatus[1] = true;
		}
	}

	usleep(100000);

	writeComBytes(fd, "AT^SGIO=2\r");
	readComBytes(fd, temp, BUFFER_SIZE);

	for (i = 0 ; i < (x - 7) ; i++){
		if (!strncmp("SGIO: 0", &temp[i],7)){
			gpioStatus[2] = false;
		}else if(!strncmp("SGIO: 1", &temp[i],7)){
			gpioStatus[2] = true;
		}
	}

}

void *modemUpdate(void* notused){
	int ret;
	char temp[256];
	int fd = open( "/dev/ttyUSB0", O_RDWR| O_NOCTTY );

	if (fd<0){
		printf("Error opening ttyUSB0!\n");
		pthread_exit(NULL);
		return 0;	
	}

	struct termios tty;
	struct termios tty_old;
	memset (&tty, 0, sizeof tty);

	if ( tcgetattr ( fd, &tty ) != 0 ) {
   		printf("Error %s from tcgetattr\n", strerror(errno));
	}

	/* Save old tty parameters */
	tty_old = tty;

	/* Set Baud Rate */
	cfsetospeed (&tty, (speed_t)B115200);
	cfsetispeed (&tty, (speed_t)B115200);

	/* Setting other Port Stuff */
	tty.c_cflag     &=  ~PARENB;            // Make 8n1
	tty.c_cflag     &=  ~CSTOPB;
	tty.c_cflag     &=  ~CSIZE;
	tty.c_cflag     |=  CS8;

	tty.c_cflag     &=  ~CRTSCTS;           // no flow control
	tty.c_cc[VMIN]   =  1;                  // read doesn't block
	tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
	tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

	/* Make raw */
	cfmakeraw(&tty);

	/* Flush Port, then applies attributes */
	tcflush( fd, TCIFLUSH );
	if ( tcsetattr ( fd, TCSANOW, &tty ) != 0) {
		printf("Error %d from tcsetattr\n", errno);
	}

	writeComBytes(fd, "AT\r");
	readComBytes(fd, temp, BUFFER_SIZE);

	writeComBytes(fd, "AT^SPIO=1\r");
	readComBytes(fd, temp, BUFFER_SIZE);

	writeComBytes(fd, "AT^SCPIN=1,0,0\r");
	readComBytes(fd, temp, BUFFER_SIZE);

	writeComBytes(fd, "AT^SCPIN=1,1,0\r");
	readComBytes(fd, temp, BUFFER_SIZE);

	writeComBytes(fd, "AT^SCPIN=1,2,0\r");
	readComBytes(fd, temp, BUFFER_SIZE);

	while(true){
		updateGPIO(fd);
		sleep(5);
	}


	writeComBytes(fd, "AT^SCPIN=0,0,0\r");
	readComBytes(fd, temp, BUFFER_SIZE);

	writeComBytes(fd, "AT^SCPIN=0,1,0\r");
	readComBytes(fd, temp, BUFFER_SIZE);

	writeComBytes(fd, "AT^SCPIN=0,2,0\r");
	readComBytes(fd, temp, BUFFER_SIZE);

	writeComBytes(fd, "AT^SPIO=0\r");
	readComBytes(fd, temp, BUFFER_SIZE);


	close(fd);
	
	pthread_exit(NULL);
}	

void modemCommunicationInit(void){
	int ret;
	pthread_t modem_thread;

	ret = pthread_create( &modem_thread, NULL, modemUpdate, (void*)0);
	if(ret){
	    fprintf(stderr,"Error - pthread_create() return code: %d\n",ret);
	}

}
