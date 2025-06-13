# Sprint 7: Implementations Library - Complete Implementation
# This file defines the Implementations static library containing concrete implementations

# Define source files for the Implementations library
set(IMPLEMENTATIONS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VoxelGridProcessor.cpp
)
set(IMPLEMENTATIONS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/implementations/VoxelGridProcessor.h
)
add_library(Implementations STATIC ${IMPLEMENTATIONS_SOURCES} ${IMPLEMENTATIONS_HEADERS})
target_include_directories(Implementations PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/implementations>
)
target_link_libraries(Implementations PUBLIC Core Interfaces Qt6::Core) # Core for Point3D, Interfaces for IPointCloudProcessor
message(STATUS "Configuring Implementations library...")
