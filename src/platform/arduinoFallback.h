#ifndef ARDUINO_FALLBACK_H
#define ARDUINO_FALLBACK_H

// fall back to using Arduino regular logic, works for all single core boards. If we end up here for a multicore
// board then there may be problems. Here we are in full arduino mode (AVR, MKR etc).
#include <Arduino.h>
# define IOA_USE_ARDUINO
typedef uint8_t pintype_t;

// could we realistically still use std::atomic? If we can then switch to that.
#if __GNUC__ >= 5 && __has_include(<atomic>)
#include "wrapAtomic.h"
#else

// otherwise the final fallback is to use volatile and toggle interrupts.
namespace tm_internal {
    typedef TimerTask *volatile TimerTaskAtomicPtr;
    typedef volatile bool TmAtomicBool;
    typedef volatile uint32_t position_t;
    typedef volatile uint32_t* position_ptr_t;

    template <typename T> static bool atomicSwapAny(volatile T* ptr, T expected, T newValue) {
        bool ret = false;
        noInterrupts();
        if(*ptr == expected) {
            *ptr = newValue;
            ret = true;
        }
        interrupts();
        return ret;
    }
#define atomicSwapBool(ptr, expected, newValue) atomicSwapAny(ptr, expected, newValue)
#define atomicSwap32(ptr, expected, newValue) atomicSwapAny(ptr, expected, newValue)


    inline bool atomicReadBool(const volatile bool *ptr) {
        return *ptr;
    }

    inline bool atomicRead32(const volatile uint32_t *ptr) {
        return *ptr;
    }

    inline void atomicWriteBool(volatile bool *ptr, bool newVal) {
        *ptr = newVal;
    }

#if defined(__AVR__)
    inline void atomicWritePtr(TimerTaskAtomicPtr* pPtr, TimerTask* newValue) {
        noInterrupts();
        *pPtr = newValue;
        interrupts();
    }

    inline TimerTask* atomicReadPtr(TimerTaskAtomicPtr* pPtr) {
        noInterrupts();
        auto ptr = *pPtr;
        interrupts();
        return ptr;
    }
#else
    // all other supported Arduino boards are atomic for pointer types
    inline void atomicWritePtr(TimerTaskAtomicPtr* pPtr, TimerTask* newValue) {
        *pPtr = newValue;
    }

    inline TimerTask* atomicReadPtr(TimerTaskAtomicPtr* pPtr) {
        return *pPtr;
    }
#endif // AVR check for PTR atomicity

}

#endif

#endif

