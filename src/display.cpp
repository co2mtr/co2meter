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
#include "display.h"
#include "compileflags.h"
#include "types.h"
#include "config.h"

extern config conf;

void display::showHeader()
{
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(5, 10, header.c_str());
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(5, 18, subheader.c_str());
}

void display::showFooter()
{

  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(5, 60, footer.c_str());
}

void display::showMode()
{
  u8g2.setFontMode(0);
  u8g2.setDrawColor(1);
  u8g2.setFont(u8g2_font_5x7_tf);
  if (conf.getShowAnalog())
  {
    u8g2.drawStr(110, 9, mode.c_str());
  }
  else
  {
    u8g2.drawStr(110, 60, mode.c_str());
  }
}

void display::drawAngLine(int n, int xc, int yc, int start, int len, int &x1, int &y1, int &x2, int &y2)
{
  float ang = (6 * n - 90) / 57.3;
  x1 = start * cos(ang) + xc;
  y1 = start * sin(ang) + yc;
  x2 = (start + len) * cos(ang) + xc;
  y2 = (start + len) * sin(ang) + (float)yc;
}

void display::showAnalogTime(uint16_t h, uint16_t m, uint16_t s)
{
  const int cx = 30;
  const int cy = 32;
  const int r = 28;
  int x1, x2, y1, y2;

  u8g2.setDrawColor(0);
  u8g2.drawBox(1, 1, 63, 60);
  u8g2.setFontMode(0);
  u8g2.setDrawColor(1);

  u8g2.drawCircle(cx, cy, r);
  for (unsigned int i = 0; i < 60; i += 5)
  {
    drawAngLine(i, cx, cy, r - 5, 5, x1, y1, x2, y2);
    u8g2.drawLine(x1, y1, x2, y2);
  }
  int hh = (h % 12) * 5 + (int)((float)m / 60.0 * 5.0);
  //  Serial.printf("%i %i %i %i", h, hh, m, s);
  drawAngLine(hh, cx, cy, 3, 15, x1, y1, x2, y2);
  u8g2.drawLine(x1, y1, x2, y2);
  drawAngLine(m, cx, cy, 3, 20, x1, y1, x2, y2);
  u8g2.drawLine(x1, y1, x2, y2);
  drawAngLine(s, cx, cy, 3, 25, x1, y1, x2, y2);
  u8g2.drawLine(x1, y1, x2, y2);
  u8g2.sendBuffer();
}

void display::showWelcome(int major, int minor, int revision)
{
  dbgmsg("Showing WELCOME message");
  logmsg(LOGSM, "Version: %s.%s.%s\n", String(major).c_str(), String(minor).c_str(), String(revision).c_str());

  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.drawStr(15, 40, "CO2 Meter");
  u8g2.setCursor(10, 60);
  u8g2.setFont(u8g2_font_baby_tf);
  u8g2.print("Version: ");
  u8g2.print(String(major));
  u8g2.print(".");
  u8g2.print(String(minor));
  u8g2.print(".");
  u8g2.print(String(revision));
  u8g2.sendBuffer();
}

void display::showConfig()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(5, 30, "Konfiguration");
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(5, 50, "Im Browser 192.168.0.1");
  showHeader();
  showFooter();
  showMode();
  u8g2.sendBuffer();
  //u8g2.writeBufferXBM(Serial);
}

void display::showError(String s)
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.setCursor(5, 30);
  u8g2.print(s.c_str());
  showHeader();
  showFooter();
  showMode();
  u8g2.sendBuffer();
  //u8g2.writeBufferXBM(Serial);
}

void display::setBG(int colour)
{
  if (colour == GREEN)
  {
    u8g2.setDrawColor(0);
    u8g2.drawBox(1, 30, 127, 60);
    u8g2.setFontMode(0);
    u8g2.setDrawColor(1);
  }
  else
  {
    int offset = 1;
    if (colour == YELLOW)
      offset = 3;

    for (unsigned int x = 1; x < 128; x = x + offset)
    {
      for (unsigned int y = 22; y < 51; y = y + offset)
      {
        u8g2.setDrawColor(1);
        u8g2.drawPixel(x, y);
      }
      if (colour == YELLOW)
      {
        u8g2.setDrawColor(1);
        u8g2.setFontMode(1);
      }
      else if (colour == RED)
      {
        u8g2.setDrawColor(0);
        u8g2.setFontMode(0);
      }
    }
  }
}

void display::showSense(int c, int t, int h)
{
  //   clear();
  u8g2.clearBuffer();

  dbgmsg("%i,%i,%i\n", co2limits[0], co2limits[1], co2limits[2]);

  if (c < co2limits[1])
    setBG(GREEN);
  else if (c < co2limits[2])
    setBG(YELLOW);
  else
    setBG(RED);

  int len = String(c).length();
  int xoffset = 0;
  if (len < 4)
    xoffset = 18;
  u8g2.setCursor(10 + xoffset, 45);
  u8g2.setFont(u8g2_font_fub17_tf);
  u8g2.print(c);
  u8g2.setCursor(80, 45);

  u8g2.print(t);
  u8g2.setFont(u8g2_font_fub11_tf);
  u8g2.print("C");
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(70, 45, "m"); // write something to the internal memory
  u8g2.drawStr(70, 38, "p"); // write something to the internal memory
  u8g2.drawStr(70, 31, "p"); // write something to the internal memory

  //  u8g2.drawStr(10,30, "Show Sensor values here...");
  u8g2.setFont(u8g2_font_5x7_tf);
  //  u8g2.drawStr(10,50, "Im Browser 192.168.0.1");
  setMode("S", false);
  u8g2.setFontMode(0);
  u8g2.setDrawColor(1);

  showHeader();
  showFooter();
  showMode();
  u8g2.sendBuffer();
  //  u8g2.writeBufferXBM(Serial);
}

void display::showSenseRightHalf(int c, int t, int h, String ip, String mdnsName)
{
  //   clear();
  //  u8g2.clearBuffer();

  u8g2.setDrawColor(0);
  u8g2.drawBox(64, 1, 128, 60);
  u8g2.setFontMode(0);
  u8g2.setDrawColor(1);

  int len = String(c).length();
  int xoffset = 0;
  if (len < 4)
    xoffset = 18;
  u8g2.setCursor(65 + xoffset, 25);
  u8g2.setFont(u8g2_font_fub14_tf);
  u8g2.print(c);

  if (conf.getSensorType() == SCD)
  {
    u8g2.setCursor(63, 42);
    u8g2.setFont(u8g2_font_fur11_tf);
    u8g2.print(t);
    u8g2.setFont(u8g2_font_fur11_tf);
    u8g2.print("C/");
    u8g2.print(h);
    u8g2.print("rH");
  }
  else
  {
    u8g2.setCursor(100, 42);
    u8g2.setFont(u8g2_font_fur11_tf);
    u8g2.print(t);
    u8g2.setFont(u8g2_font_fur11_tf);
    u8g2.print("C");
  }
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(123, 25, "m"); // write something to the internal memory
  u8g2.drawStr(123, 18, "p"); // write something to the internal memory
  u8g2.drawStr(123, 11, "p"); // write something to the internal memory

  //  u8g2.drawStr(10,30, "Show Sensor values here...");
  u8g2.setFont(u8g2_font_5x7_tf);

  u8g2.setFont(u8g2_font_baby_tf);
  u8g2.drawStr(63, 51, ip.c_str());
  u8g2.setFont(u8g2_font_baby_tf);
  u8g2.drawStr(63, 60, mdnsName.c_str());

  setMode("S", false);
  u8g2.setFontMode(0);
  u8g2.setDrawColor(1);

  //  showHeader();
  //showFooter();
  showMode();
  u8g2.sendBuffer();
  //  u8g2.writeBufferXBM(Serial);
}

void display::showConnectingWifi(String AP)
{
  static bool once = true;
  //  if (once) {
  clear();
  u8g2.clearBuffer();
  once = false;
  //  }
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(5, 30, "Connecting to WiFi ...");
  u8g2.setFont(u8g2_font_5x7_tf);
  //  String tmp ="AP: " + AP;
  //u8g2.drawStr(5,40, tmp.c_str());
  showHeader();
  showFooter();
  showMode();
  u8g2.sendBuffer();
}

void display::showConnectingMQTT(String broker)
{
  clear();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(5, 30, "Connecting MQTT Broker");
  u8g2.setFont(u8g2_font_5x7_tf);
  String tmp = "IP: " + broker;
  u8g2.drawStr(5, 40, tmp.c_str());
  showHeader();
  showFooter();
  showMode();
  u8g2.sendBuffer();
  //u8g2.writeBufferXBM(Serial);
}

void display::showConnectedMQTT()
{
  clear();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(5, 30, "Connected to MQTT server.");
  u8g2.setFont(u8g2_font_5x7_tf);
  showHeader();
  showFooter();
  showMode();
  u8g2.sendBuffer();
  //  u8g2.writeBufferXBM(Serial);
}

void display::showPublish()
{
  //  clear();
  //u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  // u8g2.drawStr(10,30, "Publish values...");
  u8g2.setFont(u8g2_font_5x7_tf);
  //  u8g2.drawStr(10,50, "Im Browser 192.168.0.1");
  setMode("P", false);

  u8g2.setDrawColor(0);
  u8g2.drawBox(5, 50, 127, 60);
  u8g2.setDrawColor(1);
  u8g2.setFontMode(0);
  showHeader();
  showFooter();
  showMode();
  u8g2.sendBuffer();
  //u8g2.writeBufferXBM(Serial);
}

void display::showPublishRightHalf()
{
  //  clear();
  //u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  // u8g2.drawStr(10,30, "Publish values...");
  u8g2.setFont(u8g2_font_5x7_tf);
  //  u8g2.drawStr(10,50, "Im Browser 192.168.0.1");
  setMode("P", false);

  //   	u8g2.setDrawColor(0);
  //   u8g2.drawBox(5,50,127,60);
  //   	u8g2.setDrawColor(1);
  // 	u8g2.setFontMode(0);
  // showHeader();
  //   showFooter();
  showMode();
  u8g2.sendBuffer();
  //u8g2.writeBufferXBM(Serial);
}

void display::clear()
{
  u8g2.setDrawColor(1);
  u8g2.clearBuffer();
  u8g2.firstPage();
  do
  {
    showHeader();
    showFooter();

  } while (u8g2.nextPage());
  u8g2.sendBuffer();
}

/* if wheel is set, a ascii wheel is attached to the mode letter */
void display::setMode(String s, bool wheel)
{
  static int counter = 0;
  String w;

  if (wheel)
  {
    if (counter % 4 == 0)
      w = " /";
    else if (counter % 4 == 1)
      w = " -";
    else if (counter % 4 == 2)
      w = " \\";
    else if (counter % 4 == 3)
      w = " |";
    counter++;
    mode = s + w;
  }
  else
    mode = s;
}

#define XW 7
#define XOFFSET 7
#define XGAP 2
#define YW 3
#define YGAP 1
#define YBOTCOORD 50

void display::drawValue(int val, int pos)
{
  if (val < 400)
    return;
  //  int xw=5, xgap=5, yw=3, ygap=2;
  int xpos = pos * (XW + XGAP);
  int q = 100;
  int qval = val % q;
  val -= qval;
  if (qval > q / 2)
    val += q;

  int height = (val - 400) / q;
  if (height > 8)
    height = 8;
  //  Serial.printf("%i %i %i ", xpos, height,  60 - height * ( yw + ygap));
  u8g2.setDrawColor(1);
  u8g2.drawBox(xpos + XOFFSET, YBOTCOORD - height * (YW + YGAP), XW, height * (YW + YGAP));
  u8g2.setDrawColor(0);
  for (int i = 0; i < height; i++)
  {
    u8g2.setDrawColor(0);
    u8g2.drawBox(xpos + XOFFSET, YBOTCOORD - (YW + YGAP) * (i + 1), XW, YGAP);
  }
}

void display::drawCS()
{
  int xpos;

  u8g2.setDrawColor(1);
  for (unsigned int i = 2; i < GRAPHCOLS + 1; i++)
  {
    xpos = i * (XW + XGAP);
    u8g2.drawBox(xpos + XOFFSET, YBOTCOORD + 1, XW, 1);
  }
  u8g2.setFont(u8g2_font_baby_tf);
  u8g2.drawStr(2, 50, "400");
  u8g2.drawStr(2, 44, "m"); // write something to the internal memory
  u8g2.drawStr(2, 37, "p"); // write something to the internal memory
  u8g2.drawStr(2, 30, "p"); // write something to the internal memory
  u8g2.drawStr(2, 25, "1000");
}

void display::showGraph(int c, int g[])
{
  clear();
  drawCS();
  int index = c + GRAPHCOLS;
  for (unsigned int i = 0; i < GRAPHCOLS; i++)
  {

    //    dbgmsg("%i, %i, %i\n", GRAPHCOLS+1-i, index % GRAPHCOLS, g[ index % GRAPHCOLS ] );
    drawValue(g[index % GRAPHCOLS], GRAPHCOLS + 1 - i);
    index--;
  }
  //  u8g2.drawBox(1,30,127,60);
  //  u8g2.setFontMode(0);
  // drawValue(600, 0);
  //   drawValue(450, 1);
  // drawValue(550, 2);
  // drawValue(650, 3);
  // drawValue(750, 4);
  // drawValue(850, 5);
  // drawValue(950, 6);
  // drawValue(650, 7);
  // drawValue(1000, 8);
  // drawValue(1100, 9);
  // drawValue(1200, 10);
  // drawValue(700, 11);
  // drawValue(700, 12);
  u8g2.sendBuffer();
}

void display::showCalibrate()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(5, 30, "Konfiguration");
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.drawStr(5, 50, "Im Browser 192.168.0.1");
  showHeader();
  showFooter();
  showMode();
  u8g2.sendBuffer();
  //u8g2.writeBufferXBM(Serial);
}

void display::showCalibrateDone()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  u8g2.setCursor(5, 30);
  u8g2.print("Kalibriere Sensor ... ");
  u8g2.print("OK");
  u8g2.drawStr(5, 50, "Starte Geraet neu...");
  u8g2.setFont(u8g2_font_5x7_tf);
  showHeader();
  showFooter();
  showMode();
  u8g2.sendBuffer();
  //u8g2.writeBufferXBM(Serial);
}

void display::reset()
{
  u8g2.initDisplay();
  u8g2.setPowerSave(0);
}
