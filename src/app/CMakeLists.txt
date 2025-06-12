# Sprint 1: Application Executable - Placeholder CMakeLists.txt
# This file defines the main application executable

# Define source files for the main application executable
# Files have been moved to src/app/src/ as part of restructuring
set(APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow_simple.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MainPresenter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MainPresenterExtensions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MainViewAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pointcloudloadmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/scanimportmanager.cpp
)

set(APP_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app/mainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app/mainwindow_simple.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app/MainPresenter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app/MainPresenter_new.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app/MainViewAdapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app/pointcloudloadmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app/scanimportmanager.h
)

# Create the main executable
add_executable(CloudRegistration ${APP_SOURCES} ${APP_HEADERS})

# Define private include directory for application-specific headers
# This allows app files to include each other using relative paths within src/app/
target_include_directories(CloudRegistration PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/..  # For main.cpp at src/root
)

# Link main executable against all top-level modular libraries and necessary Qt modules
# This is a comprehensive list to ensure all dependencies are met for the initially
# "monolithic" app.cpp, which includes components from all logical domains.
target_link_libraries(CloudRegistration PRIVATE
    Core
    Algorithms
    Analysis
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
