cmake_minimum_required(VERSION 4.1)
project(matching_engine)

set(CMAKE_CXX_STANDARD 26)

add_executable(matching_engine main.cpp
        Types.h
        LevelInfo.h
        Order.h
        OrderModify.h
        Trade.h
        OrderBook.cpp
        OrderBook.h
        Constants.h)
