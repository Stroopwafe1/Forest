cmake_minimum_required(VERSION 3.16)
project(Forest)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(Tokeniser)
add_subdirectory(Parser)

add_executable(Forest
        main.cpp
        X86_64LinuxYasmCompiler.cpp X86_64LinuxYasmCompiler.hpp)

target_link_libraries(Forest PUBLIC ForestTokeniser ForestParser)
target_include_directories(Forest PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/Tokeniser"
        "${PROJECT_SOURCE_DIR}/Parser"
)