/*
 * Copyright (c) 2018-present https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TASKMANAGER_IO_TASKTYPES_H
#define TASKMANAGER_IO_TASKTYPES_H

#include "TaskPlatformDeps.h"

#define TASKMGR_INVALIDID 0xffff

/**
 * Represents the identifier of a task, it can be used to query, alter and cancel tasks. You should not rely on
 * any characteristics of this type, it could change later, it is essentially no more than a handle to a task.
 */
typedef int taskid_t;

/**
 * Any class extending from executable can be passed by reference to task manager and the exec() method will be called
 * when the scheduled time is reached.
 */
class Executable {
public:
    /**
     * Called when the schedule is reached
     */
    virtual void exec() = 0;

    /**
     * A virtual destructor must always be provided for interfaces that can be allocated with new.
     */
    virtual ~Executable() = default;
};

// forward references to TaskManager to avoid circular include.
class TaskManager;
extern TaskManager taskManager;

/**
 * BaseEvent objects represent events that can be managed by task manager. We can create a base event as either
 * a global variable, or as a new object that lasts at least as long as it's registered with task manager.
 *
 * Events work differently to other tasks, there are two methods to implement, the first method timeOfNextCheck is
 * for events that are polled, and you control how often it is polled by the value you return. To trigger an event
 * you can call setTriggered(bool), and this will set a flag that task manager will look at during the next poll. If
 * you are in the timeOfNextCheck and you set the event as triggered, it will execute immediately. If you are in an
 * interrupt triggered by task manager, it will act on the trigger immediately. However, in other cases you can call
 * markTriggeredAndNotify which additionally notifies task manager, making the effect immediate.
 */
class BaseEvent : public Executable {
private:
    volatile taskid_t taskId;
    TaskManager *taskMgrAssociation;
    volatile bool triggered;
    volatile bool finished;
public:
    explicit BaseEvent(TaskManager *taskMgrToUse = &taskManager) :
            taskId(0xffff), taskMgrAssociation(taskMgrToUse), triggered(false), finished(false) {}

    /**
     * This method must be implemented by all event handlers. It will be called when the event is first registered with
     * task manager, then it will be called in whatever number of micro
     * @return
     */
    virtual uint32_t timeOfNextCheck() = 0;

    /**
     * Set the event as triggered, which means it will be executed next time around
     */
    void setTriggered(bool t) {
        triggered = t;
    }

    /**
     * @return true if the event has triggered and requires execution now
     */
    bool isTriggered() const {
        return triggered;
    }

    /**
     * @return true if the task has completed
     */
    bool isComplete() const {
        return finished;
    }

    /**
     * Sets the task as completed or not completed. Can be called from interrupt safely
     */
    void setCompleted(bool complete) {
        finished = complete;
    }

    /**
     * Set the event as triggered and then tell task manager that the event needs to be
     * re-evaluated. It will not run immediately, rather it will do the equivalent of
     * triggering an interrupt through taskManager. If this event already receives an
     * interrupt through task manager, then there is no need to call notify.
     */
    void markTriggeredAndNotify();

    /**
     * @return the task manager ID for this event, for making calls into task manager.
     */
    taskid_t getTaskId() {
        return taskId;
    }
};

/**
 * Definition of a function to be called back when a scheduled event is required. Takes no parameters, returns nothing.
 */
typedef void (*TimerFn)();

/**
 * Definition of a function to be called back when an interrupt is detected, marshalled by task manager into a task.
 * The pin that caused the interrupt is passed in the parameter on a best efforts basis.
 * @param pin the pin on which the interrupt occurred (best efforts)
 */
typedef void (*InterruptFn)(pinid_t pin);

#define TASK_IN_USE     0x80000000U
#define TASK_REPEATING  0x40000000U
#define TASK_MILLIS     0x20000000U
#define TASK_SECONDS    0x10000000U
#define TASK_MICROS     0x00000000U
#define TIMING_MASKING  0x30000000U
#define TASK_RUNNING    0x08000000U
#define TIMER_MASK      0x01ffffffU

#define isJobMicros(x)  ((x & TIMING_MASKING)==0)
#define isJobMillis(x)  ((x & TIMING_MASKING)==0x2000U)
#define isJobSeconds(x) ((x & TIMING_MASKING)==0x1000U)
#define timeFromExecInfo(x) ((x & TIMER_MASK))

/**
 * The time units that can be used with the schedule calls.
 */
enum TimerUnit : uint8_t {
    TIME_MICROS = 0, TIME_SECONDS = 1, TIME_MILLIS = 2
};

enum ExecutionType : uint8_t {
    EXECTYPE_FUNCTION = 0,
    EXECTYPE_EXECUTABLE = 1,
    EXECTYPE_EVENT = 2,

    EXECTYPE_MASK = 0x03,
    EXECTYPE_DELETE_ON_DONE = 0x08
};

/**
 * Internal class that represents a single task slot. You should never have to deal with this class in user code.
 *
 * Represents a single task or event that will be processed at some point in time. It stores the last evaluation time
 * and also the execution parameters. EG execute every 100 millis.
 */
class TimerTask {
private:
    volatile uint32_t executionInfo;
    volatile uint32_t scheduledAt;
    volatile union {
        TimerFn callback;
        Executable *taskRef;
        BaseEvent *eventRef;
    };
    TimerTask* volatile next;
    volatile ExecutionType executeMode;
public:
    TimerTask();

    bool isReady() const;

    unsigned long microsFromNow() const;

    void initialise(uint32_t executionInfo, TimerFn execCallback);

    void initialise(uint32_t executionInfo, Executable *executable, bool deleteWhenDone);

    void initialiseEvent(BaseEvent *event, bool deleteWhenDone);

    bool isInUse() const { return 0 != (executionInfo & TASK_IN_USE); }

    bool isRepeating() const {
        if(ExecutionType(executeMode & EXECTYPE_MASK) == EXECTYPE_EVENT) {
            // if it's an event it repeats until the event is considered "complete"
            return !eventRef->isComplete();
        }
        else {
            // otherwise it's based on the task repeating flag
            return 0 != (executionInfo & TASK_REPEATING);
        }
    }

    void clear();

    void markRunning() { executionInfo |= TASK_RUNNING; }

    void clearRunning() { executionInfo &= ~TASK_RUNNING; }

    bool isRunning() const { return (executionInfo & TASK_RUNNING) != 0; }

    bool isEvent() const {
        auto execType = ExecutionType(executeMode & EXECTYPE_MASK);
        return (isInUse() && execType == EXECTYPE_EVENT);
    }

    TimerTask *getNext() { return next; }

    void setNext(TimerTask *nextTask) { this->next = nextTask; }

    void execute();

    void processEvent();
};

#endif //TASKMANAGER_IO_TASKTYPES_H
