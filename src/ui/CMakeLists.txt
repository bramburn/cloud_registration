# Sprint 1: UI Library - Placeholder CMakeLists.txt
# This file defines the UI static library containing user interface components

# Define source files for the UI library
# Note: Many of these files are currently at src/ root but will be moved to src/ui/src/ in future sprints
set(UI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/AlignmentControlPanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ExportDialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ICPProgressWidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UIThemeManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/UserPreferences.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ViewerToolbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/WorkflowProgressWidget.cpp

    # Files currently at src/ root, but logically belong to UI module
    ${CMAKE_CURRENT_SOURCE_DIR}/../createprojectdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../loadingsettingsdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../projecthubwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../recentprojectsmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../scanimportdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../sidebarwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../sqlitemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../projecttreemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../pointcloudloadmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../progressmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../scanimportmanager.cpp

    # Interfaces also moving to UI library
    ${CMAKE_CURRENT_SOURCE_DIR}/../IMainView.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../IPointCloudViewer.h
)

set(UI_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/AlignmentControlPanel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ExportDialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ICPProgressWidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UIThemeManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/UserPreferences.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ViewerToolbar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/WorkflowProgressWidget.h

    # Headers currently at src/ root, but logically belong to UI module
    ${CMAKE_CURRENT_SOURCE_DIR}/../createprojectdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../loadingsettingsdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../projecthubwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../recentprojectsmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../scanimportdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../sidebarwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../sqlitemanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../projecttreemodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../pointcloudloadmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../progressmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../scanimportmanager.h

    # Interfaces also moving to UI library
    ${CMAKE_CURRENT_SOURCE_DIR}/../IMainView.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../IPointCloudViewer.h
)

# Create UI static library
add_library(UI STATIC ${UI_SOURCES} ${UI_HEADERS})

target_include_directories(UI PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>  # Temporary for files at src/ root
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
