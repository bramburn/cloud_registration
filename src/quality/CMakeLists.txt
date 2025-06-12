# Sprint 1: Quality Library - Placeholder CMakeLists.txt
# This file defines the Quality static library containing quality assessment components

# Define source files for the Quality library
set(QUALITY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/PDFReportGenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/QualityAssessment.cpp
)

set(QUALITY_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/PDFReportGenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QualityAssessment.h
)

# Create Quality static library
add_library(Quality STATIC ${QUALITY_SOURCES} ${QUALITY_HEADERS})

target_include_directories(Quality PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/quality>
)

# Quality library dependencies: Core, Qt components
target_link_libraries(Quality PUBLIC
    Core
    Qt6::Core
    Qt6::Gui
)

message(STATUS "Configuring Quality library...")
