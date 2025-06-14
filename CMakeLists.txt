cmake_minimum_required(VERSION 3.20)

# Project definition
project(PointCloudApplication VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set Qt6 path if not using vcpkg
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR NOT CMAKE_TOOLCHAIN_FILE MATCHES "vcpkg")
    set(CMAKE_PREFIX_PATH "C:/Qt/6.9.0/msvc2022_64" ${CMAKE_PREFIX_PATH})
endif()

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg)

# Try to find GTest, if not found, we'll skip tests for now
find_package(GTest QUIET)

# Enable Qt's MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Add subdirectories
add_subdirectory(src)

# Only add tests if GTest is found
if(GTest_FOUND)
    add_subdirectory(tests)
    enable_testing()
else()
    message(WARNING "GTest not found. Tests will be skipped.")
endif()
