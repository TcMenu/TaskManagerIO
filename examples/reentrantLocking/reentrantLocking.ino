
#include <TaskManagerIO.h>
#include <ReentrantYieldingLock.h>

// this is the global lock that that we'll use to protect the variable below myVar.
ReentrantYieldingLock myLock;
volatile int myVar = 0;

//
// A simple log function for writing to serial
//
void log(const char* str, int val) {
    Serial.print(millis());
    Serial.print(' ');
    Serial.print(str);
    Serial.print('=');
    Serial.println(val);
}

//
// here we define a nested function that is called from the task, it locks again. This test that the reentrant
// functionality is working
//
void nestedFunction() {
    TaskMgrLock locker(myLock);
    log("in nested function", myVar);
}

void setup() {
    Serial.begin(115200);

    // start a task that locks the bus, calls a nested function and yields time back to task manager.
    taskManager.scheduleFixedRate(1000, [] {
        TaskMgrLock locker(myLock);
        int myVarAtStart = myVar;
        log("start task function", myVar);

        // the nested function will lock again, which is fine because it's in the same task
        nestedFunction();

        // now we release task manager to run other tasks, the will not be able to take our lock
        taskManager.yieldForMicros(millisToMicros(500));

        // now we have the context back call the nested lock again
        nestedFunction();

        if(myVar != myVarAtStart) {
            log("ERROR in locking ", myVarAtStart);
        }

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