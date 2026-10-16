/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry)..
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TASKMANAGERIO_RENTRANTYIELDINGLOCK_H
#define TASKMANAGERIO_RENTRANTYIELDINGLOCK_H

#include "TaskManagerIO.h"

/**
 * @deprecated SimpleSpinLock is deprecated for future removal as it is unsafe, use TmSpinLock instead from tm-core
 * This overly complex locking facility is actually dangerous and likely to result in race conditions. Instead use
 * the task manager atomic constructs or TmSpinLock to construct a similar but safer locking strategy.
 */
class SimpleSpinLock {
private:
    tm_internal::TimerTaskAtomicPtr initiatingTask;
#if defined(IOA_MULTITHREADED)
    volatile void* currentThread = nullptr;
#endif
    tm_internal::TmAtomicBool locked;
    volatile uint8_t count;
public:
    /**
     * Construct a lock that represents this object
     */
    SimpleSpinLock() {
        initiatingTask = nullptr;
        locked = false;
        count = 0;
    }

    /**
     * Take the lock waiting the longest possible time for it to become available.
     */
    void lock() {
        spinLock(0xFFFFFFFFUL);
    }

    /**
     * tries the lock and returns immediately if it was already locked by us.
     * @return true if the lock was already owned by us, otherwise false
     */
    bool tryLock();

    /**
     * Attempt to take the lock using a spin wait, it only returns true if the lock was taken.
     * @param number of iterations to wait, each iteration is at least 100 mics
     * @return true if the lock was taken, otherwise false
     */
    bool spinLock(unsigned long iterations);

    /**
     * Release the lock taken by spinlock or lock. DOES NOT check that the callee is correct so use carefully.
     */
    void unlock();

    uint8_t getLockCount() const { return count; }

    bool isLocked() const { return locked; }
};

/**
 * @deprecated SimpleSpinLock is deprecated for future removal as it is unsafe, use TmSpinLock instead from tm-core
 * A wrapper around the task manager locking facilities that allow you to lock within a block of code by putting
 * an instance on the stack. Be very careful not to use yieldForMicros along with this method of locking.
 */
class TaskMgrLock {
private:
    SimpleSpinLock& lock;

public:
    TaskMgrLock(SimpleSpinLock& theLock) : lock(theLock) {
        lock.lock();
    }

    ~TaskMgrLock() {
        lock.unlock();
    }
};

#endif //TASKMANAGERIO_RENTRANTYIELDINGLOCK_H
