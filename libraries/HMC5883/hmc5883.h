/**
 * Header file containing class definitions and variables for the
 * Honeywell HMC5883 magnetometer interface class.
 * 
 * Based upon the Sparkfun library by Jordan McConnell
 * Modified by Ian McInerney, Iowa State University
 * Created for the HABET LX-131-A&B flights in April 2013
 */

// Include the Arduino I2C Library
#include <Wire.h>

// 7-bit I2C address for the chip
#define HMC5883_ADDR 0x1E

// Structure to hold the three-axis magnetometer data
typedef struct HMC5883_DATA {
	short x;
	short y;
	short z;
} HMC5883_DATA;

/**
 * Class for interfacing with a Honeywell HMC5883 magnetometer ove the SPI bus.
 */
class HMC5883 {

public:
	/**
	 * Standard C++ constructor function.
	 * This function does not do anything, to configure the magnetometer you must use the init() function
	 */
	HMC5883();
	
	/**
	 * Function to perform initial configuration for the Honeywell HMC5883 magnetometer.
	 * It will open the serial port to be 38400BPS, and then configure the UBlox GPS to output only GGA and RMC NMEA strings on its UART1 port
	 */
	void init();
	
	/**
	 * Function to retrieve the current data from the magnetometer and then return it to the user.
	 * 
	 * @return A data structure containing the magnetometer axis data
	 */
	HMC5883_DATA getData();
};