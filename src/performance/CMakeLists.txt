# Sprint 1: Performance Library - Placeholder CMakeLists.txt
# This file defines the Performance static library containing performance monitoring components

# Define source files for the Performance library
set(PERFORMANCE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/MemoryManager.cpp
)

set(PERFORMANCE_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/MemoryManager.h
)

# Create Performance static library
add_library(Performance STATIC ${PERFORMANCE_SOURCES} ${PERFORMANCE_HEADERS})

target_include_directories(Performance PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/performance>
)

# Performance library dependencies: Core, Qt components
target_link_libraries(Performance PUBLIC
    Core
    Qt6::Core
    Qt6::Gui
)

message(STATUS "Configuring Performance library...")
