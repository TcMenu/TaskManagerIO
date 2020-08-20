/*
 * Copyright (c) 2018-present https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */


#ifndef TASKMANAGERIO_PLATFORMDETERMINATION_H
#define TASKMANAGERIO_PLATFORMDETERMINATION_H

class TimerTask;

#if defined(__MBED__) || defined(ARDUINO_ARDUINO_NANO33BLE)

// check if this is Arduino mbed or regular mbed.
#if defined(ARDUINO_ARDUINO_NANO33BLE)
# define IOA_USE_ARDUINO
# include "Arduino.h"
#else
# define IOA_USE_MBED
# include "mbed.h"
#endif

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
#elif defined(ESP8266) || defined(ESP32)
#include "Arduino.h"
#include "stdatomic.h"
typedef uint8_t pintype_t;
# define IOA_USE_ARDUINO


namespace tm_internal {
    typedef _Atomic(TimerTask*) TimerTaskAtomicPtr;
    typedef atomic_bool TmAtomicBool;

    /**
     * Sets the boolean to the new value ONLY when the existing value matches expected.
     * @param ptr the bool memory location to compare / swap
     * @param expected the expected value
     * @param newValue the replacement, replaced on if expected matches
     * @return true if the replacement was done, otherwise false
     */
    inline bool atomicSwapBool(TmAtomicBool *ptr, bool expected, bool newValue) {
        auto ret = false;
        noInterrupts();
        if(atomic_load(ptr) == expected) {
            atomic_store(ptr, newValue);
            ret = true;
        }
        interrupts();
        return ret;
    }

    /**
     * Reads an atomic boolean value
     * @param pPtr the pointer to an atomic boolean value
     * @return the boolean value.
     */
    inline bool atomicReadBool(TmAtomicBool *pPtr) {
        return atomic_load(pPtr);
    }

    /**
     * Writes a boolean value atomically
     * @param pPtr the atomic ref
     * @param newVal the new value
     */
    inline void atomicWriteBool(TmAtomicBool *pPtr, bool newVal) {
        atomic_store(pPtr, newVal);
    }

    /**
     * Dereferences and returns the value of the pointer at ptr type. On mbed boards this is already an atomic operation and
     * therefore volatile is enough.
     * @tparam PTR_TYPE class type of the pointer
     * @param pPtr reference to memory of the pointer
     * @return the pointer.
     */
    inline TimerTask *atomicReadPtr(TimerTaskAtomicPtr *pPtr) {
        return atomic_load(pPtr);
    }

    /**
     * Dereferences and then sets the memory of the pointer type. On mbed boards this is already an atomic operation
     * @tparam PTR_TYPE
     * @param pPtr
     * @param newValue
     */
    inline void atomicWritePtr(TimerTaskAtomicPtr *pPtr, TimerTask *newValue) {
        atomic_store(pPtr, newValue);
    }
}

#else // fall back to using Arduino regular logic.

// here we are in full arduino mode (AVR, MKR, ESP etc).
# define IOA_USE_ARDUINO
#include <Arduino.h>
typedef uint8_t pintype_t;

namespace(tm_internal) {
typedef TimerTask volatile* TimerTaskAtomicPtr;
typedef volatile bool TmAtomicBool;

    static bool atomicSwapBool(volatile bool* ptr, bool expected, bool newValue) {
        bool ret = false;
        noInterrupts();
        if(*ptr == expected) {
            *ptr = newValue;
            ret = true;
        }
        interrupts();
        return ret;
    }

    inline bool atomicReadBool(volatile bool *ptr) {
        return *ptr;
    }

    inline void atomicWritePtr(TimerTaskAtomicPtr* pPtr, TimerTask* newValue) {
        noInterrupts();
        *pPtr = newValue;
        interrupts();
    }

    inline TimerTask* atomicReadPtr(TimerTaskAtomicPtr* pPtr) {
        noInterrupts();
        auto ptr = *ptr;
        interrupts();
        return ptr;
    }
}

#endif // __MBED__

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
#endif // DEFAULT_TASK_SIZE

namespace tm_internal {
    enum TmErrorCode {
        TM_INFO_REALLOC = 1,
        TM_ERROR_LOCK_FAILURE = 100,
        TM_WARN_HIGH_SPINCOUNT,
        TM_ERROR_FULL
    };
    static void (*loggingDelegate)(TmErrorCode code, int task) = nullptr;
    inline void tmNotification(TmErrorCode code, int task) {
        if(loggingDelegate) loggingDelegate(code, task);
    }
}

#endif //TASKMANGERIO_PLATFORMDETERMINATION_H
