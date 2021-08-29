cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(directory_watcher)

include_directories("..\\reader")
include_directories("..\\parser")
include_directories("..\\text_data")
include_directories("..\\db")
include_directories("..\\encoding_string")
include_directories("..\\timer_perf")

add_library(directory_watcher "directory_watcher.h" "directory_watcher.cpp")

target_link_libraries(${PROJECT_NAME} reader)
target_link_libraries(${PROJECT_NAME} parser)
target_link_libraries(${PROJECT_NAME} text_data)
target_link_libraries(${PROJECT_NAME} db)
target_link_libraries(${PROJECT_NAME} timer_perf)