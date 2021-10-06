#ifndef __LIGHTS_H__
#define __LIGHTS_H__
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

//////////////////////////////////////////////////////////////////////////
// Pins for RYG LEDs
//////////////////////////////////////////////////////////////////////////
#define LEDR 2  // Pin for Red LED
#define LEDY 13 // Pin for Yellow LED
#define LEDG 15 // Pin for Green LED

//////////////////////////////////////////////////////////////////////////
// Default Values for RYG LEDs
//////////////////////////////////////////////////////////////////////////
#define LIMR 1700 // Pin for Red LED
#define LIMY 1000 // Pin for Yellow LED
#define LIMG 0    // Pin for Green LED

class lights
{
public:
  lights();

  void init();
  void setLimits(int g, int y, int r);

  void setLights(int co2);

private:
  int Lim_g, Lim_y, Lim_r;
  bool initialized;
};

#endif /* __LIGHTS_H__ */
