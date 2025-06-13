# Sprint 8: Performance Tests - Complete CMakeLists.txt
# This file defines test executables for the performance library

# Performance Optimization Tests
add_executable(PerformanceOptimizationTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_performance_optimization.cpp
)
target_link_libraries(PerformanceOptimizationTests PRIVATE
    Performance         # Link against the Performance library
    Core                # Indirect dependency through Performance
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
)
target_include_directories(PerformanceOptimizationTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/performance/include # Performance headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include        # Core headers
)
add_test(NAME PerformanceOptimizationTests COMMAND PerformanceOptimizationTests)

# Performance Validation Tests
add_executable(PerformanceValidationTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_performance_validation.cpp
)
target_link_libraries(PerformanceValidationTests PRIVATE
    Performance         # Link against the Performance library
    Core                # Indirect dependency through Performance
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
)
target_include_directories(PerformanceValidationTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/performance/include # Performance headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include        # Core headers
)
add_test(NAME PerformanceValidationTests COMMAND PerformanceValidationTests)

message(STATUS "Configuring performance tests...")
