#ifndef TASKMANAGER_IO_PICO_CRITICAL_H
#define TASKMANAGER_IO_PICO_CRITICAL_H

#include <pico/critical_section.h>
#include <atomic>
#include <cstdint>

typedef uint8_t pintype_t;

#ifdef BUILD_FOR_PICO_CMAKE
#define delay(x) busy_wait_us_32(x)
#else
#define IOA_USE_ARDUINO
#endif

#if defined(TM_ENABLE_CAPTURED_LAMBDAS)
#define TM_ALLOW_CAPTURED_LAMBDA
#endif

#define PICO_NEEDS_PROTECTOR

/*
 * Because the pico is M0 level cortex it does not support CAS natively, even though it would really benefit from it
 * being dual core. This means we have to emulate it in a safe way that's compatible with both cores. We do this by
 * creating a critical section lock, and a RAII style method of accessing it.
 */

class TmCriticalSectionPico {
protected:
    critical_section_t theLock;
public:
    explicit TmCriticalSectionPico() : theLock() {
        critical_section_init(&theLock);
    }
    [[nodiscard]] critical_section_t* getLock() { return &theLock; }
};

extern TmCriticalSectionPico globalPicoCs;

class TmPicoProtector {
    TmCriticalSectionPico myLock;
public:
    TmPicoProtector() {
        myLock = globalPicoCs;
        critical_section_enter_blocking(myLock.getLock());
    }

    ~TmPicoProtector() {
        critical_section_exit(myLock.getLock());
    }
};

#include "wrapAtomic.h"

#endif
