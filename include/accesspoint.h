#ifndef __ACCESSPOINT_H__
#define __ACCESSPOINT_H__
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

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>

#define DEVICENAME_LEN 13
#define MDNSNAME_LEN 30
#define APNAME_LEN 16
class accesspoint
{

  char *ssid;
  const char *password = "00000000";
  char devicename[DEVICENAME_LEN];
  char mdnsname[MDNSNAME_LEN];
  char apname[APNAME_LEN];
  IPAddress myIP;

public:
  accesspoint(){};

  char *getSSID();
  const char *getPasswd();
  char *getAPName();
  String getAPIP();
  char *getDevicename();
  char *getMDNSName();

  void setDevicename();
  void setMDNSName(String s);

  void startAP();
};

#endif /* __APMODE_H__ */
