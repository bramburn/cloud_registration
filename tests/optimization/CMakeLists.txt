# Sprint 7.2: optimization Tests - Bundle Adjustment Tests
# This file defines test executables for the optimization library

# Bundle Adjustment tests
add_executable(test_bundleadjustment test_bundleadjustment.cpp)
target_link_libraries(test_bundleadjustment
    PRIVATE
    Optimization
    Registration
    Qt6::Core
    Qt6::Test
    gtest
    gtest_main
    gmock
)

# Add test to CTest
add_test(NAME BundleAdjustmentTest COMMAND test_bundleadjustment)

message(STATUS "Configuring optimization tests...")
