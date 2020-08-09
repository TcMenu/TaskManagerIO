// SimpleTasks.ino
//
// A very simple example of how to use task manager to schedule tasks to be done at some point in time

// To use task manager we must include the library
#include "TaskManagerIO.h"

// A simple logging function that logs the time and the log line.
void logIt(const char* toLog) {
    Serial.print(millis());
    Serial.print(':');
    Serial.println(toLog);
}

// here we globally store the task ID of our repeating task
int taskId;

void setup() {
    // schedule a task to run at a fixed rate, every 1000 milliseconds.
    taskId = taskManager.scheduleFixedRate(1000, [] {
        logIt("Fixed rate, every second");
    });

    // schedule a task to run once in 20 seconds.
    taskManager.scheduleOnce(20, [] {
        logIt("20 seconds one off task");
        logIt("stop 1 second task");
        taskManager.cancelTask(taskId);
        taskManager.scheduleFixedRate(10, [] {
            logIt("Ten more seconds done finished.")
        })
    }, TIME_SECONDS);

    // schedule a task to be executed immediately
    taskManager.scheduleImmediately([] {
        logIt("To do as soon as possible");
    });
}

// All programs using TaskManager need to call taskManager.runLoop in the loop
// method, and should never use delay(..)
void loop() {
    taskManager.runLoop();
}
