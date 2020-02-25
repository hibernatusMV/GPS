# GPS

## Prerequisites
- Arduino IDE https://www.arduino.cc/en/main/software
- Breadboard
- Libraries:  
  Adafruit_GPS.h  
  Adafruit_GFX.h  
  Adafruit_ST7735.h  
  SoftwareSerial.h  
  Timezone.h  

## Hardware Installation

**_Components_**  
  Display: ST7735 1.8" 160×128 SPI TFT https://amzn.to/2Pou4WC  
  GPS Receiver: NEO-6M GPS Module (identical to Ublox) https://amzn.to/2VoHCFk  
  e.g. Nano V3.0 with Atmega328 CH340 (compatible to Arduino Nano V3) https://amzn.to/2T0fXsA  


```
Display (Pin) - Arduino Nano (Pin)
 GND  (1) - GND  
 VCC  (2) - 5V
 SCK  (3) - D13
 SDA  (4) - D11
 RES  (5) - D8
 RS   (6) - D9
 CS   (7) - D10
 LEDA (8) - 3.3V
``` 

```
GPS (Pin) - Arduino Nano 
 GND  - GND
 VCC  - 5V
 RX   - D4
 TX   - D3
```

## Wiring
![Wiring](https://github.com/hibernatusMV/GPS/blob/master/ArduinoNano_GPS_Wiring.jpg "Wiring")
