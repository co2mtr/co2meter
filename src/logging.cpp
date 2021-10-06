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
#include <Arduino.h>
#include "logging.h"
#include "config.h"
extern config conf;

void logger::begin(logrecord *_r, uint16_t length)
{
  logmsg(LOGLOGGING, "Starting Logger-%p with %i entries\n", this, length);
  current = 0;
  lasttime = 0;
  numberOfValues = 0;
  quant = 1;
  starttime = 0;

  arrayLength = length;
  r = _r;

  for (uint16_t i = 0; i < arrayLength; i++)
  {
    r[i].timestamp = 0;
    r[i].co2value = 0;
    stpncpy(r[i].label, "", LOGLABELSIZE);
  }
}

int logger::getPrev(uint16_t prev)
{
  int v;
  if (prev < 1)
    v = arrayLength - 1;
  else
    v = prev - 1;
  return v;
}

int logger::getNext(uint16_t next)
{
  int v;
  if (next == arrayLength - 1)
    v = 0;
  else
    v = next + 1;
  return v;
}

int logger::getLastXAverage(uint16_t timeval)
{
  int sum = 0, v = 0, time = 0;
  uint16_t j = numberOfValues;
  for (uint16_t k = correctmod(current - 1, arrayLength); j > 0; k = getPrev(k))
  {
    j--;
    sum += r[k].co2value;
    v++;
    int timediff = r[k].timestamp - r[getPrev(k)].timestamp;
    time += timediff;
    //Serial.printf("***MINUTE AVERAGE %i: %i---%i---%i---%i---%i\n\n" ,k, r[k].timestamp, r[getPrev(k)].timestamp, timediff, sum, time);
    if (time >= timeval)
      break;
  }
  if (timeval > time)
    return 0;
  return (int)sum / v;
}

void logger::log(int value)
{
  now = millis();

  if (starttime == 0)
    starttime = now;

  logmsg(LOGLOGGING, "New Log[%p] entry: %i -- %i\n", (void *)this, current, value);

  if (lasttime == 0)
  {
    r[current].timestamp = 0;
  }
  else
  {
    r[current].timestamp = (int)(now - starttime) / 1000; // timestamp in seconds
  }

  //Serial.printf("Quantizing %i\n", quant);

  int qval = value % quant;
  value -= qval;
  if (qval > quant / 2)
    value += quant;

  r[current].co2value = value;

  lasttime = now;

  current = getNext(current);
  if (numberOfValues < arrayLength)
    ++numberOfValues;
}

// workaorund for correct modulo calculations for negative inputs
int logger::correctmod(int a, int b)
{
  int c = a % b;
  return (c < 0) ? c + b : c;
}

String logger::getValueList()
{
  if (numberOfValues < 1)
    return "";
  String data = "\"data\": [";
  String labels;
  if (conf.getShowRTC())
  {
    labels = "\"labels\": [";
  }
  else
  {
    labels = "";
  }

  int start = correctmod((current - numberOfValues), arrayLength);
  unsigned int i = 0;
  //Serial.printf("Value list: current %i, start %i, numberOfValues %i\n", current, start, numberOfValues);

  int maxValues = numberOfValues;

  unsigned int offset = 0;
  if (numberOfValues < maxValues)
    maxValues = numberOfValues;
  for (unsigned int j = 0; j < maxValues; j = j + 1)
  {
    i = (start + j) % arrayLength;
    data = data + String(r[i].co2value) + (j + 1 < maxValues ? ", " : "");
    String xtick = "";
    if (conf.getShowRTC())
    {
      xtick = r[i].label;
    }
    else
    {
      int n1 = ((((float)r[i].timestamp) / 60) * 100);
      float n2 = ((float)n1 / 100.0);
      xtick = "t+" + String(n2) + "";
    }
    if (j % 4 == 0)
    {
      labels = labels + "\"" + xtick + "\"" + (j + 1 < maxValues ? ", " : "");
    }
    else
    {
      labels = labels + "\"\"" + (j + 1 < maxValues ? ", " : "");
    }
  }

  data += "]";
  if (conf.getShowRTC())
  {
    labels += "]";
  }
  else
  {
    labels = "\"labels\": [" + labels + "]";
  }

  String list = "{" + data + ", " + labels + "}";
  return list;
}

String logger::getNextValue()
{
  String xtick;
  int index = correctmod(current - 1, arrayLength);
  if (conf.getShowRTC())
  {
    xtick = r[index].label;
  }
  else
  {
    int n1 = ((((float)r[index].timestamp) / 60) * 100);
    float n2 = ((float)n1 / 100.0);
    xtick = "t+" + String(n2) + "";
  }
  return String("{ \"data\": " + String(r[index].co2value) + ", \"labels\": \"" + xtick + "\" }");
}

void logger::stamp(String l)
{
  stpncpy(r[correctmod(current - 1, arrayLength)].label, l.c_str(), LOGLABELSIZE);
}

void logger::printLog()
{
  if (current > 0)
    for (unsigned int i = current; i > 0; i--)
    {
      dbgmsg("Log %i: %i-%i\n", i, r[i].timestamp, r[i].co2value);
    }
}
