/*
 * Copyright (c) 2018-present https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */


#ifndef TASKMANAGERIO_PLATFORMDETERMINATION_H
#define TASKMANAGERIO_PLATFORMDETERMINATION_H

#if defined(ARDUINO_ARDUINO_NANO33BLE)

// here we're in a hybrid of mbed and Arduino basically. We treat all abstractions as Arduino though.
#include <Arduino.h>
# define IOA_USE_ARDUINO
# define IOA_ARDUINO_MBED
typedef uint32_t pinid_t;

class TaskLocker {
public:
    TaskLocker() {
        noInterrupts();
    }

    ~TaskLocker() {
        interrupts();
    }
};


#elif defined(__MBED__)

// here we are in full mbed mode
# define IOA_USE_MBED
#include <mbed.h>
#include <rtos.h>
typedef uint32_t pinid_t;
#define IOA_OVERRIDE_PIN_FUNCTIONS

class TaskLocker {
private:
    static Mutex taskMutex;
public:
    TaskLocker() {
        taskMutex.lock();
    }

    ~TaskLocker() {
        taskMutex.unlock();
    }
};

#else

// here we are in full arduino mode (AVR, MKR, ESP etc).
# define IOA_USE_ARDUINO
#include <Arduino.h>
typedef uint8_t pinid_t;

class TaskLocker {
public:
    TaskLocker() {
        noInterrupts();
    }

    ~TaskLocker() {
        interrupts();
    }
};

#endif // __MBED__

//
// DEFAULT_TASK_SIZE definition:
// TaskManagerIO will re-allocate the task array if needed. So there's little need to adjust this for most use cases.
// For AVR boards: MEGA 2560 has a default size of 10, others have a default size of 6.
// For all other boards the default size is 16. You can change it by defining DEFAULT_TASK_SIZE yourself.
//
#ifndef DEFAULT_TASK_SIZE
#ifdef __AVR_ATmega2560__
# define DEFAULT_TASK_SIZE 10
#elif defined(__AVR__)
# define DEFAULT_TASK_SIZE 6
#else
# define DEFAULT_TASK_SIZE 16
#endif // platform
#endif // DEFAULT_TASK_SIZE

#endif //TASKMANGERIO_PLATFORMDETERMINATION_H
