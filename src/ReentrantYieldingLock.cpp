/*
 * Copyright (c) 2018-present https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "ReentrantYieldingLock.h"

bool ReentrantYieldingLock::spinLock(unsigned long micros) {
    // if our task already owns the lock then we are good.
    if(locked && initiatingTask && taskManager.getRunningTask() == initiatingTask) return true;

    // otherwise we contend to get the lock in a spin wait until we exhaust the micros provided.
    bool areWeLocked = false;
    while(!locked && micros > 50) {
        areWeLocked = tm_internal::atomicSwapBool(&locked, false, true);
        if (areWeLocked) {
            ++count;
            tm_internal::atomicWritePtr(&initiatingTask, taskManager.getRunningTask());
            return true;
        }
        else {
            taskManager.yieldForMicros(50);
        }
        micros -= 50;
    }
    return false;
}

void ReentrantYieldingLock::unlock() {
    if(count != 0) {
        --count;
        return;
    }
    else {
        tm_internal::atomicWritePtr(&initiatingTask, nullptr);
        tm_internal::atomicWriteBool(&locked, false);
    }
}
