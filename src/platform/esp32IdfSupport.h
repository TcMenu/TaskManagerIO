#ifndef TASKMANAGER_IO_ESP32_IDF
#define TASKMANAGER_IO_ESP32_IDF

#include <stdint.h>
#include <cstddef>

#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include <cctype>

#define PROGMEM
#define BOARD_SUPPORTS_PROPER_CAS
typedef uint8_t pintype_t;;
#include "platform/wrapAtomic.h"

inline uint32_t millis() {
    return static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
}

inline uint32_t micros() {
    return static_cast<uint32_t>(esp_timer_get_time());
}

inline void yield() {
    vTaskDelay(1);
}

inline void delayMicroseconds(uint32_t delay) {
    esp_rom_delay_us(delay);
}

#endif
