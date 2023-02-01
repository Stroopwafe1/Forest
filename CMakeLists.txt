cmake_minimum_required(VERSION 3.16)
project(Forest)

set(CMAKE_CXX_STANDARD 17)

add_executable(Forest
        Source/main.cpp 
        Source/Tokeniser.cpp 
        Source/Tokeniser.h)
