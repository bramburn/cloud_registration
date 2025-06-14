# Tests CMakeLists.txt

# Define test sources
set(TEST_SOURCES
    main_test.cpp
    sample_test.cpp
)

# Create test executable
add_executable(PointCloudTests ${TEST_SOURCES})

# Link libraries
target_link_libraries(PointCloudTests 
    GTest::gtest 
    GTest::gtest_main
    Qt6::Core
)

# Add test to CTest
add_test(NAME PointCloudTests COMMAND PointCloudTests)
