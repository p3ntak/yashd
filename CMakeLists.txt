cmake_minimum_required(VERSION 3.8)
project(yashd)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES yashdclient.c yashdhost.c yash.c helpers.h threads.h yashdhost.c daemon.h)
add_executable(yashd ${SOURCE_FILES})