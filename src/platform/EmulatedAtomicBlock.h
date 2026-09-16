#ifndef EMULATED_ATOMIC_BLOCK_H
#define EMULATED_ATOMIC_BLOCK_H

/**
 * @file EmulatedAtomicBlock.h
 * @brief Provides an emulated atomic block that emulates atomic operations with fallback to interrupt enablement
 * 
 * The purpose of this file is to provide a class that can be used to emulate atomic operations on platforms that do 
 * not support them natively. It does this by either using the RISC-V CSR (Control and Status Register) mechanism to disable
 * interrupts or by using the ARM PRIMASK register to disable interrupts. On ESP8266 it again uses the xt_rsil and xt_wsr_ps
 * functions to disable and enable interrupts. On some ESP32 boards, it uses port functions where hardware CAS is not
 * available.
 *
 * This is only a backup on smaller single core boards that do not fully/cannot fully implement std::atomic
 */

#include <Arduino.h>

#if defined(ESP8266)
#include <Arduino.h>

class EmulatedAtomicBlock {
private:
    uint32_t saved_ps;
public:
    inline EmulatedAtomicBlock() {
        // xt_rsil(15) raises interrupt level to 15 (disabling maskable interrupts)
        // and returns the previous PS (processor state) value.
        saved_ps = xt_rsil(15);
        // Memory barrier preventing compiler from reordering across the lock
        __asm__ __volatile__("" ::: "memory");
    }

    inline ~EmulatedAtomicBlock() {
        __asm__ __volatile__("" ::: "memory");
        // Restores the previous processor status / interrupt mask
        xt_wsr_ps(saved_ps);
    }

    EmulatedAtomicBlock(const EmulatedAtomicBlock&) = delete;
    EmulatedAtomicBlock& operator=(const EmulatedAtomicBlock&) = delete;
};
#elif defined(ARDUINO_ARCH_SAMD) || defined(__arm__) || defined(__thumb__) || defined(_M_ARM)
class EmulatedAtomicBlock {
private:
    uint32_t primask = 0;
public:
    inline EmulatedAtomicBlock() {
        __asm__ __volatile__("mrs %0, primask" : "=r"(primask) :: "memory");
        __asm__ __volatile__("cpsid i" ::: "memory");
    }

    inline ~EmulatedAtomicBlock() {
        __asm__ __volatile__("msr primask, %0" :: "r"(primask) : "memory");
    }

    EmulatedAtomicBlock(const EmulatedAtomicBlock&) = delete;
    EmulatedAtomicBlock& operator=(const EmulatedAtomicBlock&) = delete;
};
#elif defined(__riscv) || defined(ARDUINO_ARCH_RISCV)
class EmulatedAtomicBlock {
private:
    unsigned long saved_mstatus;
public:
    inline EmulatedAtomicBlock() {
        // Read previous mstatus and clear bit 3 (MIE - Machine Interrupt Enable)
        __asm__ __volatile__("csrrci %0, mstatus, 8" : "=r"(saved_mstatus) :: "memory");
    }

    inline ~EmulatedAtomicBlock() {
        // Restore previous mstatus (preserves interrupt state if called within an ISR)
        __asm__ __volatile__("csrw mstatus, %0" :: "r"(saved_mstatus) : "memory");
    }

    EmulatedAtomicBlock(const EmulatedAtomicBlock&) = delete;
    EmulatedAtomicBlock& operator=(const EmulatedAtomicBlock&) = delete;
};

#elif defined(ESP32)
// for the few ESP32 boards that don't have std::atomic implemented fully
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class EmulatedAtomicBlock {
private:
    // A standard FreeRTOS portMUX structure to track the interrupt state
    static inline portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

public:
    EmulatedAtomicBlock() {
        // Enters a critical section and saves the current interrupt status
        portENTER_CRITICAL(&mux);
    }

    ~EmulatedAtomicBlock() {
        // Exits the critical section and restores the previous interrupt status
        portEXIT_CRITICAL(&mux);
    }

    // Delete copy constructors to prevent nested/accidental duplicates
    EmulatedAtomicBlock(const EmulatedAtomicBlock&) = delete;
    EmulatedAtomicBlock& operator=(const EmulatedAtomicBlock&) = delete;
};
#else
#warning "CRITICAL: Unrecognized architecture in EmulatedAtomicBlock! Falling back to noInterrupts()/interrupts(). This implementation does NOT save/restore interrupt state and WILL re-enable interrupts prematurely if called inside an ISR or nested critical section. For multicore targets, this provides NO cross-core protection."

/**
 * Enables and disables interrupts to emulate atomic operations. The constructor will disable
 * interrupts and the destructor will enable them again.
 */
class EmulatedAtomicBlock {
public:
    EmulatedAtomicBlock() {
        noInterrupts();
    }
    ~EmulatedAtomicBlock() {
        interrupts();
    }
    EmulatedAtomicBlock(const EmulatedAtomicBlock&) = delete;
    EmulatedAtomicBlock& operator=(const EmulatedAtomicBlock&) = delete;
};
#endif

#endif
