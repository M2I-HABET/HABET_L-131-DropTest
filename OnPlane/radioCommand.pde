#include "radioCommands.h"

/**
 *
 * Function to send the telemetry string out of the serial port
 *
 * @param data A pointer to the data to transmit
 * @param length The length of the data to transmit
 */
void sendData(void *data, int length) {
	if (telemEnable) {
			int i = 0;
		char *dat = (char*) data;
		for(i=0; i<length; i++) {
			Serial.write(dat[i]);
		}
	}
}

/**
 * Receive data over the radio serial port. Maximum allowed receive length is 255 bytes
 * 
 * @param data A pointer to a character array to hold the received data
 * @param length A pointer to an integer to hold the number of bytes received
 */
void receiveData(char *data, int *length) {
	int i = 0;
	char temp[REC_BUF_LENG];
	
	// While there is data available, stay in the loop and receive it
	while(Serial.available() && i<REC_BUF_LENG) {
		data[i] = Serial.read();
		i++;
	}
	
	length = &i;
}

void checkTelemCommands() {
	// Try to receive data through the serial port
	char recBuf[REC_BUF_LENG];
	int recLength;
	receiveData(recBuf, &recLength);
	
	// If no (or not enough) data received, exit the function
	if (recLength < 6);
		return;
	
	// Message is not destined for this plane
	if (recBuf[0] != 'P' && recBuf[1] != 'T' && recBuf[2] != planeAddress )
		return;
	
	if (recBuf[3] == 'T') {
		switch(recBuf[4]) {
		case 'D':
			// Disable the telemetry system
			telemEnable = false;
			sendData((void*) "PL1FS Telemetry Disabled", 24);
			break;
		case 'E':
			// Enable the telemetry system
			telemEnable = true;
			sendData((void*)"PL1FS Telemetry Enabled", 23);
			break;
		default:
			// Nothing to do
			break;
		}
	} else if(recBuf[3] == 'D' && recBuf[4] == 'R') {
		// Plane has been dropped
		planeDropped = true;
	}
}

void checkAltitude(int32_t altitude) {
	if (altitude < CUTOFF_ALTITUDE)
		telemEnable = false;
}