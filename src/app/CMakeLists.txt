# Sprint 1: Application Executable - Placeholder CMakeLists.txt
# This file defines the main application executable

# Define source files for the main application executable
# These are files currently at src/ root that are not yet assigned to a modular library.
# They will be moved to src/app/ or other modules in future sprints.
set(APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../mainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../mainwindow_simple.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../MainPresenter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../MainPresenterExtensions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../MainViewAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../pointcloudviewerwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../ProjectStateService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../projectmanager.cpp
)

set(APP_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../mainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../mainwindow_simple.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../MainPresenter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../MainPresenter_new.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../MainViewAdapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../pointcloudviewerwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../ProjectStateService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../projectmanager.h
)

# Create the main executable
add_executable(CloudRegistration ${APP_SOURCES} ${APP_HEADERS})

# Define private include directory for application-specific headers
# This allows app files to include each other using relative paths within src/ and src/app/
target_include_directories(CloudRegistration PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..  # For includes like "project.h" that are still at src/root
)

# Link main executable against all top-level modular libraries and necessary Qt modules
# This is a comprehensive list to ensure all dependencies are met for the initially
# "monolithic" app.cpp, which includes components from all logical domains.
target_link_libraries(CloudRegistration PRIVATE
    Core
    Algorithms
    Analysis
    Camera
    CRS
    Detection
    Export
    Features
    Implementations
    Interfaces
    Optimization
    Parsers
    Performance
    Quality
    Registration
    Rendering
    UI

    # Direct Qt dependencies for the application executable (MainWindow, PointCloudViewerWidget)
    Qt6::Widgets
    Qt6::Gui
    Qt6::OpenGLWidgets
    Qt6::Sql
)

message(STATUS "Configuring CloudRegistration executable...")
