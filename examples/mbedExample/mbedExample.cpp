/**
 * This example shows how to use task manager specifically with mbed. The API is exactly the same
 * between Arduino and mbed, so you can also look at the Arduino examples for inspiration too.
 *
 * This example starts a task, registers an event, and also registers an interrupt with task manager
 */

#include <mbed.h>
#include <TaskManagerIO.h>

// Here we create a serial object to write log statements to.
BufferedSerial console(USBTX, USBRX, 115200);

// and we also store the taskId of the one second task, to remove it later.
taskid_t oneSecondTask;

// we don't want to log on every run of the microsecond task, it would overwhelm serial so just count instead.
// this class holds a number of ticks and bumps that count on every execute.
class MicrosecondTask : public Executable {
private:
    int ticks{};
public:
    // This is called by task manager when the task is ready to run.
    void exec() {//override {
        ticks++;
    }

    int getCurrentTicks() const {
        return ticks;
    }
};

// here we store a reference to the microsecond task.
MicrosecondTask* microsTask;

void log(const char* toLog) {
    console.write(toLog, strlen(toLog));
    char sz[2];
    sz[0]='\n';
    sz[1]=0;
    console.write(sz, strlen(sz));
}

void log(const char* toLog, int i) {
    console.write(toLog, strlen(toLog));
    char sz[20];
    itoa(i, sz, 10);
    console.write(sz, strlen(sz));
    sz[0]='\n';
    sz[1]=0;
    console.write(sz, strlen(sz));
}

// A job submitted to taskManager can either be a function that returns void and takes no parameters, or a class
// that extends Executable. In this case the job creates a repeating task and logs to the console.
void tenSecondJob() {
    log("30 seconds up, restart a new repeating job");
//    taskManager.scheduleFixedRate(500, [] {
//        log("Half second job");
//        console.write("Micros = ", microsTask.getCurrentTicks());
//    });
}

// Again another task manager function, we pass this as the timerFn argument later on
void twentySecondJob() {
    log("20 seconds up, delete 1 second job, schedule 10 second job");
//    taskManager.scheduleOnce(10, tenSecondJob, TIME_SECONDS);
//    taskManager.cancelTask(oneSecondTask);
}

//
// Set up all the initial tasks and events
//
void setupTasks() {
    // Here we create a new task using milliseconds; which is the default time unit for scheduling. We use a lambda
    // function as the means of scheduling.
    oneSecondTask = taskManager.scheduleFixedRate(1000, [] {
        log("One second job, micro count", microsTask->getCurrentTicks());
    });
//
//    // Here we create a new task based on the twentySecondJob function, that will be called at the appropriate time
//    // We schedule this with a unit of seconds.
//    taskManager.scheduleOnce(20, twentySecondJob, TIME_SECONDS);
//
//    // here we create a new task based on Executable and pass it to taskManager for scheduling. We provide the
//    // time unit as microseconds, and with the last parameter we tell task manager to delete it when the task
//    // is done, IE for one shot tasks that is as soon as it's done, for repeating tasks when it's cancelled.
//    microsTask = new MicrosecondTask();
//    taskManager.scheduleFixedRate(100, microsTask, TIME_MICROS, true);
}

int main() {
    log("starting up taskmanager example");
    setupTasks();

    while(1) {
        taskManager.runLoop();
    }
}
