/****************************************************************************************************************************
  Portenta_H7_ISR_Servo.h
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

#ifndef Portenta_H7_ISR_Servo_H
#define Portenta_H7_ISR_Servo_H

#if ( ( defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_PORTENTA_H7_M4) ) && defined(ARDUINO_ARCH_MBED) )
  #warning Use MBED ARDUINO_PORTENTA_H7
#else
  #error This code is intended to run on the MBED ARDUINO_PORTENTA_H7 platform! Please check your Tools->Board setting.
#endif

#if !defined(PORTENTA_H7_ISR_SERVO_VERSION)
  #define PORTENTA_H7_ISR_SERVO_VERSION       "Portenta_H7_ISR_Servo v1.0.0"
#endif

#include <stddef.h>

#include <inttypes.h>

#if defined(ARDUINO)
  #if ARDUINO >= 100
    #include <Arduino.h>
  #else
    #include <WProgram.h>
  #endif
#endif

/////////////////////////////////////////////////////

#if defined(BOARD_NAME)
  #undef BOARD_NAME
#endif

#if defined(CORE_CM7)
  #warning Using Portenta H7 M7 core
  #define BOARD_NAME              "PORTENTA_H7_M7"
#else
  #warning Using Portenta H7 M4 core
  #define BOARD_NAME              "PORTENTA_H7_M4"
#endif

/////////////////////////////////////////////////////

#include "Portenta_H7_ISR_Servo_Debug.h"
#include "Portenta_H7_FastTimerInterrupt.h"

/////////////////////////////////////////////////////

#define PORTENTA_H7_MAX_PIN           NUM_DIGITAL_PINS
#define PORTENTA_H7_WRONG_PIN         255

/////////////////////////////////////////////////////

// From Servo.h - Copyright (c) 2009 Michael Margolis.  All right reserved.

#define MIN_PULSE_WIDTH         800       // the shortest pulse sent to a servo  
#define MAX_PULSE_WIDTH         2450      // the longest pulse sent to a servo 
#define DEFAULT_PULSE_WIDTH     1500      // default pulse width when servo is attached
#define REFRESH_INTERVAL        20000     // minumim time to refresh servos in microseconds 

extern void Portenta_H7_ISR_Servo_Handler();

/////////////////////////////////////////////////////

class Portenta_H7_ISR_Servo
{

  public:
    // maximum number of servos
    const static int MAX_SERVOS = 16;

    // constructor
    Portenta_H7_ISR_Servo();

    // destructor
    ~Portenta_H7_ISR_Servo()
    {
      if (Portenta_H7_ITimer)
      {
        Portenta_H7_ITimer->detachInterrupt();
        delete Portenta_H7_ITimer;
      }
    }

    /////////////////////////////////////////////////////

    void run();

    /////////////////////////////////////////////////////

    // useTimer select which timer (0-3) of Portenta_H7 to use for Servos
    //Return true if timerN0 in range
    bool useTimer(TIM_TypeDef* timerNo)
    {
      _timerNo = timerNo;
      return true;
    }

    /////////////////////////////////////////////////////

    // Bind servo to the timer and pin, return servoIndex
    int setupServo(uint8_t pin, int min = MIN_PULSE_WIDTH, int max = MAX_PULSE_WIDTH);

    // setPosition will set servo to position in degrees
    // by using PWM, turn HIGH 'duration' microseconds within REFRESH_INTERVAL (20000us)
    // returns true on success or -1 on wrong servoIndex
    bool setPosition(unsigned servoIndex, int position);

    // returns last position in degrees if success, or -1 on wrong servoIndex
    int getPosition(unsigned servoIndex);

    // setPulseWidth will set servo PWM Pulse Width in microseconds, correcponding to certain position in degrees
    // by using PWM, turn HIGH 'pulseWidth' microseconds within REFRESH_INTERVAL (20000us)
    // min and max for each individual servo are enforced
    // returns true on success or -1 on wrong servoIndex
    bool setPulseWidth(unsigned servoIndex, unsigned int pulseWidth);

    // returns pulseWidth in microsecs (within min/max range) if success, or 0 on wrong servoIndex
    unsigned int getPulseWidth(unsigned servoIndex);

    // destroy the specified servo
    void deleteServo(unsigned servoIndex);

    // returns true if the specified servo is enabled
    bool isEnabled(unsigned servoIndex);

    // enables the specified servo
    bool enable(unsigned servoIndex);

    // disables the specified servo
    bool disable(unsigned servoIndex);

    // enables all servos
    void enableAll();

    // disables all servos
    void disableAll();

    // enables the specified servo if it's currently disabled,
    // and vice-versa
    bool toggle(unsigned servoIndex);

    // returns the number of used servos
    int getNumServos();

    /////////////////////////////////////////////////////

    // returns the number of available servos
    int getNumAvailableServos()
    {
      return MAX_SERVOS - numServos;
    };

    /////////////////////////////////////////////////////

  private:

    // Use 10 microsecs timer, just fine enough to control Servo, normally requiring pulse width (PWM) 500-2000us in 20ms.
#define TIMER_INTERVAL_MICRO        10

    /////////////////////////////////////////////////////

    void init()
    {
      Portenta_H7_ITimer = new Portenta_H7_FastTimer(_timerNo);

      // Interval in microsecs
      if ( Portenta_H7_ITimer
           && Portenta_H7_ITimer->attachInterruptInterval(TIMER_INTERVAL_MICRO,
                                                          (portenta_H7_timer_callback) Portenta_H7_ISR_Servo_Handler ) )
      {
        ISR_SERVO_LOGERROR("Starting  ITimer OK");
      }
      else
      {
        ISR_SERVO_LOGERROR("Fail setup Portenta_H7_ITimer");
      }

      for (int servoIndex = 0; servoIndex < MAX_SERVOS; servoIndex++)
      {
        memset((void*) &servo[servoIndex], 0, sizeof (servo_t));
        servo[servoIndex].count    = 0;
        servo[servoIndex].enabled  = false;
        // Intentional bad pin
        servo[servoIndex].pin      = PORTENTA_H7_WRONG_PIN;
      }

      numServos   = 0;

      // Init timerCount
      timerCount  = 1;
    }

    /////////////////////////////////////////////////////

    // find the first available slot
    int findFirstFreeSlot();

    /////////////////////////////////////////////////////

    typedef struct
    {
      uint8_t       pin;                  // pin servo connected to
      unsigned long count;                // In microsecs
      int           position;             // In degrees
      bool          enabled;              // true if enabled
      uint16_t      min;
      uint16_t      max;
    } servo_t;

    /////////////////////////////////////////////////////

    volatile servo_t servo[MAX_SERVOS];

    // actual number of servos in use (-1 means uninitialized)
    volatile int numServos;

    // timerCount starts at 1, and counting up to (REFRESH_INTERVAL / TIMER_INTERVAL_MICRO) = (20000 / 10) = 2000
    // then reset to 1. Use this to calculate when to turn ON / OFF pulse to servo
    // For example, servo1 uses pulse width 1000us =>
    //    - turned ON when timerCount = 1
    //    - turned OFF when timerCount = 1000 / TIMER_INTERVAL_MICRO = 100
    volatile unsigned long timerCount;

    // For Portenta_H7 timer
    TIM_TypeDef*            _timerNo;
    Portenta_H7_FastTimer*  Portenta_H7_ITimer;
};

/////////////////////////////////////////////////////

extern Portenta_H7_ISR_Servo Portenta_H7_ISR_Servos;  // create servo object to control up to 16 servos

/////////////////////////////////////////////////////

#endif
