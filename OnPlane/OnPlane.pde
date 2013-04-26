/**
 * The main Ardupilot source file for the firmware running on the planes
 * to be dropped for the AerE 462 senior design class.
 * 
 * Authored by Ian McInerney, Iowa State University
 * Created for the HABET LX-131-A&B flights in April 2013
 */

#include <mpu6000.h>
#include <hmc5883.h>
#include <lea6.h>
#include <SPI.h>
#include <Wire.h>

#include "radioCommands.h"

// Timer variables
#define PRELOAD 		0x06	// Value to load into the timer to get it to overflow at the right time
#define IMU_PERIOD		25		// The period of the telemetry transmission (in milliseconds)
unsigned long time = 0;			// Variable to hold the current CPU time in (milliseconds)


// Sensor variables
MPU6000 imu;
HMC5883 compass;
LEA6 gps;

// Data variables
MPU6000_RAW_DATA accel;
MPU6000_RAW_DATA gyro;
HMC5883_DATA mag;
UBLOX_RECEIVED_INFO pos;
unsigned long imuCollecTime;

// Variables to hold data for serial port transactions
char recBuf[REC_BUF_LENG];
int recLength;
char sendBuf[50];
short sendLength;

/**
 * Function to perform initialization of the Ardupilot systems
 */
void setup() {
	Serial.begin(57600);	// Serial port for the radio telemetry input/output
	Serial1.begin(4800);	// Serial port for the uBlox GPS receiver
	
	// For some reason the barometer holds the SPI line hostage, so it must be dealt with
	pinMode(40, OUTPUT);
	digitalWrite(40, HIGH);
	
	Serial.print("Initializing MPU6000\r\n");
	imu.init();
	
	Serial.print("Initializing HMC5883\r\n");
	compass.init();
	
	Serial.print("Initializing LEA6\r\n");
	gps.init();
	
	Serial.print("Initializing Timer\r\n");
	TCNT2 = PRELOAD;		// Preload the timer to get a 1ms delay between each overflow
	TCCR2B = 0b00000100;	// Set the prescaler to 64
	TIMSK2 = 0b00000001;	// Enable the overflow interrupt
	
	// Configure the send buffer to have the header for the plane in it
	sendBuf[0] = '$';
	sendBuf[1] = 'P';
	sendBuf[2] = 'F';
	sendBuf[3] = planeAddress;
	sendBuf[4] = 'S';
	sendBuf[5] = 0;
	sendLength = 5;
	Serial.print("Done Initializing\r\n");
}

void loop() {

	// Initialize the send buffer every loop to make it be appendable
	sendBuf[4] = 0;
	sendLength = 4;
	
	// See if the plane has received any commands then act on them
	switch(checkTelemCommands()) {
	case HABET_462_NO_OP:
		// Don't do anything
		break;
	case HABET_462_TELEM_ENABLE:
		// Enable telemetry transmission
		telemEnable = true;
		strcat(sendBuf, " Telemetry Enabled\n\0");
		sendLength += 19;
		break;
	case HABET_462_TELEM_DISABLE:
		// Disable telemetry transmission
		telemEnable = false;
		strcat(sendBuf, " Telemetry Disabled\n\0");
		sendLength += 20;
		break;
	case HABET_462_PLANE_DROPPED:
		// The plane has dropped, set the plane up to be in drop mode and enable telemetry accordingly
		planeDropped = true;
		telemLong = false;
		telemEnable = true;
		strcat(sendBuf, " Plane Dropped\n\0");
		sendLength += 15;
		break;
	case HABET_462_ENABLE_SHORT:
		// Send the short telemetry strings when telemetry is enabled
		telemLong = false;
		strcat(sendBuf, " Short Telemetry Enabled\n\0");
		sendLength += 25;
		break;
	case HABET_462_ENABLE_LONG:
		// Send the long telemetry strings when telemetry is enabled
		telemLong = true;
		strcat(sendBuf, " Long Telemetry Enabled\n\0");
		sendLength += 24;
		break;
	default:
		// Don't do anything, unknown command
		break;
	}
	
	// Only send the buffer if more than the header is present
	if(sendLength > 4) {
		Serial.println(sendBuf);
	}
	
	// Check every loop iteration to see if there is new GPS data
	if(gps.readGPS() == true) {
		// If there is new data, then get it and save it
		pos = gps.getPositionInfo();
	}
	
	// Run the IMU telemetry collection every IMU_PERIOD milliseconds
	if((time % IMU_PERIOD) == 0) {
		imuCollecTime = time;			// Store the current time
		accel = imu.getRawAccelData();	// Read the accelerometer data
		gyro = imu.getRawGyroData();	// Read the gyro data
		mag = compass.getData();		// Read the compass data

		// If telemetry is enabled then print out the telemetry string
		if(telemEnable) {
			Serial.print("$P");
			Serial.print(planeAddress);
			Serial.print("FT,");
			Serial.print(imuCollecTime, DEC);
			Serial.print(",");
			Serial.print(accel.x, DEC);
			Serial.print(",");
			Serial.print(accel.y, DEC);
			Serial.print(",");
			Serial.print(accel.z, DEC);
			Serial.print(",");
			Serial.print(gyro.x, DEC);
			Serial.print(",");
			Serial.print(gyro.y, DEC);
			Serial.print(",");
			Serial.print(gyro.z, DEC);
			Serial.print(",");
			Serial.print(mag.x, DEC);
			Serial.print(",");
			Serial.print(mag.y, DEC);
			Serial.print(",");
			Serial.print(mag.z, DEC);
			Serial.print(",");
			Serial.print(pos.time, DEC);
			Serial.print(",");
			Serial.print(pos.altitude, DEC);
			
			if(telemLong) {
				// Only print out these values if the long telemetry string is enabled
				Serial.print(",");
				Serial.print(pos.latitude, DEC);
				Serial.print(",");
				Serial.print(pos.longitude, DEC);
				Serial.print(",");
				Serial.print(pos.satellites, DEC);
				Serial.print(",");
				Serial.print(pos.hdop, DEC);
				Serial.print(",");
				Serial.print(pos.fixAge, DEC);
			}
			
			Serial.print("#\r\n");
		}
		
		// Disable the telemetry if the plane is below the threshold and has been dropped
		if ((pos.altitude < CUTOFF_ALTITUDE) && planeDropped)
			telemEnable = false;
	}
}

/**
 * Receive data over the radio serial port. Maximum allowed receive length is 255 bytes
 * 
 * @return the enumerated list value for the received command
 */
int checkTelemCommands() {
	int retVal = HABET_462_NO_OP;

	// While there is data available, stay in the loop and receive it
	while(Serial.available() && recLength<REC_BUF_LENG) {
		recBuf[recLength] = Serial.read();
		recLength++;
	}
	
	// Commands must be prefaced with $ to be valid
	if(recBuf[0] != '$') {
		recLength = 0;
		return(retVal);
	}
	
	// Check to see if the terminating character of # has been received
	// If it has continue processing otherwise exit the function
	if(recBuf[recLength-1] != '#')
		return(retVal);

	// Check to see if the message is detined to be received by this plane
	if(recBuf[1] != 'P' && recBuf[2] != 'T' && recBuf[3] != planeAddress ) {
		recLength = 0;
		return(retVal);
	}
	
	// Switch over the possible two character command strings
	if(recBuf[4] == 'T') {
		switch(recBuf[5]) {
		case 'D':
			// Disable the telemetry system
			retVal = HABET_462_TELEM_DISABLE;
			break;
		case 'E':
			// Enable the telemetry system
			retVal = HABET_462_TELEM_ENABLE;
			break;
		case 'L':
			// Enable long form telemetry transmission
			retVal = HABET_462_ENABLE_LONG;
			break;
		case 'S':
			// Enable short form telemetry transmission
			retVal = HABET_462_ENABLE_SHORT;
		default:
			// Nothing to do
			break;
		}
	} else if(recBuf[4] == 'D' && recBuf[5] == 'R') {
		// Plane has been dropped
		retVal = HABET_462_PLANE_DROPPED;
	}
	
	// Reset the received length counter when a new command has been received and parsed
	if(retVal != 0)
		recLength = 0;

	return(retVal);
}

/*
 * Interrupt vector for a Timer2 Overflow
 */
ISR(TIMER2_OVF_vect) {
	time++;				// Add 1 millisecond
	TCNT2 = PRELOAD;	// Reload the timer
}
