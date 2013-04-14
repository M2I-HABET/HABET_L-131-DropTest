/*
 * Arduino library for the HMC5883.
 * 
 * Modified by Ian McInerney
 * Iowa State University Make:To:Innovate Program
 * 
 * Based upon the Sparkfun library by Jordan McConnell
 */

#include "Arduino.h"
#include <Wire.h>
#include "hmc5883.h"

HMC5883::HMC5883() {
	// No need to do anything here
}

void HMC5883::init() {
	//Put the HMC5883 IC into the correct operating mode
	Wire.beginTransmission(HMC5883_ADDR); //open communication with HMC5883
	Wire.write(0x02); //select mode register
	Wire.write(0x00); //continuous measurement mode
	Wire.endTransmission();
}

HMC5883_DATA HMC5883::getData() {
	// Tell the compass which register to read
	Wire.beginTransmission(HMC5883_ADDR);
	Wire.write(0x03); //select register 3, X MSB register
	Wire.endTransmission();
	
	HMC5883_DATA data;
	
	// Read the data for each, 2 registers per axis
	Wire.requestFrom(HMC5883_ADDR, 6);
	if(6<=Wire.available()){
		data.x = Wire.read()<<8; // MSB for the X axis
		data.x |= Wire.read();	// LSB for the X axis
		data.z = Wire.read()<<8;	// MSB for the Z axis
		data.z |= Wire.read();	// LSB for the Z axis
		data.y = Wire.read()<<8;	// MSB for the Y axis
		data.y |= Wire.read();	// LSB for the Y axis
  }
  
  return(data);
}