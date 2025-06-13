# Sprint 8: App Tests - Complete CMakeLists.txt
# This file defines test executables for the app library

# MainPresenter Tests
add_executable(MainPresenterTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_mainpresenter.cpp
)
target_link_libraries(MainPresenterTests PRIVATE
    App                 # Link against the App library
    Core                # Indirect dependency through App
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
target_include_directories(MainPresenterTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/app/include     # App headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include    # Core headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include      # UI headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/parsers/include # Parsers headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/rendering/include # Rendering headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../mocks                  # Mock headers
)
add_test(NAME MainPresenterTests COMMAND MainPresenterTests)

# MainPresenter Target Detection Tests (Sprint 5.1)
add_executable(MainPresenterTargetDetectionTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_mainpresenter_targetdetection.cpp
)
target_link_libraries(MainPresenterTargetDetectionTests PRIVATE
    App                 # Link against the App library
    Core                # Indirect dependency through App
    UI                  # For UI components
    Registration        # For target detection functionality
    Parsers             # For parser interfaces
    Rendering           # For rendering components
    Interfaces          # For interface definitions
    GTest::gtest_main   # Google Test framework
    GTest::gmock        # Google Mock framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
    Qt6::Widgets        # For QWidget components
)
target_include_directories(MainPresenterTargetDetectionTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/app/include         # App headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include        # Core headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include          # UI headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include # Registration headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/parsers/include     # Parsers headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/rendering/include   # Rendering headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/interfaces/include  # Interfaces headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../mocks                      # Mock headers
)
add_test(NAME MainPresenterTargetDetectionTests COMMAND MainPresenterTargetDetectionTests)

message(STATUS "Configuring app tests...")
