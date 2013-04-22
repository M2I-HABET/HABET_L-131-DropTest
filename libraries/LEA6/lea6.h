#ifndef LEA6_H_
#define LEA6_H_

#include <stdint.h>
#include <stddef.h>
#include <HardwareSerial.h>

#define UBLOX_SET_BINARY "$PUBX,41,1,0003,0001,38400,0*26\r\n\265\142\006\001\003\000\001\006\001\022\117"
#define UBLOX_NAV_SETTING 7		// Airborne <2g acceleration dynamic model


enum UBLOX_PROTOCOL_BYTES {
	PREAMBLE1 = 0xb5,
	PREAMBLE2 = 0x62,
	CLASS_NAV = 0x01,
	CLASS_ACK = 0x05,
	CLASS_CFG = 0x06,
	MSG_ACK_NACK = 0x00,
	MSG_ACK_ACK = 0x01,
	MSG_POSLLH = 0x2,
	MSG_STATUS = 0x3,
	MSG_SOL = 0x6,
	MSG_VELNED = 0x12,
	MSG_CFG_PRT = 0x00,
	MSG_CFG_RATE = 0x08,
	MSG_CFG_SET_RATE = 0x01,
	MSG_CFG_NAV_SETTINGS = 0x24
};
enum UBLOX_NAV_FIX_TYPE {
	FIX_NONE = 0,
	FIX_DEAD_RECKONING = 1,
	FIX_2D = 2,
	FIX_3D = 3,
	FIX_GPS_DEAD_RECKONING = 4,
	FIX_TIME = 5
};
enum UBLOX_NAV_STATUS_BITS {
	NAV_STATUS_FIX_VALID = 1
};

struct UBLOX_HEADER {
	uint8_t preamble1;
	uint8_t preamble2;
	uint8_t msg_class;
	uint8_t msg_id;
	uint16_t length;
};

struct UBLOX_CFG_NAV_RATE {
	uint16_t measure_rate_ms;
	uint16_t nav_rate;
	uint16_t timeref;
};

struct UBLOX_CFG_MSG_RATE {
	uint8_t msg_class;
	uint8_t msg_id;
	uint8_t rate;
};

struct UBLOX_CFG_NAV_SETTINGS {
	uint16_t mask;
	uint8_t dynModel;
	uint8_t fixMode;
	int32_t fixedAlt;
	uint32_t fixedAltVar;
	int8_t minElev;
	uint8_t drLimit;
	uint16_t pDop;
	uint16_t tDop;
	uint16_t pAcc;
	uint16_t tAcc;
	uint8_t staticHoldThresh;
	uint8_t res1;
	uint32_t res2;
	uint32_t res3;
	uint32_t res4;
};

struct UBLOX_NAV_POSLLH {
	uint32_t time;                                  // GPS msToW
	int32_t longitude;
	int32_t latitude;
	int32_t altitude_ellipsoid;
	int32_t altitude_msl;
	uint32_t horizontal_accuracy;
	uint32_t vertical_accuracy;
};

struct UBLOX_NAV_STATUS {
	uint32_t time;                                  // GPS msToW
	uint8_t fix_type;
	uint8_t fix_status;
	uint8_t differential_status;
	uint8_t res;
	uint32_t time_to_first_fix;
	uint32_t uptime;                                // milliseconds
};

struct UBLOX_NAV_SOLUTION {
	uint32_t time;
	int32_t time_nsec;
	int16_t week;
	uint8_t fix_type;
	uint8_t fix_status;
	int32_t ecef_x;
	int32_t ecef_y;
	int32_t ecef_z;
	uint32_t position_accuracy_3d;
	int32_t ecef_x_velocity;
	int32_t ecef_y_velocity;
	int32_t ecef_z_velocity;
	uint32_t speed_accuracy;
	uint16_t position_DOP;
	uint8_t res;
	uint8_t satellites;
	uint32_t res2;
};

struct UBLOX_NAV_VELNED {
	uint32_t time;                                  // GPS msToW
	int32_t ned_north;
	int32_t ned_east;
	int32_t ned_down;
	uint32_t speed_3d;
	uint32_t speed_2d;
	int32_t heading_2d;
	uint32_t speed_accuracy;
	uint32_t heading_accuracy;
};

struct UBLOX_RECEIVED_INFO {
	uint32_t time;		// GPS Time
	int32_t latitude;	// GPS Latitude
	int32_t longitude;	// GPS Longitude
	int32_t altitude;	// GPS altitude above MSL
	uint8_t satellites;	// # of GPS satellites
	uint16_t hdop;		// Position dillution of Precision
	uint8_t fixStatus;	// Status of the GPS fix
	uint8_t fixType;	// Type of fix the GPS has
};




class LEA6 {

public:
	LEA6();
	void init();
	bool readGPS();
	UBLOX_RECEIVED_INFO getPositionInfo();
	
private:

	bool parseGPS();
	void configure_message_rate(uint8_t msg_class, uint8_t msg_id, uint8_t rate);
	void update_checksum(uint8_t *data, uint8_t len, uint8_t &ck_a, uint8_t &ck_b);
	void send_message(uint8_t msg_class, uint8_t msg_id, void *msg, uint8_t size);
	
	UBLOX_RECEIVED_INFO* info;	// Position information from the uBlox
    uint8_t fixCount;			// I have no idea what this is for, but it is here anyway
    bool newPosition;			// New position received
	uint8_t disable_counter;	// Don't start an ACK war
	
	    // Packet checksum accumulators
    uint8_t         _ck_a;
    uint8_t         _ck_b;

    // State machine state
    uint8_t         _step;
    uint8_t         _msg_id;
	uint8_t			_msg_class;
    uint16_t        _payload_length;
    uint16_t        _payload_counter;
	
	union {
		UBLOX_NAV_POSLLH posllh;
		UBLOX_NAV_STATUS status;
		UBLOX_NAV_SOLUTION solution;
		UBLOX_NAV_VELNED velned;
		UBLOX_CFG_NAV_SETTINGS nav_settings;
		uint8_t bytes[];
	} buffer;
	
};


#endif