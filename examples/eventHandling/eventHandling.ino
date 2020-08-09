/**
 * An example simple event that is triggered when analog in reaches a certain threshold. For the sake of this
 * example we've used a polled event, that tracks analog every 1ms (1000us). When the condition is met the event
 * will trigger, and it's exec() method will be called. This is a polled event, see the mbed example for an interrupt
 * based event.
 */

#include <TaskManagerIO.h>

/**
 * Here we create our event class, all events must extend from BaseEvent, and that class provides much of the boiler
 * plate needed for event handling. Here we implement a very simple latching analog level threshold event.
 */
class AnalogInExceedsEvent : public BaseEvent {
private:
    int analogThreshold;
    pinid_t analogPin;
    int lastReading;
    bool latched;
public:
    /**
     * Constructs the event class providing the analog pin to read from and the threshold for triggering.
     * @param inputPin the pin to read from
     * @param threshold the value at which to trigger the event.
     */
    AnalogInExceedsEvent(pinid_t inputPin, int threshold) : BaseEvent() {
        analogThreshold = threshold;
        analogPin = inputPin;
        lastReading = 0;
        latched = false;
    }

    /**
     * Here we are asked by task manager if the event is ready to be triggered (return 0), or if not, how long we should
     * wait before polling again. The maximum poll time is 33,554,431 micro seconds (about 30 seconds).
     * @return either 0 to indicate triggering, or the time to delay before polling again.
     */
    uint32_t timeOfNextCheck() override {
        lastReading = analogRead(analogPin);
        auto analogTrigger = lastReading > analogThreshold;
        if(analogTrigger && latched) {
            setTriggered(true);
            latched = true;
        }
        return isTriggered() ? 0 : 1000;
    }

    /**
     * This is called when the event is triggered.
     */
    void exec() override {
        latched = false;
        Serial.print("Analog event triggered, reading=");
        Serial.println(lastReading);
    }
};

// Create an instance of the event.
AnalogInExceedsEvent analogEvent(A0, 500);

void setup() {
    pinMode(A0, INPUT);
    taskManager.registerEvent(&analogEvent);
}

//
// This is the regular loop method for a taskManager sketch, just repeatedly calls runLoop. If you want to use
// this library in a low power situation, then you can use a low power library for your board, see the comments
// within loop. If you use a low power delay function, it must be able to come out of sleep for interrupts on the
// pins that you have interrupts, otherwise, taskManager will not work.
//
void loop() {
    taskManager.runLoop();

    // Here's an example of what you can do in low power situations to reduce power usage.
    //auto microsToTask = taskManager.microsToNextTask();
    //myLowPowerDelayFunction(microsToTask);
}
