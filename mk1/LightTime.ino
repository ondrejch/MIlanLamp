/*  Code for ESP8266 controller to run Smart Table Lamp and a buzzer
    Ondrej Chvala, ondrejch@gmail.com
    May 2022
    MIT license
*/
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
int pin_ledR =  4;    // D2
int pin_ledG = 14;    // D5
int pin_ledB = 12;    // D6
int pin_beep = 13;    // D7

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
  time(&now);                       // read the current time
  localtime_r(&now, &tm);           // update the structure tm with the current time
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
  if (tm.tm_hour == 7 and (tm.tm_min >=30 or tm.tm_min<60)) {
    if (tm.tm_sec % 4 < 2) {
//      Serial.println(tm.tm_sec % 5);
      r = 255; g = 165; b = 0;
    } else {
      r =  32; g = 178; b = 170;
    }
  }
  if (tm.tm_hour >= 8 and tm.tm_hour < 18) {
    r = 0; g = 0; b = 0;
  }
  // BEEP at 7am
  analogWrite(pin_beep, 0);
  if (tm.tm_hour == 7 and tm.tm_min <1) {
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
}

void showTime2() {
  time(&now);                       // read the current time
  localtime_r(&now, &tm);           // update the structure tm with the current time
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
  time(&now);                       // read the current time
  // human readable
  Serial.print("ctime:     ");
  Serial.print(ctime(&now));
}

void setup(){
  Serial.begin(115200);
  Serial.println("** Starting **");
  pinMode(pin_ledR, OUTPUT);
  pinMode(pin_ledG, OUTPUT);
  pinMode(pin_ledB, OUTPUT);
  pinMode(pin_beep, OUTPUT);
  RGB(0,0,0);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  configTime(my_tz, ntp_srv);
}

void loop() {
  showTime();
  setLights();
  delay(1000);
}
