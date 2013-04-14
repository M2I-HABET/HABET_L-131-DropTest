#include <mpu6000.h>
#include <hmc5883.h>
#include <SPI.h>
#include <Wire.h>

MPU6000 imu;
HMC5883 compass;

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
	Serial.print("Done Initializing\r\n");
}

void loop() {
	MPU6000_RAW_DATA accel;
	MPU6000_RAW_DATA gyro;
	
//	Serial.print("Reading Accelerometer");
	accel = imu.getRawAccelData();
//	Serial.print("\tReading Gyro");
	gyro = imu.getRawGyroData();
//	Serial.print("\tDone Reading\r\n");


//	HMC5883_DATA data = compass.getData();
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

	while(Serial1.available()) {
		Serial.write(Serial1.read());
	}
	
}