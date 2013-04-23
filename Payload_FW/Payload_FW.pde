#include <TinyGPS.h>

/*
 AerE 462 ChipKit Firmware
 Matthew E. Nelson
 April 2013
 Iowa State University
 
 The program is designed to be used in conjunction with the AerE 462 drop test flights
 The main goals is to act as a repeater for the RFD900 radios, transmit GPS position of the payload
 and to trigger the cutdown devices.
 
 Version 0.2
 
 Revision History
 
 0.1 - Simple echo from port 1 to port 0
 0.2 - Added GPS parsing using TinyGPS library
 0.25 - GPS library tested and tweaked, parsing NMEA strings 
 0.3 - Very basic system for reading in a char and triggering an output
 
 */


TinyGPS gps;

static char dtostrfbuffer[20];

int LED = 13;
int BURN1 = 8;
int BURN2 = 9;
int BURN3 = 10;
int state = 0;

//Define String

char  Lat[20];
char  Lon[20];


static void gpsdump(TinyGPS &gps);
static bool feedgps();
static bool newData = false;
unsigned long chars;
unsigned short sentences, failed;
static void print_float(float val, float invalid, int len);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);

long lat, lon;

void setup() {
  
  pinMode(LED, OUTPUT);
  pinMode(BURN1, OUTPUT);
  pinMode(BURN2, OUTPUT);
  pinMode(BURN3, OUTPUT);
  // initialize all the serial ports!
  Serial.begin(9600);   //USB port and Radio out
  Serial1.begin(4800);  //Data from other RFD900, echos to 0
  Serial2.begin(4800);  //GPS device connected
  
  Serial.println("Booting up HAL (High Altitude Link) 2000");
  Serial.println("Hello Matthew, would you like to play a game of chess?");
  Serial.println("Initializing HABET Computer...");
  Serial.print("Testing TinyGPS library v. "); Serial.println(TinyGPS::library_version());
  Serial.println();
  Serial.print("Sizeof(gpsobject) = "); Serial.println(sizeof(TinyGPS));
  Serial.println();
  Serial.println("GPS Library and onboard computer initialized");
  Serial.println("----------------------------------------------------------------------------------------------------------------");
  Serial.println("Sats HDOP Latitude Longitude Fix  Date       Time       Alt    Course Speed Comp.  Sentences Checksum");
  Serial.println("          (deg)    (deg)     Age        (UTC)                  (m)     from GPS     RX        Fails");
  Serial.println("----------------------------------------------------------------------------------------------------------------");

}

//Main loop, goes on forever, until I pull HALs central core
void loop() {
  
  bool newdata = false;
  unsigned long start = millis();
//This is the echo section, Anything on Serial port 1, will get echo'd to Serial port 0  
   
  if (Serial1.available()) {
    int inByte = Serial1.read();
    Serial.write(inByte); 
  }
  
  if (Serial.available())  {
    int inByte = Serial.read();
    if (inByte == '1'){
    Serial.println("Dropping plane 1...");
    digitalWrite(BURN1,1);
    delay(2000);
    digitalWrite(BURN1,0);
    }
    if (inByte == '2'){
    Serial.println("Dropping plane 2...");
    digitalWrite(BURN2,1);
    delay(2000);
    digitalWrite(BURN2,0);
    }
    if (inByte == '3'){
    Serial.println("Dropping plane 3...");
    digitalWrite(BURN3,1);
    delay(2000);
    digitalWrite(BURN3,0);
    }
  }
  
  //Send data every 5 seconds
  while (millis() - start < 2000)
  {
    if (feedgps())
      newdata = true;
  }
  
  gpsdump(gps);
  state = !state;
  digitalWrite(LED, state);
}




static void gpsdump(TinyGPS &gps)
{
  float flat, flon;
  unsigned long age, date, time, chars = 0;
  unsigned short sentences = 0, failed = 0;
  static const float LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
  
  print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5);
  print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5);
  gps.f_get_position(&flat, &flon, &age); 
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 9, 5); //LATITUDE
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 10, 5); //LONGITUDE
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);

  print_date(gps); //DATE AND TIME

  print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 8, 2);
  print_float(gps.f_course(), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  print_float(gps.f_speed_kmph(), TinyGPS::GPS_INVALID_F_SPEED, 6, 2);
  print_str(gps.f_course() == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(gps.f_course()), 6);
  //print_int(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0UL : (unsigned long)TinyGPS::distance_between(flat, flon, LONDON_LAT, LONDON_LON) / 1000, 0xFFFFFFFF, 9);
  //print_float(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : TinyGPS::course_to(flat, flon, 51.508131, -0.128002), TinyGPS::GPS_INVALID_F_ANGLE, 7, 2);
  //print_str(flat == TinyGPS::GPS_INVALID_F_ANGLE ? "*** " : TinyGPS::cardinal(TinyGPS::course_to(flat, flon, LONDON_LAT, LONDON_LON)), 6);

  gps.stats(&chars, &sentences, &failed);
  //print_int(chars, 0xFFFFFFFF, 6);
  print_int(sentences, 0xFFFFFFFF, 10);
  print_int(failed, 0xFFFFFFFF, 9);
  Serial.println();
}

static void print_int(unsigned long val, unsigned long invalid, int len)
{
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  feedgps();
}

static void print_float(float val, float invalid, int len, int prec)
{
  char sz[32];
  if (val == invalid)
  {
    strcpy(sz, "*******");
    sz[len] = 0;
        if (len > 0) 
          sz[len-1] = ' ';
    for (int i=7; i<len; ++i)
        sz[i] = ' ';
    Serial.print(sz);
  }
  else
  {
    
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1);
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(" ");
  }
  feedgps();
}

static void print_date(TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
  {
    Serial.print("*******    *******    ");
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d   ",
        month, day, year, hour, minute, second);
    Serial.print(sz);
  }
  //print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  feedgps();
}

static void print_str(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  feedgps();
}

static bool feedgps()
{
  while (Serial2.available())
  {
    if (gps.encode(Serial2.read()))
      return true;
  }
  return false;
}
