/*
 * Copyright (c) 2018-present https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TASKMANAGERIO_RENTRANTYIELDINGLOCK_H
#define TASKMANAGERIO_RENTRANTYIELDINGLOCK_H

#include "TaskManagerIO.h"

/**
 * A lock that is intended only for use within tasks, if the lock is taken it will allow task manager to run using
 * it's yield for micros call until the lock becomes free. It is a spin lock, so it is safe to use with task manager.
 * Unless you want to use the spin behaviour in a specific way, prefer using with TaskSafeLock
 */
class ReentrantYieldingLock {
private:
    tm_internal::TimerTaskAtomicPtr initiatingTask;
    tm_internal::TmAtomicBool locked;
    volatile uint8_t count;

public:
    /**
     * Construct a reentrant yielding lock that is designed for use within task manager tasks
     */
    ReentrantYieldingLock() {
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
     * Attempt to take the lock using a spin wait, it only returns true if the lock was taken.
     * @param micros micros to wait
     * @return true if the lock was taken, otherwise false
     */
    bool spinLock(unsigned long micros);

    /**
     * Release the lock taken by spinlock or lock. DOES NOT check that the callee is correct so use carefully.
     */
    void unlock();

    uint8_t getLockCount() const { return count; }

    bool isLocked() const { return locked; }
};

/**
 * A wrapper around the task manager locking facilities that allow you to lock within a block of code by putting
 * an instance on the stack, for example:
 *
 * ```
 * ReentrantYieldingLock myLock;
 * void myFunctionToLock() {
 *     TaskSafeLock(myLock);
 *     // do some work that needs the lock here. lock will always be released.
 * }
 * ```
 */
class TaskMgrLock {
private:
    ReentrantYieldingLock& lock;

public:
    TaskMgrLock(ReentrantYieldingLock& theLock) : lock(theLock) {
        lock.lock();
    }

    ~TaskMgrLock() {
        lock.unlock();
    }
};

#endif //TASKMANAGERIO_RENTRANTYIELDINGLOCK_H
