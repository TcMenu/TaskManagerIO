#ifndef TASKMANAGER_IO_WRAPATOMIC_H
#define TASKMANAGER_IO_WRAPATOMIC_H

#include <atomic>

namespace tm_internal {

    typedef std::atomic<TimerTask *> TimerTaskAtomicPtr;
    typedef std::atomic<uint32_t>    TmAtomicBool;
    typedef std::atomic<uint32_t>    position_t;
    typedef std::atomic<uint32_t>*   position_ptr_t;


#if defined(ATOMIC_INT_LOCK_FREE) && (ATOMIC_INT_LOCK_FREE == 2)
    inline bool atomicSwap32(std::atomic<uint32_t> *ptr, uint32_t expected, uint32_t newValue) {
        return ptr->compare_exchange_strong(expected, newValue);
    }
#elif defined(PICO_NEEDS_PROTECTOR)
    inline bool atomicSwap32(std::atomic<uint32_t> *ptr, uint32_t expected, uint32_t newValue) {
        // compare and swap is not implemented on ESP8266
        auto ret = false;
        TmPicoProtector protector;
        if(ptr->load() == expected) {
            ptr->store(newValue);
            ret = true;
        }
        return ret;
    }
#else
    /**
     * Sets the integer to the new value ONLY when the existing value matches expected.
     * @param ptr the memory location to compare / swap
     * @param expected the expected value
     * @param newValue the replacement, replaced on if expected matches
     * @return true if the replacement was done, otherwise false
     */
    inline bool atomicSwap32(std::atomic<uint32_t> *ptr, uint32_t expected, uint32_t newValue) {
        // compare and swap is not implemented on ESP8266
        auto ret = false;
        noInterrupts();
        if(ptr->load() == expected) {
            ptr->store(newValue);
            ret = true;
        }
        interrupts();
        return ret;
    }

#endif

    inline uint32_t atomicRead32(const std::atomic<uint32_t>* atomicType) {
        return atomicType->load();
    }

    /**
     * Sets the boolean to the new value ONLY when the existing value matches expected.
     * @param ptr the memory location to compare / swap
     * @param expected the expected value
     * @param newValue the replacement, replaced on if expected matches
     * @return true if the replacement was done, otherwise false
     */
    inline bool atomicSwapBool(TmAtomicBool *ptr, bool expected, bool newValue) {
        return atomicSwap32(ptr, expected, newValue);
    }

    /**
     * Reads an atomic boolean value
     * @param pPtr the pointer to an atomic boolean value
     * @return the boolean value.
     */
    inline bool atomicReadBool(TmAtomicBool *pPtr) {
        return pPtr->load();
    }

    /**
     * Writes a boolean value atomically
     * @param pPtr the atomic ref
     * @param newVal the new value
     */
    inline void atomicWriteBool(TmAtomicBool *pPtr, bool newVal) {
        pPtr->store(newVal);
    }

    /**
    * Dereferences and returns the value of the pointer at ptr type. On mbed boards this is already an atomic operation and
    * therefore volatile is enough.
    * @tparam PTR_TYPE class type of the pointer
    * @param pPtr reference to memory of the pointer
    * @return the pointer.
    */
    inline TimerTask *atomicReadPtr(TimerTaskAtomicPtr *pPtr) {
        return pPtr->load();
    }

    /**
     * Dereferences and then sets the memory of the pointer type. On mbed boards this is already an atomic operation
     * @tparam PTR_TYPE
     * @param pPtr
     * @param newValue
     */
    inline void atomicWritePtr(TimerTaskAtomicPtr *pPtr, TimerTask *newValue) {
        pPtr->store(newValue);
    }
}

#if defined(TM_ENABLE_CAPTURED_LAMBDAS)
# define TM_ALLOW_CAPTURED_LAMBDA
#endif

#endif // TASKMANAGER_IO_WRAPATOMIC_H