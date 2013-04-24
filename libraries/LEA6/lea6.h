#ifndef LEA6_H_
#define LEA6_H_

#include <stdint.h>
#include <stddef.h>
#include <HardwareSerial.h>
#include "TinyGPS.h"

struct UBLOX_RECEIVED_INFO {
	unsigned long	 time;		// GPS Time
	long  latitude;	// GPS Latitude
	long  longitude;	// GPS Longitude
	long  altitude;	// GPS altitude above MSL
	unsigned short  satellites;	// # of GPS satellites
	unsigned long hdop;		// Position dillution of Precision
	unsigned long fixAge;	// Status of the GPS fix
};

class LEA6 {

public:
	LEA6();
	void init();
	bool readGPS();
	UBLOX_RECEIVED_INFO getPositionInfo();
	
private:
	TinyGPS parser;
	UBLOX_RECEIVED_INFO info;	// Position information from the uBlox
	unsigned long lastTime;
};


#endif