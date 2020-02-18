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

void setup()
{
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
  
  // Verbindung mit GPS-Modul
  gpsPort.begin(GPSBaud);
  Serial.begin(9600);
}

void loop()
{
  while (gpsPort.available() > 0) // Daten vorhanden?
    if (gps.encode(gpsPort.read())) {
      showPositionData(); // ja, dann Ausgabe auf Display
    }

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println("Fehler: GPS Modul nicht gefunden");
    while(true);
  }
}

void showPositionData()
{
  if (gps.date.isValid() && gps.time.isValid())
  {
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK); // besser als fillScreen. Kein flackern mehr bei sekunden etc.
    tft.setCursor(0, 0);
    tft.println("Local Date & Time");
    tft.println("---------------------");

    utc = tmConvert_t(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
    ltime = CE.toLocal(utc, &tcr);
    sprintf(localTime, "%.02d:%.02d:%.02d", hour(ltime), minute(ltime), second(ltime));
    sprintf(localDate, "%.02d.%02d.%d", day(ltime), month(ltime), year(ltime));
    tft.print("Date : ");
    tft.println(localDate);
    tft.print("Time : ");
    tft.println(localTime);
  }
 

  // Refresh coordinates after a defined interval
  curTime = millis();
  if (gps.location.isValid() && (((curTime - gpstimer ) > GPSTIME ) || (curTime < gpstimer)))
//  if (gps.location.isValid())
  {
    gpstimer = curTime;
    tft.println("");
    tft.println("Coordinates");
    tft.println("---------------------");
    tft.print("Lat: ");
    int degLat = gps.location.lat();
    float minutesRemainderLat = abs(gps.location.lat() - degLat) * 60;
    int arcMinutesLat = minutesRemainderLat;
    int arcSecondsLat = (minutesRemainderLat - arcMinutesLat) * 60;
    tft.print(degLat < 10 ? "0" : "");
    tft.print(degLat);
    tft.print((char)247); // Grad
    tft.print(" ");
    tft.print(arcMinutesLat < 10 ? "0" : "");
    tft.print(arcMinutesLat);
    tft.print("' ");
    tft.print(arcSecondsLat < 10 ? "0" : "");
    tft.print(arcSecondsLat);
    tft.print("\"");   
    tft.println(gps.location.rawLat().negative ? " S" : " N");
    
//    tft.print(" (");
//    tft.print(gps.location.lat(), 3);
//    tft.println(")");

    tft.print("Lon: ");
//    char * myLon = DegreesToDegMinSec(gps.location.lng());
//    tft.print(myLon);
    int degLon = gps.location.lng();
    float minutesRemainderLon = abs(gps.location.lng() - degLon) * 60;
    int arcMinutesLon = minutesRemainderLon;
    int arcSecondsLon = (minutesRemainderLon - arcMinutesLon) * 60;
    tft.print(degLon < 10 ? "0" : "");
    tft.print(degLon);
    tft.print((char)247); // Grad
    tft.print(" ");
    tft.print(arcMinutesLon < 10 ? "0" : "");
    tft.print(arcMinutesLon);
    tft.print("' ");
    tft.print(arcSecondsLon < 10 ? "0" : "");
    tft.print(arcSecondsLon);
    tft.print("\"");
    tft.println(gps.location.rawLng().negative ? " W" : " E"); 

//    tft.print(" (");
//    tft.print(gps.location.lng() < 10 ? "0" : "");
//    tft.print(gps.location.lng(), 3);
//    tft.println(")");

    if (gps.altitude.isValid())
    {
      tft.println("");
      tft.print("Alt: ");
      tft.print(gps.altitude.meters());
      tft.println(" m");
    }
    
  }
}

time_t tmConvert_t(int YYYY, byte MM, byte DD, byte hh, byte mm, byte ss)
{
  tmElements_t tmSet;
  tmSet.Year = YYYY - 1970;
  tmSet.Month = MM;
  tmSet.Day = DD;
  tmSet.Hour = hh;
  tmSet.Minute = mm;
  tmSet.Second = ss;
  return makeTime(tmSet); //convert to time_t
}
