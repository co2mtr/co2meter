#ifndef __CLOCK_H__
#define __CLOCK_H__
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
#include <Arduino.h>
#include <DS3231.h>
#include <Wire.h>

/**
 * Interface class for generic rt clocks.
 * any timer based class that should be used in rtclock must
 * implement this interface
 */
class ClockIF
{
public:
  virtual int getYear() = 0;
  virtual int getMonth(bool &monthflag) = 0;
  virtual int getDate() = 0;
  virtual int getHour(bool &hour12, bool &hour24) = 0;
  virtual int getMinute() = 0;
  virtual int getSecond() = 0;
  virtual int getTemperature() = 0;
  virtual void setYear(byte y) = 0;
  virtual void setMonth(byte m) = 0;
  virtual void setDate(byte d) = 0;
  virtual void setHour(byte H) = 0;
  virtual void setMinute(byte M) = 0;
  virtual void setSecond(byte S) = 0;
  virtual int getType() = 0;
};

/**
 * RealTime Clock. This class provides local time to the application.
 * Internally, a factory detects the available clocks on the device 
 * and instantiates the most suitable one.
 */
class rtclock
{
public:
  rtclock();

  void begin();
  /**
   * Forced update of the internal time representation.
   * Must be called before one of the getters are called
   * if most current timestamp is needed.
   * This is separated from the getters to reduce time
   * overhead for slow RealTime Clock Hardware.
   */
  void checkTime();

  /**
   * Returns the time as formatted string
   * @return "hh:mm:ss" (two digits each)
   */
  String getTime();

  /**
   * Returns the date as (european style) formatted string
   * @return "dd.mm.yy" (two digits each)
   */
  String getDate();

  /**
   * Returns the temperature read from the RT device
   * @return T in °C or -1 if not available
   */
  int getTemperature();

  /**
   * Returns the time and date as (european style) formatted string
   * @return "dd.mm.yy-HH:MM:SS" (two digits each)
   */
  String getTimeDate();

  int getHours() { return H; }
  int getMinutes() { return M; }
  int getSeconds() { return S; }

  /**
   * Sets the time and date on the RT device based 
   * on the class local members. Hence the functions
   * setDate and setTime need to be called first
   */
  void setTimeDate();
  /**
   * set the date in the internal structures from a formatted
   * string. 
   * 
   * @param date A string "YYYY-MM-DD"  
   */
  void setDate(String date);
  /**
   * set the time in the internal structures from a formatted
   * string. Seconds are not set!
   * 
   * @param date A string "HH:MM" (two digits each) 
   */
  void setTime(String time);

private:
  ClockIF *c; /// points to the real time clock that was found on the hardware

  int y;
  int m;
  int d;
  int H;
  int M;
  int S;
};

#endif /* __CLOCK_H__ */
