#ifndef __CONFIG_H__
#define __CONFIG_H__
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

#include "ArduinoJson.h"
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <pmMutex.h>

typedef enum
{
  LOCALAP,
  MQTT,
  CLOUD
} devicemodes;

#define CFGSCHOOLLEN 50
#define CFGCLASSIDLEN 30
#define CFGPUPILSLEN 4
#define CFGROOMSIZELEN 6
#define CFGSSIDLEN 30
#define CFGPASSWDLEN 128
#define CFGMQTTBROKERLEN 50
#define CFGMQTTPORTLEN 6
#define CFGMQTTUSERLEN 30
#define CFGMQTTPASSWDLEN 30
#define CFGMQTTPREFIXLEN 30
#define CFGMDNSNAMELEN 30
#define CFGDEVICEMODELEN 20
#define CFGDISPLAYONLEN 2
#define CFGLEDGREENLEN 5
#define CFGLEDYELLOWLEN 5
#define CFGLEDREDLEN 5
#define CFGTEMPOFFSETLEN 4
#define CFGSENSORIDLEN 15
#define CFGABCLEN 2
#define CFGDEFAULTCVLEN 10
#define CFGDISPLAYMODELEN 4

class config
{
  char school[CFGSCHOOLLEN];     /**< name of the school */
  char classid[CFGCLASSIDLEN];   /**< class id  */
  char pupils[CFGPUPILSLEN];     /**< # of pupils in the particular classroom */
  char roomsize[CFGROOMSIZELEN]; /**< the class room size in cubic meter*/
  //wifi
  char ssid[CFGSSIDLEN];     /**< SSID of a WIFI accesspoint*/
  char passwd[CFGPASSWDLEN]; /**< the WIFI password */
  //mqtt
  char mqtt_broker[CFGMQTTBROKERLEN]; /**< MQTT Broker URL */
  char mqtt_port[CFGMQTTPORTLEN];     /**< MQTT port */
  char mqtt_user[CFGMQTTUSERLEN];     /**< MQTT user name*/
  char mqtt_passwd[CFGMQTTPASSWDLEN]; /**< MQTT user name password */
  char mqtt_prefix[CFGMQTTPREFIXLEN]; /**< MQTT prefix of the topic */
  //dns
  char mdnsname[CFGMDNSNAMELEN]; /**< configurable mdns name*/
  //device
  char devicemode[CFGDEVICEMODELEN];   /**< different device modes */
  char displayon[CFGDISPLAYONLEN];     /**< flag for turning the display on/off */
  char ledgreen[CFGLEDGREENLEN];       /**< threshold for green LED in CO2 ppm */
  char ledyellow[CFGLEDYELLOWLEN];     /**< threshold for yellow LED in CO2 ppm*/
  char ledred[CFGLEDREDLEN];           /**< threshold for red LED in CO2 ppm*/
  char tempoffset[CFGTEMPOFFSETLEN];   /**< offset for the temperature shown on display */
  char sensorid[CFGSENSORIDLEN];       /**< unique sensor id (based on MAC address) */
  char ABC[CFGABCLEN];                 /**< ABC=automatic baseline calibration of the sensor */
  char defaultCV[CFGDEFAULTCVLEN];     /**< calibrate the sensor to this value*/
  bool doCalibration;                  /**< flag for doing a calibration */
  char displaymode[CFGDISPLAYMODELEN]; /**< flag for alternative display layouts */

  // global flags
  bool savenexit;      /**< leave config mode after saving data */
  bool needsReconnect; /**< flag for checking if WIFI mode needs to be reset */
  bool showrtc;        /**< flag for showing real-time clock values on OLED */
  bool showanalog;     /**< flag for analog watch display mode */

  bool inConfig; /**< flag for browser in configuration mode */

  Mutex mtx;
  int sensorType; /**< deployed sensorType */

public:
  config()
  {

    savenexit = false; /* default behaviour is to stay in config mode */

    int mac[6];

    sscanf(WiFi.macAddress().c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    sprintf(sensorid, "s_%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    /*store default values */
    stpncpy(ssid, "", 30);
    stpncpy(passwd, "", 30);
    stpncpy(mqtt_broker, "192.168.0.1", 50);
    stpncpy(mqtt_port, "1883", 6);
    sprintf(mqtt_prefix, "%s/", sensorid);
    doCalibration = false;
    needsReconnect = false;
    inConfig = false;
#ifdef RTC
    showrtc = true;
#else
    showrtc = false;
#endif
  }

  String getSchool() { return String(school); }
  String getClassid() { return String(classid); }
  String getPupils() { return String(pupils); }
  String getRoomsize() { return String(roomsize); }
  String getSSID() { return String(ssid); }
  String getPasswd() { return String(passwd); }
  String getMQTTBrokerIP() { return String(mqtt_broker); }
  String getMQTTPort() { return String(mqtt_port); }
  String getMQTTUser() { return String(mqtt_user); }
  String getMQTTPasswd() { return String(mqtt_passwd); }
  String getMQTTPrefix() { return String(mqtt_prefix); }
  String getDevicemode() { return String(devicemode); }
  String getDisplayOn() { return String(displayon); }
  String getDisplayMode() { return String(displaymode); }
  String getTempOffset() { return String(tempoffset); }
  String getLEDgreen() { return String(ledgreen); }
  String getLEDyellow() { return String(ledyellow); }
  String getLEDred() { return String(ledred); }
  char *getSensorID() { return sensorid; }
  bool getSaveNExit() { return savenexit; }
  String getABC() { return String(ABC); }
  String getDefaultCV() { return String(defaultCV); }
  bool getDoCalibration() { return doCalibration; }
  String getMDNSName() { return String(mdnsname); }

  bool getNeedsReconnect() { return needsReconnect; }
  bool getShowRTC() { return showrtc; }
  bool getShowAnalog() { return showanalog; }

  bool getInConfig() { return inConfig; }

  int getSensorType();

  void setSchool(String s)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(school, s.c_str(), CFGSCHOOLLEN);
    school[CFGSCHOOLLEN - 1] = '\0';
  }
  void setClassid(String c)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(classid, c.c_str(), CFGCLASSIDLEN);
    classid[CFGCLASSIDLEN - 1] = '\0';
  }
  void setPupils(String p)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(pupils, p.c_str(), CFGPUPILSLEN);
    pupils[CFGPUPILSLEN - 1] = '\0';
  }
  void setRoomsize(String r)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(roomsize, r.c_str(), CFGROOMSIZELEN);
    roomsize[CFGROOMSIZELEN - 1] = '\0';
  }
  void setSSID(String s)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(ssid, s.c_str(), CFGSSIDLEN);
    ssid[CFGSSIDLEN - 1] = '\0';
  }
  void setPasswd(String p)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(passwd, p.c_str(), CFGPASSWDLEN);
    passwd[CFGPASSWDLEN - 1] = '\0';
  }
  void setMQTTBrokerIP(String mb)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(mqtt_broker, mb.c_str(), CFGMQTTBROKERLEN);
    mqtt_broker[CFGMQTTBROKERLEN - 1] = '\0';
  }
  void setMQTTPort(String mp)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(mqtt_port, mp.c_str(), CFGMQTTPORTLEN);
    mqtt_port[CFGMQTTPORTLEN - 1] = '\0';
  }
  void setMQTTUser(String u)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(mqtt_user, u.c_str(), CFGMQTTUSERLEN);
    mqtt_user[CFGMQTTUSERLEN - 1] = '\0';
  }
  void setMQTTPasswd(String p)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(mqtt_passwd, p.c_str(), CFGMQTTPASSWDLEN);
    mqtt_passwd[CFGMQTTPASSWDLEN - 1] = '\0';
  }
  void setMQTTPrefix(String mqttpre)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(mqtt_prefix, mqttpre.c_str(), CFGMQTTPREFIXLEN);
    mqtt_prefix[CFGMQTTPREFIXLEN - 1] = '\0';
  }
  void setDevicemode(String d)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(devicemode, d.c_str(), CFGDEVICEMODELEN);
    devicemode[CFGDEVICEMODELEN - 1] = '\0';
  }
  void setDisplayOn(String d)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(displayon, d.c_str(), CFGDISPLAYONLEN);
    displayon[CFGDISPLAYONLEN - 1] = '\0';
  }
  void setDisplayMode(String d)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(displaymode, d.c_str(), CFGDISPLAYMODELEN);
    displaymode[CFGDISPLAYMODELEN - 1] = '\0';
  }
  void setLEDgreen(String g)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(ledgreen, g.c_str(), CFGLEDGREENLEN);
    ledgreen[CFGLEDGREENLEN - 1] = '\0';
  }
  void setLEDyellow(String y)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(ledyellow, y.c_str(), CFGLEDYELLOWLEN);
    ledyellow[CFGLEDYELLOWLEN - 1] = '\0';
  }
  void setLEDred(String r)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(ledred, r.c_str(), CFGLEDREDLEN);
    ledred[CFGLEDREDLEN - 1] = '\0';
  }
  void setTempOffset(String t)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(tempoffset, t.c_str(), CFGTEMPOFFSETLEN);
    tempoffset[CFGTEMPOFFSETLEN - 1] = '\0';
  }
  void setSaveNExit(bool b)
  {
    LockGuard<Mutex> guard(mtx);
    savenexit = b;
  }
  void setABC(String s)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(ABC, s.c_str(), CFGABCLEN);
    ABC[CFGABCLEN - 1] = '\0';
  }
  void setDefaultCV(String d)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(defaultCV, d.c_str(), CFGDEFAULTCVLEN);
    defaultCV[CFGDEFAULTCVLEN - 1] = '\0';
  }
  void setDoCalibration(bool b)
  {
    LockGuard<Mutex> guard(mtx);
    doCalibration = b;
  }
  void setMDNSName(String n)
  {
    LockGuard<Mutex> guard(mtx);
    stpncpy(mdnsname, n.c_str(), CFGMDNSNAMELEN);
    mdnsname[CFGMDNSNAMELEN - 1] = '\0';
  }

  void setNeedsReconnect(bool b)
  {
    LockGuard<Mutex> guard(mtx);
    needsReconnect = b;
  }
  void setShowRTC(bool b)
  {
    LockGuard<Mutex> guard(mtx);
    showrtc = b;
  }
  void setShowAnalog(bool b)
  {
    LockGuard<Mutex> guard(mtx);
    showanalog = b;
  }

  void setSensorType(int);

  void setInConfig(bool b) { inConfig = b; }
  void readConfig();
  void storeConfig();
  DynamicJsonDocument getJsonConfig();
  String printConfig(int);
};
#endif /* __CONFIG_H__ */
