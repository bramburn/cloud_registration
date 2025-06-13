# Sprint 2: Core Library - Updated CMakeLists.txt
# This file defines the Core static library containing foundational data structures and utilities

# Define source files for the Core library
# Files have been moved to src/core/src/ as part of Sprint 2 modularization
set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/project.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/octree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/voxelgridfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/performance_profiler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/screenspaceerror.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ProjectStateService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/projectmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/progressmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sqlitemanager.cpp
)

set(CORE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/project.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/octree.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/pointdata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/loadingsettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/lasheadermetadata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/voxelgridfilter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/performance_profiler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/screenspaceerror.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/ProjectStateService.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/projectmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/progressmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/sqlitemanager.h
)

# Create Core static library
add_library(Core STATIC ${CORE_SOURCES} ${CORE_HEADERS})

# Define public include directory for Core library
# This allows other modules to include Core headers as #include "core/project.h"
target_include_directories(Core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/core>
)

# Core library dependencies (Qt6::Core for QObject, QString; Qt6::Gui for QVector3D; Qt6::Sql for database operations)
target_link_libraries(Core PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Sql
)

message(STATUS "Configuring Core library...")
