# Sprint 6: UI Tests - Complete Implementation
# This file defines test executables for the UI module's unit tests

add_executable(RecentProjectsManagerTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_recentprojectsmanager.cpp
)
target_link_libraries(RecentProjectsManagerTests PRIVATE
    UI # Link against the UI library
    Core # Indirect dependency through UI
    GTest::gtest_main # Google Test framework
    Qt6::Test # Qt Test module
    Qt6::Core
    Qt6::Gui # For QDir, QFileInfo etc. in tests
    Qt6::Widgets # For QTemporaryDir in tests
)
target_include_directories(RecentProjectsManagerTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core
)
add_test(NAME RecentProjectsManagerTests COMMAND RecentProjectsManagerTests)

add_executable(UIEnhancementTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_ui_enhancement.cpp
)
target_link_libraries(UIEnhancementTests PRIVATE
    UI
    Core
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
target_include_directories(UIEnhancementTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core
)
add_test(NAME UIEnhancementTests COMMAND UIEnhancementTests)

message(STATUS "Configuring UI tests...")
