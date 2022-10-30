/**
 * If you're looking around for a last minute Halloween light flasher for the pumpkin, I use LEDs in the pumpkin because
 * if it is not left too long I have the chance of making soup from it, without it being tainted by candle smoke.
 *
 * This program just requires 4 LEDs that are attached to PWM capable pins with suitable resistors, if you use an Uno,
 * or other 5V board that would be around 330R. The LEDs randomly flash using task manager to schedule at random
 * intervals. Tested in my pumpkin and works fairly well. Enjoy!
 */
#include <TaskManagerIO.h>

class LedControlTask : public Executable {
private:
    pintype_t pin;
    bool inverted;
    int maxDelay;
public:
    LedControlTask(pintype_t pin, bool inverted = false, int maxDelay = 256): pin(pin), inverted(inverted), maxDelay(maxDelay) {
    }

    void init() {
        taskManager.execute(this);
        pinMode(pin, OUTPUT);
    }

    void exec() override {
        int levelThisTime = rand() % 255;
        int delayThisTime = rand() % maxDelay;

        analogWrite(pin, levelThisTime);

        taskManager.scheduleOnce(delayThisTime, this);
    }
};

LedControlTask blueLed(6);
LedControlTask redLed(5);
LedControlTask greenLed(11, true);
LedControlTask yellowLed(10);

void setup() {
    blueLed.init();
    redLed.init();
    greenLed.init();
    yellowLed.init();
}

void loop() {
    taskManager.runLoop();
}