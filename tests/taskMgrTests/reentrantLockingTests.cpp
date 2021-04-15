
#include <AUnit.h>
#include <SimpleSpinLock.h>
#include "TaskManagerIO.h"
#include "test_utils.h"

using namespace aunit;

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
    assertEqual(nullptr, taskManager.getRunningTask());

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
    assertMore(diff, 90);

    serdebugF("Finished running task check, asserting.");

    assertTrue(task1RunningPtrCheck);
    assertTrue(task2RunningPtrCheck);
    assertMore(runCount1, 30);
    assertMore(runCount2, 250);
}

test(testReentrantLock) {
    taskManager.reset();

    allGood = true;
    runCount1 = runCount2 = runCount3 = 0;

    runTaskId1 = taskManager.scheduleFixedRate(10, [] {
        if(testLock.tryLock()) {
            if(captureCount3 != runCount3 || captureCount2 != runCount2) {
                allGood = false;
            }
            testLock.unlock();
        }
        else {
            testLock.lock();
            captureCount2 = runCount2;
            captureCount3 = runCount3;
        }
        runCount1++;
    }, TIME_MILLIS);

    runTaskId2 = taskManager.scheduleFixedRate(100, [] {
        TaskMgrLock locker(testLock);
        runCount2++;
    }, TIME_MICROS);

    taskManager.scheduleFixedRate(50, [] {
        TaskMgrLock locker(testLock);
        runCount3++;
    }, TIME_MICROS);

    while(runCount1 < 10 || testLock.isLocked()) {
        taskManager.yieldForMicros(100);
    }

    assertTrue(allGood);
    assertTrue(runTaskId1 != TASKMGR_INVALIDID);
    assertTrue(runTaskId2 != TASKMGR_INVALIDID);
    assertMore(runCount1, 9);
    assertLess(runCount1, 12);
    assertMore(runCount2, 200);
    assertMore(runCount3, 200);
}