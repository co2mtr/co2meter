#ifndef __DISPLAY_H__
#define __DISPLAY_H__
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

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef DISPLAY_096
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
#else
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
#endif

extern void startqr(String);

typedef enum
{
  GREEN,
  YELLOW,
  RED
} displayBG;

#define GRAPHCOLS 12

class display
{

  String header;
  String subheader;
  String footer;
  String mode;

  int co2limits[3];

public:
  display()
  {
    Serial.println("Initializing Display");
  }

  void setLimits(int g, int y, int r)
  {
    co2limits[0] = g;
    co2limits[1] = y;
    co2limits[2] = r;
  }

  void setHeader(String s) { header = s; }
  void setSubheader(String s) { subheader = s; }
  void setFooter(String s) { footer = s; }
  void showWelcome(int, int, int);
  void showConfig();
  void showHeader();
  void showError(String);
  void showFooter();
  void setMode(String, bool);
  void showMode();
  void drawAngLine(int, int, int, int, int, int &, int &, int &, int &);
  void showAnalogTime(uint16_t, uint16_t, uint16_t);
  void showSense(int, int, int);
  void showSenseRightHalf(int, int, int, String, String);
  void showConnectingWifi(String);
  void showConnectingMQTT(String);
  void showConnectedMQTT();
  void showPublish();
  void showPublishRightHalf();
  void showCalibrate();
  void showCalibrateDone();
  void setBG(int);
  //  void setState();
  void clear();
  void showGraph(int, int[]);
  void drawCS();
  void drawValue(int, int);
  void reset();
};

#endif /* __DISPLAY_H__ */
