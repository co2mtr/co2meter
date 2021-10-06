#ifndef __COMPILEFLAGS_H__
#define __COMPILEFLAGS_H__
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
#include "pmMutex.h"

//#define USEAVERAGE

//#define DEBUG

#define MAJORVERSION 1
#define MINORVERSION 0
#define REVISION 4

#define LOGSM 0x1
#define LOGWIFI 0x2
#define LOGMQTT 0x4
#define LOGLOGGING 0x8
#define LOGSENSOR 0x10
#define LOGWEB 0x20

#define MIN_FREE_HEAP_TO_SERVICE 6000

extern bool showrtc;
extern bool showanalog;

// Uncomment for No Wifi Option
//#define NO_WIFI

/* 
 * 1.0.4 - Disabled ntp again for a spuriuous crash when WiFi is set, but not Wifi available
 
 * 1.0.3 - Fix for standard MDNSname, if no name MDSN name is set, 
           the standard sensor name consisting of the last 4 digits of 
           the mac address is used.

 * 1.0.2 - Support for NTP re-enbled
         - Fix for calibration: Now works from all connections states 
           (APMode, WiFi, MQTT)
 
 * 1.0.1 - Fix for stack overflow in storing config

 * 1.0.0: - switched to synchronous webserver

 * 0.9.9: - added an additional OLED display layout
          - added FH MQTT mode
          - added new config settings: display types, temperature offset, FH MQTT mode

 * 0.9.8: added switching between config and data w/o restart

 * 0.9.7: -refactoring control loop

 * 0.9.6: - fixed displaying of clock time
     if RTC is not available also the clock time 
     is not being displayed at all. in the chart
     representation only timepoints are given
   - added rotate display option

 * 0.9.5: reduced Bus speed to 100kHz
 added Restart of Display
 moved some Compileflags to platformio.ini
 Bugfix RYG Limits on Data-Page


 * 0.9.4: added Display 0.96"
 support via #define DISPLAY_096
 take Temp from SCD instead of RTC

 * 0.9.3: added wifi option 
 fixed bug in "DisplayOn/Off"
 added redirect to WIFI.localIP for MQTT/WIFI Mode
*/

extern uint8_t loglevel;
extern bool dbg;
extern void logmsg(uint8_t level, const char *fmt, ...);
extern void dbgmsg(const char *fmt, ...);
extern void lock(Mutex &);
extern void unlock(Mutex &);

#endif /* __COMPILEFLAGS_H__ */
