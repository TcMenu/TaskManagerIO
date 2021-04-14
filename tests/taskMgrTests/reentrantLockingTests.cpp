
#include <AUnit.h>
#include <ReentrantYieldingLock.h>
#include "TaskManagerIO.h"
#include "test_utils.h"

using namespace aunit;

ReentrantYieldingLock testLock;
taskid_t runTaskId1;
taskid_t runTaskId2;

bool task1RunningPtrCheck = false;
bool task2RunningPtrCheck = false;

test(testGettingRunningTaskAlwaysCorrect) {
    taskManager.reset();
    assertEqual(nullptr, taskManager.getRunningTask());

    serdebugF("Starting running task check");

    runTaskId1 = taskManager.scheduleFixedRate(1, [] {
        task1RunningPtrCheck = taskManager.getRunningTask() == taskManager.getTask(runTaskId1);
        if(task1RunningPtrCheck) {
            taskManager.yieldForMicros(millisToMicros(1));

            task1RunningPtrCheck = taskManager.getRunningTask() == taskManager.getTask(runTaskId1);
        }
    });

    runTaskId2 = taskManager.scheduleFixedRate(50, [] {
        task2RunningPtrCheck = taskManager.getRunningTask() == taskManager.getTask(runTaskId2);
    }, TIME_MICROS);

    serdebugF("Scheduled running task check");

    taskManager.yieldForMicros(millisToMicros(100));

    serdebugF("Finished running task check, asserting.");

    assertTrue(task1RunningPtrCheck);
    assertTrue(task2RunningPtrCheck);
}

test(testReentrantLock) {

}