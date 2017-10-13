cmake_minimum_required(VERSION 3.8)
project(yashd)

set(CMAKE_C_STANDARD 99)

find_package( Threads )

set(SOURCE_FILES_YASHD yashd.c yash_program.c helpers.h threads.h daemon.h semaphore.h)
set(SOURCE_FILES_YASH yash.c )
add_executable(yashd ${SOURCE_FILES_YASHD})
add_executable(yash ${SOURCE_FILES_YASH})
target_link_libraries( yashd ${CMAKE_THREAD_LIBS_INIT} )