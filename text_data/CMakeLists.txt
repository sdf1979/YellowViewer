cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(text_data)

include_directories("..\\encoding_string")
include_directories("..\\db")
include_directories("..\\reader")
include_directories("..\\timer_perf")

add_library(text_data text_data.h text_data.cpp)

target_link_libraries(${PROJECT_NAME} timer_perf)