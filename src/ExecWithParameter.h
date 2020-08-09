
#ifndef TASKMANAGERIO_EXECWITHPARAMETER_H
#define TASKMANAGERIO_EXECWITHPARAMETER_H

#include "TaskTypes.h"

/**
 * Gives the ability to register a task with task manager that will call a function with a stored parameter
 * An example usage would be to store a Stream upon which to write to. Note that if you use the method below
 * you absolutely must provide the deleteWhenDone defaulted parameter as true.
 *
 * taskManager.scheduleFixedRate(1000, new ExecWithParameter<Stream*>(&Serial), TIME_MILLIS, true);
 *
 * To use static memory, create an ExecWithParameter instance **globally**, and then pass a pointer to the schedule.
 *
 * @tparam TParam The type that you want to store until called back.
 */
template <class TParam> class ExecWithParameter : public Executable {
private:
    const void (*fnParam)(TParam);
    const TParam param;
public:
    ExecWithParameter(void (*fn)(TParam), TParam storedParam) : fnParam(fn), param(storedParam){}
    ~ExecWithParameter() override = default;

    void exec() override {
        fnParam(param);
    }
};

#endif //TASKMANAGERIO_EXECWITHPARAMETER_H
