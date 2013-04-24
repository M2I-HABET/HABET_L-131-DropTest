#include "lea6.h"
#include "TinyGPS.h"

/*
 * Library to interface with a uBlox LEA6 GPS module. This library uses the TinyGPS library to parse the NMEA string
 */


LEA6::LEA6() {
 
}
 
/*
 *  configure a UBlox GPS for the given message rate
 */
void LEA6::init(void) {
    Serial1.begin(38400);
	
	// Enable NMEA protocol output on GPS Serial port 1
	Serial1.write("$PUBX,41,1,0007,0002,38400,0*21\r\n");
	
	// Set the NMEA output rate on the GPS serial port 1 to be every epoch, and off for every other port
	Serial1.write("$PUBX,40,GGA,0,1,0,0,0,0*5B\r\n");
	Serial1.write("$PUBX,40,RMC,0,1,0,0,0,0*46\r\n");

}

UBLOX_RECEIVED_INFO LEA6::getPositionInfo() {
	return(info);
}

bool LEA6::readGPS() {
	unsigned long gar_date;
	unsigned long gar_age;
	unsigned long time;
    int length;
	char data;
	bool parsed = false;

    while(Serial1.available()) {
		if(parser.encode(Serial1.read())) {
			parser.get_datetime(&gar_date, &time, &gar_age);
			if(time != lastTime) {
				parser.get_position(&(info.latitude), &(info.longitude), &(info.fixAge));
				parser.get_datetime(&gar_date, &(info.time), &gar_age);
				info.time = time;
				info.altitude = parser.altitude();
				info.satellites = parser.satellites();
				info.hdop = parser.hdop();
				
				lastTime = time;
				parsed = true;
			}
		}
	}
	
    return parsed;
}