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
    executionInfo = 0;
    callback = nullptr;
    executeMode = EXECTYPE_FUNCTION;
    tm_internal::atomicWritePtr(&next, nullptr);
    tm_internal::atomicWriteBool(&taskInUse, false);
}

void TimerTask::initialise(uint32_t execInfo, TimerFn execCallback) {
    this->executionInfo = execInfo;
    this->callback = execCallback;
    this->executeMode = EXECTYPE_FUNCTION;

    this->scheduledAt = (isJobMicros(executionInfo)) ? micros() : millis();
    tm_internal::atomicWritePtr(&next, nullptr);
}

void TimerTask::initialise(uint32_t execInfo, Executable* execCallback, bool deleteWhenDone) {
    this->executionInfo = execInfo;
    this->taskRef = execCallback;
    this->executeMode = deleteWhenDone ? ExecutionType(EXECTYPE_EXECUTABLE | EXECTYPE_DELETE_ON_DONE) : EXECTYPE_EXECUTABLE;

    this->scheduledAt = (isJobMicros(executionInfo)) ? micros() : millis();
    tm_internal::atomicWritePtr(&next, nullptr);
}

void TimerTask::initialiseEvent(BaseEvent* event, bool deleteWhenDone) {
    this->executionInfo = TASK_REPEATING;
    this->eventRef = event;
    this->executeMode = deleteWhenDone ? ExecutionType(EXECTYPE_EVENT | EXECTYPE_DELETE_ON_DONE) : EXECTYPE_EVENT;

    this->scheduledAt = micros();
    tm_internal::atomicWritePtr(&next, nullptr);
}

bool TimerTask::isReady() {
    if (!isInUse() || isRunning()) return false;

    if ((isJobMicros(executionInfo)) != 0) {
        uint32_t delay = timeFromExecInfo(executionInfo);
        return (micros() - scheduledAt) >= delay;
    }
    else if(isJobSeconds(executionInfo)) {
        uint32_t delay = timeFromExecInfo(executionInfo) * 1000L;
        return (millis() - scheduledAt) >= delay;
    }
    else {
        uint32_t delay = timeFromExecInfo(executionInfo);
        return (millis() - scheduledAt) >= delay;
    }
}

unsigned long TimerTask::microsFromNow() {
    uint32_t microsFromNow;
    if (isJobMicros(executionInfo)) {
        uint32_t delay = timeFromExecInfo(executionInfo);
        int32_t alreadyTaken = (micros() - scheduledAt);
        microsFromNow =  (delay < alreadyTaken) ? 0 : (delay - alreadyTaken);
    }
    else {
        uint32_t delay = timeFromExecInfo(executionInfo);
        if (isJobSeconds(executionInfo)) {
            delay *= ((uint32_t)1000);
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
        this->scheduledAt = isJobMicros(executionInfo) ? micros() : millis();
    }
    else {
        clear();
    }
}

void TimerTask::clear() {
    if((executeMode & EXECTYPE_DELETE_ON_DONE) != 0 && taskRef != nullptr) {
        delete taskRef;
    }
    callback = nullptr;
    executionInfo = 0;
    tm_internal::atomicWritePtr(&next, nullptr);
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
        executionInfo = executionInfo & (~TIMER_MASK);
        executionInfo |= (eventRef->timeOfNextCheck() & TIMER_MASK);
        scheduledAt = micros();
    }
}

void BaseEvent::markTriggeredAndNotify() {
    setTriggered(true);
    taskMgrAssociation->triggerEvents();
}
