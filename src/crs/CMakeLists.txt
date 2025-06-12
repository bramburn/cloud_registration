# Sprint 1: CRS Library - Placeholder CMakeLists.txt
# This file defines the CRS (Coordinate Reference System) static library

# Define source files for the CRS library
set(CRS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/CoordinateSystemManager.cpp
)

set(CRS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/CoordinateSystemManager.h
)

# Create CRS static library
add_library(CRS STATIC ${CRS_SOURCES} ${CRS_HEADERS})

target_include_directories(CRS PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/crs>
)

# CRS library dependencies: Core, Qt components
target_link_libraries(CRS PUBLIC
    Core
    Qt6::Core
    Qt6::Gui
)

message(STATUS "Configuring CRS library...")
