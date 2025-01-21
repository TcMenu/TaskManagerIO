
file(GLOB SOURCES "${CMAKE_CURRENT_LIST_DIR}/../src/*.cpp")

add_library(TaskManagerIO ${SOURCES})

target_compile_definitions(TaskManagerIO
        PUBLIC BUILD_FOR_PICO_CMAKE=1 BUILD_PICO_FORCE_UART=1 IO_LOGGING_DEBUG=1
)

target_include_directories(TaskManagerIO PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../src
)

target_link_libraries(TaskManagerIO PUBLIC TcMenuLog pico_stdlib pico_sync)
