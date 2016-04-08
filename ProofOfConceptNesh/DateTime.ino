
/*
  Simple RTC for Arduino Zero and MKR1000
  Based on http://arduino.cc/en/Tutorial/SimpleRTC
*/

#include <RTCZero.h>

/* Create an rtc object */
RTCZero rtc;

/* Change these values to set the current initial time */
const byte seconds = 30;
const byte minutes = 59;
const byte hours = 23;

/* Change these values to set the current initial date */
const byte day = 31;
const byte month = 12;
const byte year = 16;


void setup()
{
  Serial.begin(9600);
  syncRTC(year, month, day, hours, minutes, seconds);
}

void loop()
{  
  Serial.println(getTimeStamp()); // Print date...
  delay(1000);
}
void syncRTC(byte year, byte month, byte day, byte hours, byte minutes, byte seconds){
    rtc.begin(); // initialize RTC  
    rtc.setTime(hours, minutes, seconds);
    rtc.setDate(day, month, year);
}

String getTimeStamp(){
  //ISO8601 GMT yyyy-mm-ddTHH:MM:SS
  String dateTime = "20" + String(rtc.getYear()) + "-" + String(rtc.getMonth())  + "-" + String(rtc.getDay())+ "T" + String(rtc.getHours()) + ":" + String(rtc.getMinutes()) + ":" + String(rtc.getSeconds());
  return dateTime;
}


