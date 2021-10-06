#ifndef __LOGGER_H__
#define __LOGGER_H__
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

#define MAXLOG 300
#define FIVEMIN 60

#define LOGLABELSIZE 15

typedef struct
{
  uint16_t timestamp;
  uint16_t co2value;
  char label[LOGLABELSIZE];
} logrecord;

class logger
{

  uint16_t arrayLength;

  logrecord *r; //[MAXLOG];
  uint16_t current;
  uint16_t numberOfValues;
  unsigned long lasttime;
  unsigned long now;
  unsigned long starttime;

  /* quantization level */
  uint8_t quant;

public:
  //  logger(){}
  // ~logger() {
  //   free(r);
  //   free(label); }

  void setQuantization(uint8_t q)
  {
    quant = q;
  }

  void begin(logrecord *, uint16_t);
  void storeValuesToFlash(){};

  int getPrev(uint16_t);
  int getNext(uint16_t);
  int getLastXAverage(uint16_t);
  int correctmod(int, int);

  int getMinuteAverage()
  {
    return getLastXAverage(60);
  }

  int getFiveMinutesAverage()
  {
    return getLastXAverage(300);
  }

  void log(int);

  String getValueList();
  String getNextValue();

  void stamp(String);
  void printLog();
};

#endif /*__LOGGER_H__*/
