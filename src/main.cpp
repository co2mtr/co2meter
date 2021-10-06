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

/**
 * @file main.cpp
 * @brief setup and main
 *
 * ESP setup and main method. All connected devices and sensors will
 * be initialized. 
 */


#include "co2meter.h"
#include "display.h"
#include <U8g2lib.h>
#include "sensor_MHZ19B.h"
#include "sensor_SCD.h"
#include "compileflags.h"
#include "clock.h"
#include "accesspoint.h"
#include "lights.h"
#include "globalLocks.h"

//#include <GDBStub.h>

/*To install LittleFS Upload tool see: https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html */
//MHZ19  device;

#ifdef DISPLAY_096
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
#else
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);
#endif

/* global variables */
co2meter m;
sensor mhz19;
sensor_SCD scd;
display oled;
lights ryg;
config conf;
#ifdef RTC
rtclock myclock;
#endif

logger sensorlog, chartlog;
accesspoint ap;

uint8_t loglevel = 0x0;

void logmsg(uint8_t level, const char *fmt, ...)
{
  LockGuard<Mutex> guard(serialMux);
  if (loglevel & level)
  {
    char buffer[100];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 100, fmt, args);

    Serial.printf("LOGMSG-%02x: %s\n", level, buffer);
    va_end(args);
  }
}

bool dbg = false;
void dbgmsg(const char *fmt, ...)
{
#ifdef DEBUG
  LockGuard<Mutex> guard(serialMux);
  if (dbg)
  {
    char buffer[100];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 100, fmt, args);

    Serial.printf("DBGMSG: %s\n", buffer);
    va_end(args);
  }
#endif
}

void setup()
{
  system_update_cpu_freq(160);
  Serial.begin(115200);
  //gdbstub_init();

  m.begin();

  loglevel |= LOGSM;
  loglevel |= LOGWIFI;
  loglevel |= LOGMQTT;
  loglevel |= LOGLOGGING;
  loglevel |= LOGSENSOR;
  loglevel |= LOGWEB;

  dbg = true;

#ifdef RTC
  myclock.begin();
#endif

  conf.readConfig();
  delay(1000);
  conf.printConfig(1);
  delay(1000);

  // Start display with 100kHz Bus Speed
  u8g2.setBusClock(100000UL);
  u8g2.setDisplayRotation(U8G2_R0);
  u8g2.begin();
#ifdef ROTATEDISPLAY
  u8g2.setDisplayRotation(U8G2_R2);
#else
  u8g2.setDisplayRotation(U8G2_R0);

#endif

  u8g2.clearBuffer();
  delay(1000);
  m.now(millis());
  delay(1000);
}

void loop()
{
  m.control();
  m.now(millis());
}
