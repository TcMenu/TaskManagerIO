
#ifndef TASKMANAGER_IO_NATIVE_H
#define TASKMANAGER_IO_NATIVE_H

#include <stdint.h>
#include <cstddef>
#include <thread>

#define PROGMEM
unsigned long millis();
unsigned long micros();

inline void yield() {std::this_thread::yield();}
#define BOARD_SUPPORTS_PROPER_CAS
typedef uint32_t pintype_t;;
#include "wrapAtomic.h"

/** Basic implementation of delayMicroseconds for STM32Cube HAL, creates a tight loop and is only suitable for
 * short delays
 *
 * @param us Number of microseconds to delay
 */
inline void delayMicroseconds(uint32_t us) {
    std::this_thread::sleep_for(std::chrono::microseconds(20));
}

inline void delay(uint32_t m) {
    std::this_thread::sleep_for(std::chrono::milliseconds(m));

}

#endif
