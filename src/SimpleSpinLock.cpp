/*
 * Copyright (c) 2018-present https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "SimpleSpinLock.h"

bool SimpleSpinLock::tryLock() {
    // if our task already owns the lock then we are good.
    return (locked && initiatingTask && taskManager.getRunningTask() == initiatingTask);
}

bool SimpleSpinLock::spinLock(unsigned long iterations) {
    if(tryLock()) {
        ++count;
        return true;
    }

    // otherwise we contend to get the lock in a spin wait until we exhaust the micros provided.
    while(iterations) {
        if (tm_internal::atomicSwapBool(&locked, false, true)) {
            tm_internal::atomicWritePtr(&initiatingTask, taskManager.getRunningTask());
            ++count;
            return true;
        }
        else {
            taskManager.yieldForMicros(100);
        }
        --iterations;
    }

    return false;
}

void SimpleSpinLock::unlock() {
    if(count == 0) {
        return;
    }

    --count;

    if(count == 0) {
        tm_internal::atomicWritePtr(&initiatingTask, nullptr);
        tm_internal::atomicWriteBool(&locked, false);
    }
}

