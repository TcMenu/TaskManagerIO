#ifndef TASKMANAGER_IO_PICO_CRITICAL_H
#define TASKMANAGER_IO_PICO_CRITICAL_H

#include <pico/critical_section.h>

#if defined(TM_ENABLE_CAPTURED_LAMBDAS)
#define TM_ALLOW_CAPTURED_LAMBDA
#endif

typedef uint8_t pintype_t;
namespace tm_internal {
    typedef TimerTask *volatile TimerTaskAtomicPtr;
    typedef volatile bool TmAtomicBool;
    extern critical_section_t* tmLock;
    void initPicoTmLock();

    static bool atomicSwapBool(volatile bool *ptr, bool expected, bool newValue) {
        bool ret = false;
        critical_section_enter_blocking(tmLock);
        if(*ptr == expected) {
            *ptr = newValue;
            ret = true;
        }
        critical_section_exit(tmLock);
        return ret;
    }

    static void atomicWriteBool(volatile bool *ptr, bool val) {
        critical_section_enter_blocking(tmLock);
        *ptr = val;
        critical_section_exit(tmLock);
    }

    inline bool atomicReadBool(volatile bool *ptr) {
        bool ret = false;
        critical_section_enter_blocking(tmLock);
        ret = *ptr;
        critical_section_exit(tmLock);
        return ret;
    }

    inline void atomicWritePtr(TimerTaskAtomicPtr *ptr, TimerTask *newVal) {
        *ptr = newVal;
    }

    inline TimerTask *atomicReadPtr(TimerTaskAtomicPtr *ptr) {
        return *ptr;
    }
}

#define delay(x) busy_wait_us_32(x)

#endif
