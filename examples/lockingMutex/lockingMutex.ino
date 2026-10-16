//
// Using the inbuilt task manager mutex built from non-blocking constructs in your own code.
//
// Let's say you have some code that needs simple mutual exclusion, and don't want to use the
// platform provided constructs, say the case is simple enough and cross platform, then in this
// case you can use the following code.
//
// IMPORTANT NOTE - this lock is not reentrant.


#include <TaskManagerIO.h>

// here we define the global local that protects the state. Consider this the actual lock.
static tm_internal::TmAtomicBool booleanLock;

void setup() {
    // always initialise our atomics first before using taskmanager, locks or circular buffer..
    tmInitAtomics();

    // always initialise to false first.
    tm_internal::atomicWriteBool(&booleanLock, false);

    taskManager.scheduleFixedRate(250, [] {
        TmSpinLock lock(&booleanLock);
        // The spin lock above waits until it can take the lock and then flips the flag. No other thread can get in
        // while we have the lock. It is released when lock above goes out of scope.
        // It can be used across threads, but never use this method of locking in an interrupt.
    });
}

void loop() {
    taskManager.runLoop();
}
