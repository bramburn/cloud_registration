# Sprint 1: Implementations Library - Placeholder CMakeLists.txt
# This file defines the Implementations static library containing concrete implementations

# Define source files for the Implementations library
set(IMPLEMENTATIONS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/VoxelGridProcessor.cpp
)

set(IMPLEMENTATIONS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/VoxelGridProcessor.h
)

# Create Implementations static library
add_library(Implementations STATIC ${IMPLEMENTATIONS_SOURCES} ${IMPLEMENTATIONS_HEADERS})

target_include_directories(Implementations PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/implementations>
)

# Implementations library dependencies: Core, Interfaces, Qt components
target_link_libraries(Implementations PUBLIC
    Core
    Interfaces
    Qt6::Core
    Qt6::Gui
)

message(STATUS "Configuring Implementations library...")
