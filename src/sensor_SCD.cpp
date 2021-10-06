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
#include "sensor_SCD.h"
#include "compileflags.h"

// Public Functions
sensor_SCD::sensor_SCD()
{
  // Init private variables
  SCD_available = 0;
}

bool sensor_SCD::init()
{
  // Try to Enable SCD-30
  device.setDebug(SCD_DEBUG);

  if (device.begin(Wire) == false)
  {
    SCD_available = 0;
    Serial.println("SCD sensor not detected.");
  }
  else
  {
    SCD_available = 1;
    Serial.println("SCD sensor detected.");
    // Display SCD device info
    SCD_DeviceInfo();
    device.setTemperatureOffset((uint16_t)SCD_TEMP_OFFSET);
  }

  return (SCD_available > 0);
}

void sensor_SCD::getMeasurement(int &co2, int &temp, int &rH)
{
  if (device.dataAvailable())
  {
    co2_int = (int)device.getCO2();
    temp_int = (int)device.getTemperature();
    rH_int = (int)device.getHumidity();
  }
  co2 = co2_int;
  temp = temp_int;
  rH = rH_int;
}

bool sensor_SCD::calibrate(uint16_t cv)
{
  return device.setForceRecalibration(cv);
}

bool sensor_SCD::getSerial(char *sn)
{
  // Reserve at least 7 + 1 = 8 Bytes Buffersize
  // Fix for bug in Sensor library
  bool result = device.getSerialNumber(sn);
  sn[SCD30_SERIAL_NUM_WORDS * 2] = '\0';
  return result;
}

// Private Methods for SCD-30
void sensor_SCD::SCD_DeviceInfo()
{
  uint8_t val[2];
  // Array must be be one Byte bigger, because of Bug in SCD Driver
  char buf[(SCD30_SERIAL_NUM_WORDS * 2) + 2];

  // Read SCD30 serial number as printed on the device
  // buffer MUST be at least 33 digits (32 serial + 0x0)
  if (device.getSerialNumber(buf))
  {
    buf[(SCD30_SERIAL_NUM_WORDS * 2)] = '\0';
    Serial.print(F("SCD30 serial number : "));
    Serial.println(buf);
  }

  // read Firmware level
  if (device.getFirmwareLevel(val))
  {
    Serial.print(F("SCD30 Firmware level: Major: "));
    Serial.print(val[0]);

    Serial.print(F("\t, Minor: "));
    Serial.println(val[1]);
  }
  else
  {
    Serial.println(F("Could not obtain firmware level"));
  }
}
