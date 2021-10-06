#ifndef __CO2METER_H__
#define __CO2METER_H__
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
#include "types.h"
#include <functional>
#include <Arduino.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include "config.h"
#include "display.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "logging.h"
#include "sensor_SCD.h"

/**
 * class co2meter defines the state machine of the device 
 */

class co2meter
{
public:
  co2meter(); /**<  constructor */
  co2meter(unsigned long ap_wait_thresh);

  /**
   * setter for the sensor values
   * 
   * @param c  co2value in ppm
   * @param t  temperature in C
   * @param h  humidity in %
   */
  void setValues(int c, int t, int h);

  /**
   * getter for the sensor values
   * 
   * @param c  co2value in ppm
   * @param t  temperature in C
   * @param h  humidity in %
   */
  void getValues(int &c, int &t, int &h);

  /**
   * getter for the internal sensor ID of the SCD sensor
   *
   * @return String - string of the sensor's vendor serial number
   */
  String getSensorSN();

  /**
   * class inistialiser
   * 
   * each state transition interval and time out values in milliseconds
   * are initialised in the threshold array. 
   *
   * The state machine is started in init state
   * 
   * The software version is stored 
   * 
   * graph to be displayed on the oled is initialised
   */
  void begin();

  /**
   * getter for the current state the machine is in.
   */
  int getState();

  /** 
   * main loop of the state machine
   */
  void control();

  /**
   * connect to a WiFI accespoint 
   */
  bool WifiConnect();

  /**
   * method to reconnect to an MQTT broker in case no connection was established
   * it connection was lost.
   */
  bool MQTTReconnect();

  /**
   * method to reconnect to an MQTT broker in case no connection was established
   * it connection was lost.
   */
  void getMQTTCredentials(String &, String &, String &, String &);

  /**
   * getter for device startup phase
   */
  bool isAtStartup();

  void setAtStartup(bool);
  /**
   * getter for MQTT flag
   */
  bool isMQTTMode();

  /**
   * getter for FHMQTT flag
   */
  bool isFHMQTTMode();

  /**
   * getter for AP flag
   */
  bool isHTTPMode();

  /**
   * getter for checking whether the previous mode, i.e. before the wifi 
   * connection was reset, was AP mode
   *
   */
  bool wasHTTPMode();

  /**
   * setting WIFI device mode
   */
  bool isWIFIMode();

  /**
   * setting MQTT device mode
   */
  void setMQTTMode();

  /**
   * setting FH MQTT Mode
   */
  void setFHMQTTMode();

  /**
   * setting AP mode
   */
  void setHTTPMode();

  /**
   * getter for WIFI flag
   */
  void setWIFIMode();

  /**
   * current time stamp
   */
  void now(unsigned long);

  /**
   * checkTime triggers if the current state is staged for execution
   * given its update interval
   */
  bool checkTime(unsigned int TIMER);

  /**
   * at a state transition, the timer for the successor state need to be 
   * reset to catch the right trigger times 
   */
  void resetTimer(unsigned long t);

  /**
   * getter for the firmware version 
   */
  int getFWMajor();

  /**
   * getter for the firmware version 
   */
  int getFWMinor();

  /**
   * getter for the firmware version 
   */
  int getFWMRevision();

  /**
   * getter for the deployed sensor type 
   */
  int getSensorType();

  /**
   * helper to switch Wifimode
   */
  bool configChanged();

private:
  void initStatemachine();
  void updateRTCValues();
  void processInitState();
  void processAPState();
  void processMQTTState();
  void processWiFiState();
  void processSenseState();
  void processPublishState();
  void processLoggingState();
  void processCalibrateState();

  // holds the maximum time [in ms] for a state. If threshold is exceeded state change will happen.
  unsigned int threshold[MAX_TIMER]; /**< array to store time out values */
                                     // counts the time the program is in a state;
                                     // there is a timer for being active in the state and one for waiting in the state
  unsigned int timer[MAX_TIMER];     /**< call interval of respective state */

  bool atStartup; /**< flag for device in startup mode */
  bool useWIFI;   /**< flag for WiFi option */
  bool useMQTT;   /**< flag for MQTT option */
  bool useFHMQTT; /**< flag for FH MQTT option */
  bool useHTTP;   /**< flag for local accespoint option */
  bool wasHTTP;   /**< previous mode was accespoint mode */
  int state;      /**< stores the current state of the state machine */
  int sens;       /**< flag for the sensor; in use */

  int co2value; /**< members for storing co2value */
  int temp;     /**< members for storing temperature */
  int rH;       /**< members for storing humidity */

  unsigned long uptime; /**< member to keep track of the device's uptime */
  unsigned long msgCnt; /**< monotone counter for MQTT messages */

  WiFiClient espClient;     /**< Wifi client member */
  IPAddress mqttBrokerAddr; /**< resolved mqttBrokerAddress */
  String mqttBrokerIP;      /**< mqttBrokerIP (name) this must be on class level or broker reqistration fails */
  PubSubClient client;      /**< MQTT client member */

  int FWMajor;    /**< firmware major member */
  int FWMinor;    /**< firmware minor member */
  int FWRevision; /**< firmware revision member */

  /** handle time stamps */
  unsigned long currentTime;  /**< current time stamp */
  unsigned long previousTime; /**< previously stamped time */

  char *clientName; /**< stores the unique device name (the MAC address) */

  int graph[GRAPHCOLS]; /**< holds the co2 values to be displayed on the OLED as a graph; organised as a ring buffer */
  int graph_current;    /**< graph index of the current value in the ring buffer for the co2 graph  */

  std::function<void()> statemachine[static_cast<int>(states::LAST) * sizeof(int *)];
};

#endif /* __CO2METER_H__ */
