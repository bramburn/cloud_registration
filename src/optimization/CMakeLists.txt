# Sprint 1: Optimization Library - Placeholder CMakeLists.txt
# This file defines the Optimization static library containing optimization algorithms

# Define source files for the Optimization library
set(OPTIMIZATION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/BundleAdjustment.cpp
)

set(OPTIMIZATION_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/BundleAdjustment.h
)

# Create Optimization static library
add_library(Optimization STATIC ${OPTIMIZATION_SOURCES} ${OPTIMIZATION_HEADERS})

target_include_directories(Optimization PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/optimization>
)

# Optimization library dependencies: Core, Algorithms, Registration, Qt components, and optionally Eigen3
target_link_libraries(Optimization PUBLIC
    Core
    Algorithms
    Registration
    Qt6::Core
    Qt6::Gui
)

# Link Eigen3 only if available
if(EIGEN3_AVAILABLE)
    target_link_libraries(Optimization PUBLIC Eigen3::Eigen)
endif()

message(STATUS "Configuring Optimization library...")
