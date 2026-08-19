/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */


#ifndef TASKMANAGERIO_PLATFORMDETERMINATION_H
#define TASKMANAGERIO_PLATFORMDETERMINATION_H

/**
 * @file TaskPlatformDeps.h
 * @brief provides the platform specific configuration for task manager
 */

class TimerTask;

// You can add your own local definitions header file here, this enables you to adjust build flags in environments
// where there is no easy way to do so with compiler options. Just create an include file "io_local_definitions.h"
// at the top level of your project source tree. This file will be honoured by all our libraries.
#if defined __has_include
#  if __has_include ("zio_local_definitions.h")
#    include "zio_local_definitions.h"
#  endif
#endif // has include "io_local_definitions"

// when not on mbed, we need to load Arduino.h to get the right defines for some boards.
#if defined(BUILD_FOR_PICO_CMAKE)
#include <pico/stdlib.h>
#include <valarray>
#elif !defined(__MBED__) && !defined(BUILD_FOR_STM32CUBE_CMAKE)
#include <Arduino.h>
#endif

#if defined(__MBED__) && !defined(ARDUINO_PICO_REVISION)
#include "platform/mbedCas.h"
#elif defined(ESP8266) || defined(ESP32) || defined(ARDUINO_PICO_REVISION)
typedef uint8_t pintype_t;
# define IOA_USE_ARDUINO
#if defined(ESP8266) || defined(ARDUINO_PICO_REVISION)
#include "platform/wrapAtomic.h"
#else
#endif
#include "platform/idfRtosCas.h"
#elif defined(BUILD_FOR_PICO_CMAKE)
#include "platform/picoCritical.h"
#elif defined(BUILD_FOR_STM32CUBE_CMAKE)
#include "platform/stmCubeCas.h"
#else
#include "platform/arduinoFallback.h"
#endif // All platform checks

// for all mbed and ESP boards we already enable lambda captures, SAMD is a known extra case that works.
// we can only enable on larger boards with enough memory to take the extra size of the structures.
#if defined(TM_ENABLE_CAPTURED_LAMBDAS) && defined(ARDUINO_ARCH_SAMD)
# define TM_ALLOW_CAPTURED_LAMBDA
#endif

//
// Scheduling size. On all boards by default task manager uses 32 bit schedule data to make it more general purpose.
// Note that even on 8 bit boards, all the math still needs to be 32 bit to deal with times, so there is very little
// to no performance gain by doing this.
//
// If you need the few extra bytes back, and can live with 16 bit schedule values then define TM_FORCE_16BIT_SCHEDULER
//
#if defined(TM_FORCE_16BIT_SCHEDULER)
typedef uint16_t sched_t;
#else
typedef uint32_t sched_t;
#endif // TM_FORCE_16BIT_SCHEDULER

//
// DEFAULT_TASK_SIZE definition:
// TaskManagerIO will re-allocate the task array if needed. So there's little need to adjust this for most use cases.
// For AVR boards: MEGA 2560 has a default size of 10, others have a default size of 6.
// For all other boards the default size is 16. You can change it by defining DEFAULT_TASK_SIZE yourself.
//
#ifndef DEFAULT_TASK_SIZE
#ifdef __AVR_ATmega2560__
# define DEFAULT_TASK_SIZE 12
# define DEFAULT_TASK_BLOCKS 8
#elif defined(__AVR__)
# define DEFAULT_TASK_SIZE 6
# define DEFAULT_TASK_BLOCKS 4
#else
# define DEFAULT_TASK_SIZE 16
# define DEFAULT_TASK_BLOCKS 16
#endif // platform
#else
#ifndef DEFAULT_TASK_BLOCKS
#define DEFAULT_TASK_BLOCKS 8
#endif // DEFAULT_TASK_BLOCKS not defined when task size is
#endif // DEFAULT_TASK_SIZE defined already

//
// Here we define an attribute needed for interrupt support on ESP8266 and ESP32 boards, any interrupt code that is
// going to run on these boards should be marked with this attribute.
//
#undef ISR_ATTR
#if defined(ESP8266) || defined(ESP32)
# define ISR_ATTR IRAM_ATTR
#else
# define ISR_ATTR
#endif

//
// Here we have one last go at determining if we should enable capture lambdas by checking if the functional include
// is available, we only do so if we are on GCC > 5
//
# if !defined(TM_ALLOW_CAPTURED_LAMBDA) && defined(TM_ENABLE_CAPTURED_LAMBDAS) && __GNUC__ >= 5
#if __has_include(<functional>)
# define TM_ALLOW_CAPTURED_LAMBDA
#endif // _has_include
#endif // GCC>=5 and !TM_ALLOW_CAPTURED_LAMBDA

#ifndef internal_min
#define internal_min(a, b)  ((a) > (b) ? (b) : (a))
#endif // internal_min

#ifndef internal_max
#define internal_max(a, b)  ((a) < (b) ? (b) : (a));
#endif // internal_max

#endif //TASKMANGERIO_PLATFORMDETERMINATION_H
