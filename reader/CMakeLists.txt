cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(reader)

include_directories("..\\timer_perf")
include_directories("..\\encoding_string")

add_library(reader reader.h reader.cpp)