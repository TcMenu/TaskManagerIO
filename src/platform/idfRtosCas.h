#ifndef TASKMANAGER_IO_IDF_RTOS_CAS_H
#define TASKMANAGER_IO_IDF_RTOS_CAS_H

# define IOA_MULTITHREADED
inline void* getCurrentThreadId() { return xTaskGetCurrentTaskHandle() ; }

namespace tm_internal {

    typedef TimerTask* volatile TimerTaskAtomicPtr;
    typedef volatile uint32_t TmAtomicBool; // to use CAS, the bool must be 32 bits wide


// `uxPortCompareSet` was removed in ESP-IDF v5.0
// See https://github.com/TcMenu/TaskManagerIO/issues/59
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    inline bool atomicSwapBool(TmAtomicBool *ptr, bool expected, bool newValue) {
        uint32_t exp32 = expected;
        uint32_t new32 = newValue;
        uxPortCompareSet(ptr, exp32, &new32);
        return new32 == expected;
    }
#else
    inline bool atomicSwapBool(TmAtomicBool *ptr, bool expected, bool newValue) {
        // function added in ESP-IDF v5.0
        return esp_cpu_compare_and_set(ptr, expected, newValue);
    }
#endif

    /**
     * Reads an atomic boolean value
     * @param pPtr the pointer to an atomic boolean value
     * @return the boolean value.
     */
    inline bool atomicReadBool(TmAtomicBool *pPtr) {
        return *pPtr != 0;
    }

    /**
     * Writes a boolean value atomically
     * @param pPtr the atomic ref
     * @param newVal the new value
     */
    inline void atomicWriteBool(TmAtomicBool *pPtr, bool newVal) {
        *pPtr = newVal;
    }

    /**
     * Dereferences and returns the value of the pointer at ptr type. On mbed boards this is already an atomic operation and
     * therefore volatile is enough.
     * @tparam PTR_TYPE class type of the pointer
     * @param pPtr reference to memory of the pointer
     * @return the pointer.
     */
    inline TimerTask *atomicReadPtr(TimerTaskAtomicPtr *pPtr) {
        return *pPtr;
    }

    /**
     * Dereferences and then sets the memory of the pointer type. On mbed boards this is already an atomic operation
     * @tparam PTR_TYPE
     * @param pPtr
     * @param newValue
     */
    inline void atomicWritePtr(TimerTaskAtomicPtr *pPtr, TimerTask *newValue) {
        *pPtr = newValue;
    }
}


#endif
