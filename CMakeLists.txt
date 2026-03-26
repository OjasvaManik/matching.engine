cmake_minimum_required(VERSION 4.1)
project(matching_engine)

set(CMAKE_CXX_STANDARD 26)

add_executable(matching_engine main.cpp
        types.h)
