cmake_minimum_required(VERSION 3.16)
project(ForestParser VERSION 1.0.0 DESCRIPTION "Parser library for Forest")

include_directories(../Tokeniser)
add_library(ForestParser STATIC
        Parser.hpp
        Parser.cpp
        ConfigParser.hpp
        ConfigParser.cpp
        Expression.cpp
        Expression.hpp
)

target_include_directories(ForestParser PUBLIC .)
set_target_properties(ForestParser PROPERTIES VERSION ${PROJECT_VERSION})
