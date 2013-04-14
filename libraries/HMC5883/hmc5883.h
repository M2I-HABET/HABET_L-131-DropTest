/*
 * Arduino library for the HMC5883.
 * 
 * Modified by Ian McInerney
 * Iowa State University Make:To:Innovate Program
 * 
 * Based upon the Sparkfun library by Jordan McConnell
 */

// Include the Arduino I2C Library
#include <Wire.h>

// 7-bit I2C address for the chip
#define HMC5883_ADDR 0x1E

typedef struct HMC5883_DATA {
	short x;
	short y;
	short z;
} HMC5883_DATA;

class HMC5883 {

public:
	HMC5883();
	void init();
	HMC5883_DATA getData();
};