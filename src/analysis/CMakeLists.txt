# Sprint 1: Analysis Library - Placeholder CMakeLists.txt
# This file defines the Analysis static library containing analysis components

# Define source files for the Analysis library
set(ANALYSIS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/DifferenceAnalysis.cpp
)

set(ANALYSIS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/DifferenceAnalysis.h
)

# Create Analysis static library
add_library(Analysis STATIC ${ANALYSIS_SOURCES} ${ANALYSIS_HEADERS})

target_include_directories(Analysis PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/analysis>
)

# Analysis library dependencies: Core, Qt components
target_link_libraries(Analysis PUBLIC
    Core
    Qt6::Core
    Qt6::Gui
)

message(STATUS "Configuring Analysis library...")
