# Sprint 7: Application Integration - Updated CMakeLists.txt
# This file defines the main application executable after all possible files have been migrated to their modular homes

# Define source files for the main application executable
set(APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MainPresenter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MainPresenterExtensions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MainViewAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pointcloudviewerwidget.cpp
    # Note: All other files previously at src/ root are assumed to be migrated to their modules
    # and should NOT be listed here.
)

set(APP_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app/mainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app/MainPresenter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app/MainViewAdapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/app/pointcloudviewerwidget.h
    # If MainPresenter_new.h is still distinct, include it here or integrate its content into MainPresenter.h
    # Assuming MainPresenter_new.h is consolidated into MainPresenter.h for this sprint.
)

# Create the main executable
add_executable(CloudRegistration ${APP_SOURCES} ${APP_HEADERS})

# Define private include directory for application-specific headers
target_include_directories(CloudRegistration PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/app)

# Link main executable against ALL modular libraries and necessary Qt modules
target_link_libraries(CloudRegistration PRIVATE
    Core
    Algorithms
    Parsers
    Rendering
    UI
    Registration
    Quality
    Analysis # New library for DifferenceAnalysis
    CRS # New library for CoordinateSystemManager
    Export # New library for PointCloudExporter and FormatWriters
    Features # New library for FeatureExtractor
    Implementations # New library for VoxelGridProcessor etc.
    Interfaces # New library for IPointCloudProcessor (likely INTERFACE, so may not need direct link)
    Optimization # New library for BundleAdjustment
    Performance # New library for MemoryManager, ParallelProcessing

    # Direct Qt dependencies for the application executable (MainWindow, PointCloudViewerWidget)
    Qt6::Widgets
    Qt6::Gui
    Qt6::OpenGLWidgets
    Qt6::Sql
)

message(STATUS "Configuring CloudRegistration executable...")
