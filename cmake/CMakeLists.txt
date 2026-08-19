cmake_minimum_required(VERSION 3.13)

add_library(TaskManagerIO 
        ../src/SimpleSpinLock.cpp
        ../src/TaskManagerIO.cpp
        ../src/TaskTypes.cpp
        ../src/TmLongSchedule.cpp
)

if (BUILD_FOR_STM32CUBE)
    target_compile_definitions(TaskManagerIO
            PUBLIC BUILD_FOR_STM32CUBE_CMAKE=1 IO_LOGGING_DEBUG=1
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
