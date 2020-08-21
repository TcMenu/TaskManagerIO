/*
 * Copyright (c) 2018-present https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */


#include "TaskTypes.h"
#include "TaskManager.h"

/**
 * A small internal helper class that manages the running state by scope. Create this lightweight class as a local
 * variable and it will control the running state appropriately for the managed TimerTask object.
 */
class RunningState {
private:
    TimerTask* task;
public:
    explicit RunningState(TimerTask* task_) {
        task = task_;
        task->markRunning();
    }

    ~RunningState() {
        task->clearRunning();
    }
};

TimerTask::TimerTask() {
    // set everything to not in use.
    timingInformation = TIME_MILLIS;
    myTimingSchedule = 0;
    callback = nullptr;
    executeMode = EXECTYPE_FUNCTION;
    tm_internal::atomicWritePtr(&next, nullptr);
    tm_internal::atomicWriteBool(&taskInUse, false);
}

void TimerTask::initialise(sched_t execInfo, TimerUnit unit, TimerFn execCallback) {
    this->myTimingSchedule = execInfo;
    this->timingInformation = unit;
    this->callback = execCallback;
    this->executeMode = EXECTYPE_FUNCTION;

    this->scheduledAt = (isJobMicros(timingInformation)) ? micros() : millis();
    tm_internal::atomicWritePtr(&next, nullptr);
}

void TimerTask::initialise(uint32_t execInfo, TimerUnit unit, Executable* execCallback, bool deleteWhenDone) {
    this->myTimingSchedule = execInfo;
    this->timingInformation = unit;
    this->taskRef = execCallback;
    this->executeMode = deleteWhenDone ? ExecutionType(EXECTYPE_EXECUTABLE | EXECTYPE_DELETE_ON_DONE) : EXECTYPE_EXECUTABLE;

    this->scheduledAt = (isJobMicros(timingInformation)) ? micros() : millis();
    tm_internal::atomicWritePtr(&next, nullptr);
}

void TimerTask::initialiseEvent(BaseEvent* event, bool deleteWhenDone) {
    this->timingInformation = (TimerUnit)(TIME_MICROS | TM_TIME_REPEATING);
    this->myTimingSchedule = 0;
    this->eventRef = event;
    this->executeMode = deleteWhenDone ? ExecutionType(EXECTYPE_EVENT | EXECTYPE_DELETE_ON_DONE) : EXECTYPE_EVENT;

    this->scheduledAt = micros();
    tm_internal::atomicWritePtr(&next, nullptr);
}

bool TimerTask::isReady() {
    if (!isInUse() || isRunning()) return false;

    if ((isJobMicros(timingInformation)) != 0) {
        uint32_t delay = myTimingSchedule;
        return (micros() - scheduledAt) >= delay;
    }
    else if(isJobSeconds(timingInformation)) {
        uint32_t delay = uint32_t(myTimingSchedule) * 1000UL;
        return (millis() - scheduledAt) >= delay;
    }
    else {
        uint32_t delay = myTimingSchedule;
        return (millis() - scheduledAt) >= delay;
    }
}

unsigned long TimerTask::microsFromNow() {
    uint32_t microsFromNow;
    if (isJobMicros(timingInformation)) {
        uint32_t delay = myTimingSchedule;
        int32_t alreadyTaken = (micros() - scheduledAt);
        microsFromNow =  (delay < alreadyTaken) ? 0 : (delay - alreadyTaken);
    }
    else {
        uint32_t delay = myTimingSchedule;
        if (isJobSeconds(timingInformation)) {
            delay *= 1000UL;
        }
        uint32_t alreadyTaken = (millis() - scheduledAt);
        microsFromNow = (delay < alreadyTaken) ? 0 : ((delay - alreadyTaken) * 1000UL);
    }
    return microsFromNow;
}

void TimerTask::execute() {
    if (callback == nullptr) return;

    RunningState runningState(this);

    auto execType = (ExecutionType) (executeMode & EXECTYPE_MASK);
    switch (execType) {
        case EXECTYPE_EVENT:
            processEvent();
            return;
        case EXECTYPE_EXECUTABLE:
            taskRef->exec();
            break;
        case EXECTYPE_FUNCTION:
        default:
            callback();
            break;
    }

    if (isRepeating()) {
        this->scheduledAt = isJobMicros(timingInformation) ? micros() : millis();
    }
}

void TimerTask::clear() {
    // if needed delete the event/executable object and then clear it.
    if((executeMode & EXECTYPE_DELETE_ON_DONE) != 0 && taskRef != nullptr) {
        delete taskRef;
    }
    callback = nullptr;

    // clear timing info
    scheduledAt = 0;
    timingInformation = TIME_MILLIS;

    // lastly remove the next pointer and then mark as available.
    tm_internal::atomicWritePtr(&next, nullptr);
    tm_internal::atomicWriteBool(&taskInUse, false);
}

void TimerTask::processEvent() {
    if(eventRef->isTriggered()) {
        eventRef->setTriggered(false);
        eventRef->exec();
    }

    if(eventRef->isComplete()) {
        clear();
    }
    else {
        myTimingSchedule = eventRef->timeOfNextCheck();
        scheduledAt = micros();
    }
}

void BaseEvent::markTriggeredAndNotify() {
    setTriggered(true);
    taskMgrAssociation->triggerEvents();
}
