cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(timer_perf)

include_directories("..\\encoding_string")

add_library(timer_perf timer_perf.h timer_perf.cpp)