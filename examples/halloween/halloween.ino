/**
 * If you're looking around for a last minute Halloween light flasher for the pumpkin, I use LEDs in the pumpkin because
 * if it is not left too long I have the chance of making soup from it, without it being tainted by candle smoke.
 *
 * This program just requires 4 LEDs that are attached to PWM capable pins with suitable resistors, if you use an Uno,
 * or other 5V board that would be around 330R. The LEDs randomly flash using task manager to schedule at random
 * intervals. Tested in my pumpkin and works fairly well. Enjoy!
 */
#include <TaskManagerIO.h>

// here we define 4 PWM capable pins to connect the LEDs to, the LED controllers will be defined further down.
const int blueLedPin = 6;
const int redLedPin = 5;
const int yellowLedPin = 10;
const int greenLedPin = 11;

//
// We use instances of a class extending executable, this allows us to store state for each LED and be called by task
// manager on the schedule we define, it will call back the exec() method. This holds the LED pin and if the pin is
// inverted, IE if the connection is reversed and the LED is on when LOW.
//
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

// define the 4 LED controllers globally

LedControlTask blueLed(blueLedPin);
LedControlTask redLed(redLedPin);
LedControlTask greenLed(greenLedPin, true);
LedControlTask yellowLed(yellowLedPin);

void setup() {
    // when we initialise them, they start a schedule with task manager.
    blueLed.init();
    redLed.init();
    greenLed.init();
    yellowLed.init();
}

// as with all task manager based programs, you must call the runLoop method frequently.
void loop() {
    taskManager.runLoop();
}