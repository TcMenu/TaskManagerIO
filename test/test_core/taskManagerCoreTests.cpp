#include <Arduino.h>
#include <unity.h>
#include <ExecWithParameter.h>
#include <IoLogging.h>
#include "TaskManagerIO.h"
#include "../utils/test_utils.h"

TimingHelpFixture fixture;

void setUp() {
    fixture.setup();
}

void tearDown() {}

// these variables are set during test runs to time and verify tasks are run.
bool scheduled = false;
bool scheduled2ndJob = false;
unsigned long microsStarted = 0, microsExecuted = 0, microsExecuted2ndJob = 0;
int count1 = 0, count2 = 0;
uint8_t pinNo = 0;

void recordingJob() {
    microsExecuted = micros();
    count1++;
    scheduled = true;
}

void recordingJob2() {
    microsExecuted2ndJob = micros();
    count2++;
    scheduled2ndJob = true;
}


class TestingExec : public Executable {
public:
    int noOfTimesRun;

    TestingExec() {
        noOfTimesRun = 0;
    }

    void exec() override {
        noOfTimesRun++;
    }
};

TestingExec exec;

void testRunningUsingExecutorClass() {
    taskManager.scheduleFixedRate(10, &::exec);
    taskManager.scheduleOnce(250, recordingJob);

    fixture.assertThatTaskRunsOnTime(250000L, MILLIS_ALLOWANCE);
    TEST_ASSERT_GREATER_THAN(10, ::exec.noOfTimesRun);
}

void testSchedulingTaskOnceInMicroseconds() {
    taskManager.scheduleOnce(800, recordingJob, TIME_MICROS);
    fixture.assertThatTaskRunsOnTime(800, MICROS_ALLOWANCE);
    fixture.assertTasksSpacesTaken(0);
}

void testSchedulingTaskOnceInMilliseconds() {
    taskManager.scheduleOnce(20, recordingJob, TIME_MILLIS);
    fixture.assertThatTaskRunsOnTime(19500, MILLIS_ALLOWANCE);
    fixture.assertTasksSpacesTaken(0);
}

void testSchedulingTaskOnceInSeconds() {
    taskManager.scheduleOnce(2, recordingJob, TIME_SECONDS);
    // Second scheduling is not as granular, we need to allow +- 100mS.
    fixture.assertThatTaskRunsOnTime(2000000L, MILLIS_ALLOWANCE);
    fixture.assertTasksSpacesTaken(0);
}

void testScheduleManyJobsAtOnce() {
    taskManager.scheduleOnce(1, [] {}, TIME_SECONDS);
    taskManager.scheduleOnce(200, recordingJob, TIME_MILLIS);
    taskManager.scheduleOnce(250, recordingJob2, TIME_MICROS);

    fixture.assertThatTaskRunsOnTime(199500, MILLIS_ALLOWANCE);
    fixture.assertThatSecondJobRan(250, MICROS_ALLOWANCE);
    fixture.assertTasksSpacesTaken(1);
}

void testEnableAndDisableSupport() {
    static int myTaskCounter = 0;
    auto myTaskId = taskManager.scheduleFixedRate(1, [] { myTaskCounter++; }, TIME_MILLIS);
    taskManager.yieldForMicros(20000);
    TEST_ASSERT_NOT_EQUAL(0, myTaskCounter);

    // "turn off" the task
    taskManager.setTaskEnabled(myTaskId, false);

    // It can take one cycle for the task to switch enablement state.
    taskManager.yieldForMicros(2000);
    auto oldTaskCount = myTaskCounter;

    // Now run the task for some time, it should never get scheduled.
    taskManager.yieldForMicros(20000);
    TEST_ASSERT_EQUAL(myTaskCounter, oldTaskCount);

    // "turn on" the task and see if it increases again
    taskManager.setTaskEnabled(myTaskId, true);
    taskManager.yieldForMicros(20000);
    TEST_ASSERT_NOT_EQUAL(myTaskCounter, oldTaskCount);
}

void testScheduleFixedRate() {
    TEST_ASSERT_EQUAL(nullptr, taskManager.getFirstTask());

    auto taskId1 = taskManager.scheduleFixedRate(10, recordingJob, TIME_MILLIS);
    auto taskId2 = taskManager.scheduleFixedRate(100, recordingJob2, TIME_MICROS);

    // Now check the task registration in detail.
    TEST_ASSERT_NOT_EQUAL(TASKMGR_INVALIDID, taskId1);
    TimerTask* task = taskManager.getFirstTask();
    TEST_ASSERT_NOT_EQUAL(nullptr, task);
    TEST_ASSERT_FALSE(task->isMillisSchedule());
    TEST_ASSERT_TRUE(task->isMicrosSchedule());

    // Now check the task registration in detail.
    TEST_ASSERT_NOT_EQUAL(TASKMGR_INVALIDID, taskId2);
    task = task->getNext();
    TEST_ASSERT_NOT_EQUAL(nullptr, task);
    TEST_ASSERT_TRUE(task->isMillisSchedule());
    TEST_ASSERT_FALSE(task->isMicrosSchedule());

    dumpTasks();

    uint32_t timeStartYield = millis();
    taskManager.yieldForMicros(secondsToMillis(22));
    uint32_t timeTaken = millis() - timeStartYield;

    dumpTasks();

    // Make sure the yield timings were in range.
    TEST_ASSERT_LESS_THAN(25U, timeTaken);
    TEST_ASSERT_GREATER_THAN(19U, timeTaken);

    // Now make sure that we got in the right ballpark of calls.
    TEST_ASSERT_GREATER_THAN(1, count1);
    TEST_ASSERT_GREATER_THAN(150, count2);
}

void testCancellingAJobAfterCreation() {
    TEST_ASSERT_EQUAL(nullptr, taskManager.getFirstTask());

    auto taskId = taskManager.scheduleFixedRate(10, recordingJob, TIME_MILLIS);

    // Now check the task registration in detail.
    TEST_ASSERT_NOT_EQUAL(TASKMGR_INVALIDID, taskId);
    TimerTask* task = taskManager.getFirstTask();
    TEST_ASSERT_NOT_EQUAL(nullptr, task);
    TEST_ASSERT_TRUE(task->isMillisSchedule());
    TEST_ASSERT_FALSE(task->isMicrosSchedule());
    TEST_ASSERT_GREATER_THAN(8000UL, task->microsFromNow());

    fixture.assertThatTaskRunsOnTime(10000, MILLIS_ALLOWANCE);

    // Cancel the task and make sure everything is cleared down
    fixture.assertTasksSpacesTaken(1);
    taskManager.cancelTask(taskId);
    taskManager.yieldForMicros(100); // Needs to run the cancellation task.
    fixture.assertTasksSpacesTaken(0);

    TEST_ASSERT_EQUAL(nullptr, taskManager.getFirstTask());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(testRunningUsingExecutorClass);
    RUN_TEST(testSchedulingTaskOnceInMicroseconds);
    RUN_TEST(testSchedulingTaskOnceInMilliseconds);
    RUN_TEST(testSchedulingTaskOnceInSeconds);
    RUN_TEST(testScheduleManyJobsAtOnce);
    RUN_TEST(testEnableAndDisableSupport);
    RUN_TEST(testScheduleFixedRate);
    RUN_TEST(testCancellingAJobAfterCreation);
    UNITY_END();
}

void loop() {}