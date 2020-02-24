/* Pinnung
 *  Display (Pin) - Arduino Nano
 *  GND  (1) - GND  
 *  VCC  (2) - 5V
 *  SCK  (3) - D13
 *  SDA  (4) - D11
 *  RES  (5) - D8
 *  RS   (6) - D9
 *  CS   (7) - D10
 *  LEDA (8) - 3.3V
 *  
 *  LEDA kann auch an 5V doch dann wird das Display sehr schnell sehr heiß - was ich nicht für optimal halte.
 *  Beim Betrieb mit 3.3V ist das Display nur minimal dunkler und bleibt kalt.
 */

#define TFT_PIN_CS   10 // Arduino-Pin an Display CS   
#define TFT_PIN_DC   9  // Arduino-Pin an 
#define TFT_PIN_RST  8  // Arduino Reset-Pin

#define GPSTIME   5000              // GPS coordinates are shown every 5s

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>             // SPI für die Kommunikation
#include <Adafruit_GFX.h>    // Adafruit Grafik-Bibliothek wird benötigt
#include <Adafruit_ST7735.h> // Adafruit ST7735-Bibliothek wird benötigt
#include <Timezone.h>

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_PIN_CS, TFT_PIN_DC, TFT_PIN_RST);  // ST7735-Bibliothek Setup

// Verwendete digitale Pins für SoftwareSerial
static const int RX = 3, TX = 4;
// !!! Baudrate des GPS-Moduls: Anpassung an eigene HW notwendig
static const uint32_t GPSBaud = 9600;

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

// Zugriff auf TinyGPS++ ueber gps
TinyGPSPlus gps;

// Emulierter Port zum GPS Geraet
SoftwareSerial gpsPort(RX, TX);

void setup() {
  /***
  * ST7735-Chip initialisieren (INITR_BLACKTAB / INITR_REDTAB / INITR_GREENTAB) 
  ***/
  tft.initR(INITR_BLACKTAB);

  tft.setTextWrap(false);
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(1);
  tft.setCursor(0, 0);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);

  drawFrame();
  
  // Verbindung mit GPS-Modul
  gpsPort.begin(GPSBaud);
  Serial.begin(9600);
}

void loop() {
  while (gpsPort.available() > 0) // Daten vorhanden?
    if (gps.encode(gpsPort.read())) {
      showPositionData(); // ja, dann Ausgabe auf Display
    }

  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println("Fehler: GPS Modul nicht gefunden");
    displayText(4, 70, "Fehler: GPS Modul nicht gefunden", ST7735_RED, ST7735_BLACK);
    while(true);
  }
}

void showPositionData() {
  if (gps.date.isValid() && gps.time.isValid()) {
    displayText(4, 4, "Local Date & Time", ST7735_WHITE, ST7735_BLACK);
    
    utc = tmConvert_t(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
    ltime = CE.toLocal(utc, &tcr);
    sprintf(localTime, "%.02d:%.02d:%.02d", hour(ltime), minute(ltime), second(ltime));
    sprintf(localDate, "%.02d.%02d.%d", day(ltime), month(ltime), year(ltime));
    
    displayText(4, 20, "Date : ", ST7735_WHITE, ST7735_BLACK);
    displayText(4, 30, "Time : ", ST7735_WHITE, ST7735_BLACK);
    
    displayText(50, 20, localDate, ST7735_WHITE, ST7735_BLACK);
    displayText(50, 30, localTime, ST7735_WHITE, ST7735_BLACK);
  }
 
  displayText(4, 50, "Coordinates", ST7735_WHITE, ST7735_BLACK);
  drawSatellite(20, 50, ST7735_WHITE);
  
  // Refresh coordinates after a defined interval
  curTime = millis();
//  if (gps.location.isValid() && (((curTime - gpstimer ) > GPSTIME ) || (curTime < gpstimer)))
  if (gps.location.isValid()) {
    if (((curTime - gpstimer ) > GPSTIME ) || (curTime < gpstimer)) {
      displayText(40, 70, "No GPS fix ...", ST7735_BLACK, ST7735_BLACK);
      
      gpstimer = curTime;
      
      int degLat = gps.location.lat();
      float minutesRemainderLat = abs(gps.location.lat() - degLat) * 60;
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
      latitude += "\"";
      latitude += (gps.location.rawLat().negative ? " S" : " N");

      displayText(4, 65, "Lat : ", ST7735_WHITE, ST7735_BLACK);
      displayText(50, 65, latitude, ST7735_WHITE, ST7735_BLACK);
      displayText(50, 75, "(" + String(gps.location.lat(), 6) + ")", ST7735_WHITE, ST7735_BLACK);
      
      int degLon = gps.location.lng();
      float minutesRemainderLon = abs(gps.location.lng() - degLon) * 60;
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
      longitude += "\"";
      longitude += (gps.location.rawLng().negative ? " W" : " E");

      displayText(4, 90, "Lon : ", ST7735_WHITE, ST7735_BLACK);
      displayText(50,90, longitude, ST7735_WHITE, ST7735_BLACK);
      displayText(50,100, "(" + String(gps.location.lng(), 6) + ")", ST7735_WHITE, ST7735_BLACK);
  
      if (gps.altitude.isValid()) {
        String alt = String((int) gps.altitude.meters());
        alt += " m";
        displayText(4, 110, "Alt : ", ST7735_WHITE, ST7735_BLACK);
        displayText(50, 110, alt, ST7735_WHITE, ST7735_BLACK);
      }
    }
  } else {
    displayText(40, 70, "No GPS fix ...", ST7735_RED, ST7735_BLACK);
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
  
}

void drawSatellite(uint16_t x, uint16_t y, uint16_t color){
  tft.drawLine((x + 2), (y + 7), (x + 7), (y + 3), color);
  tft.drawLine((x + 7), (y + 3), (x + 9), (y + 5), color);
  tft.drawLine((x + 5), (y + 9), (x + 9), (y + 5), color);
  tft.drawLine((x + 2), (y + 7), (x + 5), (y + 9), color);

  tft.drawLine((x + 1), (y + 1), (x + 2), y, color);
  tft.drawLine((x + 2), y, (x + 6), (y + 4), color);
  tft.drawLine((x + 4), (y + 4), (x + 6), (y + 4), color);
  tft.drawLine((x + 1), (y + 1), (x + 4), (y + 4), color);

  tft.drawLine((x + 8), (y + 8), (x + 9), (y + 7), color);
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
