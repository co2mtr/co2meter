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
#include "co2meter.h"
#include "compileflags.h"
#include "types.h"
#include "apmode.h"
#include <ESP8266WiFi.h>
#include "display.h"
#include "accesspoint.h"
#include <PubSubClient.h>
#include "sensor_MHZ19B.h"
#include "sensor_SCD.h"
#include "lights.h"
#include "clock.h"
#include "globalLocks.h"
#include "cryptstring.h"

int co2;
int temp;
int rH;

extern co2meter m;
extern sensor mhz19;
extern sensor_SCD scd;
extern accesspoint ap;
extern lights ryg;
extern display oled;
extern config conf;
extern ESP8266WebServer server;
extern bool canHandleRequest();
extern void startHandleRequest();
extern void stopHandleRequest();
extern void webserverRegisterCallbacks();

logrecord *sensorlogrecord, *chartlogrecord;
extern logger sensorlog, chartlog;
#ifdef RTC
extern rtclock myclock;
#endif

co2meter::co2meter()
{
  client.setKeepAlive(60);
  client.setSocketTimeout(60);
  initStatemachine();
}

co2meter::co2meter(unsigned long ap_wait_thresh)
{
  threshold[ACCESSPOINT_WAIT] = ap_wait_thresh;
  sens = NONE;
  client.setKeepAlive(60);
  client.setSocketTimeout(60);
  initStatemachine();
}

void co2meter::initStatemachine()
{
  statemachine[INIT] = std::bind(&co2meter::processInitState, this);
  statemachine[ACCESSPOINT] = std::bind(&co2meter::processAPState, this);
  statemachine[MQTT_CONNECT] = std::bind(&co2meter::processMQTTState, this);
  statemachine[WIFI_CONNECT] = std::bind(&co2meter::processWiFiState, this);
  statemachine[SENSE] = std::bind(&co2meter::processSenseState, this);
  statemachine[PUBLISH] = std::bind(&co2meter::processPublishState, this);
  statemachine[LOGGING] = std::bind(&co2meter::processLoggingState, this);
  statemachine[CALIBRATE] = std::bind(&co2meter::processCalibrateState, this);
}

void co2meter::setValues(int c, int t, int h)
{
  co2value = c;
#ifdef USETEMPOFFSET
  temp = t + atoi(conf.getTempOffset().c_str());
#else
  temp = t;
#endif
  rH = h;
}

void co2meter::getValues(int &c, int &t, int &h)
{
  c = co2value;
  t = temp;
  h = rH;
}

String co2meter::getSensorSN()
{
  char sensorSN[20];

  switch (sens)
  {
  case SCD:
    scd.getSerial(sensorSN);
    return String(sensorSN);
    break;
  }
  return String("0xdeadbeef");
}

void co2meter::begin()
{
  for (unsigned int i = 0; i < MAX_TIMER; i++)
  {
    timer[i] = 0;
    threshold[i] = 0;
  }
  threshold[INIT] = 1000;
  threshold[INIT_WAIT] = 10000;
  threshold[ACCESSPOINT] = 500;
  threshold[ACCESSPOINT_WAIT] = 45000;
  threshold[WIFI_CONNECT] = 1000;
  threshold[WIFI_CONNECT_WAIT] = 10000;
  threshold[MQTT_CONNECT] = 1000;
  threshold[MQTT_CONNECT_WAIT] = 10000;
  threshold[SENSE] = 2500;
  threshold[PUBLISH] = 2500;
  threshold[LOGGING] = 10000;
  threshold[CALIBRATE] = 500;
  threshold[TIME] = 1000;
  state = INIT;
  useWIFI = false;
  useMQTT = false;
  useFHMQTT = false;
  useHTTP = false;
  atStartup = true;

  FWMajor = MAJORVERSION;
  FWMinor = MINORVERSION;
  FWRevision = REVISION;

  for (unsigned int i = 0; i < GRAPHCOLS; i++)
  {
    graph[i] = 300;
    /* for testing */
    //graph[i]=400 + 100*i;
  }
  graph_current = 0;
  msgCnt = 0;
}

int co2meter::getState()
{
  return state;
}

bool co2meter::isAtStartup()
{
  return atStartup;
}

void co2meter::setAtStartup(bool b)
{
  atStartup = b;
}

bool co2meter::isMQTTMode()
{
  return useMQTT;
}

bool co2meter::isFHMQTTMode()
{
  return useFHMQTT;
}

bool co2meter::isHTTPMode()
{
  return useHTTP;
}

bool co2meter::wasHTTPMode()
{
  return wasHTTP;
}

bool co2meter::isWIFIMode()
{
  return useWIFI;
}

void co2meter::setMQTTMode()
{
  useMQTT = true;
  useFHMQTT = false;
  useHTTP = false;
  useWIFI = false;
  atStartup = false;
}

void co2meter::setFHMQTTMode()
{
  useMQTT = false;
  useFHMQTT = true;
  useHTTP = false;
  useWIFI = false;
  atStartup = false;
}

void co2meter::setHTTPMode()
{
  useMQTT = false;
  useFHMQTT = false;
  useHTTP = true;
  useWIFI = false;
  wasHTTP = true;
  atStartup = false;
}

void co2meter::setWIFIMode()
{
  useMQTT = false;
  useFHMQTT = false;
  useHTTP = false;
  useWIFI = true;
  atStartup = false;
}

bool co2meter::checkTime(unsigned int TIMER)
{
  /* update timer */
  if (timer[TIMER] < threshold[TIMER])
  {
    return false;
  }
  return true;
}

void co2meter::resetTimer(unsigned long t)
{
  timer[t] = 0;
}

void co2meter::getMQTTCredentials(String &broker, String &port, String &user, String &passwd)
{
  if (isMQTTMode())
  {
    broker = conf.getMQTTBrokerIP();
    port = conf.getMQTTPort();
    user = conf.getMQTTUser();
    passwd = conf.getMQTTPasswd();
    setMQTTMode();
  }
  else if (isFHMQTTMode())
  {
    broker = "149.201.87.27";
    port = "1883";
    user = "co2meter";
    passwd = "qi8fWQ747uK2SWlsz7bd";
    setFHMQTTMode();
  }
}

bool co2meter::MQTTReconnect()
{

  static bool once = true;

  // String mqttbroker=conf.getMQTTBrokerIP();
  // String mqttport=conf.getMQTTPort();
  // String mqttuser=conf.getMQTTUser();
  // String mqttpasswd=conf.getMQTTPasswd();
  String mqttbroker, mqttport, mqttuser, mqttpasswd;

  getMQTTCredentials(mqttbroker, mqttport, mqttuser, mqttpasswd);
  logmsg(LOGMQTT, "Trying to connect to MQTTBroker %s/%s\n", mqttbroker.c_str(), mqttport.c_str());

  /* connect MQTT Broker to WiFi client */
  if (!once)
  {
    if (client.connected())
    {
      logmsg(LOGMQTT, "Still connected");
      return true;
    }
    else
    {
      logmsg(LOGMQTT, "Disconnecting client");
      client.disconnect();
    }
  }
  else
  {
    logmsg(LOGMQTT, "MQTT SET CLIENT\n");
    //    client.setClient(secEspClient);
    client.setClient(espClient);
    once = false;
  }

  // convert the config string to an IP address
  // the mqttBrokerIP must be a string that has a lifetime equal to the PubSubClient
  // as the setting of the adress is by c-pointer. Hence a local conversion will fail
  int success = WiFi.hostByName(mqttbroker.c_str(), mqttBrokerAddr);
  mqttBrokerIP = mqttBrokerAddr.toString();
  client.setServer(mqttBrokerIP.c_str(), atoi(mqttport.c_str()));

  if (!client.connected())
  {

    oled.setMode("M", true);

    logmsg(LOGMQTT, "MQTT Reconnecting...");
    Serial.printf("MQTTBroker %s:%s/%s/%s/%s\n", conf.getMQTTPrefix().c_str(), mqttbroker.c_str(), mqttport.c_str(), mqttuser.c_str(), mqttpasswd.c_str());

    bool connected;
    if (mqttuser != "" && mqttpasswd != "")
    {
      connected = client.connect(getSensorSN().c_str(), mqttuser.c_str(), mqttpasswd.c_str());
    }
    else if (mqttuser == "" && mqttpasswd != "")
    {
      connected = client.connect(getSensorSN().c_str(), NULL, mqttpasswd.c_str());
    }
    else
    {
      connected = client.connect(getSensorSN().c_str());
    }

    // Possible values for client.state()
    // #define MQTT_CONNECTION_TIMEOUT     -4
    // #define MQTT_CONNECTION_LOST        -3
    // #define MQTT_CONNECT_FAILED         -2
    // #define MQTT_DISCONNECTED           -1
    // #define MQTT_CONNECTED               0
    // #define MQTT_CONNECT_BAD_PROTOCOL    1
    // #define MQTT_CONNECT_BAD_CLIENT_ID   2
    // #define MQTT_CONNECT_UNAVAILABLE     3
    // #define MQTT_CONNECT_BAD_CREDENTIALS 4
    // #define MQTT_CONNECT_UNAUTHORIZED    5
    if (!connected)
    {
      logmsg(LOGMQTT, "MQTT reconnect failed, rc= %i\n", (int)client.state());
      delay(1000);
      return false;
    }
  }
  return true;
}

bool co2meter::WifiConnect()
{

  static bool once = true;
  String SSID = conf.getSSID();
  String passwd = conf.getPasswd();

  webserverStop();

  /* if we are already connected to WiFi, we are good
  ** and can go on connecting to MQTT                 */

  if (!once)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      webserverStart();
      return true;
    }
  }
  /* otherwise, we need to connect to the WiFI */
  else
  {
    logmsg(LOGWIFI, "Connecting to %s", SSID.c_str());

    WiFi.begin(SSID, passwd);
    once = false;
  }
  //  oled.showConnectingMQTT(mqttbroker);

  if (WiFi.status() != WL_CONNECTED)
  {
    int trials = 0;
    while (trials < 20)
    {
      delay(500);
      dbgmsg(".");
      oled.setFooter(SSID);
      oled.setMode("W", true);
      oled.showConnectingWifi(SSID);
      trials++;
      dbgmsg("WIFI status: %i\n", WiFi.status());
      if (WiFi.status() == WL_CONNECTED)
      {
        webserverStart();
        return true;
      }
    }
  }
  if (WiFi.status() != WL_CONNECTED)
    return false;
  webserverStart();
  return true;
}

void printHeapAndStack()
{
#ifdef DEBUG
  LockGuard<Mutex> guard(serialMux);
  Serial.printf("Free Heap = %d\n", ESP.getFreeHeap());
  Serial.printf("MinFreeHeap = %d\n", ESP.getMaxFreeBlockSize());
  Serial.printf("Free Stack = %d\n", ESP.getFreeContStack());
#endif
}

bool co2meter::configChanged()
{

  if (isAtStartup())
    return false;

  if (conf.getSaveNExit())
  {
    conf.setSaveNExit(false);

    resetTimer(SENSE);

    //    ESP.restart();

    //stopHandleRequest();
    logmsg(LOGWEB, "Need to Reconnect WIFI Mode: %i\n", conf.getNeedsReconnect());

    if (conf.getNeedsReconnect())
    {
      conf.setNeedsReconnect(false);

      webserverStop();
      delay(500);
      // MDNS.begin(conf.getMDNSName().c_str());

      // disconnect from previous Wifi connection
      if (wasHTTPMode())
      {
        if (!WiFi.softAPdisconnect(true))
          logmsg(LOGWIFI, "Failed to disconnect AP\n");
        delay(1000);
      }

      if (WiFi.status() == WL_CONNECTED)
      {
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        WiFi.forceSleepBegin();
        delay(100);
      }

      // reconnect with new wifi setting
      logmsg(LOGWIFI, "DEVICEMODE: %s\n", conf.getDevicemode().c_str());
      if (conf.getDevicemode() == "WIFI")
      {
        LockGuard<Mutex> guard(wsMux);
        WiFi.begin(conf.getSSID(), conf.getPasswd());
        WiFi.mode(WIFI_STA);
        // if (WifiConnect()) {
        // 	delay(1000);
        // }

        setWIFIMode();

        resetTimer(WIFI_CONNECT);
        state = WIFI_CONNECT;
        return true;
      }
      else if (conf.getDevicemode() == "MQTT")
      {
        LockGuard<Mutex> guard(wsMux);
        WiFi.begin(conf.getSSID(), conf.getPasswd());
        WiFi.mode(WIFI_STA);
        // if (WifiConnect()) {
        // 	delay(1000);
        // 	startHandleRequest();
        // 	delay(1000);
        // }
        setMQTTMode();

        resetTimer(WIFI_CONNECT);
        state = WIFI_CONNECT;
        return true;
      }
      else if (conf.getDevicemode() == "FHMQTT")
      {
        LockGuard<Mutex> guard(wsMux);

        WiFi.begin(conf.getSSID(), conf.getPasswd());
        WiFi.mode(WIFI_STA);
        // if (WifiConnect()) {
        // 	delay(1000);
        // 	startHandleRequest();
        // 	delay(1000);
        // }
        // setMQTTMode();
        setFHMQTTMode();
        resetTimer(WIFI_CONNECT);
        state = WIFI_CONNECT;
        return true;
      }
      else if (conf.getDevicemode() == "LOCALAP")
      {
        LockGuard<Mutex> guard(wsMux);

        if (WiFi.status() == WL_CONNECTED)
        {
          WiFi.disconnect();
          delay(1000);
        }

        ap.startAP();
        delay(2000);

        webserverStart();
        delay(1000);
        state = SENSE;
        setHTTPMode();
        oled.clear();
      }
      else
      {
        state = SENSE;
        oled.clear();
      }
    } /* conf.getNeedsReconnect() */
    return true;
  } /* conf.getSaveNExit() */
  return false;
}

void co2meter::updateRTCValues()
{
  //  printHeapAndStack();
  if (checkTime(TIME))
  {
#ifdef RTC
    if (myclock.getTime() == "25:165:165")
    {
      conf.setShowRTC(false);
    }
    else
    {
      conf.setShowRTC(true);
    }

    if (conf.getDisplayMode() == "D1")
    {
      conf.setShowAnalog(false);
    }
    else if (conf.getDisplayMode() == "D2")
    {
      conf.setShowAnalog(true);
    }

    if (conf.getShowRTC())
    {
      logmsg(LOGSM, "Clock: %s", myclock.getTime().c_str());
      dbgmsg("Mode(conf-start- ap - mqtt-fhmqt-wifi): %i-%i-%i-%i-%i-%i\n", conf.getInConfig(), m.isAtStartup(), m.isHTTPMode(), m.isMQTTMode(), m.isFHMQTTMode(), m.isWIFIMode());
      dbgmsg("DISPLAYMODE %s %i\n", conf.getDisplayMode().c_str(), conf.getShowAnalog());
      printHeapAndStack();

      myclock.checkTime();

      if (conf.getShowAnalog() && (state == SENSE || state == PUBLISH || state == LOGGING))
      {
        oled.showAnalogTime(myclock.getHours(), myclock.getMinutes(), myclock.getSeconds());
      }
    }
    else
    {
      logmsg(LOGSM, "CLOCK N/A");
    }

#endif
    resetTimer(TIME);
  }
}

void co2meter::processInitState()
{
  static bool once = true;
  atStartup = true;

  //logmsg("%i: INIT state  prev: %i diff: %i\r", currentTime, previousTime, currentTime-previousTime);
  if (checkTime(state))
  {

    logmsg(LOGSM, "In state init for %i more seconds", (unsigned long)(threshold[INIT_WAIT] - timer[INIT_WAIT]) / 1000);

    oled.showWelcome(FWMajor, FWMinor, FWRevision);
    //u8g2.writeBufferXBM(Serial);
    //

    // Try to find connected sensors
    if (sens == NONE)
    {
      if (scd.init())
      {
        sens = SCD;
        logmsg(LOGSENSOR, "SCD 30 connected");
      }
      else if (mhz19.getStatus() >= 0)
      {
        sens = MHZ;
        logmsg(LOGSENSOR, "MHZ19 connected");
      }
    }

    // store sensor type in config object
    conf.setSensorType(getSensorType());

    ryg.init();

    //starting logger instances
    if (once)
    {
      sensorlogrecord = (logrecord *)malloc(20 * sizeof(logrecord));
      sensorlog.begin(sensorlogrecord, 20);
      chartlogrecord = (logrecord *)malloc(120 * sizeof(logrecord));
      chartlog.begin(chartlogrecord, 120);
      once = false;
    }

    //    oled.showAnalogTime(myclock.getHours(),myclock.getMinutes(), myclock.getSeconds());

    resetTimer(state);
  }

  if (timer[INIT_WAIT] >= threshold[INIT_WAIT])
  {
    resetTimer(INIT_WAIT);

#ifdef NO_WIFI
    resetTimer(SENSE);
    state = SENSE;
#else
    resetTimer(ACCESSPOINT_WAIT);
    state = ACCESSPOINT;
#endif
  }
}

void co2meter::processAPState()
{
  static bool inactive = true;
  static bool setup = true;
  static bool loop = false;
  static bool connected = false;

  if (checkTime(state))
  {
    if (inactive)
    {
      logmsg(LOGSM, "\n\n----------------------\nIn state ACCESSPOINT for %i more seconds", (unsigned long)(threshold[ACCESSPOINT_WAIT] - timer[ACCESSPOINT_WAIT]) / 1000);
    }
    if (setup)
    {
      logmsg(LOGWIFI, "Starting AP ...");
      ap.startAP();
      webserverRegisterCallbacks();
      //      startHandleRequest();

      setup = false;
      loop = true;
      startqr("WIFI:T:WPA;S:" + String(ap.getSSID()) + ";" + "P:" + String(ap.getPasswd()) + ";");
    }

    if (loop)
    {

      /* if there is a calibration request, change the state and leave */
      if (conf.getDoCalibration())
      {
        resetTimer(state);
        resetTimer(CALIBRATE);
        state = CALIBRATE;
        return;
      }

      if (WiFi.softAPgetStationNum() > 0)
      {
        //	  static int counter=0;
        inactive = false;
        connected = true;
        logmsg(LOGWIFI, "Connected Devices: %d SaveNExit %i AtStartup %i\n", WiFi.softAPgetStationNum(), conf.getSaveNExit(), m.isAtStartup());
        oled.setHeader("SensorID: " + String(conf.getSensorID()));
        oled.setFooter(String(ap.getSSID()));
        oled.setMode("A", true);
        //	  oled.show();
        oled.showConfig();
      } /* (WiFi.softAPgetStationNum()>0) */
      else
      {
        u8g2.setDrawColor(0);
        //	  u8g2.DrawBox(90,0, 127, 63);
        u8g2.setFont(u8g2_font_baby_tf);
        u8g2.drawStr(62, 20, ap.getSSID());
        u8g2.setFont(u8g2_font_fub14_tf);
        u8g2.setDrawColor(1);
        u8g2.setFontMode(1);
        u8g2.drawBox(80, 35, 25, 20);
        u8g2.setDrawColor(0);
        u8g2.setCursor(80, 50);
        u8g2.print((unsigned long)(threshold[ACCESSPOINT_WAIT] - timer[ACCESSPOINT_WAIT]) / 1000);

        u8g2.setFontMode(0);
        u8g2.setDrawColor(1);
        u8g2.sendBuffer();
        // /* make screenshot */
        // //u8g2.writeBufferXBM(Serial);

        u8g2.setDrawColor(1);
        inactive = true;
      } /* (WiFi.softAPgetStationNum()>0) */
      AP_loop();
    } /* loop */
    resetTimer(state);
  }

  /* in case no input was given within the max time span for this state
     * the sense state is invoked */
  if ((timer[ACCESSPOINT_WAIT] > threshold[ACCESSPOINT_WAIT] && inactive) || conf.getSaveNExit())
  {
    resetTimer(state);
    resetTimer(ACCESSPOINT_WAIT);

    oled.clear();

    if (conf.getDisplayMode() == "D1")
    {
      conf.setShowAnalog(false);
    }
    else if (conf.getDisplayMode() == "D2")
    {
      conf.setShowAnalog(true);
    }

    setAtStartup(false);
    wasHTTP = true;
    setHTTPMode();
    conf.setSaveNExit(true);
    conf.setNeedsReconnect(true);
    configChanged();

    /*
    
    if (conf.getDevicemode()=="WIFI") {
      logmsg(LOGSM, "Switching to WIFI mode");
      WiFi.softAPdisconnect(true);
      delay(500);
      setAtStartup(false);
      state = WIFI_CONNECT;
      resetTimer(WIFI_CONNECT);
      setWIFIMode();
      webserverStop();
      logmsg(LOGSM, "Switching to WIFI mode - done");      
      return;
    } 
    else if (conf.getDevicemode()=="MQTT" ){
      logmsg(LOGSM, "Switching to MQTT mode");
      setAtStartup(false);
      state =  WIFI_CONNECT;
      resetTimer(WIFI_CONNECT);
      setMQTTMode();
      webserverStop();
      //      WiFi.softAPdisconnect(true);
      //delay(500);
      //      return;
    }
    else if (conf.getDevicemode()=="FHMQTT" ){
      logmsg(LOGSM, "Switching to FHMQTT mode");
      setAtStartup(false);
      state =  WIFI_CONNECT;
      resetTimer(WIFI_CONNECT);
      setFHMQTTMode();
      webserverStop();
      //WiFi.softAPdisconnect(true);
      //delay(500);
      //      return;
    }
    else if (conf.getDevicemode() == "LOCALAP") {
      logmsg(LOGSM, "Switching to AP mode");
      setAtStartup(false);
      setHTTPMode();
      state = SENSE;
      oled.clear();
      //      return;
    }
    else {
      state = SENSE;
      oled.clear();
    }
    */
  }
}

void co2meter::processMQTTState()
{
  static bool setup = true;
  static bool loop = false;
  static bool connected = false;

  if (checkTime(state))
  {
    logmsg(LOGSM, "\n\n----------------------\nIn state MQTT_CONNECT\n----------------------\n");
    //      String mqttbroker=conf.getMQTTBrokerIP();
    //String mqttport=conf.getMQTTPort();
    String mqttbroker, mqttport, mqttuser, mqttpasswd;
    String SSID = conf.getSSID();
    String passwd = conf.getPasswd();

    // respective MQTTMode (setFHMQTT()/setMQTT()) is set in getMQTTCredential function
    getMQTTCredentials(mqttbroker, mqttport, mqttuser, mqttpasswd);

    if (setup)
    {
      /* connect to WiFi */
      LockGuard<Mutex> guard(wsMux);
      WifiConnect();

      delay(3000);

      logmsg(LOGWIFI, "WiFi connected!\n IPaddress: %s", WiFi.localIP().toString().c_str());

      WiFi.setAutoReconnect(true);

      /* connect to MQTT Broker */

      if (mqttbroker == "")
      {
        logmsg(LOGWIFI, "NO MQTT BROKER IP set, restarting");
        oled.showError("No MQTT Broker IP set! Restarting ...");
        delay(10000);
        ESP.restart();
      }

      if (MQTTReconnect())
      {
        delay(2000);
        connected = true;
      }

      setup = false;
      loop = true;
      //        return;
    }

    if (loop)
    {

      oled.setHeader("SensorID: " + String(conf.getSensorID()));
      oled.setFooter(WiFi.localIP().toString());

      if (WiFi.status() != WL_CONNECTED)
      {
        dbgmsg("Reconnecting in Loop");
        LockGuard<Mutex> guard(wsMux);
        WifiConnect();
      }

      if (client.state() != MQTT_CONNECTED || !client.connected())
      {

        oled.showConnectingMQTT(mqttbroker);
        MQTTReconnect();
      }
    }

    if (connected)
      webserverStart();

    if (connected || timer[MQTT_CONNECT_WAIT] > threshold[MQTT_CONNECT_WAIT])
    {
      resetTimer(state);
      resetTimer(MQTT_CONNECT_WAIT);
      resetTimer(SENSE);
      state = SENSE;
      delay(1000);
    }
  }
}

void co2meter::processWiFiState()
{
  static bool setup = true;
  static bool loop = false;
  static bool connected = false;

  if (checkTime(state))
  {
    logmsg(LOGSM, "\n\n----------------------\nIn state WIFI_CONNECT\n----------------------\n");

    if (conf.getSSID() == "")
    {
      oled.showError("No SSID set! Restarting...");
      delay(10000);
      ESP.restart();
    }

    if (setup)
    {

      LockGuard<Mutex> guard(wsMux);

      /* connect to WiFi */
      connected = WifiConnect();

      //	delay(1000);

      dbgmsg("WiFi connected!: %i\n", connected);
      dbgmsg("IP address: ");
      dbgmsg(WiFi.localIP().toString().c_str());
      //if (WiFi.localIP()==NULL) return;

      WiFi.setAutoReconnect(true);

      // oled.setMode("M", true);
      // oled.setFooter(String(ap.getSSID()));
      // oled.showConnectingMQTT(mqttbroker);

      setup = false;
      loop = true;
      delay(1000);
      //return;
    }

    if (loop)
    {

      oled.setHeader("SensorID: " + String(conf.getSensorID()));
      oled.setFooter(WiFi.localIP().toString());

      if (WiFi.status() != WL_CONNECTED)
      {
        LockGuard<Mutex> guard(wsMux);

        dbgmsg("Reconnecting in Loop");
        connected = WifiConnect();
        WiFi.setAutoReconnect(true);
      }
    }

    if (connected)
    {
      delay(1000);
      webserverStart();
    }

    if (connected || timer[WIFI_CONNECT_WAIT] > threshold[WIFI_CONNECT_WAIT])
    {
      resetTimer(state);
      resetTimer(WIFI_CONNECT_WAIT);

      if (isWIFIMode())
      {
        resetTimer(SENSE);
        state = SENSE;
      }
      else if (isMQTTMode() || isFHMQTTMode())
      {
        resetTimer(MQTT_CONNECT);
        state = MQTT_CONNECT;
      }
      delay(1000);
    }
  }
}

void co2meter::processSenseState()
{
  // leave state immediately, if a calibration is requested. Sensed values are not
  // correct until calibration is done
  if (conf.getDoCalibration())
  {
    resetTimer(state);
    resetTimer(CALIBRATE);
    state = CALIBRATE;
    return;
  }
  if (checkTime(state))
  {
    logmsg(LOGSM, "\n\n----------------------\nIn state SENSE\n----------------------\n");

    // check whether the config page has been updated
    if (configChanged())
      return;

    /* set the co2 limits to disply and RYG LEDs to visualise traffic lights */
    oled.setLimits(conf.getLEDgreen().toInt(), conf.getLEDyellow().toInt(), conf.getLEDred().toInt());
    ryg.setLimits(conf.getLEDgreen().toInt(), conf.getLEDyellow().toInt(), conf.getLEDred().toInt());

    String cid = String(conf.getClassid());
    if (!conf.getShowAnalog())
    {
      if (cid != "")
      {
        oled.setSubheader("ClassID: " + cid);
      }
    }

    switch (getSensorType())
    {
    case NONE:
      co2 = -1;
      temp = -1;
      rH = -1;
      break;
    case MHZ:
      mhz19.getMeasurement(co2, temp);
      rH = -1;
      break;
    case SCD:
      scd.getMeasurement(co2, temp, rH);
      break;
    case S8:
      break;
    }

    /* log value, comment ount or set to 1 for non quantized values */
    sensorlog.setQuantization(10);
    sensorlog.log(co2);

#ifdef USEAVERAGE
    int co2av = co2; //sensorlog.getMinuteAverage();
    if (co2av != 0 && co2 != -1)
    {
      co2 = co2av;
    }
#endif

    setValues(co2, temp, rH);

    if (conf.getDisplayOn() == "1")
    {
      /* Set RYG LED's */
      ryg.setLights(co2);
    }
    logmsg(LOGSENSOR, "New Sensor Reading: CO2: %i T: %i H:%i\n", co2, temp, rH);

    if (conf.getDisplayOn() == "1")
    {
      if (!conf.getShowAnalog())
      {
        oled.setHeader("SensorID: " + String(conf.getSensorID()));
        if (isMQTTMode() || isFHMQTTMode() || isWIFIMode())
        {
          oled.setFooter(WiFi.localIP().toString());
        }
        else if (isHTTPMode())
        {
          oled.setFooter("AP: " + String(ap.getAPName()));
          //	  oled.setFooter(ap.getAPIP());
        }
      }
    }
    else
    {
      oled.clear();
      oled.showHeader();
      oled.showFooter();
      oled.setMode("S", false);
      oled.showMode();
      u8g2.sendBuffer();
    }

    if (conf.getDisplayOn() == "1")
    {
      if (!conf.getShowAnalog())
      {
        oled.showSense(co2, temp, rH);
      }
      else
      {
#ifdef RTC
        oled.showAnalogTime(myclock.getHours(), myclock.getMinutes(), myclock.getSeconds());
#endif
        if (isHTTPMode())
        {
          oled.showSenseRightHalf(co2, temp, rH, String(F("192.168.0.1")), conf.getMDNSName() + ".local");
          //	  oled.setHeader("SensorID: " + String(conf.getSensorID()));
        }
        else
        {
          oled.showSenseRightHalf(co2, temp, rH, WiFi.localIP().toString().c_str(), conf.getMDNSName() + ".local");
        }
      }
    }

    /* change to publish state */
    resetTimer(state);

#ifdef NO_WIFI
    resetTimer(LOGGING);
    state = LOGGING;
#else
    resetTimer(PUBLISH);
    state = PUBLISH;
#endif
  }
}

void co2meter::processPublishState()
{
  static bool startMDNS = true;
  static int transmissionError = 0;
  static int counter = 0;
  if (checkTime(state))
  {
    logmsg(LOGSM, "\n\n----------------------\nIn state PUBLISH\n----------------------\n");
    if (!startMDNS)
    {
      bool success = MDNS.update();
      logmsg(LOGSM, "MDNSUpdate %i", success);
      oled.setSubheader("mDNS: " + String(conf.getMDNSName()) + ".local");
    }

#ifdef RTC
    if (conf.getShowRTC())
    {
      if (!conf.getShowAnalog())
      {
        if ((counter++) % 2 == 0)
          oled.setFooter(myclock.getTime());
        else
          oled.setFooter(myclock.getDate());
      }
      else
      {
        oled.showAnalogTime(myclock.getHours(), myclock.getMinutes(), myclock.getSeconds());
      }
    }
#endif

    if (isWIFIMode())
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        logmsg(LOGSM, "PUBLISH: WiFi Status is %d", WiFi.status());
        resetTimer(state);
        resetTimer(SENSE);
        resetTimer(WIFI_CONNECT);
        state = WIFI_CONNECT;
        return;
      }
    }
    else if (isMQTTMode())
    {
      String mqttprefix = conf.getMQTTPrefix();
      String mqttco2 = mqttprefix + "c";
      String mqtttemp = mqttprefix + "t";
      String mqttschool = mqttprefix + "s";
      String mqttclassid = mqttprefix + "k";
      if (client.state() == MQTT_CONNECTED)
      {
        dbgmsg("Submitting: %s %s\n%s %s\n%s %s\n%s %s\n", mqttco2.c_str(), String(co2).c_str(), mqtttemp.c_str(), String(temp).c_str(), mqttschool.c_str(), conf.getSchool().c_str(), mqttclassid.c_str(), conf.getClassid().c_str());
        if (!client.publish(mqttco2.c_str(), String(co2).c_str()) ||
            !client.publish(mqtttemp.c_str(), String(temp).c_str()) ||
            !client.publish(mqttschool.c_str(), conf.getSchool().c_str()) ||
            !client.publish(mqttclassid.c_str(), conf.getClassid().c_str()))
        {
          oled.clear();
          oled.setHeader("SensorID: " + String(conf.getSensorID()));
          oled.setFooter("MQTT transmission error!");
          transmissionError++;
        }
        transmissionError = 0;
      }
      else
      {
        transmissionError++;
      }
    } /* useMQTTT */
    else if (useFHMQTT)
    {
      //  build the mqtt message
      String topic = conf.getMQTTPrefix();
      String value("");
      value += "m:";
      value += String(msgCnt);
      value += "\ts:";
      value += conf.getSensorID();
      value += "_";
      value += getSensorSN();
      value += "\tv:";
      value += String(MAJORVERSION) + "." + String(MINORVERSION) + "." + String(REVISION);
      value += "\tc:";
      value += String(co2value);
      value += "\tt:";
      value += String(temp);
      value += "\tr:";
      value += String(rH);
      value += "\n";

      if (client.state() == MQTT_CONNECTED)
      {
        logmsg(LOGMQTT, "MQTT_TOPIC:%s\tMQTT_PUB: %s", topic.c_str(), value.c_str());
        logmsg(LOGMQTT, "(Total: %d Bytes)", topic.length() + value.length());
        if (!client.publish(topic.c_str(), value.c_str()))
        {
          oled.clear();
          oled.setHeader("SensorID: " + String(conf.getSensorID()));
          oled.setFooter("MQTT transmission error!");
          transmissionError++;
        }
        transmissionError = 0;
        msgCnt++;
        yield();
      }
      else
      {
        transmissionError++;
      }

      /* 3 messages  timeout */
      if (transmissionError > 2)
      {
        resetTimer(state);
        resetTimer(SENSE);
        resetTimer(MQTT_CONNECT);
        state = MQTT_CONNECT;
        return;
      }
    } /* useFHMQTTT */
    else if (isHTTPMode())
    {
      dbgmsg("Update webpage");
    }

    if (startMDNS)
    {
      dbgmsg("IN startMDSN");

      String mdns = conf.getMDNSName();
      if (mdns != "")
      {
        ap.setMDNSName(mdns);
        logmsg(LOGWIFI, "Setup MDSN: %s\n", WiFi.localIP().toString().c_str());
      }

      // if we changed from ap-mode to sth else the MDNS should be running
      // otherwise we start it
      if (!MDNS.isRunning())
      { //, WiFi.localIP()) {
        MDNS.begin(ap.getMDNSName());
        logmsg(LOGWIFI, "MDNS not running, restarting %s!", ap.getMDNSName());
        oled.setSubheader(ap.getMDNSName());
      }
      if (MDNS.isRunning())
      {
        logmsg(LOGWIFI, "mDNS responder started");
        MDNS.addService("http", "tcp", 80);

        if (!conf.getShowAnalog())
        {
          oled.setSubheader("mDNS: " + String(ap.getMDNSName()) + ".local");
        }
        startMDNS = false;
      }
    }

    /* read sensor results and change state to publish */
    if (!conf.getShowAnalog())
    {
      oled.showPublish();
    }
    else
    {
#ifdef RTC
      oled.showAnalogTime(myclock.getHours(), myclock.getMinutes(), myclock.getSeconds());
#endif
      oled.showPublishRightHalf();
    }

    /* change to sense state */
    resetTimer(state);
    //      resetTimer(LOGGING);
    state = LOGGING;
  }
}

void co2meter::processLoggingState()
{
  if (checkTime(state))
  {
    logmsg(LOGSM, "\n----------------------\nIn state LOGGING\n----------------------\n");
    oled.setMode("L", true);
    static int cycles = 0;
    if (conf.getDisplayOn() == "1")
    {
      if (!conf.getShowAnalog())
      {
        oled.showGraph(graph_current, graph);
      }
      else
      {
#ifdef RTC
        oled.showAnalogTime(myclock.getHours(), myclock.getMinutes(), myclock.getSeconds());
#endif
        oled.setMode("L", false);
        oled.showMode();
      }
    }

    dbgmsg("current graph index: %i", graph_current);
    //int av = 500 + cycles*100;//l.getMinuteAverage();
    int av = co2; //l.getMinuteAverage();
    //	Serial.printf("LOG: Average values over last 1 minute: %i\n", av);
    if (av > 0)
    {
      graph[graph_current] = av;
      graph_current++;
      if (graph_current >= GRAPHCOLS)
        graph_current = 0;
    }

    // 2nd logger for website
    if (cycles % 3 == 1)
    {

      av = sensorlog.getMinuteAverage();
      //Serial.printf("LOG: Average values over last 1 minutes: %i\n", av);
      if (av == 0)
        av = co2;
      chartlog.log(av);
#ifdef RTC
      if (conf.getShowRTC())
      {
        chartlog.stamp(myclock.getTime());
      }
#endif
    }

    if ((cycles % 20) == 0)
    {
      oled.reset();
      dbgmsg("resetting display");
      logmsg(LOGSM, "Resetting display");
    }

    cycles++;

    resetTimer(state);
    resetTimer(LOGGING);
    resetTimer(SENSE);
    state = SENSE;
  }
  else
  {
    state = SENSE;
    resetTimer(SENSE);
  }
}

void co2meter::processCalibrateState()
{
  logmsg(LOGSM, "\n\n----------------------\nIn state CALIBRATE\n----------------------\n");
  oled.setMode("C", false);
  oled.showCalibrate();
  delay(3000);

  uint16_t cv = (uint16_t)conf.getDefaultCV().toInt();

  switch (getSensorType())
  {
  case NONE:
    break;
  case MHZ:
    logmsg(LOGSENSOR, "CALIBRATING MHZ TO 400 PPM");
    mhz19.calibrateZero();
    break;
  case SCD:
    logmsg(LOGSENSOR, "CALIBRATING SCD TO %i ppm\n", cv);
    scd.calibrate(cv);
    break;
  case S8:
    break;
  }

  oled.showCalibrateDone();

  delay(10000);
  ESP.restart();
}

void co2meter::control()
{

  server.handleClient();
  /// update real-time clock
  updateRTCValues();

  /// update MDNS service
  MDNS.update();

  // Call the function of the respective state. State transistions are
  // handeled implicitely in the state functions. Hence, there is no
  // transition matrix required, here.
  statemachine[state]();
}

void co2meter::now(unsigned long t)
{
  previousTime = currentTime;
  currentTime = t;

  /* update timer intervals */
  for (unsigned int i = 0; i < MAX_TIMER; i++)
  {
    timer[i] += (currentTime - previousTime);
  }
}

int co2meter::getFWMajor()
{
  return FWMajor;
}

int co2meter::getFWMinor()
{
  return FWMinor;
}

int co2meter::getFWMRevision()
{
  return FWRevision;
}

int co2meter::getSensorType()
{
  return sens;
}
