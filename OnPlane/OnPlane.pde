#include <mpu6000.h>
#include <hmc5883.h>
#include <SPI.h>
#include <Wire.h>

MPU6000 imu;
HMC5883 compass;

void setup() {
	Serial.begin(57600);
	
//	Serial.print("Initializing MPU6000\n");
//	imu.init();
	
	Serial.print("Initializing HMC5883\n");
	compass.init();
	Serial.print("Done Initializing\n");
}

void loop() {
	HMC5883_DATA data = compass.getData();
	Serial.print("X: ");
	Serial.print(data.x, DEC);
	Serial.print("  Y: ");
	Serial.print(data.y, DEC);
	Serial.print("  Z: ");
	Serial.print(data.z, DEC);
	Serial.print("\n");
}