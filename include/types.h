#ifndef __TYPES_H__
#define __TYPES_H__
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
#define MAX_TIMER 13

typedef enum
{
  NONE,
  MHZ,
  SCD,
  S8
} sensor_type;

typedef enum
{
  INIT,              /**< device in initialising mode */
  INIT_WAIT,         /**< time out for init state */
  ACCESSPOINT,       /**< at startup, the device offers an accespoint */
  ACCESSPOINT_WAIT,  /**< time out for accesspoint mode */
  WIFI_CONNECT,      /**< connect to a WIFI accesspoint */
  WIFI_CONNECT_WAIT, /**< time out for WIFI mode */
  MQTT_CONNECT,      /**< connect to a MQTT broker */
  MQTT_CONNECT_WAIT, /**< time out for MQTT mode */
  SENSE,             /**< sense state; update sensor data */
  PUBLISH,           /**< publish state; publish data via different cues */
  LOGGING,           /**< logging state; log the data to internal log buffer */
  CALIBRATE,         /**< calibrate state; the sensor is now being calibrated */
  TIME,              /**< time state; update from real-time clock */
  LAST               /**< no states beyond this point. This is used for size calculations */
} states;

#endif
