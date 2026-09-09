
#include <thread>
#include <TaskManagerIO.h>

#include "unity.h"

static int theCounterToBump = 0;

class TriggerableEvent : public BaseEvent {
private:
    std::atomic<int> triggers = 0;

public:
    void exec() override {
        triggers.fetch_add(1);
    }
    uint32_t timeOfNextCheck() override {
        return 100000;
    }

    bool hasFinished() const {
        return triggers.load() >= 100 && theCounterToBump >= 100;
    }
};

static TriggerableEvent trigEvt;


static void bumpCount() {
    theCounterToBump++;
}

void testMultiThreadedAccessToTaskManager() {
    taskManager.registerEvent(&trigEvt);
    std::jthread otherThread([] {
        for (int i=0;i<100; i++) {
            trigEvt.markTriggeredAndNotify();
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }

        for (int i=0; i<100;i++) {
            taskManager.execute(bumpCount);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    const auto millisStart = millis();
    while (!trigEvt.hasFinished() && (millis() - millisStart) < 10000) {
        taskManager.runLoop();
    }
    TEST_ASSERT_TRUE(trigEvt.hasFinished());

    otherThread.join();
    taskManager.reset();
}