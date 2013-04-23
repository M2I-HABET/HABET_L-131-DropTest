#ifndef NEW_RADIO_COMMANDS_H_
#define NEW_RADIO_COMMANDS_H_

extern char planeAddress = '1';
extern bool planeDropped = false;
extern bool telemEnable = true;

//#define CLI_ENABLED ENABLED

// Altitude to stop sending telemetry at (in centimeters)
#define CUTOFF_ALTITUDE 300

// Length of the receive buffer in bytes
#define REC_BUF_LENG 255

/*
 * Data type sizes:
 * float = 4 bytes
 * int16 = 2 bytes
 * int32 = 4 bytes
 * uint32 = 4 bytes
 */
typedef struct HABET_462_telem {
	char sync1;				// Sync Character 1 (P)
	char direc;				// Direction of data packet (To/From)
	char addr;				// Address of sending radio
	char type;				// Type of the packet (T) for telemetry
	uint32_t time;			// GPS time (milliseconds from epoch)
	int32_t altitude;		// GPS altitude in m
/*	int32_t roll;			// Roll angle (deg*100)
	int32_t pitch;			// Pitch angle (deg*100)
	int32_t yaw;			// Yaw angle (deg*100)
*/	int16_t rollspeed;		// Angular speed around X axis (raw)
	int16_t pitchspeed;		// Angular speed around Y axis (raw)
	int16_t yawspeed;		// Angular speed around Z axis (raw)
	int16_t xacc;			// X acceleration (raw)
	int16_t yacc;			// Y acceleration (raw)
	int16_t zacc;			// Z acceleration (raw)
	int16_t xmag;			// X Magnetic field (raw)
	int16_t ymag;			// Y Magnetic field (raw)
	int16_t zmag;			// Z Magnetic field (raw)
	char ending;			// Ending character (#)
} HABET_462_telem;

// Size of the telemetry packet in bytes
#define HABET_462_TELEM_SIZE 43

#endif