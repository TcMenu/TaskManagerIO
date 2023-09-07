/**
 * @file SimpleTasks.ino
 * A very simple example of how to use task manager to schedule tasks to be done
 *
 * In this example we demonstrate how to create tasks that execute at a point in time,
 * that repeat at a given interval, and tasks that are executed as soon as possible
 * by task manager. We also show how to cancel a running task.
 *
 * There is a getting started guide including video available:
 * https://www.thecoderscorner.com/products/arduino-libraries/taskmanager-io/
 */

// To use task manager we must include the library
#include <Arduino.h>
#include "TaskManagerIO.h"

// A counter that will be increased in the microsecond task.
int counter = 0;

// here we globally store the task ID of our repeating task, we need this to cancel it later.
// ADVANCED: On supported 32 bit boards you could enable the lambda support and pass this to the other task.
int taskId;

//
// A simple logging function that logs the time and the log line.
//
void logIt(const char* toLog) {
    Serial.print(millis());
    Serial.print(':');
    Serial.print(counter);
    Serial.print(':');
    Serial.println(toLog);
}

//
// A task can either be a function that takes no parameters and returns void, a class that extends Executable or
// if you want to call with parameters either ExecWithParameter or ExecWith2Parameters
//
void twentySecondJob() {
    logIt("20 seconds one off task");
    logIt("stop 1 second task");
    taskManager.cancelTask(taskId);
    taskManager.schedule(onceSeconds(10), [] {
        logIt("Ten more seconds done finished.");
    });
}


//
// In setup we prepare our tasks, this is what a usual task manager sketch looks like
//
void setup() {
    Serial.begin(115200);

    taskManager.schedule(repeatMicros(100), []{
        counter++;
    });

    // schedule a task to run at a fixed rate, every 1000 milliseconds.
    taskId = taskManager.schedule(repeatMillis(1000), [] {
        logIt("Fixed rate, every second");
    });

    // schedule a task to run once in 20 seconds.
    taskManager.schedule(onceSeconds(20), twentySecondJob);

    // schedule a task to be executed as soon as possible as a taskManager task.
    taskManager.execute([] {
        logIt("To do as soon as possible");
    });
}

//
// All programs using TaskManager need to call taskManager.runLoop in the loop
// method, and should never use delay(..)
//
void loop() {
    // Optional:
    // If you wanted to go into a low power mode between tasks, you can use taskManager.microsToNextTask() to determine
    // how long you can sleep before the next execution. If you use interrupts, ensure the low power mode supports them.
    //auto delay = taskManager.microsToNextTask();
    //yourLowPowerMode.sleep(delay);

    taskManager.runLoop();
}
