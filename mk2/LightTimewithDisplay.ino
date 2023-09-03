/*  Code for ESP8266 controller to run Smart Table Lamp and a buzzer
    Ondrej Chvala, ondrejch@gmail.com
    May 2022
    MIT license
*/
#include <Adafruit_GFX.h>       // include Adafruit graphics library
#include <Adafruit_ILI9341.h>   // include Adafruit ILI9341 TFT library

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <time.h>

const char *my_tz    = "EST5EDT,M3.2.0,M11.1.0";
const char *ntp_srv  = "pool.ntp.org";

//const char *ssid     = "SSID";
//const char *password = "PASSWORD";
#include "wificreds.h"		   // keping real wifi credentials outside repository

/* Time */
time_t now;                         // this is the epoch
tm tm;                              // the structure tm holds time information in a more convient way
/* LED pinout */
#define pin_ledR   1
#define pin_ledG   3
#define pin_ledB   5
#define pin_beep  16

#define TFT_CS    D2     // TFT CS  pin is connected to NodeMCU pin D2
#define TFT_RST   D3     // TFT RST pin is connected to NodeMCU pin D3
#define TFT_DC    D4     // TFT DC  pin is connected to NodeMCU pin D4
// initialize ILI9341 TFT library with hardware SPI module
// SCK (CLK) ---> NodeMCU pin D5 (GPIO14)
// MOSI(DIN) ---> NodeMCU pin D7 (GPIO13)
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

void RGB(int red_value, int green_value, int blue_value) {
  analogWrite(pin_ledR, red_value);
  analogWrite(pin_ledG, green_value);
  analogWrite(pin_ledB, blue_value);
  Serial.print("brightness: ");
  Serial.print(red_value);
  Serial.print(" ");
  Serial.print(green_value);
  Serial.print(" ");
  Serial.println(blue_value);
}

void setLights() {
  if (tm.tm_year + 1900 < 2022) {   // wait until time sets
    return;
  }

  int r = 0, g = 0, b = 0;          // stores RGB values
  // MORNING
  if (tm.tm_hour <= 6) {
    r = 0; g = 0; b = 0;
  }
  if (tm.tm_hour == 6 and tm.tm_min == 0) {
    r = 135; g = 206; b = 250;
  }
  if (tm.tm_hour == 6 and tm.tm_min > 0) {
    r = 255; g = 155+tm.tm_min; b= 0;
  }
  if ( (tm.tm_hour == 6 and tm.tm_min >= 50) or
       (tm.tm_hour == 7 and tm.tm_min <= 10) ) {
    if (tm.tm_sec % 4 < 2) {
//      Serial.println(tm.tm_sec % 5);
      r = 255; g = 165; b = 0;
    } else {
      r =  32; g = 178; b = 170;
    }
  }
  if (tm.tm_hour == 7 and tm.tm_min  > 10) {
     r = 178; g = 178; b = 178;
  }
  if (tm.tm_hour >= 8 and tm.tm_hour < 18) {
    r = 0; g = 0; b = 0;
  }
  // BEEP at 6:50am
  analogWrite(pin_beep, 0);
  if ( (tm.tm_hour == 7 and tm.tm_min <  1) or  
       (tm.tm_hour == 6 and tm.tm_min >=50) ) {
    if (tm.tm_sec % 2 == 0) {
      analogWrite(pin_beep, 250);
    }
  }
  // NIGHT
  if (tm.tm_hour == 18) {
    r = 2*tm.tm_min; g = 139+tm.tm_min; b = 2*tm.tm_min;
  }
  if (tm.tm_hour == 19 and tm.tm_min < 20) {
    r = 34; g = 139; b = 34;
  }
  if (tm.tm_hour == 19 and tm.tm_min >= 20 and tm.tm_min <= 30) {
    r = 178; g = 34;  b = 34;
  }
  if (tm.tm_hour == 19 and tm.tm_min >= 30) {
    r = 255; g = 140 - (tm.tm_min-30); b = 0;
  }
  if (tm.tm_hour == 20) {
    r = 255-tm.tm_min+30; g = 60-2*tm.tm_min; b = 0;
  }
  if (tm.tm_hour >= 20 and tm.tm_min >= 31) {
    r = 0; g = 0; b = 0;
  }
  RGB(r, g, b);
//  RGB(255, 255, 255);
}

void showTime2() {
  Serial.print("year:");
  Serial.print(tm.tm_year + 1900);  // years since 1900
  Serial.print("\tmonth:");
  Serial.print(tm.tm_mon + 1);      // January = 0 (!)
  Serial.print("\tday:");
  Serial.print(tm.tm_mday);         // day of month
  Serial.print("\thour:");
  Serial.print(tm.tm_hour);         // hours since midnight  0-23
  Serial.print("\tmin:");
  Serial.print(tm.tm_min);          // minutes after the hour  0-59
  Serial.print("\tsec:");
  Serial.print(tm.tm_sec);          // seconds after the minute  0-61*
  Serial.print("\twday");
  Serial.print(tm.tm_wday);         // days since Sunday 0-6
  if (tm.tm_isdst == 1)             // Daylight Saving Time flag
    Serial.print("\tDST");
  else
    Serial.print("\tstandard");
  Serial.println();
}

void showTime() {
  Serial.print("ctime:     ");
  Serial.print(ctime(&now));
}

unsigned long testFillScreen() {
  unsigned long start = micros();
  tft.fillScreen(ILI9341_BLACK);
  tft.fillScreen(ILI9341_RED);
  tft.fillScreen(ILI9341_GREEN);
  tft.fillScreen(ILI9341_BLUE);
  tft.fillScreen(ILI9341_BLACK);
  return micros() - start;
}

void tftTimeOld() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(1);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.println(ctime(&now));
}

void tftSetup() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(1);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_RED, ILI9341_BLACK);
  tft.setTextSize(4);
  tft.println("Milan's clock");
  tft.setCursor(230, 230);
  tft.setTextSize(1);
  tft.println(WiFi.localIP());
}

void tftTime() {
  char buff1[40], buff2[40], buff3[40];
  
  strftime(buff1, 40, "%A  ", &tm);
  strftime(buff2, 40, "%F  ", &tm);
  strftime(buff3, 40, "%T %Z", &tm);
  tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK); 
  tft.setTextSize(3); 
  tft.setCursor(50, 65);
  tft.print(buff1);  
  tft.setTextColor(ILI9341_GREEN, ILI9341_BLACK); 
  tft.setCursor(50, 110);
  tft.print(buff2); 
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK); 
  tft.setCursor(50, 155);
  tft.print(buff3); 
}
  
unsigned long testText() {
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);
  tft.println(1234.56);
  tft.setTextColor(ILI9341_RED);    tft.setTextSize(3);
  tft.println(0xDEADBEEF, HEX);
  tft.println();
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(5);
  tft.println("Groop");
  tft.setTextSize(2);
  tft.println("I implore thee,");
  tft.setTextSize(1);
  tft.println("my foonting turlingdromes.");
  return micros() - start;
}


void setup(){
  Serial.begin(115200);
  Serial.println("** Starting **");
  pinMode(pin_ledR, OUTPUT);
  pinMode(pin_ledG, OUTPUT);
  pinMode(pin_ledB, OUTPUT);
  pinMode(pin_beep, OUTPUT);
  RGB(0,0,0);
  
  tft.begin();
  // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
  Serial.println(F("Benchmark                Time (microseconds)"));
  Serial.print(F("Screen fill              "));
  Serial.println(testFillScreen());
//  delay(100);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  tftSetup();
  configTime(my_tz, ntp_srv);
}

void loop() {
  time(&now);                       // read the current time
  localtime_r(&now, &tm);           // update the structure tm with the current time
  showTime();
  tftTime();
  setLights();
  delay(1000);
}
