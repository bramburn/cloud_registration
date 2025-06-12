# Sprint 2: Core Tests - Updated CMakeLists.txt
# This file defines test executables for the Core library

# VoxelGridFilter Tests
add_executable(VoxelGridFilterTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_voxelgridfilter.cpp
)
target_link_libraries(VoxelGridFilterTests PRIVATE
    Core                # Link against the Core library
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString (might be pulled by Core anyway, but explicit is fine)
    Qt6::Gui            # For Vector3D/QVector3D usage in Loadingsettings and LasHeaderMetadata
)
target_include_directories(VoxelGridFilterTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include # Core headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src # For temporary includes from root src/ (if any)
)
add_test(NAME VoxelGridFilterTests COMMAND VoxelGridFilterTests)

# PerformanceProfiler Tests
add_executable(PerformanceProfilerTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_performance_profiler.cpp
)
target_link_libraries(PerformanceProfilerTests PRIVATE
    Core                # Link against the Core library
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QElapsedTimer which might use Gui implicitly
)
target_include_directories(PerformanceProfilerTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)
add_test(NAME PerformanceProfilerTests COMMAND PerformanceProfilerTests)

message(STATUS "Configuring Core tests...")
