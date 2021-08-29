cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(parser)

include_directories("..\\encoding_string")
include_directories("..\\timer_perf")

add_library(parser parser.h parser.cpp)