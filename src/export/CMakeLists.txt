# Sprint 1: Export Library - Placeholder CMakeLists.txt
# This file defines the Export static library containing export functionality

# Define source files for the Export library
set(EXPORT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/PointCloudExporter.cpp
)

set(EXPORT_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/IFormatWriter.h
)

# Create Export static library
add_library(Export STATIC ${EXPORT_SOURCES} ${EXPORT_HEADERS})

target_include_directories(Export PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/export>
)

# Export library dependencies: Core, Qt components
target_link_libraries(Export PUBLIC
    Core
    Qt6::Core
    Qt6::Gui
)

message(STATUS "Configuring Export library...")
