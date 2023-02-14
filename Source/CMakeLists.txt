cmake_minimum_required(VERSION 3.16)
project(Forest)

set(CMAKE_CXX_STANDARD 17)

add_executable(Forest
        main.cpp
        Tokeniser.cpp
        Tokeniser.h
        Parser.cpp
        Parser.hpp
)
