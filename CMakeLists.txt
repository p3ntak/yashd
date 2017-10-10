cmake_minimum_required(VERSION 3.8)
project(yashd)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES yash.c yashd.c yash_program.c helpers.h threads.h yashd.c daemon.h semaphore.h)
add_executable(yashd ${SOURCE_FILES})
add_executable(yash ${SOURCE_FILES})