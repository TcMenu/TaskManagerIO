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
    taskid_t getTaskId() const {
        return taskId;
    }
};

/**
 * Definition of a function to be called back when a scheduled event is required. Takes no parameters, returns nothing.
 */
typedef void (*TimerFn)();

/**
 * The time units that can be used with the schedule calls.
 */
enum TimerUnit : uint8_t {
    TIME_MICROS = 0,
    TIME_SECONDS = 1,
    TIME_MILLIS = 2,

    TM_TIME_REPEATING = 0x10,
    TM_TIME_RUNNING = 0x20,

    TIME_REP_MICROS = TIME_MICROS | TM_TIME_REPEATING,
    TIME_REP_SECONDS = TIME_SECONDS | TM_TIME_REPEATING,
    TIME_REP_MILLIS = TIME_MILLIS | TM_TIME_REPEATING,
};

enum ExecutionType : uint8_t {
    EXECTYPE_FUNCTION = 0,
    EXECTYPE_EXECUTABLE = 1,
    EXECTYPE_EVENT = 2,

    EXECTYPE_MASK = 0x03,
    EXECTYPE_DELETE_ON_DONE = 0x08,

    EXECTYPE_DEL_EXECUTABLE = EXECTYPE_EXECUTABLE | EXECTYPE_DELETE_ON_DONE,
    EXECTYPE_DEL_EVENT = EXECTYPE_EVENT | EXECTYPE_DELETE_ON_DONE
};

#define isJobMicros(x)  (((x) & 0x0f)==TIME_MICROS)
#define isJobMillis(x)  (((x) & 0x0f)==TIME_MILLIS)
#define isJobSeconds(x) (((x) & 0x0f)==TIME_SECONDS)

/**
 * Internal class that represents a single task slot. You should never have to deal with this class in user code.
 *
 * Represents a single task or event that will be processed at some point in time. It stores the last evaluation time
 * and also the execution parameters. EG execute every 100 millis.
 */
class TimerTask {
private:
    /** the thing that needs to be executed when the time is reached or event is triggered */
    volatile union {
        TimerFn callback;
        Executable *taskRef;
        BaseEvent *eventRef;
    };
    /** TimerTask is essentially stored in a linked list by time in TaskManager, this represents the next item */
    tm_internal::TimerTaskAtomicPtr next;

    /** the time at which the task was last scheduled, used to compare against the current time */
    volatile uint32_t scheduledAt;
    /** The timing information for this task, or it's interval */
    volatile sched_t myTimingSchedule;

    // 8 bit values start here.

    /** The timing information for this task IE, millis, seconds or micros and if it's running or repeating. */
    volatile TimerUnit timingInformation;
    /** An atomic flag used to indicate if the task is in use, it should be set before doing anything to a task */
    tm_internal::TmAtomicBool taskInUse;
    /** the mode in which the task executes, IE call a function, call an event or executable. Also if memory is owned */
    volatile ExecutionType executeMode;
public:
    TimerTask();

    bool isReady();

    unsigned long microsFromNow();

    void initialise(sched_t executionInfo, TimerUnit unit, TimerFn execCallback);

    void initialise(sched_t executionInfo, TimerUnit unit, Executable *executable, bool deleteWhenDone);

    void initialiseEvent(BaseEvent *event, bool deleteWhenDone);

    bool isInUse() { return tm_internal::atomicReadBool(&taskInUse); }

    bool isRepeating() const {
        if(ExecutionType(executeMode & EXECTYPE_MASK) == EXECTYPE_EVENT) {
            // if it's an event it repeats until the event is considered "complete"
            return !eventRef->isComplete();
        }
        else {
            // otherwise it's based on the task repeating flag
            return 0 != (timingInformation & TM_TIME_REPEATING);
        }
    }

    void clear();

    bool allocateIfPossible() {
        return tm_internal::atomicSwapBool(&taskInUse, false, true);
    }

    void markRunning() { timingInformation = TimerUnit(timingInformation | TM_TIME_RUNNING); }

    void clearRunning() { timingInformation = TimerUnit(timingInformation & ~TM_TIME_RUNNING); }

    bool isRunning() const { return (timingInformation & TM_TIME_RUNNING) != 0; }

    bool isEvent() {
        auto execType = ExecutionType(executeMode & EXECTYPE_MASK);
        return (isInUse() && execType == EXECTYPE_EVENT);
    }

    TimerTask *getNext() { return tm_internal::atomicReadPtr(&next); }

    void setNext(TimerTask *nextTask) { tm_internal::atomicWritePtr(&this->next, nextTask); }

    void execute();

    void processEvent();
};

#endif //TASKMANAGER_IO_TASKTYPES_H
