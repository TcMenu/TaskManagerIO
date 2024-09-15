add_library(TaskManagerIO
        ../src/SimpleSpinLock.cpp
        ../src/TaskManagerIO.cpp
        ../src/TaskTypes.cpp
        ../src/TmLongSchedule.cpp
)

target_compile_definitions(TaskManagerIO
        PUBLIC BUILD_FOR_PICO_CMAKE=1 BUILD_PICO_FORCE_UART=1 IO_LOGGING_DEBUG=1
)

target_include_directories(TaskManagerIO PUBLIC
        ${PROJECT_SOURCE_DIR}/lib/TaskManagerIO/src
)

target_link_libraries(TaskManagerIO PUBLIC TcMenuLog pico_stdlib pico_sync)
