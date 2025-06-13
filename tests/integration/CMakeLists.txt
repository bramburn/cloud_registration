# Sprint 8: Integration Tests - Complete CMakeLists.txt
# This file defines test executables for integration testing

# Integration Tests
add_executable(IntegrationTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_integration.cpp
)
target_link_libraries(IntegrationTests PRIVATE
    App                 # Link against the App library
    Core                # Core functionality
    UI                  # For UI components
    Parsers             # For parser interfaces
    Rendering           # For rendering components
    GTest::gtest_main   # Google Test framework
    GTest::gmock        # Google Mock framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
    Qt6::Widgets        # For QWidget components
)
target_include_directories(IntegrationTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/app/include     # App headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include    # Core headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include      # UI headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/parsers/include # Parsers headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/rendering/include # Rendering headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../mocks                  # Mock headers
)
add_test(NAME IntegrationTests COMMAND IntegrationTests)

# End-to-End Tests
add_executable(EndToEndIntegrationTests
    ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end_testing.cpp
)
target_link_libraries(EndToEndIntegrationTests PRIVATE
    App                 # Link against the App library
    Core                # Core functionality
    UI                  # For UI components
    Parsers             # For parser interfaces
    Rendering           # For rendering components
    Algorithms          # For algorithm components
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
    Qt6::Widgets        # For QWidget components
)
target_include_directories(EndToEndIntegrationTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/app/include     # App headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include    # Core headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include      # UI headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/parsers/include # Parsers headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/rendering/include # Rendering headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/algorithms/include # Algorithms headers
)
add_test(NAME EndToEndIntegrationTests COMMAND EndToEndIntegrationTests)

# Target Detection Integration Tests (Sprint 5.1)
add_executable(TargetDetectionIntegrationTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_target_detection_integration.cpp
)
target_link_libraries(TargetDetectionIntegrationTests PRIVATE
    App                 # Link against the App library
    Core                # Core functionality
    UI                  # For UI components
    Registration        # For registration and target detection
    Parsers             # For parser interfaces
    Rendering           # For rendering components
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
    Qt6::Widgets        # For QWidget components
)
target_include_directories(TargetDetectionIntegrationTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/app/include         # App headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include        # Core headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include          # UI headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include # Registration headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/parsers/include     # Parsers headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/rendering/include   # Rendering headers
)
add_test(NAME TargetDetectionIntegrationTests COMMAND TargetDetectionIntegrationTests)

message(STATUS "Configuring integration tests...")
