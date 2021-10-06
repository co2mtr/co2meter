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
#include "lights.h"
#include "compileflags.h"

#include <Arduino.h>

lights::lights()
{
  Lim_g = LIMG;
  Lim_y = LIMY;
  Lim_r = LIMR;

  initialized = false;
}

void lights::init()
{
  if (!initialized)
  {
    initialized = true;

    pinMode(LEDR, OUTPUT);
    pinMode(LEDY, OUTPUT);
    pinMode(LEDG, OUTPUT);

    // LED Test
    digitalWrite(LEDR, HIGH);
    delay(200);
    digitalWrite(LEDR, LOW);
    digitalWrite(LEDY, HIGH);
    delay(200);
    digitalWrite(LEDY, LOW);
    digitalWrite(LEDG, HIGH);
    delay(200);
    digitalWrite(LEDG, LOW);
  }
}

void lights::setLimits(int g, int y, int r)
{
  Lim_g = g;
  Lim_y = y;
  Lim_r = r;
}

void lights::setLights(int co2)
{
  if (co2 < Lim_y)
  {
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDY, LOW);
    digitalWrite(LEDR, LOW);
  }

  if ((co2 >= Lim_y) && (co2 < Lim_r))
  {
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDY, HIGH);
    digitalWrite(LEDR, LOW);
  }

  if (co2 >= Lim_r)
  {
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDY, LOW);
    digitalWrite(LEDR, HIGH);
  }
}
