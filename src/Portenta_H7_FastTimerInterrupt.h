/****************************************************************************************************************************
  Portenta_H7_FastTimerInterrupt.h
  For Portenta_H7 boards using Arduino-mbed mbed_portenta core
  Written by Khoi Hoang

  Built by Khoi Hoang https://github.com/khoih-prog/Portenta_H7_ISR_Servo
  Licensed under MIT license

  Based on SimpleTimer - A timer library for Arduino.
  Author: mromani@ottotecnica.com
  Copyright (c) 2010 OTTOTECNICA Italy

  Based on BlynkTimer.h
  Author: Volodymyr Shymanskyy

  Version: 1.0.0

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0   K Hoang      16/09/2021 15/09/2021 Initial coding for Portenta_H7
 *****************************************************************************************************************************/

#pragma once

#ifndef Portenta_H7_FastTimerInterrupt_h
#define Portenta_H7_FastTimerInterrupt_h

#if !( ( defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_PORTENTA_H7_M4) ) && defined(ARDUINO_ARCH_MBED) )
  #error This code is intended to run on the MBED ARDUINO_PORTENTA_H7 platform! Please check your Tools->Board setting.
#endif

#if !defined(PORTENTA_H7_ISR_SERVO_VERSION)
  #define PORTENTA_H7_ISR_SERVO_VERSION       "Portenta_H7_ISR_Servo v1.0.0"
#endif

///////////////////////////////////////////

#include "stm32/HardwareTimer.h"

///////////////////////////////////////////

#include "Portenta_H7_ISR_Servo_Debug.h"

#if defined(TIMER_SERVO)
  #define DEFAULT_PORTENTA_H7_TIMER_NO          TIMER_SERVO       // TIM7 for many boards
#else
  #define DEFAULT_PORTENTA_H7_TIMER_NO          TIM7              // TIM7 for many boards
#endif

// TIMER_NUM defined in stm32/timer.h
#define MAX_PORTENTA_H7_NUM_TIMERS      TIMER_NUM

class Portenta_H7_FastTimerInterrupt;

typedef Portenta_H7_FastTimerInterrupt Portenta_H7_FastTimer;

typedef void (*portenta_H7_timer_callback)  ();

/////////////////////////////////////////////////////

class Portenta_H7_FastTimerInterrupt
{
  private:

    TIM_TypeDef*    _timer;
    HardwareTimer*  _hwTimer = NULL;

    float             _frequency;       // Timer frequency
    uint64_t          _timerCount;      // count to activate timer

    portenta_H7_timer_callback _callback;        // pointer to the callback function

    /////////////////////////////////////////////////////

  public:

    /////////////////////////////////////////////////////

    Portenta_H7_FastTimerInterrupt(TIM_TypeDef* timer = DEFAULT_PORTENTA_H7_TIMER_NO)
    {
      _timer = timer;

      _hwTimer = new HardwareTimer(_timer);

      _frequency  = 0;
      _timerCount = 0;
      _callback = NULL;
    };

    /////////////////////////////////////////////////////

    ~Portenta_H7_FastTimerInterrupt()
    {
      if (_hwTimer)
        delete _hwTimer;
    }

    /////////////////////////////////////////////////////

    // frequency (in hertz)
    // No params and duration now. To be added in the future by adding similar functions here
    bool setFrequency(float frequency, portenta_H7_timer_callback callback)
    {
      // select timer frequency is 1MHz for better accuracy. We don't use 16-bit prescaler for now.
      // Will use later if very low frequency is needed.
      _frequency  = 1000000;
      _timerCount = (uint32_t) _frequency / frequency;

      ISR_SERVO_LOGERROR1(F("Portenta_H7_FastTimerInterrupt: Timer Input Freq (Hz) ="), _hwTimer->getTimerClkFreq());
      ISR_SERVO_LOGERROR3(F("Frequency ="), _frequency, F(", _count ="), (uint32_t) (_timerCount));


      _hwTimer->setCount(0, MICROSEC_FORMAT);
      _hwTimer->setOverflow(_timerCount, MICROSEC_FORMAT);

      _hwTimer->attachInterrupt(callback);
      _hwTimer->resume();

      return true;
    }

    /////////////////////////////////////////////////////

    // interval (in microseconds) and duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    // No params and duration now. To be addes in the future by adding similar functions here
    bool attachInterruptInterval(unsigned long interval, portenta_H7_timer_callback callback)
    {
      return setFrequency( (float) ( 1000000.0f / interval), callback);
    }

    /////////////////////////////////////////////////////

    void detachInterrupt()
    {
      _hwTimer->detachInterrupt();
    }

    /////////////////////////////////////////////////////

    // Duration (in milliseconds). Duration = 0 or not specified => run indefinitely
    void reattachInterrupt()
    {
      setFrequency(_frequency, _callback);
    }
}; // class Portenta_H7_FastTimerInterrupt

/////////////////////////////////////////////////////

#endif      // Portenta_H7_FastTimerInterrupt_h
