#include <Arduino.h>
#include <unity.h>
#include <SimpleSpinLock.h>
#include "TaskManagerIO.h"
#include "../utils/test_utils.h"

void setUp() {}

void tearDown() {}

SimpleSpinLock testLock;
taskid_t runTaskId1;
taskid_t runTaskId2;

bool task1RunningPtrCheck = false;
bool task2RunningPtrCheck = false;
bool task3RunningPtrCheck = false;
bool allGood = false;

int runCount1, runCount2, runCount3;
int captureCount2, captureCount3;

void testGettingRunningTaskAlwaysCorrect() {
    task1RunningPtrCheck = false;
    task2RunningPtrCheck = false;
    runCount1 = runCount2 = runCount3 = 0;

    taskManager.reset();
    TEST_ASSERT_EQUAL(nullptr, taskManager.getRunningTask());

    serdebugF("Starting running task check");

    runTaskId1 = taskManager.scheduleFixedRate(1, [] {
        task1RunningPtrCheck = taskManager.getRunningTask() == taskManager.getTask(runTaskId1);
        if (task1RunningPtrCheck) {
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
    TEST_ASSERT_GREATER_THAN(90, diff);

    serdebugF("Finished running task check, asserting.");

    TEST_ASSERT_TRUE(task1RunningPtrCheck);
    TEST_ASSERT_TRUE(task2RunningPtrCheck);
    TEST_ASSERT_GREATER_THAN(30, runCount1);
    TEST_ASSERT_GREATER_THAN(250, runCount2);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(testGettingRunningTaskAlwaysCorrect);
    UNITY_END();
}

void loop() {}
