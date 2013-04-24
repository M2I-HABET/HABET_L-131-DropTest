#include <mpu6000.h>
#include <hmc5883.h>
#include <lea6.h>
#include <SPI.h>
#include <Wire.h>

// Timer variables
#define PRELOAD 		0x06	// Value to load into the timer to get it to overflow at the right time
#define IMU_FREQUENCY 	50		// Frequency of IMU telemetry transmission in Hz
#define IMU_PERIOD		(1/IMU_FREQUENCY)*1000		// Macro to compute the period of the IMU telemetry transmission using the above information (in milliseconds)
int time = 0;	// Variable to hold the current CPU time in (milliseconds)


// Sensor variables
MPU6000 imu;
HMC5883 compass;
LEA6 gps;

// Data variables
MPU6000_RAW_DATA accel;
MPU6000_RAW_DATA gyro;
HMC5883_DATA mag;
UBLOX_RECEIVED_INFO pos;
int imuCollecTime;

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
	Serial.print("Done Initializing\r\n");
}

void loop() {

	// Only run the IMU telemetry collection every IMU_PERIOD milliseconds
	if((time % IMU_PERIOD) == 0) {
		imuCollecTime = time;			// Store the current time
//		Serial.print("Reading Accelerometer");
		accel = imu.getRawAccelData();	// Read the accelerometer data
//		Serial.print("\tReading Gyro");
		gyro = imu.getRawGyroData();	// Read the gyro data
//		Serial.print("\tReading Compass");
		mag = compass.getData();		// Read the compass data
//		Serial.print("\tDone Reading\r\n");
	}

	if(gps.readGPS() == true) {
		pos = gps.getPositionInfo();

		Serial.print("#");
		Serial.print(pos.time, DEC);
		Serial.print(",");
		Serial.print(pos.latitude, DEC);
		Serial.print(",");
		Serial.print(pos.longitude, DEC);
		Serial.print(",");
		Serial.print(pos.altitude, DEC);
		Serial.print(",");
		Serial.print(pos.satellites, DEC);
		Serial.print(",");
		Serial.print(pos.hdop, DEC);
		Serial.print(",");
		Serial.print(pos.fixAge, DEC);
		Serial.print("\n\r");
	}

/* 	Serial.print("X: ");
	Serial.print(accel.x, DEC);
	Serial.print("  Y: ");
	Serial.print(accel.y, DEC);
	Serial.print("  Z: ");
	Serial.print(accel.z, DEC);
	
	Serial.print("X: ");
	Serial.print(gyro.x, DEC);
	Serial.print("  Y: ");
	Serial.print(gyro.y, DEC);
	Serial.print("  Z: ");
	Serial.print(gyro.z, DEC);
	Serial.print("\r\n"); */
}

/*
 * Interrupt vector for a Timer2 Overflow
 */
ISR(TIMER2_OVF_vect) {
	time++;				// Add 1 millisecond
	TCNT2 = PRELOAD;	// Reload the timer
}