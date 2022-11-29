cmake_minimum_required(VERSION 3.23)
project(Own_Programming_Language)

set(CMAKE_CXX_STANDARD 14)

add_executable(Own_Programming_Language
        Source/main.cpp Source/Tokeniser.cpp Source/Tokeniser.h)
