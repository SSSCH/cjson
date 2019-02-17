cmake_minimum_required(VERSION 2.6)
project(json_tutorial06)
#[[if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ansi -pedantic -Wall")
endif()]]
add_library(leptjson leptjson.c)
add_executable(json_tutorial06 test.c leptjson.c)
target_link_libraries(json_tutorial06 leptjson)