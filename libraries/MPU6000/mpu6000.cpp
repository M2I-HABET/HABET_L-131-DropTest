#include "Arduino.h"
#include "mpu6000.h"
#include <SPI.h>

MPU6000::MPU6000() {

}

void MPU6000::init() {
	SPI.setClockDivider(SPI_CLOCK_DIV16);      // SPI at 1Mhz (on 16Mhz clock)
	SPI.begin();
    // SPI initialization
    delay(10);
    
    // Chip reset
    writeRegister(MPU6000_REG_PWR_MGMT_1, MPU6000_BIT_H_RESET);
    delay(100);
	
    // Wake up device and select GyroZ clock (better performance)
    writeRegister(MPU6000_REG_PWR_MGMT_1, MPU6000_CLK_SEL_PLLGYROZ);
    delay(1);
	
    // Disable I2C bus 
    writeRegister(MPU6000_REG_USER_CTRL, MPU6000_BIT_I2C_IF_DIS);
	
    // SAMPLE RATE
    writeRegister(MPU6000_REG_SMPLRT_DIV, 19);     // Sample rate = 50Hz    Fsample= 1Khz/(19+1) = 50Hz
	
    // FS & DLPF   FS=2000º/s, DLPF = 20Hz (low pass filter)
    writeRegister(MPU6000_REG_CONFIG, MPU6000_BITS_DLPF_CFG_20HZ);  
    delay(1);
	
    writeRegister(MPU6000_REG_GYRO_CONFIG, MPU6000_BITS_FS_2000DPS);  // Gyro scale 2000º/s
    delay(1);
    writeRegister(MPU6000_REG_ACCEL_CONFIG, 0x08);            // Accel scale 4g (4096LSB/g)
    delay(1);   

    // Oscillator set
    writeRegister(MPU6000_REG_PWR_MGMT_1, MPU6000_CLK_SEL_PLLGYROZ);
    delay(1);
}

MPU6000_RAW_DATA MPU6000::getRawGyroData() {
	MPU6000_RAW_DATA data;
	int i = 0;
	short buf[6];

	// Pull chip select low to begin
	digitalWrite(MPU6000_CHIP_SELECT, LOW);
	
	// Send the address for the lowest register value
	SPI.transfer(MPU6000_REG_GYRO_X_H+0x80);
	
	// Read 6 shorts worth of data and place it in a temporary register
	for(i=0; i<6;i++) {
		buf[i] = SPI.transfer(0x00);
	}
	
	// Copy the temporary data into the structure to return it
	data.x = (buf[0]<<8)|(buf[1]);
	data.y = (buf[2]<<8)|(buf[3]);
	data.z = (buf[4]<<8)|(buf[5]);
	
	// Pull the chip select high to end
	digitalWrite(MPU6000_CHIP_SELECT, HIGH);

	
	return(data);
}

MPU6000_RAW_DATA MPU6000::getRawAccelData() {
	MPU6000_RAW_DATA data;
	int i = 0;
	short buf[6];
	
	// Pull chip select low to begin
	digitalWrite(MPU6000_CHIP_SELECT, LOW);
	
	// Send the address for the lowest register value
	SPI.transfer(MPU6000_REG_ACCEL_X_H+0x80);
	
	// Read 6 shorts worth of data and place it in a temporary register
	for(i=0; i<6;i++) {
		buf[i] = SPI.transfer(0x00);
	}
	
	// Copy the temporary data into the structure to return it
	data.x = (buf[0]<<8)|(buf[1]);
	data.y = (buf[2]<<8)|(buf[3]);
	data.z = (buf[4]<<8)|(buf[5]);
	
	// Pull the chip select high to end
	digitalWrite(MPU6000_CHIP_SELECT, HIGH);

	return(data);
}

int MPU6000::getRawTemperature() {
	int data = (readRegister(MPU6000_REG_TEMP_H)<<8)+(readRegister(MPU6000_REG_TEMP_L));
	return(data);
}

short MPU6000::readRegister(short reg) {
	short result = 0;   // result to return

	// Pull chip select low to begin
	digitalWrite(MPU6000_CHIP_SELECT, LOW);
	
	// send the device the register you want to read:
	SPI.transfer(reg+0x80);
	// send a value of 0 to read the first short returned:
	result = SPI.transfer(0x00);
	
	// Pull the chip select high to end transaction
	digitalWrite(MPU6000_CHIP_SELECT, HIGH);
	// return the result:
	return(result);
}

void MPU6000::writeRegister(short reg, short val) {
	// Pull the chip select line low to begin
	digitalWrite(MPU6000_CHIP_SELECT, LOW);
	
	// Transfer the register address and the data value
	SPI.transfer(reg);
	SPI.transfer(val);
	
	// Pull the chip select line high to end
	digitalWrite(MPU6000_CHIP_SELECT, HIGH);
}