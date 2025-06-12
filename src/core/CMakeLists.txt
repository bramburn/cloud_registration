# Sprint 1: Core Library - Placeholder CMakeLists.txt
# This file defines the Core static library containing foundational data structures and utilities

# Define source files for the Core library
# Note: These files are currently at src/ root but will be moved to src/core/src/ in future sprints
set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../project.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../octree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../voxelgridfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../performance_profiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../screenspaceerror.cpp
)

set(CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/../project.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../octree.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../pointdata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../loadingsettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../lasheadermetadata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../voxelgridfilter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../performance_profiler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../screenspaceerror.h
)

# Create Core static library
add_library(Core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

# Define public include directory for Core library
# This allows other modules to include Core headers as #include "core/project.h"
target_include_directories(Core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>  # Temporary for files at src/ root
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/core>
)

# Core library dependencies (Qt6::Core for QObject, QString; Qt6::Gui for QVector3D)
target_link_libraries(Core PUBLIC
    Qt6::Core
    Qt6::Gui
)

message(STATUS "Configuring Core library...")
