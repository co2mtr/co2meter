#ifndef PMMUTEX_H
#define PMMUTEX_H
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
#include <ESP.h>

/**
 * This is a simple Mutex implementation for ESP8266. Global interrupt enabling
 * and disabling is used. w/o any other special measures. Hence this code should
 * not be used in cases, where the hardware has multiple cores with caches 
 * and/or separate interrupt subsystems. 
 * Mutexes can not be copied, assigned or transferred.
 */
class Mutex
{
public:
    Mutex() = default;
    ~Mutex() = default;

    Mutex(const Mutex &) = delete;
    Mutex &operator=(const Mutex &) = delete;

    void lock()
    {
        noInterrupts();
        while (_handle == true)
        {
            interrupts();
            delay(1);
            noInterrupts(); // stop interrupts, when accessing the mutex
        }
        _handle = true;
        interrupts();
    }

    bool try_lock()
    {
        bool success = false;
        noInterrupts();
        if (_handle == false)
        {
            _handle = true;
            success = true;
        }
        interrupts();
        return success;
    }

    void unlock()
    {
        noInterrupts();
        _handle = false;
        interrupts();
    }

    bool isLocked()
    {
        return _handle;
    }

private:
    bool _handle;
};

/**
 * The Lock guard class can be used within block scope to lock and automatically
 * unlock a mutex. This provides an exception safe implementation for resource
 * locking. 
 * LockGuards can not be copied, assigned or transferred
 */
template <class _Mutex = Mutex>
class LockGuard
{
public:
    LockGuard(_Mutex &t) : _t(t)
    {
        _t.lock();
    }

    ~LockGuard()
    {
        _t.unlock();
    }
    LockGuard(const LockGuard &) = delete;
    LockGuard &operator=(const LockGuard &) = delete;

private:
    _Mutex &_t;
};
#endif
