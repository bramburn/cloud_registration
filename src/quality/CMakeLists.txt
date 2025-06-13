# Sprint 6: Quality Library - Complete Implementation
# This file defines the Quality static library containing quality assessment and reporting components

# Define source files for the Quality library
set(QUALITY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PDFReportGenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/QualityAssessment.cpp
)

set(QUALITY_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/quality/PDFReportGenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/quality/QualityAssessment.h
)

# Create Quality static library
add_library(Quality STATIC ${QUALITY_SOURCES} ${QUALITY_HEADERS})

target_include_directories(Quality PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration>
)

# Quality library dependencies: Core, Qt components, Qt PrintSupport
target_link_libraries(Quality PUBLIC
    Core
    Qt6::Core
    Qt6::Gui
    Qt6::PrintSupport  # For QPrinter in PDFReportGenerator
)

message(STATUS "Configuring Quality library...")
