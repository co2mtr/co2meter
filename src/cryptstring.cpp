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
#include "cryptstring.h"

AESLib aesLib;

// AES Encryption Key
byte aes_key[] = {
#include "../include/secret.h"
};

//  0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30

// General initialization vector (you must use your own IV's in production for full security!!!)
byte aes_iv[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Generate IV (once)
void aes_init()
{
  aesLib.gen_iv(aes_iv);
  aesLib.set_paddingmode((paddingMode)0);
}

String encrypt(char *msg, byte iv[])
{
  int msgLen = strlen(msg);
  char encrypted[2 * msgLen + 24]; // this must be at least two times the message size +24 Bytes as an empty message encodes to 24 bytes
  aesLib.encrypt64(msg, msgLen, encrypted, aes_key, sizeof(aes_key), iv);

  return String(encrypted);
}

String decrypt(char *msg, byte iv[])
{
  int msgLen = strlen(msg);
  char decrypted[msgLen / 2];
  aesLib.decrypt64(msg, msgLen, decrypted, aes_key, sizeof(aes_key), iv);

  return String(decrypted);
}

void aes_begin()
{
  aes_init();
}

/* non-blocking wait function */
//void wait(unsigned long milliseconds) {
//  unsigned long timeout = millis() + milliseconds;
// while (millis() < timeout) {
//   yield();
// }
//}

//unsigned long loopcount = 0;

String getCryptString(String input)
{
  byte enc_iv[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // const_cast is uncritical as string is on stack
  return encrypt(const_cast<char *>(input.c_str()), enc_iv);
}

String getClearText(String input)
{
  byte dec_iv[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  // const_cast is uncritical as string is on stack
  return decrypt(const_cast<char *>(input.c_str()), dec_iv);
}
