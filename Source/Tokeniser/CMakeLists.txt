cmake_minimum_required(VERSION 3.16)
project(ForestTokeniser VERSION 1.0.0 DESCRIPTION "Tokenisation library for Forest")

add_library(ForestTokeniser STATIC
        Tokeniser.hpp
        Tokeniser.cpp
)

target_include_directories(ForestTokeniser PUBLIC .)
set_target_properties(ForestTokeniser PROPERTIES VERSION ${PROJECT_VERSION})