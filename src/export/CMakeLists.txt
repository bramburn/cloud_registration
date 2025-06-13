# Sprint 7: Export Library - Complete Implementation
# This file defines the Export static library containing export functionality

# Define source files for the Export library
set(EXPORT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PointCloudExporter.cpp
)
set(EXPORT_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/export/IFormatWriter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/export/PointCloudExporter.h
)
add_library(Export STATIC ${EXPORT_SOURCES} ${EXPORT_HEADERS})
target_include_directories(Export PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/export>
)
target_link_libraries(Export PUBLIC Core Qt6::Core Qt6::Gui) # Core for Point struct, Qt6::Gui for QVector
message(STATUS "Configuring Export library...")
