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

    inline bool atomicReadBool(const volatile bool *ptr) {
        return *ptr;
    }

    inline void atomicWriteBool(volatile bool *ptr, bool newVal) {
        *ptr = newVal;
    }

#if defined(__AVR__)
#include <util/atomic.h>
    template <typename T> static bool atomicSwapAny(volatile T* ptr, T expected, T newValue) {
        bool ret = false;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            if(*ptr == expected) {
                *ptr = newValue;
                ret = true;
            }
        }
        return ret;
    }

    inline void atomicWritePtr(TimerTaskAtomicPtr* pPtr, TimerTask* newValue) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            *pPtr = newValue;
        }
    }

    inline TimerTask* atomicReadPtr(TimerTaskAtomicPtr* pPtr) {
        TimerTask* val;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            val = *pPtr;
        }
        return val;
    }

    inline uint32_t atomicRead32(const volatile uint32_t *ptr) {
        uint32_t val;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
             val = *ptr;
        }
        return val;
    }

#else
#include "EmulatedAtomicBlock.h"
    inline void atomicWritePtr(TimerTaskAtomicPtr* pPtr, TimerTask* newValue) {
        *pPtr = newValue;
    }

    inline TimerTask* atomicReadPtr(TimerTaskAtomicPtr* pPtr) {
        return *(volatile TimerTask* const volatile *)pPtr;
    }

    inline uint32_t atomicRead32(const volatile uint32_t *ptr) {
        return *ptr;
    }
    template <typename T> static bool atomicSwapAny(volatile T* ptr, T expected, T newValue) {
        bool ret = false;
        {
            EmulatedAtomicBlock ib;
            if(*ptr == expected) {
                *ptr = newValue;
                ret = true;
            }
        }
        return ret;
    }
#endif // AVR check for PTR atomicity

}

#define atomicSwapBool(ptr, expected, newValue) atomicSwapAny(ptr, expected, newValue)
#define atomicSwap32(ptr, expected, newValue) atomicSwapAny(ptr, expected, newValue)

#endif

#endif

