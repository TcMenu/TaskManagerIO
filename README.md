## TaskManagerIO scheduling and event based library for Arudino and mbed

### Library status

**This library is under heavy development and is not yet ready for anyone to use in their projects. Continue to use the task manager in IoAbstraction for the moment**. 

Current status is compilation on mbed and simple tasks are working. 

### What this library supports:

* Simple coroutines style task management, execute now, at a point in time, or on a schedule.
* Ability to add events from different threads for either delayed or immediate execution. 
* Polled event based programming where you set a schedule to be asked if you're event is ready to fire.
* Interrupt and external thread event based programming, where you create an event then trigger it from an interrupt or other thread.
* Marshalled interrupt support, where task manager handles the raw interrupt ISR, and then calls your interrupt task.

### Known working and supported boards:



This library is not a full RTOS, rather it can be used on top of FreeRTOS via ESP32 or mbed RTOS. It is a complimentary technology that can assist with certain types of work-load. It has a major advantage, that the same code runs on many platforms as listed below.

 