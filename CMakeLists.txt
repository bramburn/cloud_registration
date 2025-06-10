# Sprint 1: Minimal CMake Foundation
cmake_minimum_required(VERSION 3.16)

project(CloudRegistration VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure MSVC
if(MSVC)
    # Warning level 4
    add_compile_options(/W4)
    # UTF-8 encoding
    add_compile_options(/utf-8)
endif()

# Configure GCC/Clang compilers (Linux/macOS)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # UTF-8 encoding
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
    # High warning level
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Create minimal executable for Sprint 1
add_executable(CloudRegistration src/main.cpp)
