#ifndef TASKMANAGER_IO_MBED_RTOS_CAS_H
#define TASKMANAGER_IO_MBED_RTOS_CAS_H
#include "stmCubeCas.h"

#if defined(TM_ENABLE_CAPTURED_LAMBDAS)
#define TM_ALLOW_CAPTURED_LAMBDA
#endif

// check if this is Arduino mbed or regular mbed.
// list of devices is pulled from https://github.com/arduino/ArduinoCore-mbed/blob/master/full.variables
// set TMIOA_FORCE_ARDUINO_MBED to force IoAbstraction to use Arduino-mbed mode.
#if defined(ARDUINO_NANO_RP2040_CONNECT) || \
    defined(ARDUINO_ARDUINO_NANO33BLE) || \
    defined(ARDUINO_RASPBERRY_PI_PICO) || \
    defined(ARDUINO_PORTENTA_H7_M7) || \
    defined(ARDUINO_PORTENTA_H7_M4) || \
    defined(ARDUINO_EDGE_CONTROL) || \
    defined(ARDUINO_NICLA) || \
    defined(ARDUINO_NICLA_VISION) || \
    defined(TMIOA_FORCE_ARDUINO_MBED) || \
    defined(ARDUINO_ARCH_MBED)
# define IOA_USE_ARDUINO
# define ARDUINO_MBED_MODE
# include "Arduino.h"
# define IOA_MULTITHREADED
#include "rtos/rtos.h"
inline void* getCurrentThreadId() { return rtos::ThisThread::get_id(); }
#else
# define IOA_USE_MBED
# include "mbed.h"
# define IOA_MULTITHREADED
inline void* getCurrentThreadId() { return ThisThread::get_id(); }

# if !defined(PIO_NEEDS_RTOS_WORKAROUND)
#  include "rtos.h"
# endif // PIO_NEED_RTOS_WORKAROUND
#endif // mbed and arduino-mbed checks

#include <mbed_atomic.h>
typedef uint32_t pintype_t;

namespace tm_internal {
    typedef TimerTask* volatile TimerTaskAtomicPtr;
    typedef volatile bool TmAtomicBool;

    /**
     * Sets the boolean to the new value ONLY when the existing value matches expected.
     * @param ptr the bool memory location to compare / swap
     * @param expected the expected value
     * @param newValue the replacement, replaced on if expected matches
     * @return true if the replacement was done, otherwise false
     */
    inline bool atomicSwapBool(volatile bool *ptr, bool expected, bool newValue) {
        return core_util_atomic_cas_bool(ptr, &expected, newValue);
    }

    /**
     * Reads the value in an atomic boolean object
     */
    inline bool atomicReadBool(TmAtomicBool *pPtr) {
        return *pPtr;
    }

    inline void atomicWriteBool(TmAtomicBool *pPtr, bool newVal) {
        *pPtr = newVal;
    }

    /**
     * Dereferences and returns the value of the pointer at ptr type. On mbed boards this is already an atomic operation and
     * therefore volatile is enough.
     * @tparam PTR_TYPE class type of the pointer
     * @param pPtr reference to memory of the pointer
     * @return the pointer.
     */
    inline TimerTask *atomicReadPtr(TimerTaskAtomicPtr *pPtr) {
        return *pPtr;
    }

    /**
     * Dereferences and then sets the memory of the pointer type. On mbed boards this is already an atomic operation
     * @tparam PTR_TYPE
     * @param pPtr
     * @param newValue
     */
    inline void atomicWritePtr(TimerTaskAtomicPtr *pPtr, TimerTask * volatile newValue) {
        //core_util_atomic_store_ptr((void* volatile*)pPtr,  newValue);
        *pPtr = newValue;
    }
}

inline void delay(uint32_t ms) {
    delayMicroseconds(ms * 1000);
}

#endif
