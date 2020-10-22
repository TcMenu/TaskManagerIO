
#include <AUnit.h>
#include <ExecWithParameter.h>
#include "TaskManagerIO.h"
#include "test_utils.h"

using namespace aunit;

bool taskWithinEvent;

class TestPolledEvent : public BaseEvent {
private:
    int execCalls;
    int scheduleCalls;
    uint32_t interval;
    bool triggerNow;
public:
    TestPolledEvent() {
        execCalls = scheduleCalls = 0;
        interval = 100000; // 100 millis
        triggerNow = false;
        taskWithinEvent = false;
    }

    ~TestPolledEvent() override = default;

    void exec() override {
        execCalls++;
        taskManager.scheduleOnce(100, [] {
            taskWithinEvent = true;
        });
        setCompleted(true);
    }

    uint32_t timeOfNextCheck() override {
        scheduleCalls++;
        setTriggered(triggerNow);
        return interval;
    }

    void startTriggering() {
        triggerNow = true;
        interval = 10000;
    }

    int getScheduleCalls() const { return scheduleCalls; }

    int getExecCalls() const { return execCalls; }
} polledEvent;

testF(TimingHelpFixture, testRepeatedRaisingOfEvent) {
    fail();
}

testF(TimingHelpFixture, testInterruptTestStartsTask) {
    fail();
}

typedef bool (*TMPredicate)();

bool runScheduleUntilMatchOrTimeout(TMPredicate predicate) {
    unsigned long startTime = millis();
    // wait until the predicate matches, or it takes too long.
    while (!predicate() && (millis() - startTime) < 1000) {
        taskManager.yieldForMicros(10000);
    }
    return predicate();
}

testF(TimingHelpFixture, testRaiseEventStartTaskCompleted) {
    EnsureExecutionWithin timelyChecker(500);

    //taskManager.registerEvent(&polledEvent);

    assertTrue(runScheduleUntilMatchOrTimeout([] { return polledEvent.getScheduleCalls() >= 10; } ));

    // and now we tell the event to trigger itself
    polledEvent.startTriggering();

    // wait until the exec() method is called
    assertTrue(runScheduleUntilMatchOrTimeout([] { return polledEvent.getExecCalls() != 0; }));

    // and then make sure that the task registed inside the event triggers
    assertTrue(runScheduleUntilMatchOrTimeout([] { return taskWithinEvent; }));

    assertTrue(timelyChecker.ensureTimely());
}
