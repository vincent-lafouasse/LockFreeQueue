cmake_minimum_required(VERSION 3.16)

project(LockFreeQueue VERSION 1.0.0)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ./LockFreeQueue.h ./LockFreeQueue.c)

target_include_directories(${PROJECT_NAME} PUBLIC .)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic
                                               -fdiagnostics-color=always)

# this is a sentinel value, hence why we do not use FORCE here
set(CLF_QUEUE_SIZE 0 CACHE STRING "Queue size override (must be power of 2)")
if(CLF_QUEUE_SIZE EQUAL 0)
    message(FATAL_ERROR "CLF_QUEUE_SIZE must be set")
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC CLF_QUEUE_SIZE=${CLF_QUEUE_SIZE})
