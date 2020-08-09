/*
 * Copyright (c) 2018-present https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "TaskPlatformDeps.h"
#include "TaskManager.h"

#undef ISR_ATTR
#if defined(ESP8266)
# define ISR_ATTR ICACHE_RAM_ATTR
#elif defined(ESP32)
# define ISR_ATTR IRAM_ATTR
#else
# define ISR_ATTR
#endif

TaskManager taskManager;

ISR_ATTR void TaskManager::markInterrupted(pinid_t interruptNo) {
	taskManager.lastInterruptTrigger = interruptNo;
	taskManager.interrupted = true;
}

TaskManager::TaskManager() {
	this->numberOfSlots = DEFAULT_TASK_SIZE;
	interrupted = false;
	first = nullptr;
	interruptCallback = nullptr;
	lastInterruptTrigger = 0;
	tasks = new TimerTask[DEFAULT_TASK_SIZE];
}

int TaskManager::findFreeTask() {
	for (taskid_t i = 0; i < numberOfSlots; ++i) {
		if (!tasks[i].isInUse()) {
			return i;
		}
	}

	int newSlots = numberOfSlots + DEFAULT_TASK_SIZE;
	auto newTasks = new TimerTask[newSlots];
	if(newTasks != nullptr) {
	    int firstNewSlot = numberOfSlots;
	    memcpy(newTasks, tasks, sizeof(TimerTask) * numberOfSlots);
	    tasks = newTasks;
	    numberOfSlots = newSlots;

        return firstNewSlot;
	}

	return TASKMGR_INVALIDID;
}

inline uint32_t toTimerValue(uint32_t v, TimerUnit unit) {
	if (unit == TIME_MILLIS && v > TIMER_MASK) {
		unit = TIME_SECONDS;
		v = v / 1000U;
	}
	return (v & TIMER_MASK) | (((uint32_t)unit) << 28U);
}

taskid_t TaskManager::scheduleOnce(uint32_t when, TimerFn timerFunction, TimerUnit timeUnit) {
    TaskLocker locker;
	auto taskId = findFreeTask();
	if (taskId != TASKMGR_INVALIDID) {
		tasks[taskId].initialise(toTimerValue(when, timeUnit) | TASK_IN_USE, timerFunction);
		putItemIntoQueue(&tasks[taskId]);
	}
	return taskId;
}

taskid_t TaskManager::scheduleFixedRate(uint32_t when, TimerFn timerFunction, TimerUnit timeUnit) {
    TaskLocker locker;
	auto taskId = findFreeTask();
	if (taskId != TASKMGR_INVALIDID) {
		tasks[taskId].initialise(toTimerValue(when, timeUnit) | TASK_IN_USE | TASK_REPEATING, timerFunction);
		putItemIntoQueue(&tasks[taskId]);
	}
	return taskId;
}

taskid_t TaskManager::scheduleOnce(uint32_t when, Executable* execRef, TimerUnit timeUnit, bool deleteWhenDone) {
    TaskLocker locker;
	auto taskId = findFreeTask();
	if (taskId != TASKMGR_INVALIDID) {
		tasks[taskId].initialise(toTimerValue(when, timeUnit) | TASK_IN_USE, execRef, deleteWhenDone);
		putItemIntoQueue(&tasks[taskId]);
	}
	return taskId;
}

taskid_t TaskManager::scheduleFixedRate(uint32_t when, Executable* execRef, TimerUnit timeUnit, bool deleteWhenDone) {
    TaskLocker locker;
	auto taskId = findFreeTask();
	if (taskId != TASKMGR_INVALIDID) {
		tasks[taskId].initialise(toTimerValue(when, timeUnit) | TASK_IN_USE | TASK_REPEATING, execRef, deleteWhenDone);
		putItemIntoQueue(&tasks[taskId]);
	}
	return taskId;
}

taskid_t TaskManager::registerEvent(BaseEvent *eventToAdd, bool deleteWhenDone) {
    TaskLocker locker;
    auto taskId = findFreeTask();
    if(taskId != TASKMGR_INVALIDID) {
        tasks[taskId].initialiseEvent(eventToAdd, deleteWhenDone);
    }
    return taskId;
}

void TaskManager::cancelTask(taskid_t task) {
    TaskLocker locker;
	if (task < numberOfSlots) {
		removeFromQueue(&tasks[task]);
		tasks[task].clear();
	}
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "LoopDoesntUseConditionVariableInspection"
void TaskManager::yieldForMicros(uint32_t microsToWait) {
	yield();

	unsigned long microsStart = micros();
	do {
        runLoop();
	} while((micros() - microsStart) < microsToWait);
}
#pragma clang diagnostic pop

void TaskManager::dealWithInterrupt() {
    interrupted = false;
    if(interruptCallback != nullptr) interruptCallback(lastInterruptTrigger);

    auto task = first;
    while(task != nullptr) {
        if(task->isEvent()) task->processEvent();
        task = task->getNext();
    }
}

void TaskManager::runLoop() {
	// when there's an interrupt, we marshall it into a timer interrupt.
	if (interrupted) dealWithInterrupt();

	// go through the timer (scheduled) tasks in priority order. they are stored
	// in a linked list ordered by first to be scheduled. So we only go through
	// these until the first one that isn't ready.
	TimerTask* tm = first;
	while(tm != nullptr) {
#if defined(ESP8266) || defined(ESP32)
	    // here we are making extra sure we are good citizens on ESP boards
	    yield();
#endif
        {
            // here we start a new block to ensure that we lock while we test if execution is needed and
            // then find the next task.
            TaskLocker locker;
            if (!tm->isReady()) {
                break;
            }
            tm = tm->getNext();
        }

        // by here we know that the task is in use. If it's in use nothing will touch it until it's marked as
        // available. We can do this part without a lock, knowing that we are the only thing that will touch
        // the task. We further know that all non-immutable fields on TimerTask are volatile.
        tm->execute();
	}
}

void TaskManager::putItemIntoQueue(TimerTask* tm) {

	// shortcut, no first yet, so we are at the top!
	if (first == nullptr) {
		first = tm;
		tm->setNext(nullptr);
		return;
	}

	// if we are the new first..
	if (first->microsFromNow() > tm->microsFromNow()) {
		tm->setNext(first);
		first = tm;
		return;
	}

	// otherwise we have to find the place in the queue for this item by time
	TimerTask* current = first->getNext();
	TimerTask* previous = first;

	while (current != nullptr) {
		if (current->microsFromNow() > tm->microsFromNow()) {
			previous->setNext(tm);
			tm->setNext(current);
			return;
		}
		previous = current;
		current = current->getNext();
	}

	// we are at the end of the queue
	previous->setNext(tm);
	tm->setNext(nullptr);
}

void TaskManager::removeFromQueue(TimerTask* tm) {
	
	// shortcut, if we are first, just remove us by getting the next and setting first.
	if (first == tm) {
		first = tm->getNext();
		tm->setNext(nullptr);
		return;
	}

	// otherwise, we have a single linked list, so we need to keep previous and current and
	// then iterate through each item
	TimerTask* current = first->getNext();
	TimerTask* previous = first;

	while (current != nullptr) {

		// we've found the item, unlink it from the queue and nullify its next.
		if (current == tm) {
			previous->setNext(current->getNext());
			current->setNext(nullptr);
			break;
		}

		previous = current;
		current = current->getNext();
	}
}

ISR_ATTR void interruptHandler1() {
	taskManager.markInterrupted(1);
}
ISR_ATTR void interruptHandler2() {
	taskManager.markInterrupted(2);
}
ISR_ATTR void interruptHandler3() {
	taskManager.markInterrupted(3);
}
ISR_ATTR void interruptHandler4() {
	taskManager.markInterrupted(4);
}
ISR_ATTR void interruptHandler5() {
	taskManager.markInterrupted(5);
}
ISR_ATTR void interruptHandler6() {
	taskManager.markInterrupted(6);
}
ISR_ATTR void interruptHandler7() {
	taskManager.markInterrupted(7);
}
ISR_ATTR void interruptHandler8() {
	taskManager.markInterrupted(8);
}
ISR_ATTR void interruptHandler9() {
	taskManager.markInterrupted(9);
}
ISR_ATTR void interruptHandler10() {
	taskManager.markInterrupted(10);
}
ISR_ATTR void interruptHandler11() {
	taskManager.markInterrupted(11);
}
ISR_ATTR void interruptHandler12() {
	taskManager.markInterrupted(12);
}
ISR_ATTR void interruptHandler13() {
	taskManager.markInterrupted(13);
}
ISR_ATTR void interruptHandler14() {
	taskManager.markInterrupted(14);
}
ISR_ATTR void interruptHandler15() {
	taskManager.markInterrupted(15);
}
ISR_ATTR void interruptHandler18() {
	taskManager.markInterrupted(18);
}
ISR_ATTR void interruptHandlerOther() {
	taskManager.markInterrupted(0xff);
}

void TaskManager::addInterrupt(InterruptAbstraction* ioDevice, pinid_t pin, uint8_t mode) {
	if (interruptCallback == nullptr) return;

	switch (pin) {
	case 1: ioDevice->attachInterrupt(pin, interruptHandler1, mode); break;
	case 2: ioDevice->attachInterrupt(pin, interruptHandler2, mode); break;
	case 3: ioDevice->attachInterrupt(pin, interruptHandler3, mode); break;
	case 4: ioDevice->attachInterrupt(pin, interruptHandler4, mode); break;
	case 5: ioDevice->attachInterrupt(pin, interruptHandler5, mode); break;
	case 6: ioDevice->attachInterrupt(pin, interruptHandler6, mode); break;
	case 7: ioDevice->attachInterrupt(pin, interruptHandler7, mode); break;
	case 8: ioDevice->attachInterrupt(pin, interruptHandler8, mode); break;
	case 9: ioDevice->attachInterrupt(pin, interruptHandler9, mode); break;
	case 10: ioDevice->attachInterrupt(pin, interruptHandler10, mode); break;
	case 11: ioDevice->attachInterrupt(pin, interruptHandler11, mode); break;
	case 12: ioDevice->attachInterrupt(pin, interruptHandler12, mode); break;
	case 13: ioDevice->attachInterrupt(pin, interruptHandler13, mode); break;
	case 14: ioDevice->attachInterrupt(pin, interruptHandler14, mode); break;
	case 15: ioDevice->attachInterrupt(pin, interruptHandler15, mode); break;
	case 18: ioDevice->attachInterrupt(pin, interruptHandler18, mode); break;
	default: ioDevice->attachInterrupt(pin, interruptHandlerOther, mode); break;
	}
}

void TaskManager::setInterruptCallback(InterruptFn handler) {
	interruptCallback = handler;
}

char* TaskManager::checkAvailableSlots(char* data, int dataSize) const {
	taskid_t i;
	taskid_t slots = numberOfSlots;
	auto len = min(taskid_t(dataSize - 1), slots);
	for (i = 0; i < len; ++i) {
		data[i] = tasks[i].isRepeating() ? 'R' : (tasks[i].isInUse() ? 'U' : 'F');
		if (tasks[i].isRunning()) data[i] = tolower(data[i]);
	}
	data[i] = 0;
	return data;
}

#ifdef IOA_USE_MBED

Mutex TaskLocker::taskMutex("task");

volatile bool timingStarted = false;
Timer ioaTimer;

void yield() {
    ThisThread::yield();
}

unsigned long millis() {
    if(!timingStarted) {
        timingStarted = true;
        ioaTimer.start();
    }
    return ioaTimer.read_ms();
}

unsigned long micros() {
    if(!timingStarted) {
        timingStarted = true;
        ioaTimer.start();
    }
    return (unsigned long) ioaTimer.read_high_resolution_us();
}

#endif
