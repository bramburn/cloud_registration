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

# Target Detection Dialog Tests (Sprint 5.1)
add_executable(TargetDetectionDialogTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_targetdetectiondialog.cpp
)
target_link_libraries(TargetDetectionDialogTests PRIVATE
    UI                  # Link against the UI library
    Core                # Indirect dependency through UI
    Registration        # For target detection functionality
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
    Qt6::Widgets        # For QWidget components
)
target_include_directories(TargetDetectionDialogTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include/ui           # UI headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core       # Core headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include/registration # Registration headers
)
add_test(NAME TargetDetectionDialogTests COMMAND TargetDetectionDialogTests)

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

# Sprint 7.2: Bundle Adjustment Progress Dialog Tests
add_executable(BundleAdjustmentProgressDialogTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_bundleadjustmentprogressdialog.cpp
)
target_link_libraries(BundleAdjustmentProgressDialogTests PRIVATE
    UI                  # Link against the UI library
    Optimization        # For BundleAdjustment classes
    Registration        # For PoseGraph classes
    Core                # Indirect dependency
    GTest::gtest_main   # Google Test framework
    GMock::gmock        # Google Mock framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
    Qt6::Widgets        # For QWidget components
)
target_include_directories(BundleAdjustmentProgressDialogTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include/ui           # UI headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/optimization/include/optimization # Optimization headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/registration/include/registration # Registration headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core       # Core headers
)
add_test(NAME BundleAdjustmentProgressDialogTests COMMAND BundleAdjustmentProgressDialogTests)

# Sprint 6.3: Report Options Dialog Tests
add_executable(ReportOptionsDialogTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_reportoptionsdialog.cpp
)
target_link_libraries(ReportOptionsDialogTests PRIVATE
    UI                  # Link against the UI library
    Quality             # For PDFReportGenerator classes
    Core                # Indirect dependency
    GTest::gtest_main   # Google Test framework
    GMock::gmock        # Google Mock framework
    Qt6::Test           # Qt Test module
    Qt6::Core           # For QObject, QString
    Qt6::Gui            # For QVector3D, QMatrix4x4
    Qt6::Widgets        # For QWidget components
)
target_include_directories(ReportOptionsDialogTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ui/include/ui           # UI headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/quality/include/quality # Quality headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core       # Core headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/interfaces/include/interfaces # Interface headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../mocks                         # Mock headers
)
add_test(NAME ReportOptionsDialogTests COMMAND ReportOptionsDialogTests)

message(STATUS "Configuring UI tests...")
