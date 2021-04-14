
#include <TaskManagerIO.h>
#include <ReentrantYieldingLock.h>

// this is the global lock that can only be
ReentrantYieldingLock myLock;
volatile int myVar = 0;

void log(const char* str, int val) {
    Serial.print(millis());
    Serial.print(' ');
    Serial.println(str);
}

void nestedFunction() {
    TaskMgrLock locker(myLock);
    log("in nested function", myVar);
}

void setup() {
    Serial.begin(115200);

    taskManager.scheduleFixedRate(1000, [] {
        TaskMgrLock locker(myLock);
        log("start task function", myVar);

        // the nested function will lock again, which is fine because it's in the same task
        nestedFunction();

        // now we release taskmanager to run other tasks, the will not be able to take our lock
        taskManager.yieldForMicros(millisToMicros(500));

        // now we have the context back lock again
        nestedFunction();
        log("exit task function", myVar);
    });

    taskManager.scheduleFixedRate(5, [] {
        // here we have another task that locks, it will need to acquire the lock exclusively before entering
        TaskMgrLock locker(myLock);
        // do something that's unlikely to be optimised out, to waste some time
        for(int i=0; i < 100; i++) {
            myVar++;
        }
    });
}

void loop() {
    taskManager.runLoop();
}