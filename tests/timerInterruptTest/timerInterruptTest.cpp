
#include <testing/SimpleTest.h>

using namespace SimpleTest;

IOLOG_MBED_PORT_IF_NEEDED(USBTX, USBRX)

HardwareTimer* myTimer;
int taskExec = 0;

class TimerInterruptEvent : public BaseEvent {
private:
    uint32_t count;
public:
    void exec() override {
        count++;
    }

    uint32_t getCount() { return count; }

    uint32_t timeOfNextCheck() override {
        return secondsToMicros(10);
    }
} timerInterruptEvent;

volatile int intCount = 0;
void timerHasChanged() {
    timerInterruptEvent.markTriggeredAndNotify();

    if(intCount % 10 == 0) {
        taskManager.execute([] {
            taskExec++;
        });
    }
    intCount = intCount + 1;
}

void setup() {
    IOLOG_START_SERIAL
    startTesting();

#if defined(TIM1)
    TIM_TypeDef *Instance = TIM1;
#else
    TIM_TypeDef *Instance = TIM2;
#endif

    myTimer = new HardwareTimer(Instance);
    pinMode(LED_BUILTIN, OUTPUT);
    myTimer->setOverflow(250, HERTZ_FORMAT);
    myTimer->attachInterrupt(timerHasChanged);
}

DEFAULT_TEST_RUNLOOP

test(testInterruptEventHandling) {
    taskManager.registerEvent(&timerInterruptEvent);
    myTimer->resume();
    unsigned long then = millis();
    while(timerInterruptEvent.getCount() < 2000 && (millis() - then) < 10000) {
        taskManager.yieldForMicros(10000);
    }

    assertMoreThan((uint32_t)1800, timerInterruptEvent.getCount());
    assertMoreThan(100, taskExec);

    taskManager.reset();
    myTimer->pause();
}
