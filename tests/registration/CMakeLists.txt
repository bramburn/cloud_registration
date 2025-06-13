# Sprint 6: Registration Tests - Complete Implementation
# This file defines test executables for the Registration module's unit tests

add_executable(TargetTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_target.cpp
)
target_link_libraries(TargetTests PRIVATE
    Registration # Link against the Registration library
    Core # Indirect dependency for PointFullData, etc.
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui # For QVector3D, QVariantMap, QList, QVariantList
)
target_include_directories(TargetTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include/registration
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core
)
add_test(NAME TargetTests COMMAND TargetTests)

add_executable(TargetManagerTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_targetmanager.cpp
)
target_link_libraries(TargetManagerTests PRIVATE
    Registration
    Core
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui # For QTemporaryFile
)
target_include_directories(TargetManagerTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include/registration
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core
)
add_test(NAME TargetManagerTests COMMAND TargetManagerTests)

add_executable(NaturalPointSelectorTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_naturalpointselector.cpp
)
target_link_libraries(NaturalPointSelectorTests PRIVATE
    Registration
    Core
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
)
target_include_directories(NaturalPointSelectorTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include/registration
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core
)
add_test(NAME NaturalPointSelectorTests COMMAND NaturalPointSelectorTests)

add_executable(SphereDetectorTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_spheredetector.cpp
)
target_link_libraries(SphereDetectorTests PRIVATE
    Registration
    Core
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
)
target_include_directories(SphereDetectorTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include/registration
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core
)
add_test(NAME SphereDetectorTests COMMAND SphereDetectorTests)

add_executable(RegistrationWorkflowTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_registration_workflow.cpp
)
target_link_libraries(RegistrationWorkflowTests PRIVATE
    Registration
    Core
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets # For QStackedWidget, QPushButton etc.
)
target_include_directories(RegistrationWorkflowTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include/registration
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include/ui # For WorkflowProgressWidget
)
add_test(NAME RegistrationWorkflowTests COMMAND RegistrationWorkflowTests)

add_executable(PoseGraphTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_sprint9_registration.cpp
)
target_link_libraries(PoseGraphTests PRIVATE
    Registration
    Core
    Algorithms # For Optimization::BundleAdjustment, Features::FeatureExtractor etc.
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
    Eigen3::Eigen
)
target_include_directories(PoseGraphTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include/registration
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/algorithms/include/algorithms
)
add_test(NAME PoseGraphTests COMMAND PoseGraphTests)

add_executable(AlignmentEngineTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_alignment_engine.cpp
)
target_link_libraries(AlignmentEngineTests PRIVATE
    Registration
    Core
    Algorithms # For LeastSquaresAlignment
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
)
target_include_directories(AlignmentEngineTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include/registration
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/algorithms/include/algorithms
)
add_test(NAME AlignmentEngineTests COMMAND AlignmentEngineTests)

add_executable(ErrorAnalysisTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_error_analysis.cpp
)
target_link_libraries(ErrorAnalysisTests PRIVATE
    Registration
    Core
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
)
target_include_directories(ErrorAnalysisTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include/registration
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core
)
add_test(NAME ErrorAnalysisTests COMMAND ErrorAnalysisTests)

message(STATUS "Configuring Registration tests...")
