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

# Export Dialog Tests
add_executable(ExportDialogTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_export_dialog.cpp
)
target_link_libraries(ExportDialogTests PRIVATE
    UI                  # Link against the UI library
    Core                # Indirect dependency through UI
    Export              # For export functionality
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
    Qt6::Widgets        # For QWidget components
)
target_include_directories(ExportDialogTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include/ui       # UI headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core   # Core headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/export/include/export # Export headers
)
add_test(NAME ExportDialogTests COMMAND ExportDialogTests)

# ICP Parameter Dialog Tests
add_executable(ICPParameterDialogTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_icpparameterdialog.cpp
)
target_link_libraries(ICPParameterDialogTests PRIVATE
    UI                  # Link against the UI library
    Core                # Indirect dependency through UI
    Algorithms          # For ICPRegistration and PointCloud
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
    Qt6::Widgets        # For QWidget components
)
target_include_directories(ICPParameterDialogTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include/ui           # UI headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core       # Core headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/algorithms/include     # Algorithm headers
)
add_test(NAME ICPParameterDialogTests COMMAND ICPParameterDialogTests)

# AlignmentControlPanel Tests (MVP3 S3.1)
add_executable(AlignmentControlPanelTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_alignmentcontrolpanel.cpp
)
target_link_libraries(AlignmentControlPanelTests PRIVATE
    UI                  # Link against the UI library
    Registration        # For AlignmentEngine
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
    Qt6::Widgets        # For QWidget components
)
target_include_directories(AlignmentControlPanelTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include/ui           # UI headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include/registration # Registration headers
)
add_test(NAME AlignmentControlPanelTests COMMAND AlignmentControlPanelTests)

# Sprint 7.1: Pose Graph Viewer Widget Tests
add_executable(PoseGraphViewerWidgetTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_posegraphviewerwidget.cpp
)
target_link_libraries(PoseGraphViewerWidgetTests PRIVATE
    UI                  # Link against the UI library
    Registration        # For PoseGraph classes
    Core                # Indirect dependency
    GTest::gtest_main   # Google Test framework
    GMock::gmock        # Google Mock framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
    Qt6::Widgets        # For QWidget components
)
target_include_directories(PoseGraphViewerWidgetTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include/ui           # UI headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include/registration # Registration headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core       # Core headers
)
add_test(NAME PoseGraphViewerWidgetTests COMMAND PoseGraphViewerWidgetTests)

message(STATUS "Configuring UI tests...")
