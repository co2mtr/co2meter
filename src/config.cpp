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
// ArduinoJSON lib is required

#include "config.h"
#include "types.h"
#include "compileflags.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "cryptstring.h"
#include "pmMutex.h"
#include "accesspoint.h"

#define DOC_SIZE 1280

extern accesspoint ap;

Mutex mtx1;
void lock(Mutex &m)
{
  m.lock();
}

void unlock(Mutex &m)
{
  m.unlock();
}

void config::setSensorType(int type)
{
  sensorType = type;
}

int config::getSensorType()
{
  return sensorType;
}

String config::printConfig(int type)
{
  String newline;
  if (type == 1)
  { /* Serial output */
    newline = "\n";
  }
  else if (type == 2)
  { /* html output */
    newline = "<br>";
  }

  String message =
      "Schule: " + String(school) + newline +
      "Klasse : " + String(classid) + newline +
      "Anzahl Schueler: " + String(pupils) + newline +
      "Raumgroesse: " + String(roomsize) + newline +
      "Betriebmodus: " + String(devicemode) + newline +
      "Display An?: " + String(displayon) + newline +
      "Display Mode: " + String(displaymode) + newline +
      "LED gruen: " + String(ledgreen) + newline +
      "LED gelb: " + String(ledyellow) + newline +
      "LED rot: " + String(ledred) + newline +
      "Temp Offset: " + String(tempoffset) + newline +
      "SSID: " + String(ssid) + newline +
      "MDSN: " + String(mdnsname) + newline +
      "MQTT Broker IP: " + String(mqtt_broker) + newline +
      "MQTT Port " + String(mqtt_port) + newline +
      "MQTT User " + String(mqtt_user) + newline +
      "MQTT Passwd " + String(mqtt_passwd) + newline +
      "MQTT Prefix: " + String(mqtt_prefix) + newline +
      "ABC: " + String(ABC) + newline +
      "DefaultCV: " + String(defaultCV) + newline;

  //    "Passwort: " +  getCryptString(passwd) + newline +

  if (type == 1)
  {
    Serial.print(message);
  }

  return message;
}

void config::readConfig()
{
  if (LittleFS.begin())
  {
    Serial.println("mounted file system");

    /* example from: https://arduinojson.org/v6/example/config/ */
    // Open file for reading
    File file = LittleFS.open("config.json", "r");

    lock(mtx1);
    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    StaticJsonDocument<DOC_SIZE> doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, file);
    if (error)
      Serial.println(F("Failed to read file, using default configuration"));

    stpncpy(ssid, doc["ssid"], CFGSSIDLEN);
    stpncpy(passwd, getClearText(doc["passwd"]).c_str(), CFGPASSWDLEN);
    stpncpy(mdnsname, doc["mdnsname"], CFGMDNSNAMELEN);
    if (strlen(mdnsname) == 0)
    {
      ap.setDevicename();
      setMDNSName(ap.getMDNSName());
      stpncpy(mdnsname, getMDNSName().c_str(), CFGMDNSNAMELEN);
    }
    stpncpy(mqtt_broker, doc["mqtt_broker"], CFGMQTTBROKERLEN);
    stpncpy(mqtt_port, doc["mqtt_port"], CFGMQTTPORTLEN);
    stpncpy(mqtt_user, doc["mqtt_user"], CFGMQTTUSERLEN);
    stpncpy(mqtt_passwd, doc["mqtt_passwd"], CFGMQTTPASSWDLEN);
    if (doc["mqtt_prefix"] != "")
      stpncpy(mqtt_prefix, doc["mqtt_prefix"], CFGMQTTPREFIXLEN);
    stpncpy(devicemode, doc["devicemode"], CFGDEVICEMODELEN);
    stpncpy(displayon, doc["displayon"], CFGDISPLAYONLEN);
    stpncpy(displaymode, doc["displaymode"], CFGDISPLAYMODELEN);
    stpncpy(school, doc["school"], CFGSCHOOLLEN);
    stpncpy(classid, doc["classid"], CFGCLASSIDLEN);
    stpncpy(pupils, doc["pupils"], CFGPUPILSLEN);
    stpncpy(roomsize, doc["roomsize"], CFGROOMSIZELEN);
    stpncpy(ledgreen, doc["ledgreen"], CFGLEDGREENLEN);
    stpncpy(ledyellow, doc["ledyellow"], CFGLEDYELLOWLEN);
    stpncpy(ledred, doc["ledred"], CFGLEDREDLEN);
    stpncpy(ABC, doc["ABC"], CFGABCLEN);
    stpncpy(defaultCV, doc["defaultCV"], CFGDEFAULTCVLEN);

    unlock(mtx1);
    file.close();
  }
}

DynamicJsonDocument config::getJsonConfig()
{
  DynamicJsonDocument doc(DOC_SIZE);

  doc["school"] = school;
  doc["classid"] = classid;
  doc["pupils"] = pupils;
  doc["roomsize"] = roomsize;
  doc["ledgreen"] = ledgreen;
  doc["ledyellow"] = ledyellow;
  doc["ledred"] = ledred;
  doc["tempoffset"] = tempoffset;
  doc["ssid"] = ssid;
  doc["passwd"] = passwd;
  doc["mdnsname"] = mdnsname;
  doc["mqtt_broker"] = mqtt_broker;
  doc["mqtt_port"] = mqtt_port;
  doc["mqtt_user"] = mqtt_user;
  doc["mqtt_passwd"] = mqtt_passwd;
  doc["mqtt_prefix"] = mqtt_prefix;
  doc["devicemode"] = devicemode;
  doc["displayon"] = displayon;
  doc["displaymode"] = displaymode;
  doc["ABC"] = ABC;
  doc["defaultCV"] = defaultCV;

  return doc;
}

void config::storeConfig()
{
  // Open file for writing
  File file = LittleFS.open("config.json", "w");
  if (!file)
  {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<DOC_SIZE> doc;

  dbgmsg("DISPLAY ON %s DEVICE MODE %s\n", displayon, devicemode);

  doc["school"] = school;
  doc["classid"] = classid;
  doc["pupils"] = pupils;
  doc["roomsize"] = roomsize;
  doc["ledgreen"] = ledgreen;
  doc["ledyellow"] = ledyellow;
  doc["ledred"] = ledred;
  doc["tempoffset"] = tempoffset;
  doc["ssid"] = ssid;
  char p[CFGPASSWDLEN];
  delay(10);
  stpncpy(p, getCryptString(String(passwd)).c_str(), CFGPASSWDLEN);
  p[CFGPASSWDLEN - 1] = '\0';
  delay(10);
  doc["passwd"] = p;
  doc["mdnsname"] = mdnsname;
  doc["mqtt_broker"] = mqtt_broker;
  doc["mqtt_port"] = mqtt_port;
  doc["mqtt_user"] = mqtt_user;
  doc["mqtt_passwd"] = mqtt_passwd;
  doc["mqtt_prefix"] = mqtt_prefix;
  doc["devicemode"] = devicemode;
  doc["displayon"] = displayon;
  doc["displaymode"] = displaymode;
  doc["ABC"] = ABC;
  doc["defaultCV"] = defaultCV;

  delay(10);
  dbgmsg("Writing config file ...");

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0)
  {
    Serial.println(F("Failed to write to file"));
  }
  delay(10);

  // Close the file
  file.close();
  delay(10);
  dbgmsg("Done ...\n");
}
