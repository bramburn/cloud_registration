# Sprint 3: Algorithms Tests - CMakeLists.txt
# This file defines test executables for the Algorithms library

# ICPRegistration Tests
add_executable(ICPRegistrationTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_icp_registration.cpp
)
target_link_libraries(ICPRegistrationTests PRIVATE
    Algorithms          # Link against the Algorithms library
    Core                # Indirect dependency through Algorithms
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QSignalSpy
    Qt6::Gui            # For QVector3D, QMatrix4x4
)
target_include_directories(ICPRegistrationTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/algorithms  # Algorithms headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core        # Core headers
)
add_test(NAME ICPRegistrationTests COMMAND ICPRegistrationTests)

# LeastSquaresAlignment Tests
add_executable(LeastSquaresAlignmentTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_least_squares_alignment.cpp
)
target_link_libraries(LeastSquaresAlignmentTests PRIVATE
    Algorithms          # Link against the Algorithms library
    Core                # Indirect dependency through Algorithms
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core
    Qt6::Gui
    Eigen3::Eigen       # Directly used by LeastSquaresAlignment
)
target_include_directories(LeastSquaresAlignmentTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/algorithms
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core
)
add_test(NAME LeastSquaresAlignmentTests COMMAND LeastSquaresAlignmentTests)

# PointToPlaneICP Tests
add_executable(PointToPlaneICPTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_point_to_plane_icp.cpp
)
target_link_libraries(PointToPlaneICPTests PRIVATE
    Algorithms          # Link against the Algorithms library
    Core                # Indirect dependency through Algorithms
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core
    Qt6::Gui
)
target_include_directories(PointToPlaneICPTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/algorithms
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core
)
add_test(NAME PointToPlaneICPTests COMMAND PointToPlaneICPTests)

message(STATUS "Configuring Algorithms tests...")
