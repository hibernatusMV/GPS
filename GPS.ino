/* Display (Pin) - Arduino Nano
 *  GND  (1) - GND  
 *  VCC  (2) - 5V
 *  SCK  (3) - D13
 *  SDA  (4) - D11
 *  RES  (5) - D8
 *  RS   (6) - D9
 *  CS   (7) - D10
 *  LEDA (8) - 3.3V
 */

/* GPS (Pin) - Arduino Nano 
 * GND  - GND
 * VCC  - 5V
 * RX   - D4
 * TX   - D3
 */

#define TFT_PIN_CS   10 // Arduino pin on display CS   
#define TFT_PIN_DC   9  // Arduino pin on display DC
#define TFT_PIN_RST  8  // Arduino pin on display reset pin

#define GPSTIME   5000 // GPS coordinates are shown every 5s

#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>    // Adafruit grafic library
#include <Adafruit_ST7735.h> // Adafruit ST7735 library
#include <Timezone.h>

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_PIN_CS, TFT_PIN_DC, TFT_PIN_RST);  // ST7735 library setup

// Change this to suit your Time Zone
TimeChangeRule CEST = {"", Last, Sun, Mar, 2, 120};     
TimeChangeRule CET = {"", Last, Sun, Oct, 3, 60}; 
Timezone CE(CEST, CET);
TimeChangeRule *tcr;
time_t utc;   // Universal Time
time_t ltime; // Local Time
char UTCTime[32];
char UTCDate[32];
char localTime[32];
char localDate[32];

long  gpstimer;
long  curTime;

// Emulated port to GPS module
SoftwareSerial mySerial(3, 4);
Adafruit_GPS gps(&mySerial);

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  false

void setup() {
  // connection parameters to GP module
  Serial.begin(115200);
  delay(1000);
  Serial.println("Adafruit GPS library test!");
  
  gps.begin(9600);

  // turn on RMC (recommended minimum) and GGA (fix data) including altitude
  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

  // Set the update rate
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate

  // Request updates on antenna status, comment out to keep quiet
  gps.sendCommand(PGCMD_ANTENNA);

  delay(1000);
  
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);
  
  /***
  * ST7735 Chip initialize (INITR_BLACKTAB / INITR_REDTAB / INITR_GREENTAB) 
  ***/
  tft.initR(INITR_BLACKTAB);

  tft.setTextWrap(false);
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(1);
  tft.setCursor(0, 0);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);

  drawFrame();
}

uint32_t timer = millis();
void loop() {
  char c = gps.read();
  // if you want to debug, this is a good time to do it!
  if ((c) && (GPSECHO))
    Serial.write(c);

  // if a sentence is received, we can check the checksum, parse it...
  if (gps.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    //Serial.println(gps.lastNMEA());   // this also sets the newNMEAreceived() flag to false

    if (!gps.parse(gps.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every 1 seconds or so, print out the current stats
  if (millis() - timer > 1000) {
    timer = millis(); // reset the timer
    
    showPositionData(); // print output to display 
  }
}

void showPositionData() {
  utc = tmConvert_t((2000 + gps.year), gps.month, gps.day, gps.hour, gps.minute, gps.seconds);
  ltime = CE.toLocal(utc, &tcr);
  sprintf(localTime, "%.02d:%.02d:%.02d", hour(ltime), minute(ltime), second(ltime));
  sprintf(localDate, "%.02d.%02d.%d", day(ltime), month(ltime), year(ltime));

  displayText(50, 20, localDate, ST7735_WHITE, ST7735_BLACK);
  displayText(50, 30, localTime, ST7735_WHITE, ST7735_BLACK);
 
  if (gps.fix) {
    drawSatellite(130, 25, ST7735_WHITE);
    
    // Refresh coordinates after a defined interval
    curTime = millis();
    if (((curTime - gpstimer ) > GPSTIME ) || (curTime < gpstimer)) {
      gpstimer = curTime;
      
      int degLat = gps.latitudeDegrees;
      float minutesRemainderLat = abs(gps.latitudeDegrees - degLat) * 60;
      int arcMinutesLat = minutesRemainderLat;
      int arcSecondsLat = (minutesRemainderLat - arcMinutesLat) * 60;
  
      String latitude = (degLat < 10 ? "0" : "");
      latitude += degLat;
      latitude += ((char)247);
      latitude += " ";
      latitude += (arcMinutesLat < 10 ? "0" : "");
      latitude += arcMinutesLat;
      latitude += "' ";
      latitude += (arcSecondsLat < 10 ? "0" : "");
      latitude += arcSecondsLat;
      latitude += "\" ";
      latitude += (gps.lat);
  
      displayText(50, 65, latitude, ST7735_WHITE, ST7735_BLACK);
      displayText(50, 75, "(" + String(gps.latitudeDegrees, 4) + ")", ST7735_WHITE, ST7735_BLACK);
      
      int degLon = gps.longitudeDegrees;
      float minutesRemainderLon = abs(gps.longitudeDegrees - degLon) * 60;
      int arcMinutesLon = minutesRemainderLon;
      int arcSecondsLon = (minutesRemainderLon - arcMinutesLon) * 60;
  
      String longitude = (degLon < 10 ? "0" : "");
      longitude += degLon;
      longitude += ((char)247);
      longitude += " ";
      longitude += (arcMinutesLon < 10 ? "0" : "");
      longitude += arcMinutesLon;
      longitude += "' ";
      longitude += (arcSecondsLon < 10 ? "0" : "");
      longitude += arcSecondsLon;
      longitude += "\" ";
      longitude += (gps.lon);
  
      displayText(50,90, longitude, ST7735_WHITE, ST7735_BLACK);
      displayText(50,100, "(" + String(gps.longitudeDegrees, 4) + ")", ST7735_WHITE, ST7735_BLACK);
  
      String alt = String((int) gps.altitude);
      alt += " m";
      displayText(50, 110, alt, ST7735_WHITE, ST7735_BLACK);
    }
  } else {
    drawSatellite(130, 25, ST7735_RED);
  }
}

time_t tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss) {
  tmElements_t tmSet;
  tmSet.Year = YYYY - 1970;
  tmSet.Month = MM;
  tmSet.Day = DD;
  tmSet.Hour = hh;
  tmSet.Minute = mm;
  tmSet.Second = ss;
  return makeTime(tmSet); //convert to time_t
}

void drawFrame(){
  tft.drawRect(0,0,160,128,ST7735_WHITE);
  tft.drawLine(1,15,160,15,ST7735_WHITE);
  tft.drawLine(1,60,160,60,ST7735_WHITE);
  displayText(4, 4, "Local Date & Time", ST7735_WHITE, ST7735_BLACK);
  displayText(4, 20, "Date : ", ST7735_WHITE, ST7735_BLACK);
  displayText(4, 30, "Time : ", ST7735_WHITE, ST7735_BLACK);
  displayText(4, 50, "Coordinates", ST7735_WHITE, ST7735_BLACK);
  displayText(4, 65, "Lat : ", ST7735_WHITE, ST7735_BLACK);
  displayText(4, 90, "Lon : ", ST7735_WHITE, ST7735_BLACK);
  displayText(4, 110, "Alt : ", ST7735_WHITE, ST7735_BLACK);
}

void drawSatellite(uint16_t x, uint16_t y, uint16_t color){
  tft.drawLine((x + 2), (y + 7), (x + 7), (y + 3), color);
  tft.drawLine((x + 7), (y + 3), (x + 9), (y + 5), color);
  tft.drawLine((x + 5), (y + 9), (x + 9), (y + 5), color);
  tft.drawLine((x + 2), (y + 7), (x + 5), (y + 9), color);

  tft.drawLine((x + 1), (y + 1), (x + 2), y, color);
  tft.drawLine((x + 2), y, (x + 6), (y + 4), color);
  tft.drawLine((x + 1), (y + 1), (x + 4), (y + 4), color);

  // tft.drawLine((x + 8), (y + 8), (x + 9), (y + 7), color);
  tft.drawLine((x + 9), (y + 7), (x + 12), (y + 10), color);
  tft.drawLine((x + 11), (y + 11), (x + 12), (y + 10), color);
  tft.drawLine((x + 8), (y + 8), (x + 11), (y + 11), color);

  tft.drawLine((x + 2), (y + 9), (x + 3), (y + 10), color);
  tft.drawPixel((x + 2), (y + 10), color);

  tft.drawLine(x, (y + 9), x, (y + 10), color);
  tft.drawPixel(x, (y + 10), color);
  tft.drawLine((x + 2), (y + 12), (x + 3), (y + 12), color);
}

//void displayText(uint16_t x, uint16_t y, char *text , uint16_t color, uint16_t bgcolor) {
void displayText(uint16_t x, uint16_t y, String text , uint16_t color, uint16_t bgcolor) {
  tft.setCursor(x, y);
  tft.setTextColor(color, bgcolor);
  tft.print(text);
}
