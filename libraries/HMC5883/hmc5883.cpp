/*
 * Arduino library for the HMC5883.
 * 
 * Modified by Ian McInerney
 * Iowa State University Make:To:Innovate Program
 * 
 * Based upon the Sparkfun library by Jordan McConnell
 */

#include "hmc5883.h"

HMC5883::HMC5883() {
	// No need to do anything here
}

void HMC5883::init() {
	//Put the HMC5883 IC into the correct operating mode
	Wire.beginTransmission(HMC5883_ADDR); //open communication with HMC5883
	Wire.send(0x02); //select mode register
	Wire.send(0x00); //continuous measurement mode
	Wire.endTransmission();
}

HMC5883_data HMC5883::getData() {
	// Tell the compass which register to read
	Wire.beginTransmission(HMC5883_ADDR);
	Wire.send(0x03); //select register 3, X MSB register
	Wire.endTransmission();
	
	HMC5338_data data;
	
	// Read the data for each, 2 registers per axis
	Wire.requestFrom(address, 6);
	if(6<=Wire.available()){
		data.x = Wire.receive()<<8; // MSB for the X axis
		data.x |= Wire.receive();	// LSB for the X axis
		data.z = Wire.receive()<<8;	// MSB for the Z axis
		data.z |= Wire.receive();	// LSB for the Z axis
		data.y = Wire.receive()<<8;	// MSB for the Y axis
		data.y |= Wire.receive();	// LSB for the Y axis
  }
  
  return(data);
}