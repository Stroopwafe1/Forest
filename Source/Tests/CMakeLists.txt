cmake_minimum_required(VERSION 3.16)
project(ForestTesting)

include_directories(../Parser ../Tokeniser)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG bc860af08783b8113005ca7697da5f5d49a8056f
)

FetchContent_MakeAvailable(googletest)

add_executable(ForestTesting
        Testing_testing.cpp
        TokeniserTests.cpp ParserTests.cpp)

target_link_libraries(
        ForestTesting
        GTest::gtest_main
        ForestTokeniser
        ForestParser
)

include(GoogleTest)
gtest_discover_tests(ForestTesting)