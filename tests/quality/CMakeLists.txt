# Sprint 6: Quality Tests - Complete Implementation
# This file defines test executables for the Quality module's unit tests

add_executable(QualityReportingTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_quality_reporting.cpp
)
target_link_libraries(QualityReportingTests PRIVATE
    Quality # Link against the Quality library
    Core # Indirect dependency through Quality
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui # For QVector3D, QMatrix4x4
    Qt6::PrintSupport # For PDF generation tests
    Qt6::Widgets # For QTemporaryDir in tests
)
target_include_directories(QualityReportingTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/quality/include/quality
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core
)
add_test(NAME QualityReportingTests COMMAND QualityReportingTests)

# Quality Functionality Tests
add_executable(QualityFunctionalityTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_quality_functionality.cpp
)
target_link_libraries(QualityFunctionalityTests PRIVATE
    Quality             # Link against the Quality library
    Core                # Indirect dependency through Quality
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
)
target_include_directories(QualityFunctionalityTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/quality/include # Quality headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include    # Core headers
)
add_test(NAME QualityFunctionalityTests COMMAND QualityFunctionalityTests)

message(STATUS "Configuring Quality tests...")
