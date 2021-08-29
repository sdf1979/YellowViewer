cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(app)

include_directories("..\\wnd")
include_directories("..\\edit")
include_directories("..\\ms_edit")
include_directories("..\\status_bar")
include_directories("..\\text_data")
include_directories("..\\directory_watcher")
include_directories("..\\reader")
include_directories("..\\parser")
include_directories("..\\encoding_string")
include_directories("..\\db")
include_directories("..\\timer_perf")

add_library(app app.h app.cpp)