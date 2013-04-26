/**
 * Header file containing class definitions and variables for the LEA6 GPS interface class.
 * 
 * Authored by Ian McInerney, Iowa State University
 * Created for the HABET LX-131-A&B flights in April 2013
 */
#ifndef LEA6_H_
#define LEA6_H_

#include <stdint.h>
#include <stddef.h>
#include <HardwareSerial.h>
#include "TinyGPS.h"

// Data structure which can contain all the useful GPS information
struct UBLOX_RECEIVED_INFO {
	unsigned long time;			// GPS Time
	long latitude;				// GPS Latitude
	long longitude;				// GPS Longitude
	long altitude;				// GPS altitude above MSL
	unsigned short satellites;	// # of GPS satellites
	unsigned long hdop;			// Position dillution of Precision
	unsigned long fixAge;		// Status of the GPS fix
};

/**
 * A class to interface with a uBlox LEA6 GPS and retrieve the GPS data.
 * This class uses the open source TinyGPS parsing library to interpret
 * the NEMA strings received from the GPS. This means that a copy of the
 * library must be included in the same folder as this class.
 */
class LEA6 {

public:
	/**
	 * Standard C++ constructor function.
	 * This function does not do anything, to configure the GPS you must use the init() function
	 */
	LEA6();
	
	/**
	 * Function to perform initial configuration for the uBlox GPS.
	 * It will open the serial port to be 38400BPS, and then configure the UBlox GPS to output only GGA and RMC NMEA strings on its UART1 port
	 */
	void init();
	
	/**
	 * Function to poll the GPS and then parse the returned data into the GPS data structure.
	 * This function does not return the data, to retrieve the data you must call the
	 * getPositionInfo() function.
	 * 
	 * @return Returns true if new GPS data is present in the data structure
	 */
	bool readGPS();
	
	/**
	 * Function to retrieve the current GPS data
	 * 
	 * @return A copy of the UBLOX_RECEIVED_INFO object with the most recent GPS data
	 */
	UBLOX_RECEIVED_INFO getPositionInfo();
	
private:
	TinyGPS parser;				// TinyGPS library object used to parse the GPS data received
	UBLOX_RECEIVED_INFO info;	// Position information from the uBlox
	unsigned long lastTime;		// The timestamp of the last GPS packet received
};


#endif