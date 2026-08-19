#ifndef TASKMANAGER_IO_WRAPATOMIC_H
#define TASKMANAGER_IO_WRAPATOMIC_H

#include <atomic>

namespace tm_internal {

    typedef std::atomic<TimerTask *> TimerTaskAtomicPtr;

    typedef std::atomic<uint32_t> TmAtomicBool;

#ifdef BOARD_SUPPORTS_PROPER_CAS
    inline bool atomicSwapBool(TmAtomicBool *ptr, bool exp, bool nv) {
        uint32_t expected = exp;
        const uint32_t newValue = nv;
        return ptr->compare_exchange_strong(expected, newValue);
    }
#else
    /**
     * Sets the boolean to the new value ONLY when the existing value matches expected.
     * @param ptr the bool memory location to compare / swap
     * @param expected the expected value
     * @param newValue the replacement, replaced on if expected matches
     * @return true if the replacement was done, otherwise false
     */
    inline bool atomicSwapBool(TmAtomicBool *ptr, bool expected, bool newValue) {
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