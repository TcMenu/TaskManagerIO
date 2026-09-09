cmake_minimum_required(VERSION 3.13)

add_library(TaskManagerIO 
        ../src/SimpleSpinLock.cpp
        ../src/TaskManagerIO.cpp
        ../src/TaskTypes.cpp
        ../src/TmLongSchedule.cpp
        ../src/BTreeList.cpp
        ../src/SCCircularBuffer.cpp
        ../src/SimpleSpinLock.cpp
        ../src/platform/ThreadHelperCode.cpp
)

if (BUILD_FOR_STM32CUBE)
    target_compile_definitions(TaskManagerIO
            PUBLIC BUILD_FOR_STM32CUBE_CMAKE=1 IO_LOGGING_DEBUG=1
    )
elseif (BUILD_NATIVE_TESTS)
    target_compile_definitions(TaskManagerIO
            PUBLIC BUILD_FOR_NATIVE_PLATFORM=1 IO_LOGGING_DEBUG=1
    )
else ()
    target_compile_definitions(TaskManagerIO
            PUBLIC BUILD_FOR_PICO_CMAKE=1 BUILD_PICO_FORCE_UART=1 IO_LOGGING_DEBUG=1
    )
endif ()

target_include_directories(TaskManagerIO PUBLIC
        ../src
)

target_link_libraries(TaskManagerIO PUBLIC TcMenuLog)

# only bring in pico libs when compiling for PicoSDK
if(TARGET pico_stdlib AND TARGET pico_sync)
    message("Build is pico, adding libs")
    target_link_libraries(TaskManagerIO PUBLIC pico_stdlib pico_sync)
elseif (TARGET stm32cubemx)
    message("Build is stm32cube adding stm32cubemx")
    target_link_libraries(TaskManagerIO PUBLIC stm32cubemx)
endif()

if (BUILD_NATIVE_TESTS)
    get_unity_if_needed()
    add_executable(test_taskmanager_core
            ../test/test_core/taskManagerCoreTests.cpp
            ../test/test_core/eventTestCases.cpp
            ../test/test_core/HighThroughputTestCases.cpp
            ../test/test_core/reentrantLockingTests.cpp
            ../test/test_core/taskManagerConcurrent.cpp
            ../test/test_core/collectionsTest.cpp
    )

    target_link_libraries(TaskManagerIO PUBLIC)
    target_link_libraries(test_taskmanager_core PRIVATE unity TcMenuLog TaskManagerIO)
    add_test(NAME test_taskmanager_core COMMAND test_taskmanager_core)
endif ()
