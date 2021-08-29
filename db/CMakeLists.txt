cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(db)

include_directories("..\\sqlite")
include_directories("..\\encoding_string")

add_library(db "db.h" "db.cpp")

target_link_libraries(${PROJECT_NAME} sqlite)