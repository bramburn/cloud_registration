# Manual test for Sprint 2.1
cmake_minimum_required(VERSION 3.16)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Xml)

# Create the manual test executable
add_executable(Sprint21ManualTest
    test_sprint2_1_manual.cpp
    src/e57parser.cpp
    src/e57parser.h
)

# Link Qt6 libraries
target_link_libraries(Sprint21ManualTest
    Qt6::Core
    Qt6::Xml
)

# Set C++ standard
set_target_properties(Sprint21ManualTest PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Include directories
target_include_directories(Sprint21ManualTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
