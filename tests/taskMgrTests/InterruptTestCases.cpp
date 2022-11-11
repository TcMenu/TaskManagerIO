
#include <testing/SimpleTest.h>
#include "TaskManagerIO.h"
#include "test_utils.h"

using namespace SimpleTest;

class MockedInterruptAbstraction : public InterruptAbstraction {
private:
    pintype_t interruptPin;
    RawIntHandler intHandler;
    uint8_t theMode;
public:
    MockedInterruptAbstraction() {
        interruptPin = 0;
        intHandler = nullptr;
        theMode = 0;
    }

    void attachInterrupt(pintype_t pin, RawIntHandler fn, uint8_t mode) override {
        interruptPin = pin;
        intHandler = fn;
        theMode = mode;
    }

    pintype_t getInterruptPin() const {
        return interruptPin;
    }

    bool isIntHandlerNull() const {
        return intHandler == nullptr;
    }

    uint8_t getTheMode() const {
        return theMode;
    }

    void runInterrupt() {
        intHandler();
    }
};

void intHandler(pinid_t pin) {
    scheduled = true;
    microsExecuted = micros();
    count1++;
    pinNo = pin;
}

MockedInterruptAbstraction interruptAbs;

testF(TimingHelpFixture, interruptSupportMarshalling) {
    taskManager.setInterruptCallback(intHandler);
    taskManager.addInterrupt(&interruptAbs, 2, CHANGE);

    // make sure the interrupt is properly registered.
    assertEquals(pintype_t(2), interruptAbs.getInterruptPin());
    assertEquals(CHANGE, interruptAbs.getTheMode());
    assertFalse(interruptAbs.isIntHandlerNull());

    // now pretend the interrupt took place.
    (interruptAbs.runInterrupt());

    // and wait for task manager to schedule.
    assertThatTaskRunsOnTime(0, 250);

    // and the pin should be 2
    assertEquals(2, pinNo);
}
