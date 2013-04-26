/**
 * Source file containing functions for the
 * Honeywell HMC5883 magnetometer interface class.
 * 
 * Based upon the Sparkfun library by Jordan McConnell
 * Modified by Ian McInerney, Iowa State University
 * Created for the HABET LX-131-A&B flights in April 2013
 */

#include "Arduino.h"
#include <Wire.h>
#include "hmc5883.h"

/**
 * Standard C++ constructor function.
 * This function does not do anything, to configure the magnetometer you must use the init() function
 */
HMC5883::HMC5883() {
	// No need to do anything here
}

/**
 * Function to perform initial configuration for the Honeywell HMC5883 magnetometer.
 * It will open the serial port to be 38400BPS, and then configure the UBlox GPS to output only GGA and RMC NMEA strings on its UART1 port
 */
void HMC5883::init() {
	//Put the HMC5883 into the correct operating mode
	Wire.beginTransmission(HMC5883_ADDR); //open communication with HMC5883
	Wire.write(0x02); //select mode register
	Wire.write(0x00); //continuous measurement mode
	Wire.endTransmission();
}

/**
 * Function to retrieve the current data from the magnetometer and then return it to the user.
 * 
 * @return A data structure containing the magnetometer axis data
 */
HMC5883_DATA HMC5883::getData() {
	// Tell the compass which register to read
	Wire.beginTransmission(HMC5883_ADDR);
	Wire.write(0x03); //select register 3, X MSB register
	Wire.endTransmission();
	
	HMC5883_DATA data;
	
	// Read the data for each axis, 2 registers per axis
	// Do all of this in a multi-byte read so we can reduce the number of SPI transactions
	Wire.requestFrom(HMC5883_ADDR, 6);
	if(6<=Wire.available()){
		data.x = Wire.read()<<8;	 // MSB for the X axis
		data.x |= Wire.read();		// LSB for the X axis
		data.z = Wire.read()<<8;	// MSB for the Z axis
		data.z |= Wire.read();		// LSB for the Z axis
		data.y = Wire.read()<<8;	// MSB for the Y axis
		data.y |= Wire.read();		// LSB for the Y axis
	}
	return(data);
}