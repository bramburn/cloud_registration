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

message(STATUS "Configuring Quality tests...")
