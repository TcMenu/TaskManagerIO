
#include <testing/SimpleTest.h>
#include <SimpleSpinLock.h>
#include "TaskManagerIO.h"
#include "test_utils.h"

using namespace SimpleTest;

SimpleSpinLock testLock;
taskid_t runTaskId1;
taskid_t runTaskId2;

bool task1RunningPtrCheck = false;
bool task2RunningPtrCheck = false;
bool task3RunningPtrCheck = false;
bool allGood = false;

int runCount1, runCount2, runCount3;
int captureCount2, captureCount3;

test(testGettingRunningTaskAlwaysCorrect) {
    task1RunningPtrCheck = false;
    task2RunningPtrCheck = false;
    runCount1 = runCount2 = runCount3 = 0;

    taskManager.reset();
    assertEquals(nullptr, taskManager.getRunningTask());

    serdebugF("Starting running task check");

    runTaskId1 = taskManager.scheduleFixedRate(1, [] {
        task1RunningPtrCheck = taskManager.getRunningTask() == taskManager.getTask(runTaskId1);
        if(task1RunningPtrCheck) {
            taskManager.yieldForMicros(millisToMicros(1));

            task1RunningPtrCheck = taskManager.getRunningTask() == taskManager.getTask(runTaskId1);
        }
        runCount1++;
    });

    runTaskId2 = taskManager.scheduleFixedRate(50, [] {
        task2RunningPtrCheck = taskManager.getRunningTask() == taskManager.getTask(runTaskId2);
        runCount2++;
    }, TIME_MICROS);

    serdebugF("Scheduled running task check");

    unsigned long then = millis();
    taskManager.yieldForMicros(millisToMicros(100));
    int diff = int(millis() - then);
    assertMoreThan(90, diff);

    serdebugF("Finished running task check, asserting.");

    assertTrue(task1RunningPtrCheck);
    assertTrue(task2RunningPtrCheck);
    assertMoreThan(30, runCount1);
    assertMoreThan(250, runCount2);
}
