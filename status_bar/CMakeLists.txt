cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(status_bar)

include_directories("..\\wnd")

add_library(status_bar status_bar.h status_bar.cpp)