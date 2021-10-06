#ifndef __SENSOR_MHZ19B_H__
#define __SENSOR_MHZ19B_H__
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
#include <MHZ19.h>

class sensor
{

  MHZ19 device;

public:
  sensor()
  {
    device.begin(14, 12);
  }

  void enableABC(bool v)
  {
    if (v)
      device.setAutoCalibration(true);
    else
      device.setAutoCalibration(false);
  }

  int getStatus()
  {
    return device.getStatus();
  }

  void getMeasurement(int &co2, int &temp)
  {
    measurement_t value = device.getMeasurement();
    co2 = value.co2_ppm;
    temp = value.temperature;
  }

  void calibrateZero()
  {
    /* do only one calibration at a time not to destroy the sensor */

    static bool once = true;
    if (once)
    {
      device.calibrateZero();
      once = false;
    }
  }
};

#endif /* __SENSOR_H__ */
