cmake_minimum_required(VERSION 3.16)

# Sprint 4 Tests Configuration
project(Sprint4Tests)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Test Gui)
find_package(Eigen3 REQUIRED NO_MODULE)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../src)

# Test for LeastSquaresAlignment
add_executable(test_least_squares_alignment
    test_least_squares_alignment.cpp
    ../../src/algorithms/LeastSquaresAlignment.cpp
)

target_link_libraries(test_least_squares_alignment
    Qt6::Core
    Qt6::Test
    Qt6::Gui
    Eigen3::Eigen
)

# Test for ErrorAnalysis
add_executable(test_error_analysis
    test_error_analysis.cpp
    ../../src/registration/ErrorAnalysis.cpp
)

target_link_libraries(test_error_analysis
    Qt6::Core
    Qt6::Test
    Qt6::Gui
)

# Test for AlignmentEngine
add_executable(test_alignment_engine
    test_alignment_engine.cpp
    ../../src/registration/AlignmentEngine.cpp
    ../../src/algorithms/LeastSquaresAlignment.cpp
    ../../src/registration/ErrorAnalysis.cpp
)

target_link_libraries(test_alignment_engine
    Qt6::Core
    Qt6::Test
    Qt6::Gui
    Eigen3::Eigen
)

# Enable Qt's MOC for all tests
set_target_properties(test_least_squares_alignment PROPERTIES AUTOMOC ON)
set_target_properties(test_error_analysis PROPERTIES AUTOMOC ON)
set_target_properties(test_alignment_engine PROPERTIES AUTOMOC ON)

# Add tests to CTest
enable_testing()

add_test(NAME LeastSquaresAlignmentTest COMMAND test_least_squares_alignment)
add_test(NAME ErrorAnalysisTest COMMAND test_error_analysis)
add_test(NAME AlignmentEngineTest COMMAND test_alignment_engine)

# Set test properties
set_tests_properties(LeastSquaresAlignmentTest PROPERTIES TIMEOUT 30)
set_tests_properties(ErrorAnalysisTest PROPERTIES TIMEOUT 30)
set_tests_properties(AlignmentEngineTest PROPERTIES TIMEOUT 60)

message(STATUS "Sprint 4 tests configured successfully")
