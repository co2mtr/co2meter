#ifndef ESP_TEMPLATE_PROCESSOR_H
#define ESP_TEMPLATE_PROCESSOR_H
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
// adopted from
// from https://github.com/winder/ESPTemplateProcessor/blob/master/ESPTemplateProcessor.h

#ifdef ESP8266
#define WebServer ESP8266WebServer
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif

//#include <FS.h>
#include <LittleFS.h>
#define SPIFFS LittleFS
#ifdef ESP32
#include <SPIFFS.h>
#endif

typedef String ProcessorCallback(const String &key);

class ESPTemplateProcessor
{
public:
  ESPTemplateProcessor(WebServer &_server) : server(_server)
  {
  }

  bool send(const String &filePath, ProcessorCallback &processor, char bookend = '%', bool silentSerial = false)
  {
    // Open file.
    if (!SPIFFS.exists(filePath))
    {
      if (!silentSerial)
      {
        Serial.print("Cannot process ");
        Serial.print(filePath.c_str());
        Serial.println(": Does not exist.");
      }
      return false;
    }

    File file = SPIFFS.open(filePath, "r");
    if (!file)
    {
      if (!silentSerial)
      {
        Serial.print("Cannot process ");
        Serial.print(filePath.c_str());
        Serial.println(": Failed to open.");
      }
      return false;
    }

    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.sendHeader("Content-Type", "text/html", true);
    server.sendHeader("Cache-Control", "no-cache");
    server.send(200);
    //server.sendContent(<chunk>)

    // Process!
    static const uint16_t MAX = 100;
    String buffer;
    int bufferLen = 0;
    String keyBuffer;
    int val;
    char ch;
    while ((val = file.read()) != -1)
    {
      ch = char(val);

      // Lookup expansion.
      if (ch == bookend)
      {
        // Clear out buffer.
        if (buffer.length() > 0)
        {
          server.sendContent(buffer);
          //Serial.printf("[%s]-1-\n", buffer.c_str());
        }
        buffer = "";
        bufferLen = 0;

        // Process substitution.
        keyBuffer = "";
        bool found = false;
        while (!found && (val = file.read()) != -1)
        {
          ch = char(val);
          if (ch == bookend)
          {
            found = true;
          }
          else
          {
            keyBuffer += ch;
          }
        }

        // Check for bad exit.
        // TODO: Needs fix. If a single % (bookend) is found, the page cannot be parsed
        if (val == -1 && !found)
        {
          if (!silentSerial)
          {
            logmsg(LOGWEB, "Cannot process %s. Unable to parse.\n", filePath.c_str());
          }
          return false;
        }

        // Get substitution
        String processed = processor(keyBuffer);
        if (!silentSerial)
        {
          logmsg(LOGWEB, "Template Engine: %s=%s\n", keyBuffer.c_str(), processed.c_str());
        }
        if (processed.length() > 0)
        {
          server.sendContent(processed);
          //Serial.printf("[%s]-2-\n", processed.c_str());
        }
      }
      else
      {
        bufferLen++;
        buffer += ch;
        //Serial.printf("BUFFER %s", buffer.c_str());

        if (bufferLen >= MAX)
        {
          server.sendContent(buffer);
          //Serial.printf("[%s]-3-\n", buffer.c_str());
          bufferLen = 0;
          buffer = "";
        }
      }
    }

    if (val == -1)
    {
      server.sendContent(buffer);
      server.sendContent("");
      return true;
    }
    else
    {
      if (!silentSerial)
      {
        logmsg(LOGWEB, "Failed to process %s. Didn't reach the end of the file.\n", filePath.c_str());
      }
    }

    return true;
  }

private:
  WebServer &server;
};

#endif
