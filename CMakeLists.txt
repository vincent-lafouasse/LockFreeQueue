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

# size presets
# BIG overrides SMALL if both a present
option(CLFQ_BIG "Use 16k sample buffer" OFF)
option(CLFQ_SMALL "Use 512 sample buffer" OFF)

if(CLFQ_BIG)
    add_compile_definitions(CLFQ_BIG)
elseif(CLFQ_SMALL)
    add_compile_definitions(CLFQ_SMALL)
endif()

# e.g. pass -DCLF_QUEUE_SIZE=8192 from the command line
# this overrides the BIG/SMALL options
set(CLF_QUEUE_SIZE "" CACHE STRING "Manual override for queue size (must be power of 2)")
if(CLF_QUEUE_SIZE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC CLF_QUEUE_SIZE=${CLF_QUEUE_SIZE})
endif()

# if no size was chosen at all, the default size of 4096 samples, ie 16KB, is
# chosen
