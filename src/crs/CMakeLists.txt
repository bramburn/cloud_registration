# Sprint 7: CRS Library - Complete Implementation
# This file defines the CRS (Coordinate Reference System) static library

# Define source files for the CRS library
set(CRS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CoordinateSystemManager.cpp
)
set(CRS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/crs/CoordinateSystemManager.h
)
add_library(CRS STATIC ${CRS_SOURCES} ${CRS_HEADERS})
target_include_directories(CRS PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/crs>
)
target_link_libraries(CRS PUBLIC Core Export Qt6::Core Qt6::Gui) # Core for Point struct, Export for Point, Qt6::Gui for QVector
message(STATUS "Configuring CRS library...")
