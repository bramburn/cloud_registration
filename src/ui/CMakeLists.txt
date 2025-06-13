# Sprint 6: UI Library - Complete Implementation
# This file defines the UI static library containing user interface components

# Define source files for the UI library (now includes migrated files from src/ root)
set(UI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlignmentControlPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExportDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICPParameterDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICPProgressWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TargetDetectionDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UIThemeManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UserPreferences.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ViewerToolbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/WorkflowProgressWidget.cpp

    # Files migrated from src/ root (as per repomix-output2.md analysis)
    ${CMAKE_CURRENT_SOURCE_DIR}/src/createprojectdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/loadingsettingsdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/projecthubwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/recentprojectsmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scanimportdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sidebarwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/projecttreemodel.cpp

    # Sprint 7.1: Pose Graph Visualization
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PoseGraphViewerWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RegistrationTabWidget.cpp

    # Sprint 7.2: Bundle Adjustment Progress Dialog
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BundleAdjustmentProgressDialog.cpp
)

# Define header files for the UI library (now includes migrated headers from src/ root)
set(UI_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/AlignmentControlPanel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/ExportDialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/ICPParameterDialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/ICPProgressWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/TargetDetectionDialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/UIThemeManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/UserPreferences.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/ViewerToolbar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/WorkflowProgressWidget.h

    # Headers migrated from src/ root (as per repomix-output2.md analysis)
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/createprojectdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/loadingsettingsdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/projecthubwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/recentprojectsmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/scanimportdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/sidebarwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/projecttreemodel.h

    # Sprint 7.1: Pose Graph Visualization
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/PoseGraphViewerWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/RegistrationTabWidget.h

    # Sprint 7.2: Bundle Adjustment Progress Dialog
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/BundleAdjustmentProgressDialog.h
)

# Create UI static library
add_library(UI STATIC ${UI_SOURCES} ${UI_HEADERS})

target_include_directories(UI PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/ui>
)

# UI library dependencies: Core, Algorithms, Parsers, Rendering, Registration, Optimization, Qt UI components
target_link_libraries(UI PUBLIC
    Core
    Algorithms  # UI components like AlignmentControlPanel might use algorithms directly
    Parsers     # UI components for import/export might use parsers
    Rendering   # UI components interact with viewer/rendering settings
    Registration  # UI components for pose graph visualization
    Optimization  # UI components for Bundle Adjustment progress dialog
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)

message(STATUS "Configuring UI library...")
