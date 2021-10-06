/* MIT License

Copyright (c) 2021 CO2Meter Dey, Elsen, Ferrein, Frauenrath, Reke, Schiffer GbR  

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "compileflags.h"
#include "clock.h"
#include "config.h"
#include <DS3231.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

/**
 * Dummy clock class. This class will be intantiated, 
 * if neither a DS3231 nor an NTP server could be found
 * 
 * This class will return dummy values for y, m, d and
 * calculated values for h, m, s based on millis
 * Note, that during a millis() wraparound, the time calculations
 * using the RTClockDummy may fail.
 */
class RTClockDummy : public ClockIF
{
public:
  int getYear() { return 1707; };
  int getMonth(bool &monthflag) { return 4; };
  int getDate() { return 15; };
  int getHour(bool &hour12, bool &hour24) { return (millis() / millisPerHour % 24); }; // return hour in day
  int getMinute() { return (millis() / millisPerMinute) % 60; };
  int getSecond() { return (millis() / millisPerSecond) % 60; };
  int getTemperature() { return -273; };
  void setYear(byte y){};
  void setMonth(byte m){};
  void setDate(byte d){};
  void setHour(byte H){};
  void setMinute(byte M){};
  void setSecond(byte S){};
  int getType() { return 0; };

private:
  const int millisPerSecond = 1000;
  const int millisPerMinute = 60 * millisPerSecond;
  const int millisPerHour = 60 * millisPerMinute;
};

#ifdef ENABLE_NTP
class NTPClock : public ClockIF
{
public:
  NTPClock(int utcOffsetInSeconds);
  virtual int getYear();
  virtual int getMonth(bool &monthflag);
  virtual int getDate();
  virtual int getHour(bool &hour12, bool &hour24);
  virtual int getMinute();
  virtual int getSecond();
  virtual int getTemperature() { return -273; };
  virtual void setYear(byte y){};
  virtual void setMonth(byte m){};
  virtual void setDate(byte d){};
  virtual void setHour(byte H){};
  virtual void setMinute(byte M){};
  virtual void setSecond(byte S){};
  virtual int getType() { return 1; };

private:
  WiFiUDP ntpUDP;
  NTPClient timeClient;
  unsigned long epochTime;
};

NTPClock::NTPClock(int utcOffsetInSeconds) : ntpUDP(), timeClient(ntpUDP, utcOffsetInSeconds)
{
  timeClient.begin();
  timeClient.setTimeOffset(utcOffsetInSeconds);
}

int NTPClock::getHour(bool &hour12, bool &hour24)
{
  //bool success = timeClient.update();
  return timeClient.getHours();
}

int NTPClock::getMinute()
{
  //bool success = timeClient.update();
  return timeClient.getMinutes();
}

int NTPClock::getSecond()
{
  ///bool success = timeClient.update();
  return timeClient.getSeconds();
}

int NTPClock::getYear()
{
  bool success = timeClient.update();
  epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);

  return ptm->tm_year + 1900;
}

int NTPClock::getMonth(bool &monthflag)
{
  //bool success = timeClient.update();
  epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);
  return ptm->tm_mon + 1;
}

int NTPClock::getDate()
{
  //bool success = timeClient.update();
  epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);

  return ptm->tm_mday;
}
#endif

class RTClockDS3231 : public ClockIF
{
public:
  RTClockDS3231(){};
  virtual int getYear() { return c.getYear(); };
  virtual int getMonth(bool &monthflag) { return c.getMonth(monthflag); };
  virtual int getDate() { return c.getDate(); };
  virtual int getHour(bool &hour12, bool &hour24) { return c.getHour(hour12, hour24); };
  virtual int getMinute() { return c.getMinute(); };
  virtual int getSecond() { return c.getSecond(); };
  virtual int getTemperature() { return c.getTemperature(); };
  virtual void setYear(byte y) { c.setYear(y); };
  virtual void setMonth(byte m) { c.setMonth(m); };
  virtual void setDate(byte d) { c.setDate(d); };
  virtual void setHour(byte H) { c.setHour(H); };
  virtual void setMinute(byte M) { c.setMinute(M); };
  virtual void setSecond(byte S) { c.setSecond(S); };
  virtual int getType() { return 2; };

private:
  DS3231 c;
};

bool hasDS3231()
{
#define CLOCK_ADDRESS 0x68 // unfortunately, this is hidden in the DS3231 implementation
  DS3231 c;
  Wire.beginTransmission(CLOCK_ADDRESS);
  return (Wire.endTransmission() == 0); // clock is responsive on I2C
}

extern config conf;
bool hasNTP()
{
  // if an SSID is set, we try to use the NTP
  //return conf.getSSID() != "";
  return false;
}

static RTClockDS3231 RTCds3231;
#ifdef ENABLE_NTP
static NTPClock RTCNTP(3600);
#endif
static RTClockDummy RTCDummy;

/**
 * create a real time clock depending on the installed hardware
 * preference is, to use a local RTC based on DS3231, 
 * if that is not available an NTP server will be used, if WiFi is available
 * default is to use a dummy implementation, that gives at least 
 * reasonable time value in ascending order on subsequent calls within a day.
 */
ClockIF *rtClockFactory()
{
  // I2C must be initialized to check for DS3231
  Wire.setClock(100000UL);
  Wire.begin();

  if (hasDS3231())
  {
    return &RTCds3231;
  }
#ifdef ENABLE_NTP
  else if (hasNTP())
  {
    return &RTCNTP;
  }
#endif
  else // default path, do not change
  {
    return &RTCDummy;
  }
}

// get the currently installed clock
rtclock::rtclock() : c(rtClockFactory())
{
}

void rtclock::begin()
{
  y = m = d = H = M = S = 0;
  logmsg(LOGSM, "Starting Clock of type %d\n", c->getType());
}

void rtclock::checkTime()
{
  bool hour12, hour24, monthflag;

  y = c->getYear();
  m = c->getMonth(monthflag);
  d = c->getDate();
  H = c->getHour(hour12, hour24);
  M = c->getMinute();
  S = c->getSecond();
}

String rtclock::getTime()
{
  char time[9];
  sprintf(time, "%02i:%02i:%02i", H, M, S);
  return String(time);
}

String rtclock::getDate()
{
  char date[11];
  sprintf(date, "%02i.%02i.%02i", d, m, y);
  return String(date);
}

int rtclock::getTemperature()
{
  return c->getTemperature();
}

String rtclock::getTimeDate()
{
  char datetime[20];
  sprintf(datetime, "%02i.%02i.%02i-%02i:%02i:%02i", d, m, y, H, M, S);
  return String(datetime);
}

void rtclock::setTimeDate()
{
  Serial.printf("Setting Time and Date");
  c->setYear(y);
  c->setMonth(m);
  c->setDate(d);
  c->setHour(H);
  c->setMinute(M);
  c->setSecond(S);
}

void rtclock::setDate(String date)
{
  if (date.length() > 8)
  {
    logmsg(LOGSM, "Setting Date to %s\n", date.c_str());
    sscanf(date.c_str(), "%04d-%02d-%02d", &y, &m, &d);
    logmsg(LOGSM, "Date set to to %d-%d-%d\n", y, m, d);
  }
  else
  {
    logmsg(LOGSM, "setDate(): Date String \"%s\", too short", date.c_str());
  }
  y -= 2000;
}

void rtclock::setTime(String time)
{
  if (time.length() > 4)
  {
    logmsg(LOGSM, "Setting Time to %s\n", time.c_str());
    sscanf(time.c_str(), "%02d:%02d", &H, &M);
    logmsg(LOGSM, "Time set to %d:%d\n", H, M);
  }
  else
  {
    logmsg(LOGSM, "setTime(): Date String \"%s\", too short", time.c_str());
  }
}
