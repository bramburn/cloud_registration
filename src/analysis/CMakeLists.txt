# Sprint 7: Analysis Library - Complete Implementation
# This file defines the Analysis static library containing analysis components

# Define source files for the Analysis library
set(ANALYSIS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/DifferenceAnalysis.cpp
)
set(ANALYSIS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/analysis/DifferenceAnalysis.h
)
add_library(Analysis STATIC ${ANALYSIS_SOURCES} ${ANALYSIS_HEADERS})
target_include_directories(Analysis PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/analysis>
)
target_link_libraries(Analysis PUBLIC Core Qt6::Core Qt6::Gui) # Core for Point3D, Qt6::Gui for QVector
message(STATUS "Configuring Analysis library...")
