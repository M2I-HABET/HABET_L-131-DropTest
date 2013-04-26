/**
 * Header file containing miscellaneous variables and structures for the main
 * part of the flight software used on the planes.
 * 
 * Authored by Ian McInerney, Iowa State University
 * Created for the HABET LX-131-A&B flights in April 2013
 */
#ifndef RADIO_COMMANDS_H_
#define RADIO_COMMANDS_H_

// Altitude to stop sending telemetry at (in millimeters)
#define CUTOFF_ALTITUDE 12192000	// 40,000ft

// Length of the receive buffer in bytes
#define REC_BUF_LENG 255

// The address of the plane which is used in all communciations
extern char planeAddress = '6';

// Miscellaneous variables to keep track of the current state of the plane
extern bool planeDropped = false;
extern bool telemEnable = true;
extern bool telemLong = true;

// Enumerated list of all the commands which can be sent to the plane
typedef enum HABET_462_COMMANDS {
	HABET_462_NO_OP = 0,
	HABET_462_TELEM_ENABLE = 1,
	HABET_462_TELEM_DISABLE = 2,
	HABET_462_PLANE_DROPPED = 3,
	HABET_462_ENABLE_SHORT = 4,
	HABET_462_ENABLE_LONG = 5,
};

#endif
