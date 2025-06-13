# Sprint 1: Test Suite - Placeholder CMakeLists.txt
# This file includes subdirectories for each module's tests

# Add subdirectories for each module's test suite
add_subdirectory(algorithms)
add_subdirectory(analysis)
add_subdirectory(app)
add_subdirectory(camera)
add_subdirectory(core)
add_subdirectory(crs)
# detection tests moved to registration in Sprint 6
add_subdirectory(export)
add_subdirectory(features)
add_subdirectory(implementations)
add_subdirectory(interfaces)
add_subdirectory(optimization)
add_subdirectory(parsers)
add_subdirectory(performance)
add_subdirectory(quality)
add_subdirectory(registration)
add_subdirectory(rendering)
add_subdirectory(ui)
add_subdirectory(integration)  # Integration tests
add_subdirectory(mocks)  # If mocks have tests or common CMake definitions

# Define a custom target to run all tests
# This list must accurately reflect the executable names defined in each
# module's tests/<module_name>/CMakeLists.txt
set(ALL_TESTS
    # Core Tests
    VoxelGridFilterTests
    PerformanceProfilerTests
    ProjectManagerTests

    # Algorithms Tests
    LeastSquaresAlignmentTests
    ICPRegistrationTests
    PointToPlaneICPTests

    # Parsers Tests
    E57LinkageTest
    LasParserTests
    E57ParserTests
    E57ParserCoreTests
    E57ParserLibTests
    E57ComprehensiveTests

    # Rendering Tests
    CameraControllerTests
    OpenGLInitTest
    GpuCullerTests
    OpenGLRendererTests
    PointCloudViewerDecouplingTests
    PointCloudViewerRenderingR4Tests

    # UI Tests
    RecentProjectsManagerTests
    UIEnhancementTests
    ExportDialogTests
    TargetDetectionDialogTests

    # Application Tests
    MainPresenterTests
    MainPresenterTargetDetectionTests

    # Integration Tests
    IntegrationTests
    EndToEndIntegrationTests
    TargetDetectionIntegrationTests
    ManualAlignmentE2ETests

    # Performance Tests
    PerformanceOptimizationTests
    PerformanceValidationTests

    # Registration Tests
    TargetTests
    TargetManagerTests
    NaturalPointSelectorTests
    SphereDetectorTests
    RegistrationWorkflowTests
    RegistrationWorkflowWidgetTests
    AlignmentEngineTests
    ErrorAnalysisTests

    # Quality Tests
    QualityReportingTests
    QualityFunctionalityTests

    # Export Tests
    ExportFunctionalityTests

    # CRS Tests
    CoordinateSystemManagerTests

    # Analysis Tests (placeholders)
    # DifferenceAnalysisTests

    # Features Tests (placeholders)
    # FeatureBasedRegistrationTests

    # Optimization Tests (placeholders)
    # BundleAdjustmentTests
)

add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG>
    DEPENDS ${ALL_TESTS}
    COMMENT "Running all unit and integration tests across all modular libraries"
    VERBATIM
)

# Define category-specific test targets (example, adapt as needed)
# These targets can be used to run a subset of tests.
add_custom_target(test_core_only
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C $<CONFIG> -R "(VoxelGridFilterTests|PerformanceProfilerTests)"
    COMMENT "Running Core library tests"
    VERBATIM
)

# Other category-specific targets can be added here
# Example: add_custom_target(test_parsers_only ...)

message(STATUS "Configuring test suite...")
