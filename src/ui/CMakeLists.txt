# Sprint 6: UI Library - Complete Implementation
# This file defines the UI static library containing user interface components

# Define source files for the UI library (now moved to src/ui/src/)
set(UI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AlignmentControlPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ExportDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ICPProgressWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TargetDetectionDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UIThemeManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/UserPreferences.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ViewerToolbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/WorkflowProgressWidget.cpp

    # Files now located in src/ui/src/ after migration from src/ root
    ${CMAKE_CURRENT_SOURCE_DIR}/src/createprojectdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/loadingsettingsdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow_simple.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/projecthubwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/recentprojectsmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scanimportdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sidebarwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/projecttreemodel.cpp
)

# Define header files for the UI library (now moved to src/ui/include/ui/)
set(UI_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/AlignmentControlPanel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/ExportDialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/ICPProgressWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/TargetDetectionDialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/UIThemeManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/UserPreferences.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/ViewerToolbar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/WorkflowProgressWidget.h

    # Headers now located in src/ui/include/ui/ after migration from src/ root
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/createprojectdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/loadingsettingsdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/mainwindow_simple.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/projecthubwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/recentprojectsmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/scanimportdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/sidebarwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/projecttreemodel.h

    # Interfaces also moving to UI library
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/IMainView.h
)

# Create UI static library
add_library(UI STATIC ${UI_SOURCES} ${UI_HEADERS})

target_include_directories(UI PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/ui>
)

# UI library dependencies: Core, Algorithms, Parsers, Rendering, Qt UI components
target_link_libraries(UI PUBLIC
    Core
    Algorithms  # UI components like AlignmentControlPanel might use algorithms directly
    Parsers     # UI components for import/export might use parsers
    Rendering   # UI components interact with viewer/rendering settings
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)

message(STATUS "Configuring UI library...")
