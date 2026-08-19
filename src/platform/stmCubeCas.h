
#ifndef TASKMANAGER_IO_STM_CUBE_CAS_H
#define TASKMANAGER_IO_STM_CUBE_CAS_H

#include <stdint.h>
#include <cstddef>

#include "cmsis_os.h"

#define PROGMEM

#if defined(TCLOG_STM32_HAL_INCLUDE)
#include TCLOG_STM32_HAL_INCLUDE
#elif __has_include("main.h")
#include "main.h"
#else
#error "STM32Cube needs either TCLOG_STM32_HAL_INCLUDE or a visible main.h"
#endif

extern "C" {
    extern uint32_t millis();
    extern uint32_t micros();
    extern void yield();
}
#define BOARD_SUPPORTS_PROPER_CAS
typedef uint32_t pintype_t;;
#include "platform/wrapAtomic.h"

/** Basic implementation of delayMicroseconds for STM32Cube HAL, creates a tight loop and is only suitable for
 * short delays
 *
 * @param us Number of microseconds to delay
 */
inline void delayMicroseconds(uint32_t us) {
    const uint32_t start = micros();
    while (micros() - start < us);
}

inline void delay(uint32_t m) {
    osDelay(m);
}

#endif
