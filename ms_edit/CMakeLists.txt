cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(ms_edit)

include_directories("..\\wnd")

add_library(ms_edit ms_edit.h ms_edit.cpp)