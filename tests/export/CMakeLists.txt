# Sprint 8: Export Tests - Complete CMakeLists.txt
# This file defines test executables for the export library

# Export Functionality Tests
add_executable(ExportFunctionalityTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_export_functionality.cpp
)
target_link_libraries(ExportFunctionalityTests PRIVATE
    Export              # Link against the Export library
    Core                # Indirect dependency through Export
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
)
target_include_directories(ExportFunctionalityTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/export/include # Export headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include   # Core headers
)
add_test(NAME ExportFunctionalityTests COMMAND ExportFunctionalityTests)

message(STATUS "Configuring export tests...")
