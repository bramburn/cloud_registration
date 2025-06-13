# Sprint 7: Optimization Library - Complete Implementation
# This file defines the Optimization static library containing optimization algorithms

# Define source files for the Optimization library
set(OPTIMIZATION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BundleAdjustment.cpp
)
set(OPTIMIZATION_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/optimization/BundleAdjustment.h
)
add_library(Optimization STATIC ${OPTIMIZATION_SOURCES} ${OPTIMIZATION_HEADERS})
target_include_directories(Optimization PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/optimization>
)
target_link_libraries(Optimization PUBLIC Core Algorithms Registration Qt6::Core Qt6::Gui Eigen3::Eigen) # Registration for PoseGraph
message(STATUS "Configuring Optimization library...")
