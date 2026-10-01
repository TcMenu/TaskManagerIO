## TaskManagerIO scheduling and event based library for Arudino and mbed
[![Build](https://github.com/TcMenu/TaskManagerIO/actions/workflows/build.yml/badge.svg)](https://github.com/TcMenu/TaskManagerIO/actions/workflows/build.yml)
[![Native C++ Tests](https://github.com/TcMenu/tcLibraryDev/actions/workflows/native-tests.yml/badge.svg)](https://github.com/TcMenu/tcLibraryDev/actions/workflows/native-tests.yml)
[![License: Apache 2.0](https://img.shields.io/badge/license-Apache--2.0-green.svg)](https://github.com/TcMenu/TaskManagerIO/blob/main/LICENSE)
[![GitHub release](https://img.shields.io/github/release/TcMenu/TaskManagerIO.svg?maxAge=3600)](https://github.com/TcMenu/TaskManagerIO/releases)
[![davetcc](https://img.shields.io/badge/davetcc-dev-blue.svg)](https://github.com/davetcc)
[![JSC TechMinds](https://img.shields.io/badge/JSC-TechMinds-green.svg)](https://www.jsctm.cz)

TcMenu organisation made this library available for you to use. It takes significant effort to keep all our libraries current and working on a wide range of boards. Please consider making at least a one off donation via the sponsor button if you find it useful. In forks, please keep text to here intact.

<a href="https://www.buymeacoffee.com/davetcc" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-blue.png" alt="Buy Me A Coffee" style="height: 60px !important;width: 217px !important;" ></a>

TaskManagerIO is an evolution of the task management class that was originally situated in IoAbstraction. It is backed by a simple queue that supports: immediate queuing, scheduled tasks, and events. It is safe to add tasks from another thread, and safe to trigger events from interrupts. However, your tasks are shielded from threads and interrupts making your code simpler.

We are in a new era of embedded development, where RTOS, multiple threads (and even cores) have become a relatity. Any viable task manager needs to be capable in these environments, while still protecting tasks from multithreaded concerns. We are pleased to say, this version meets both goals. Importantly, any sketch that worked on IoAbstraction task manager will work with this library unaffected. 

Along with this library working on most Arduino based devices, it is also tested by us on PicoSDK and ESP-IDF with an Arduino component..

Below, we list the main features of TaskManagerIO:

* Simple coroutines style task management, execute now, at a point in time, or on a schedule.
* Your tasks do not need to be thread or interrupt safe, they will only be called from task manager.
* Ability to add events that can be triggered from different threads or interrupts, for either delayed or ASAP execution. Again, always called on the task manager thread.
* Polled event based programming where you set a schedule to be asked if your event is ready to fire.
* Marshalled interrupt support, where task manager handles the raw interrupt ISR, and then calls your interrupt task.

## Getting started with taskManager

Youtube guide that goes through most important concepts: https://youtu.be/N1ILzBfu5Zc

Here we just demonstrate the most basic usage. Take a look at the examples for more complex cases, or the reference documentation that's built from the source.

Include the header file:

```
#include <TaskManagerIO.h>
```

In the setup method, add an function callback that gets fired once in the future:

```
	// Create a task scheduled once every 100 miliis
	taskid_t taskId = taskManager.schedule(repeatMillis(100), [] {
		// some work to be done.
	});
	
	// Create a task that's scheduled every second
	taskid_t taskId = taskManager.schedule(repeatSeconds(1), [] {
		// work to be done.
	});
```

From 1.2 onwards: On most 32 bit Arduino boards you can also *enable* argument capture in lambda expressions. By default, the feature is off because it is quite a heavy feature that many may never be used.

To enable add the following flag to your compile options, and it will be enabled if the board supports it: `-DTM_ENABLE_CAPTURED_LAMBDAS`

An example of this usage follows:

```
    int capturedValue = 42;
    taskManager.schedule(repeatSeconds(2), [capturedValue]() {
        log("Execution with captured value = ", capturedValue);
    });

```

You can also create a class that extends from `Executable` and schedule that instead. For example:

```
    // Create the class
    class MyClassToSchedule : public Executable {
        //... your other stuff

        void exec() override {
            // your code to be executed upon schedule.
        }
    };
    
    // Create an instance
    MyClassToSchedule myClass;
    
    // Register with taskManager for once a second execution
    taskManager.schedule(repeatSeconds(1), &myClass);
```

The helper time functions that can be passed as the first time parameter are:

* `onceMicros(N)` run once in N microseconds
* `onceMillis(N)` run once in N milliseconds
* `onceSeconds(N)` run once in N seconds
* `repeatMicros(N)` repeatedly run in N microseconds
* `repeatMillis(N)` repeatedly run in N milliseconds
* `repeatSeconds(N)` repeatedly run in N seconds

Then in the loop method you need to call: 

```
  void loop() {
  	 taskManager.runLoop();
  }
```

To schedule tasks that have an interval of longer than an hour, use the long schedule support as follows (more details in the longSchedule example):

First create a long schedule either globally or using the new operator:

    TmLongSchedule hourAndHalfSchedule(makeHourSchedule(1, 30), &myTaskExec); // runs until cancelled
    TmLongSchedule hourAndHalfSchedule(makeHourSchedule(1, 30), &myTaskExec, runOnlyOnce);

Then add it to task manager during setup or as needed:

    taskManager.registerEvent(&hourAndHalfSchedule);
    
After this the callback (or event object) registered in the TmLongSchedule will be called whenever scheduled. 

To enable or disable a task

	taskManager.setTaskEnabled(taskId, enabled);

If you have a shared resource that you need to lock around, you can do this in tasks. See the reentrantLocking example for more details.

Arduino Only - If you want to use the legacy interrupt marshalling support instead of building an event you must additionally include the following:

	#include <BasicInterruptAbstraction.h>


## Further documentation and getting help

* [TaskManagerIO documentation pages](https://www.thecoderscorner.com/products/arduino-libraries/taskmanager-io/)
* [TaskManagerIO reference documentation](https://www.thecoderscorner.com/ref-docs/taskmanagerio/html)

Community questions can be asked in the discussions section of this repo, or using the Arduino forum. We generally answer most community questions but the responses will not be timely. Before posting into the community make sure you've recreated the problem in a simple sketch, and please consider making at least a one time donation (see links further up):

<a href="https://www.buymeacoffee.com/davetcc" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-blue.png" alt="Buy Me A Coffee" style="height: 60px !important;width: 217px !important;" ></a>

* [discussions section of the Task Manager repo](https://github.com/TcMenu/TaskManagerIO/discussions)
* [Arduino discussion forum](https://forum.arduino.cc/) where questions can be asked, please tag me using `@davetcc`.
* [Legacy discussion forum probably to be made read only soon](https://www.thecoderscorner.com/jforum/).

## Creating and using a list

We must understand that this list is associative and sorted by a key, it is based on a binary search algorithm, so it is relatively slow to insert into the underlying array as it will need to be inserted into the array at the right point. However, in return for this, lookup by key is very fast - in big-O notation it is approximately Log(N) or in simple terms to look up in a 256 item list by key would take maximum 8 iterations. However, insertion carries a possible copy penalty if the items need reordering.

All collections in this library are in the namespace tccollection, by default SimpleCollection.h adds a statement to use this namespace automatically.

### Restrictions on what you put in the list

This list works by copying items into the list, so the things you store must follow a couple of simple rules.

The key type can be any type that is 4 bytes or fewer. This is a limitation of the underlying way we implement the storage, to significantly reduce compiled sizes on smaller boards. For example, it could be `int`, `uint32_t`, `unit8_t` etc.

* The type must have a default constructor and a copy constructor.
* The type must have an assignment operator
* It must expose a `getKey` method that returns the key type and marked as const.
* It is best to stick to quite simple classes, as during insert operations they will be copied.
* If you want to use this as a general purpose list and are not interested in ordering, just make getKey return a larger number for each item you add.

## Quick start - create a list, iterate, get by key

Contents of the iteration example to get you started, you can either copy into your ide or open the iteration example. In short, first we create the MyStorage type that will be stored in the list, it has a key of type uint8_t. We then create the list object, populating it in the `setup()` method. In the loop we then read back the values using various techniques.

    #include <Arduino.h>
    #include <BTreeList.h>

    class MyStorage {
    private:
        uint8_t key;
        uint32_t value;
    public:
        // we must define
        MyStorage() = default;
        MyStorage(const MyStorage& other) = default;
        MyStorage& operator=(const MyStorage& other) = default;
    
        MyStorage(uint8_t key, uint32_t value) : key(key), value(value) {}
    
        uint8_t getKey() const { return key; }
        uint32_t getValue() const { return value; }
    };

    BtreeList<uint8_t, MyStorage> myList;

    void setup() {
        myList.add(MyStorage(0, 2093));
        myList.add(MyStorage(1, 0xf00dface));
        myList.add(MyStorage(2, 0xdeadbeef));
    }

    void loop() {
        Serial.println("Range iteration");
        for(auto item : myList) {
            Serial.println(item.getValue());
        }

        Serial.println("ForEach iteration");
        myList.forEachItem([] (MyStorage* storage) {
            Serial.println(storage->getValue());
        });
    
        auto item = myList.getByKey(2);
        if(item) {
            Serial.println("Get By Key");
            Serial.println(item->getValue());
        }
        else {
            Serial.println("Get By Key Failed");
        }
    
        Serial.println("Count and capacity");
        Serial.println(myList.count());
        Serial.println(myList.capacity());
        delay(4000);
    }

## List sizing and defaults

On Uno, the initial number of items is lowered to 5 by default, with grow mode set to grow by 5 each time, you can lower this in the constructor if needed. On MEGA2560 it will start with 10 items, and grow by 5 each time. On all 32 bit boards it will start at 10 and double each time. To change the default use the following method

    BtreeList<KeyType, Value> myList(size, howToGrow)

Where the size is the initial capacity of the list, and the grow by mode is one of: `GROW_NEVER, GROW_BY_5, GROW_BY_DOUBLE`

## Other helpful methods

    bsize_t nearestLocation(const K& key) // get the location nearet to key

    const V* items() // get the underlying item array.

    V* itemAtIndex(bsize_t idx) // get the item at a particular index

    bsize_t count() // the number of items in the list

    bsize_t capacity() // the current allocated size of the array

## Concurrent Circular Buffers

This library also supports concurrent circular buffers that work on most boards listed below. These buffers have independent writer and reader positions. This means that one thread can offer data, and another thread can read that data. It is entirely non-blocking and therefore safe across threads or even in interrupts. Be aware that if used in interrupts, the writer position is managed using CAS instructions (or emulation thereof) and will be slow if more than one thread does the writing (because of busy spin waiting).

There is an example that shows the usage of the circular buffer, but the API is really simple.

### Creating a circular buffer for storage of bytes (uint8_t)

We first create an instance and indicate the size needed, the size is fixed and if the writer exceeds the reader, it will wrap and data is lost. See further down for circular buffers of more complex types.

    #include <SimpleCollections.h>
    #include <SCCircularBuffer.h>

    SCCircularBuffer buffer(20);

### Writing to the buffer

Write a byte to the buffer by calling `put`, it will **wrap** if the reader gets behind.

    buffer.put(dataByte);

### Reading and checking the buffer

Only ever call `get` after checking that data is `available`, only one thread should ever be reading at once.

    if(buffer.available()) {
        uint8_t data = buffer.get();
        // do something with "data"
    }

## Creating a GenericCircularBuffer for a type other than uint8_t

You can create a circular buffer for type other than byte, to do so, you use the `GenericCircularBuffer` instead. It takes a type parameter and is a template, so only use when you need to store other than byte in it.

Bear in mind, that if the item you are storing in the circular buffer is not atomic, such as a pointer, or a machine length word, you risk it being corrupt when you see it on the other thread. To get around this we recommend that you have two circular buffers, one acting as a memory pool, and the other as the actual buffer. They should be the same type:

    // let's say we want to store this structure in the buffer 
    struct WriterStruct {
        volatile uint32_t sequence;
        volatile uint32_t data1;
        volatile uint32_t data2;

        void setData(uint32_t s, uint32_t d1, uint32_t d2) {
            sequence = s;
            data1 = d1;
            data2 = d2;
        }
    };

    // we first create a buffer that acts as a pool, notice the 2nd parameter. It has the same number of above structures as the actual queue
    GenericCircularBuffer<WriterStruct> writerMemoryAlloc(10,GenericCircularBuffer::MEMORY_POOL);
    // We then create the actual buffer, it takes pointers to the structure
    GenericCircularBuffer<WriterStruct*> actualBuffer(10);

    void putSomethingIntoQueue() {
        // first we get the next available structure from the pool
        auto &alloc = writerMemoryAlloc.get();
        // now we prepare it to be sent, it must be entirely ready!
        alloc.setData(nextSequence, nextSequence * 1000, nextSequence * 2000);
        // now we send it.
        actualBuffer.put(&alloc);
    }

The queue is read back as normal, but we get back a pointer.

    if(actualBuffer.available()) {
        auto myData = actualBuffer.get();
        auto localData1 = myData->data1;
    }

In short, you should never queue an object until it is fully and atomically ready. Again, just like with circular buffers themselves, the memory pool will wrap if the writer gets too far ahead of the reader.


## Known working and supported boards:

https://www.thecoderscorner.com/products/arduino-libraries/

Many thanks to contributors for helping us to confirm that this software runs on a wide range of hardware.

## What is TaskManagerIO?

TaskManagerIO library is not a full RTOS, rather it can be used on top of an existing RTOS. It is a complimentary technology that can assist with certain types of work-load. It has a major advantage, that the same code runs on many platforms as listed above. It is a core building block of [IoAbstraction](https://github.com/TcMenu/IoAbstraction) and [tcMenu framework](https://github.com/TcMenu/IoAbstraction)

## Important notes around scheduling tasks and events

TaskManagerIO is a cooperative scheduler, and cooperative schedulers by their very nature have unfair semantics. In practice this means that you should not create repeating events or fixed rate schedules that have a 0 delay, if you do, no other tasks will run because the one with 0 delay will always win.  

## Multi-tasking - advanced usage

TaskManager uses a lock free design, based on "compare and exchange" to acheive thread safety on larger boards, atomic operations on AVR, and interrupt locking back-up on other boards. Below, we discuss the multi-threaded features in more detail.

* On any board, it is safe to add tasks and raise events from any thread. We use whatever atomic operations are available for that board to ensure safety.
* On ESP32 FreeRTOS, PicoSDK, and Arduino RTOS based boards it is safe to add tasks to a taskManager from another core, on these platforms task manager uses the processors compare and exchange functionality to ensure thread safety as much as possible.
* On any board, you can start another thread and run a task manager on it. Only ever call task-manager's runLoop() from the same thread.

## Helping out

We are always glad to accept bug fixes and features. However, please always raise an issue first, and for significant work, it's worth waiting for us to reply first. Please see the contributing guide.
