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
#include "accesspoint.h"
#include "compileflags.h"

char *accesspoint::getSSID()
{
  return ssid;
};

const char *accesspoint::getPasswd()
{
  return password;
}

char *accesspoint::getAPName()
{
  apname[APNAME_LEN - 1] = '\0';
  return apname;
}

String accesspoint::getAPIP()
{
  return myIP.toString();
}

char *accesspoint::getDevicename()
{
  devicename[DEVICENAME_LEN - 1] = '\0';
  return devicename;
}

char *accesspoint::getMDNSName()
{
  mdnsname[MDNSNAME_LEN - 1] = '\0';
  return mdnsname;
}

void accesspoint::setDevicename()
{
  int mac[6];

  sscanf(WiFi.macAddress().c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
  sprintf(apname, "AP%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  sprintf(devicename, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  sprintf(mdnsname, "s%02x%02x", mac[4], mac[5]);
}

void accesspoint::setMDNSName(String s)
{
  logmsg(LOGWIFI, "Setting MDSNName %s", s.c_str());
  stpncpy(mdnsname, s.c_str(), 30);
}

void accesspoint::startAP()
{

  //ESP.eraseConfig();

  //delay(1000);
  // ESP.restart();
  setDevicename();
  logmsg(LOGWIFI, "Starting AP %s ...", apname);

  WiFi.mode(WIFI_AP); //need both to serve the webpage and take commands via tcp
  IPAddress ip(192, 168, 0, 1);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(apname, "00000000"); //Access point is open

  delay(1000);

  myIP = WiFi.softAPIP();
  logmsg(LOGWIFI, "AP IP address: %s", myIP.toString().c_str());
  ssid = strdup(apname);
  delay(1000);
}
