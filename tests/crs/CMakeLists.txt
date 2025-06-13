# Sprint 8: CRS Tests - Complete CMakeLists.txt
# This file defines test executables for the crs library

# Coordinate System Manager Tests
add_executable(CoordinateSystemManagerTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_coordinate_system_manager.cpp
)
target_link_libraries(CoordinateSystemManagerTests PRIVATE
    CRS                 # Link against the CRS library
    Core                # Indirect dependency through CRS
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
)
target_include_directories(CoordinateSystemManagerTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/crs/include # CRS headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include # Core headers
)
add_test(NAME CoordinateSystemManagerTests COMMAND CoordinateSystemManagerTests)

message(STATUS "Configuring crs tests...")
