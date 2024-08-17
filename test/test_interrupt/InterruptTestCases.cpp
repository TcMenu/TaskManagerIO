#include <Arduino.h>
#include <unity.h>
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

void testInterruptSupportMarshalling() {
    taskManager.setInterruptCallback(intHandler);
    taskManager.addInterrupt(&interruptAbs, 2, CHANGE);

    // make sure the interrupt is properly registered.
    TEST_ASSERT_EQUAL_UINT8(2, interruptAbs.getInterruptPin());
    TEST_ASSERT_EQUAL(CHANGE, interruptAbs.getTheMode());
    TEST_ASSERT_FALSE(interruptAbs.isIntHandlerNull());

    // now pretend the interrupt took place.
    interruptAbs.runInterrupt();

    // and wait for task manager to schedule. Note that the delay is very large, this is because the test is running
    // on CI where the time to handle such an interrupt on an emulator will be more significant, we are not testing
    // performance, rather functionality.
    fixture.assertThatTaskRunsOnTime(0, 1500);

    // and the pin should be 2
    TEST_ASSERT_EQUAL(2, pinNo);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(testInterruptSupportMarshalling);
    UNITY_END();
}

void loop() {}
