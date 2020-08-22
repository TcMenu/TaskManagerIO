## TaskManagerIO scheduling and event based library for Arudino and mbed

### Library status

**This library is under heavy development and is not yet ready for anyone to use in their projects. Continue to use the task manager in IoAbstraction for the moment**. 

Current status is all functions are working on mbed and ESP8266. See issues for what's left.

### What this library supports:

* Simple coroutines style task management, execute now, at a point in time, or on a schedule.
* Ability to add events from different threads for either delayed or immediate execution. 
* Polled event based programming where you set a schedule to be asked if you're event is ready to fire.
* Interrupt and external thread event based programming, where you create an event then trigger it from an interrupt or other thread.
* Marshalled interrupt support, where task manager handles the raw interrupt ISR, and then calls your interrupt task.

### Known working and supported boards:

| CPU / OS  | Boards using CPU  | Status  |
| --------- | ----------------- | ------- |
| ARM mbed  | STM32, nRF.       | Working |
| ESP8266   | Node MCU, Huzzah  | Working |
| ESP32     | Wifi32, Huzzah32  | Not yet |
| SAMD ARM  | MKR, IoT, Zero.   | Not yet |
| AVR       | Uno, Mega Mighty  | Not yet |


## What is TaskManagerIO?

TaskManagerIO library is not a full RTOS, rather it can be used on top of FreeRTOS via ESP32 or mbed RTOS. It is a complimentary technology that can assist with certain types of work-load. It has a major advantage, that the same code runs on many platforms as listed above. It is a core building block of [IoAbstraction](https://github.com/davetcc/IoAbstraction)

## Helping out

At the moment the library is under heavy development. Please contact me first either using an issue here or [https://www.thecoderscorner.com/contact-us/] so that we don't duplicate effort.
 
