cmake_minimum_required(VERSION 3.10.0)
set(CMAKE_CXX_STANDARD 17)

project(sqlite)

add_library(sqlite sqlite3.h sqlite3.c)