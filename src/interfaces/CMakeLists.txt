# Sprint 1: Interfaces Library - Placeholder CMakeLists.txt
# This file defines the Interfaces static library containing interface definitions

# Define source files for the Interfaces library
set(INTERFACES_SOURCES
    # Most interfaces are header-only, but include any implementation files here
)

set(INTERFACES_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/IPointCloudProcessor.h
)

# Create Interfaces static library (might be header-only INTERFACE library later)
add_library(Interfaces STATIC ${INTERFACES_SOURCES} ${INTERFACES_HEADERS})

target_include_directories(Interfaces PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/interfaces>
)

# Interfaces library dependencies: Core, Qt components
target_link_libraries(Interfaces PUBLIC
    Core
    Qt6::Core
)

message(STATUS "Configuring Interfaces library...")
