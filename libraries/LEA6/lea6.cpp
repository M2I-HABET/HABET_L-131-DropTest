#include "lea6.h"

/*
 * Library to interface with a uBlox LEA6 GPS module.
 * This code is based upon the uBlox GPS library included with the APM 2.5 source code.
 * Modified by Ian McInerney
 */


LEA6::LEA6() {
 
}
 
/*
 *  configure a UBlox GPS for the given message rate
 */
void LEA6::init(void) {
    struct UBLOX_CFG_NAV_RATE msg;
    const unsigned baudrates[4] = {9600U, 19200U, 38400U, 57600U};

	Serial.println("Setting the baudrate");
    // the GPS may be setup for a different baud rate. This ensures
    // it gets configured correctly
    for (uint8_t i=0; i<4; i++) {
		// Do not remove this print statement. If it is missing then the GPS fails to configure for some reason.
		// I have no idea why, but this fixes it. SO DO NOT DELETE THIS LINE UNDER PENALTY OF DEATH.
		Serial.println(baudrates[i]);
		Serial1.begin(baudrates[i]);
		/*
		 *  try to put a UBlox into binary mode. This is in two parts. First we
		 *  send a PUBX asking the UBlox to receive NMEA and UBX, and send UBX,
		 *  with a baudrate of 38400. Then we send a UBX message setting rate 1
		 *  for the NAV_SOL message. The setup of NAV_SOL is to cope with
		 *  configurations where all UBX binary message types are disabled.
		 */
		Serial.print(Serial1.write("$PUBX,41,1,0003,0001,38400,0*26\r\n\265\142\006\001\003\000\001\006\001\022\117"));
 //       _write_progstr_block(_port, _ublox_set_binary, _ublox_set_binary_size);
    }
    Serial1.begin(38400U);

    // ask for navigation solutions every 200ms
    msg.measure_rate_ms = 200;
    msg.nav_rate        = 1;
    msg.timeref         = 0;     // UTC time
    send_message(CLASS_CFG, MSG_CFG_RATE, &msg, sizeof(msg));

    // ask for the messages we parse to be sent on every navigation solution
    configure_message_rate(CLASS_NAV, MSG_POSLLH, 1);
    configure_message_rate(CLASS_NAV, MSG_STATUS, 1);
    configure_message_rate(CLASS_NAV, MSG_SOL, 1);
	configure_message_rate(CLASS_NAV, MSG_PVT, 1);
    configure_message_rate(CLASS_NAV, MSG_VELNED, 1);

    // ask for the current navigation settings
    send_message(CLASS_CFG, MSG_CFG_NAV_SETTINGS, NULL, 0);
	newPositionAvailable = false;
}

UBLOX_RECEIVED_INFO* LEA6::getPositionInfo() {
	newPositionAvailable = false;
	return(info);
}

bool LEA6::hasNewPosition() {
	this->readGPS();
	this->parseGPS();
	return(newPositionAvailable);
}


// Process bytes available from the stream
//
// The stream is assumed to contain only messages we recognise.  If it
// contains other messages, and those messages contain the preamble
// bytes, it is possible for this code to fail to synchronise to the
// stream immediately.  Without buffering the entire message and
// re-processing it from the top, this is unavoidable. The parser
// attempts to avoid this when possible.
//
bool LEA6::readGPS() {
    uint8_t data;
    int16_t numc;
    bool parsed = false;

    numc = Serial1.available();
    for (int16_t i = 0; i < numc; i++) {        // Process bytes received
		// read the next byte
        data = Serial1.read();

	reset:
        switch(_step) {

        // Message preamble detection
        //
        // If we fail to match any of the expected bytes, we reset
        // the state machine and re-consider the failed byte as
        // the first byte of the preamble.  This improves our
        // chances of recovering from a mismatch and makes it less
        // likely that we will be fooled by the preamble appearing
        // as data in some other message.
        //
        case 1:
            if (PREAMBLE2 == data) {
                _step++;
                break;
            }
            _step = 0;
            Serial.println("reset");
        // FALLTHROUGH
        case 0:
            if(PREAMBLE1 == data)
                _step++;
            break;

        // Message header processing
        //
        // We sniff the class and message ID to decide whether we
        // are going to gather the message bytes or just discard
        // them.
        //
        // We always collect the length so that we can avoid being
        // fooled by preamble bytes in messages.
        //
        case 2:
            _step++;
            _msg_class = data;
            _ck_b = _ck_a = data;                               // reset the checksum accumulators
            break;
        case 3:
            _step++;
            _ck_b += (_ck_a += data);                   // checksum byte
            _msg_id = data;
            break;
        case 4:
            _step++;
            _ck_b += (_ck_a += data);                   // checksum byte
            _payload_length = data;                             // payload length low byte
            break;
        case 5:
            _step++;
            _ck_b += (_ck_a += data);                   // checksum byte

            _payload_length += (uint16_t)(data<<8);
            if (_payload_length > 512) {
//                Serial.println("large payload");
                // assume very large payloads are line noise
                _payload_length = 0;
                _step = 0;
				goto reset;
            }
            _payload_counter = 0;                               // prepare to receive payload
            break;

        // Receive message data
        //
        case 6:
            _ck_b += (_ck_a += data);                   // checksum byte
            if (_payload_counter < sizeof(buffer)) {
                buffer.bytes[_payload_counter] = data;
            }
            if (++_payload_counter == _payload_length)
                _step++;
            break;

        // Checksum and message processing
        //
        case 7:
            _step++;
            if (_ck_a != data) {
//                Serial.println("bad cka");
                _step = 0;
				goto reset;
            }
            break;
        case 8:
            _step = 0;
            if (_ck_b != data) {
//                Serial.println("bad ckb");
                break;                                                  // bad checksum
            }

            if (parseGPS()) {
                parsed = true;
            }
        }
    }
    return parsed;
}

bool LEA6::parseGPS() {

	// ACK received from the GPS, don't process any further
	if (_msg_class == CLASS_ACK) {
//		Serial.println("ACK");
		return false;
	}
	if (_msg_class == CLASS_CFG && _msg_id == MSG_CFG_NAV_SETTINGS) {
//		Serial.println("Got engine settings");
		if(buffer.nav_settings.dynModel != UBLOX_NAV_SETTING) {
			// we've received the current nav settings, change the engine
			// settings and send them back
//			Serial.println("Changing engine setting");
			buffer.nav_settings.dynModel = UBLOX_NAV_SETTING;
			send_message(CLASS_CFG, MSG_CFG_NAV_SETTINGS, &buffer.nav_settings, sizeof(buffer.nav_settings));
		}
		return false;
	}

	// Message received is not a navigation message or a configuration message, ignore
	if (_msg_class != CLASS_NAV) {
//		Serial.println("Unexpected message 0x%02x 0x%02x");
//		Serial.print(_msg_class, HEX);
		if (++disable_counter == 0) {
			// disable future sends of this message id, but
			// only do this every 256 messages, as some
			// message types can't be disabled and we don't
			// want to get into an ack war
//			Serial.println("Disabling message 0x%02x 0x%02x");
			configure_message_rate(_msg_class, _msg_id, 0);
		}
		return false;
	}
//	Serial.println("Parsing Data");

	switch (_msg_id) {
	case MSG_POSLLH:
//		Serial.println("MSG_POSLLH next_fix=%u");
		info->time = buffer.posllh.time;
		info->longitude = buffer.posllh.longitude;
		info->latitude = buffer.posllh.latitude;
		info->altitude = buffer.posllh.altitude_ellipsoid / 1000;
//		Serial.println(buffer.posllh.altitude_ellipsoid/1000);
//		fix             = next_fix;
		newPosition = true;
		newPositionAvailable = true;
		break;
	case MSG_STATUS:
//		Serial.println("MSG_STATUS");
/*		next_fix        = (buffer.status.fix_status & NAV_STATUS_FIX_VALID) && (buffer.status.fix_type == FIX_3D);
		if (!next_fix) {
			fix = false;
		}
*/
		info->fixStatus = buffer.status.fix_status;
		info->fixType = buffer.status.fix_type;
		break;
	case MSG_SOL:
//		Serial.println("MSG_SOL");
/*		next_fix        = (buffer.solution.fix_status & NAV_STATUS_FIX_VALID) && (buffer.solution.fix_type == FIX_3D);
		if (!next_fix) {
			fix = false;
		}
*/
		info->satellites = buffer.solution.satellites;
		info->hdop = buffer.solution.position_DOP;
		break;
	case MSG_PVT:
		Serial.println("MSG_PVT");
		break;
	case MSG_VELNED:

//		Serial.println("MSG_VELNED");
/*		speed_3d        = buffer.velned.speed_3d;                              // cm/s
		ground_speed = buffer.velned.speed_2d;                         // cm/s
		ground_course = buffer.velned.heading_2d / 1000;       // Heading 2D deg * 100000 rescaled to deg * 100
		_have_raw_velocity = true;
		_vel_north  = buffer.velned.ned_north;
		_vel_east   = buffer.velned.ned_east;
		_vel_down   = buffer.velned.ned_down;
		_new_speed = true;
*/
		break;
	default:
//		Serial.print("Unexpected NAV message 0x02x");
//		Serial.print(_msg_id, HEX);
//		Serial.print("\r\n");
		if (disable_counter == 0) {
//			Serial.println("Disabling NAV message 0x%02x");
			configure_message_rate(CLASS_NAV, _msg_id, 0);
		}
		return false;
	}

	// we only return true when we get new position and speed data
	// this ensures we don't use stale data
	if (newPosition) {
		newPosition = false;
		fixCount++;
		if (fixCount == 100) {
			// ask for nav settings every 20 seconds
//			Serial.println("Asking for engine setting\n");
			send_message(CLASS_CFG, MSG_CFG_NAV_SETTINGS, NULL, 0);
		}
		return true;
	}
	return false;

}

/*
 *  configure a UBlox GPS for the given message rate for a specific
 *  message class and msg_id
 */
void LEA6::configure_message_rate(uint8_t msg_class, uint8_t msg_id, uint8_t rate) {
    struct UBLOX_CFG_MSG_RATE msg;
    msg.msg_class = msg_class;
    msg.msg_id    = msg_id;
    msg.rate          = rate;
    send_message(CLASS_CFG, MSG_CFG_SET_RATE, &msg, sizeof(msg));
}

/*
 *  update checksum for a set of bytes
 */
void LEA6::update_checksum(uint8_t *data, uint8_t len, uint8_t &ck_a, uint8_t &ck_b) {
    while (len--) {
        ck_a += *data;
        ck_b += ck_a;
        data++;
    }
}

/*
 *  send a ublox message
 */
void LEA6::send_message(uint8_t msg_class, uint8_t msg_id, void *msg, uint8_t size) {
    struct UBLOX_HEADER header;
    uint8_t ck_a=0, ck_b=0;
    header.preamble1 = PREAMBLE1;
    header.preamble2 = PREAMBLE2;
    header.msg_class = msg_class;
    header.msg_id    = msg_id;
    header.length    = size;

    update_checksum((uint8_t *)&header.msg_class, sizeof(header)-2, ck_a, ck_b);
    update_checksum((uint8_t *)msg, size, ck_a, ck_b);

    Serial1.write((const uint8_t *)&header, sizeof(header));
    Serial1.write((const uint8_t *)msg, size);
    Serial1.write((const uint8_t *)&ck_a, 1);
    Serial1.write((const uint8_t *)&ck_b, 1);
}