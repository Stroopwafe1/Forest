cmake_minimum_required(VERSION 3.16)
project(Forest)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(Source/Tokeniser)
add_subdirectory(Source/Parser)
add_subdirectory(Source/Tests)

enable_testing()

add_executable(Forest
        Source/main.cpp
        Source/X86_64LinuxYasmCompiler.cpp
        Source/X86_64LinuxYasmCompiler.hpp)

target_link_libraries(Forest PUBLIC ForestTokeniser ForestParser)
target_include_directories(Forest PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/Source/Tokeniser"
        "${PROJECT_SOURCE_DIR}/Source/Parser"
)
