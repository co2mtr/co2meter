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
#include "apmode.h"
#include "accesspoint.h"
#include "co2meter.h"
#include "sensor_MHZ19B.h"
#include "ArduinoJson.h"
#include "clock.h"
#include "logging.h"
#include "globalLocks.h"
#include "config.h"

#define SEND server.send
#define HOST server.hostHeader()
#define URL server.uri()

#include "ESPTemplateProcessor.h"
#define SENDDENIAL                                                                                                                                                                                                                                                                                                                   \
  {                                                                                                                                                                                                                                                                                                                                  \
    SEND(429, "text/html", "<!DOCTYPE html><html><head><link rel=\"icon\" href=\"data:,\"><meta http-equiv=\"refresh\" content=\"10; url=/\"></head><body onload=\"myFunction()\"><script>function myFunction(){alert(\"Anfrage kann derzeit nicht bearbeitet werden, bitte versuchen Sie es spaeter!\");}</script></body></html>"); \
    logmsg(LOGWEB, " REQUEST DENIED (Mem: %.2fk): http://%s%s\n", (float)ESP.getFreeHeap() / 1024., HOST.c_str(), URL.c_str());                                                                                                                                                                                                      \
    return;                                                                                                                                                                                                                                                                                                                          \
  }

#define SENDINCONFIG                                                                                                                                                                                                                                                                                           \
  {                                                                                                                                                                                                                                                                                                            \
    SEND(429, "text/html", "<!DOCTYPE html><html><head><link rel=\"icon\" href=\"data:,\"><meta http-equiv=\"refresh\" content=\"10; url=/\"></head><body onload=\"myFunction()\"><script>function myFunction(){alert(\"Im Konfigurationsmodus, bitte versuchen Sie es spaeter!\");}</script></body></html>"); \
    logmsg(LOGWEB, " REQUEST DENIED (Mem: %.2fk): http://%s%s\n", (float)ESP.getFreeHeap() / 1024., HOST.c_str(), URL.c_str());                                                                                                                                                                                \
    return;                                                                                                                                                                                                                                                                                                    \
  }

/// stringlen for JSON string in /getData callback
#define GETDATASTRINGLEN 160
/// browser cache max age
#define MAXAGE 30000

extern co2meter m;
extern accesspoint ap;
extern config conf;

#ifdef RTC
extern rtclock myclock;
#endif

extern logger sensorlog, chartlog; // short-term log, long-term log

#ifndef HTTPUSER
#define HTTPUSER "admin"
#endif
#ifndef HTTPPASSWD
#define HTTPASSWD "admin"
#endif

const char *http_username = HTTPUSER;
const char *http_password = HTTPPASSWD;

ESP8266WebServer server(80);

bool timeflag = false;
bool handleRequest = true;

/** check if a config option causes the wifi mode to be changed.
 * 
 *  Some params will require to reconnect the wifi, while some do not
 *  The function set a global config parameter NeedsReconnect in class
 */
template <typename T, typename S>
void checkNeedsReconnect(T val1, S val2)
{
  if (val1 != val2)
  {
    conf.setNeedsReconnect(true);
  }
}

void waitrnd()
{
}

bool canHandleRequest(const char *f)
{
  logmsg(LOGWEB, "HANDLE REQUEST from %s: %i\n\n", f, handleRequest);
  return handleRequest;
}

void startHandleRequest()
{
}

void stopHandleRequest()
{
}

bool tooLittleHeap(float scale = 1.0)
{
  if (ESP.getFreeHeap() > scale * MIN_FREE_HEAP_TO_SERVICE)
    return false;
  return true;
}

String processor(const String &var)
{

  int co2, temp, rH;
  m.getValues(co2, temp, rH);

  //  if (!canHandleRequest("processor"))
  //    return "";

  //  dbgmsg(var.c_str());
  if (var == "DEVICENAME")
  {
    return String(ap.getDevicename());
  }
  else if (var == "TIMEDATE")
  {
#ifdef RTC
    if (conf.getShowRTC())
      return myclock.getTimeDate();
    else
      return "  ";
#else
    return "";
#endif
  }
  else if (var == "TIME")
  {
#ifdef RTC
    if (conf.getShowRTC())
      return myclock.getTime();
    else
      return " ";
#else
    return "";
#endif
  }
  else if (var == "DATE")
  {
#ifdef RTC
    if (conf.getShowRTC())
      return myclock.getDate();
    else
      return " ";
#else
    return "";
#endif
  }
  else if (var == "SENSORSN")
  {
    if (m.getSensorType() == SCD)
    {
      //      Serial.printf("****%s\n\n", m.getSensorSN().c_str());
      return m.getSensorSN();
    }
    else
    {
      return (" ");
    }
  }
  else if (var == "CONFIGSCHOOL")
  {
    return conf.getSchool();
  }
  else if (var == "CONFIGCLASS")
  {
    return conf.getClassid();
  }
  else if (var == "CONFIGPUPILS")
  {
    return conf.getPupils();
  }
  else if (var == "CONFIGROOMSIZE")
  {
    return conf.getRoomsize();
  }
  else if (var == "CONFIGSSID")
  {
    return conf.getSSID();
  }
  else if (var == "CONFIGPASSWD")
  {
    return conf.getPasswd();
  }
  else if (var == "CONFIGMDNSNAME")
  {
    return conf.getMDNSName();
  }
  else if (var == "CONFIGMQTTBROKERIP")
  {
    return conf.getMQTTBrokerIP();
  }
  else if (var == "CONFIGMQTTPORT")
  {
    return conf.getMQTTPort();
  }
  else if (var == "CONFIGMQTTUSER")
  {
    return conf.getMQTTUser();
  }
  else if (var == "CONFIGMQTTPASSWD")
  {
    return conf.getMQTTPasswd();
  }
  else if (var == "CONFIGMQTTPREFIX")
  {
    return conf.getMQTTPrefix();
  }
  else if (var == "CONFIGLEDGREEN")
  {
    return conf.getLEDgreen();
  }
  else if (var == "CONFIGLEDYELLOW")
  {
    return conf.getLEDyellow();
  }
  else if (var == "CONFIGLEDRED")
  {
    return conf.getLEDred();
  }
  else if (var == "CONFIGTEMPOFFSET")
  {
    return conf.getTempOffset();
  }
  else if (var == "DATE")
  {
#ifdef RTC
    if (conf.getShowRTC())
      return myclock.getDate();
    else
      return " ";
#else
    return "";
#endif
  }
  else if (var == "TIME")
  {
#ifdef RTC
    if (conf.getShowRTC())
      return myclock.getTime();
    else
      return " ";
#else
    return "";
#endif
  }
  else if (var == "CONFIGDISPLAY")
  {
    if (conf.getDisplayOn())
      return "checked=\"checked\"";
    else
      return "";
  }
  else if (var == "CONFIGDISP1")
  {
    if (conf.getDisplayMode() == "D1")
      return "checked=\"checked\"";
    else
      return "";
  }
  else if (var == "CONFIGDISP2")
  {
    if (conf.getDisplayMode() == "D2")
      return "checked=\"checked\"";
    else
      return "";
  }
  else if (var == "CONFIGLOCALAP")
  {
    dbgmsg("IN LOCALAP MODE SELECTED\n\n");
    if (conf.getDevicemode() == "LOCALAP")
    {
      dbgmsg("IN LOCALAP MODE SELECTED\n\n");
      return "checked=\"checked\"";
    }
    else
      return "";
  }
  else if (var == "CONFIGMQTT")
  {
    if (conf.getDevicemode() == "MQTT")
    {
      dbgmsg("MQTT MODE SELECTED\n\n");
      return "checked=\"checked\"";
    }
    else
      return "";
  }
  else if (var == "CONFIGFHMQTT")
  {
    if (conf.getDevicemode() == "FHMQTT")
    {
      dbgmsg("MQTT MODE SELECTED\n\n");
      return "checked=\"checked\"";
    }
    else
      return "";
  }
  else if (var == "CONFIGWIFI")
  {
    if (conf.getDevicemode() == "WIFI")
    {
      dbgmsg("WIFI MODE SELECTED\n\n");
      return "checked=\"checked\"";
    }
    else
      return "";
  }
  else if (var == "CONFIGMHZ")
  {
    if (m.getSensorType() != SCD)
      return "disabled";
    else
      return "";
  }
  else if (var == "CO2")
  {
    return String(co2).c_str();
  }
  else if (var == "TEMPERATURE")
  {
    return String(temp).c_str();
  }
  else if (var == "RH")
  {
    if (m.getSensorType() == SCD)
    {
      return String(rH).c_str();
    }
    else
    {
      return String(F("N/A"));
    }
  }
  else if (var == "YELLOW")
  {
    return conf.getLEDyellow().c_str();
  }
  else if (var == "RED")
  {
    return conf.getLEDred().c_str();
  }
  else if (var == "CONFIGABC")
  {
    dbgmsg("GetABC %s\n", conf.getABC().c_str());
    if (conf.getABC() == "1")
      return "checked=\"checked\"";
    else
      return "";
  }
  else if (var == "CONFIGDEFAULTCV")
    return conf.getDefaultCV();

  return String("");
}

void webserverStop()
{
  //    stopHandleRequest();
}

void webserverCleanup()
{
  //  server.reset();
}

void webserverStart()
{
  //  startHandleRequest();
}

bool handleParams()
{
  int co2, temp, rH;
  m.getValues(co2, temp, rH);

  if (!canHandleRequest("handleParams") || tooLittleHeap())
    return false;

  bool displayonflag = false;

  unsigned int numberOfParams = server.args();

  dbgmsg("Number of Get params: %i\n", numberOfParams);
  for (unsigned int i = 0; i < numberOfParams; i++)
  {

#define GETARG server.argName(i)
#define GETVAL server.arg(i)

    dbgmsg("%s %s\n", GETARG.c_str(), GETVAL.c_str());

    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (GETARG == "school")
    {
      conf.setSchool(GETVAL);
    }
    else if (GETARG == "classid")
    {
      conf.setClassid(GETVAL);
    }
    else if (GETARG == "pupils")
    {
      conf.setPupils(GETVAL);
    }
    else if (GETARG == "roomsize")
    {
      conf.setRoomsize(GETVAL);
    }
    else if (GETARG == "ssid")
    {
      checkNeedsReconnect(GETVAL, conf.getSSID());
      conf.setSSID(GETVAL);
    }
    else if (GETARG == "passwd")
    {
      checkNeedsReconnect(GETVAL, conf.getPasswd());
      conf.setPasswd(GETVAL);
    }
    else if (GETARG == "mdnsname")
    {
      checkNeedsReconnect(GETVAL, conf.getMDNSName());
      if (GETVAL == "")
      {
        ap.setDevicename();
        conf.setMDNSName(ap.getMDNSName());
      }
      else
      {
        conf.setMDNSName(GETVAL);
      }
    }
    else if (GETARG == "dm1")
    {
      if (GETVAL == "DISPLAY")
      {
        conf.setDisplayOn("1");
        displayonflag = true;
      }
    }
    else if (GETARG == "display")
    {
      conf.setDisplayMode(GETVAL);
    }
    else if (GETARG == "wifimode")
    {

      if (GETVAL == "LOCALAP")
      {
        checkNeedsReconnect(m.isHTTPMode(), (GETVAL == "LOCALAP"));
        checkNeedsReconnect(m.isAtStartup(), (GETVAL == "LOCALAP"));
      }
      else if (GETVAL == "WIFI")
      {
        checkNeedsReconnect(m.isWIFIMode(), (GETVAL == "WIFI"));
      }
      else if (GETVAL == "MQTT")
      {
        checkNeedsReconnect(m.isMQTTMode(), (GETVAL == "MQTT"));
      }
      else if (GETVAL == "FHMQTT")
      {
        checkNeedsReconnect(m.isFHMQTTMode(), (GETVAL == "FHMQTT"));
      }
      conf.setDevicemode(GETVAL);
    }
    else if (GETARG == "mqttbrokerip")
    {
      checkNeedsReconnect(GETVAL, conf.getMQTTBrokerIP());
      conf.setMQTTBrokerIP(GETVAL);
    }
    else if (GETARG == "mqttbrokerport")
    {
      checkNeedsReconnect(GETVAL, conf.getMQTTPort());
      conf.setMQTTPort(GETVAL);
    }
    else if (GETARG == "mqttbrokeruser")
    {
      checkNeedsReconnect(GETVAL, conf.getMQTTUser());
      conf.setMQTTUser(GETVAL);
    }
    else if (GETARG == "mqttbrokerpasswd")
    {
      checkNeedsReconnect(GETVAL, conf.getMQTTPasswd());
      conf.setMQTTPasswd(GETVAL);
    }
    else if (GETARG == "mqttprefix")
    {
      conf.setMQTTPrefix(GETVAL);
    }
    else if (GETARG == "ledgreen")
    {
      conf.setLEDgreen(GETVAL);
    }
    else if (GETARG == "ledyellow")
    {
      conf.setLEDyellow(GETVAL);
    }
    else if (GETARG == "ledred")
    {
      conf.setLEDred(GETVAL);
    }
    else if (GETARG == "tempoffset")
    {
      conf.setTempOffset(GETVAL);
    }
#ifdef RTC
    else if (GETARG == "uhrzeit")
    {
      if (GETVAL != "")
      {
        myclock.setTime(GETVAL);
      }
    }
    else if (GETARG == "datum")
    {
      if (GETVAL != "")
      {
        myclock.setDate(GETVAL);
      }
    }
#endif
    else if (GETARG == "save")
    {
      if (GETVAL == "Speichern+Start")
      {
        //Serial.printf("******SAVE N EXIT \n\n\n\n");
        conf.setSaveNExit(true);
        m.setAtStartup(false);
      }
      conf.setInConfig(false);
    }
#ifdef RTC
    else if (GETARG == "setclock")
    {
      timeflag = true;
      myclock.setTimeDate();
    }
#endif
  }
  if (!displayonflag)
    conf.setDisplayOn("0");
  dbgmsg("%s DONE", __FUNCTION__);

  return true;
}

bool handleParamsCalib()
{
  bool ABCflag = false;
  unsigned int numberOfParams = server.args();

  //  if (wsMux.isLocked() || tooLittleHeap()) return false;
  if (!canHandleRequest("handleParamsCalib") || tooLittleHeap())
    return false;

  dbgmsg("Number of Get params: %i\n", numberOfParams);
  for (unsigned int i = 0; i < numberOfParams; i++)
  {

#define GETARG server.argName(i)
#define GETVAL server.arg(i)

    dbgmsg("%s %s\n", GETARG.c_str(), GETVAL.c_str());

    if (GETARG == "save")
    {
      if (GETVAL == "Speichern+Start")
      {
        conf.setSaveNExit(true);
      }
      conf.setInConfig(false);
    }
    else if (GETARG == "ABC")
    {
      if (GETVAL == "ABC")
      {
        conf.setABC("1");
        ABCflag = true;
      }
    }
    else if (GETARG == "calibval")
    {
      conf.setDefaultCV(GETVAL);
    }
    else if (GETARG == "calibrate")
    {
      dbgmsg("KALIBRIERUNG STARTEN");
      conf.setDoCalibration(true);
    }
  }

  if (!ABCflag)
    conf.setABC("0");

  return true;
}

void handleRoot();
void handleNotFound();
void handleConfigPage();
void handleDataPage();
void handleCalibPage();

void handleRoot()
{
  if (m.isAtStartup())
  {
    conf.setInConfig(false);
    stopHandleRequest();
    handleConfigPage();
    startHandleRequest();
  }
  else
  {
    stopHandleRequest();
    handleDataPage();
    startHandleRequest();
  }

  // if (ESPTemplateProcessor(server).send("index.html", processor)) {
  //   dbgmsg("YAY");
  // }
  // else {
  //   dbgmsg("NAY");
  //   server.send(200, "text/plain", "NOT FOUND");
  // }

  // //  server.serveStatic("/", LittleFS, "index.html", "max-age=MAXAGE");
}

void handleNotFound()
{
  logmsg(LOGWEB, "In handleNotFound\n");

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  dbgmsg("HANDLEFNF %s", message.c_str());
}

void handleConfigPage()
{
  logmsg(LOGWEB, "In %s%i\n", __FUNCTION__);

  if (conf.getInConfig())
    SENDINCONFIG;

  if (!canHandleRequest("/config") || tooLittleHeap())
    SENDDENIAL;

  conf.setInConfig(true);

  dbgmsg("LOCKING CONFIG PAGE");

  if (!server.authenticate(http_username, http_password))
  {
    conf.setInConfig(false);
    return server.requestAuthentication();
  }
  if (ESPTemplateProcessor(server).send("index.html", processor))
  {
    dbgmsg("Successfully uploaded index.html");
    conf.setInConfig(true);
  }
  else
  {
    dbgmsg("Failed to upload index.html");
    server.send(503, "text/plain", "Failed to upload index.html");
  }
}

void handleConfigData()
{
  logmsg(LOGWEB, "In %s\n", __FUNCTION__);

  if (tooLittleHeap())
    SENDDENIAL;
  //if (server.method() == HTTP_POST) {

  //   for (uint8_t i = 0; i < server.args(); i++) {
  //     Serial.printf("%s %s\n", server.argName(i).c_str(),server.arg(i).c_str());
  //   }
  // }

  if (handleParams())
  {
    if (timeflag)
    {
      timeflag = false;
      if (ESPTemplateProcessor(server).send("index.html", processor))
      {
        dbgmsg("Successfully uploaded index.html");
      }
      else
      {
        dbgmsg("Failed to upload index.html");
        server.send(503, "text/plain", "Failed to upload index.html");
      }
    }
    else
    {
      String Answermessage = conf.printConfig(2);
      server.send(200, "text/html", "<head><link rel=\"icon\" href=\"data:,\"><meta http-equiv=\"refresh\" content=\"5; url=/\"></head> Daten erhalten! <br>" + Answermessage + "<br>");
      //      conf.printConfig(1);
      delay(10);
      conf.storeConfig();
      delay(10);
    }
  }
  conf.setInConfig(false);
  dbgmsg("LEAVING handleConfigData");
}

void handleCalibPage()
{
  logmsg(LOGWEB, "In %s%i\n", __FUNCTION__);

  if (!canHandleRequest("/calib") || tooLittleHeap())
    SENDDENIAL;

  if (handleParamsCalib())
  {
    String Answermessage = conf.printConfig(2);
    stopHandleRequest();
    server.send(200, "text/html", "Kalibrierung durchgef&uuml;hrt<br><a href=\"/data\">Sensorwerte anzeigen</a>");
    // output in serial console
    conf.printConfig(1);
    conf.storeConfig();
    delay(1000);
    startHandleRequest();
  }
}

void handleDataPage()
{
  logmsg(LOGWEB, "In %s%i\n", __FUNCTION__);

  if (!canHandleRequest("/data") || tooLittleHeap())
    SENDDENIAL;

  stopHandleRequest();
  if (ESPTemplateProcessor(server).send("data.html", processor))
  {
    dbgmsg("Successfully uploaded data.html");
    conf.setInConfig(false);
  }
  else
  {
    dbgmsg("Failed to upload data.html");
    server.send(503, "text/plain", "Failed to upload data.html");
  }
  startHandleRequest();
}

void handleGetChartDataRequest()
{
  logmsg(LOGWEB, "In %s%i\n", __FUNCTION__);

  if (!canHandleRequest("/getChartData") || tooLittleHeap())
    SENDDENIAL;

  stopHandleRequest();
  if (ESP.getFreeHeap() > 2 * MIN_FREE_HEAP_TO_SERVICE)
  {
    String list = chartlog.getValueList();
    server.send_P(200, "text/plain", list.c_str());
  }
  else
  {
    server.send_P(200, "text/plain", "data: [], labels[]");
  }
  startHandleRequest();
}

void handleGetNextChartEntryRequest()
{
  logmsg(LOGWEB, "In %s%i\n", __FUNCTION__);

  if (!canHandleRequest("/getNextChartEntry") || tooLittleHeap(1.0))
    SENDDENIAL;
  server.send_P(200, "text/plain", chartlog.getNextValue().c_str());
}

void handleGetDataRequest()
{

  logmsg(LOGWEB, "In %s%i\n", __FUNCTION__);

  if (!canHandleRequest("/getData") || tooLittleHeap(1))
    SENDDENIAL;

  char jsonstring[GETDATASTRINGLEN];
  String timedate;
  int co2, temp, rH;
  m.getValues(co2, temp, rH);

  if (conf.getShowRTC())
  {
    timedate = myclock.getTimeDate();
  }
  else
  {
    timedate = "";
  }
  snprintf(jsonstring, GETDATASTRINGLEN, "{\"td\":\"%s\",\"co2\":\"%i\",\"rh\":\"%i\",\"t\":\"%i\",\"y\":\"%s\",\"r\":\"%s\",\"device\":\"%s-%s\"}", timedate.c_str(), co2, rH, temp, conf.getLEDyellow().c_str(), conf.getLEDred().c_str(), ap.getDevicename(), (m.getSensorType() == SCD ? m.getSensorSN().c_str() : ""));
  delay(10);
  jsonstring[GETDATASTRINGLEN - 1] = '\0';
  //       Serial.printf("%s\n\n", jsonstring);
  dbgmsg("Sending jsonstring in /getData callback!");
  server.send_P(200, "text/plain", jsonstring);
  dbgmsg("Done sending jsonstring in /getData callback!");
  delay(10);
}

void handleCo2Request()
{
  logmsg(LOGWEB, "In %s%i\n", __FUNCTION__);

  if (!canHandleRequest("/co2"))
    SENDDENIAL;

  int co2, temp, rH;
  m.getValues(co2, temp, rH);
  server.send_P(200, "text/plain", String(co2).c_str());
}

void handleRhRequest()
{
  logmsg(LOGWEB, "In %s%i\n", __FUNCTION__);

  if (!canHandleRequest("/rh"))
    SENDDENIAL;

  int co2, temp, rH;
  m.getValues(co2, temp, rH);
  server.send_P(200, "text/plain", String(rH).c_str());
}

void handleTimedateRequest()
{
  logmsg(LOGWEB, "In %s%i\n", __FUNCTION__);

  if (conf.getShowRTC())
    server.send_P(200, "text/plain", myclock.getTimeDate().c_str());
  else
    server.send_P(200, "text/plain", "");
}

void handleTimeRequest()
{
  logmsg(LOGWEB, "In %s%i\n", __FUNCTION__);

  if (conf.getShowRTC())
    server.send_P(200, "text/plain", myclock.getTime().c_str());
  else
    server.send_P(200, "text/plain", "");
}

void handleTemperatureRequest()
{
  logmsg(LOGWEB, "In %s%i\n", __FUNCTION__);

  if (!canHandleRequest("/temperature"))
    SENDDENIAL;

  int co2, temp, rH;
  m.getValues(co2, temp, rH);
  dbgmsg("Updating TEMPERATURE");
  server.send_P(200, "text/plain", String(temp).c_str());
}

void webserverRegisterCallbacks()
{

  logmsg(LOGWEB, "Starting Webserver");

  server.on("/", handleRoot);
  server.on("/config", handleConfigPage);
  server.on("/post", handleConfigData);
  server.on("/calib", handleCalibPage);
  server.on("/data", handleDataPage);
  server.on("/toData", handleDataPage);
  server.on("/getChartData", handleGetChartDataRequest);
  server.on("/getNextChartEntry", handleGetNextChartEntryRequest);
  server.on("/getData", handleGetDataRequest);
  server.on("/co2", handleCo2Request);
  server.on("/rh", handleRhRequest);
  server.on("/timedate", handleTimedateRequest);
  server.on("/time", handleTimeRequest);
  server.on("/temperature", handleTemperatureRequest);

  server.onNotFound(handleNotFound);

  server.serveStatic("/timedate.js", LittleFS, "timedate.js");
  server.serveStatic("/style.css", LittleFS, "style.css", "max-age=MAXAGE");
  server.serveStatic("/Chart.min.js", LittleFS, "Chart.min.js", "max-age=MAXAGE");
  server.serveStatic("/Chart.min.css", LittleFS, "Chart.min.css", "max-age=MAXAGE");

  server.begin();
}

void AP_loop()
{
  //  logmsg(LOGWEB, "Running AP ...\n Updating MDNS ...");
  //  MDNS.update();
}
