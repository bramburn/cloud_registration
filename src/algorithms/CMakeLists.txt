# Sprint 1: Algorithms Library - Placeholder CMakeLists.txt
# This file defines the Algorithms static library containing mathematical algorithms

# Define source files for the Algorithms library
set(ALGORITHMS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/ICPRegistration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LeastSquaresAlignment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PointToPlaneICP.cpp
)

# Define header files for the Algorithms library
set(ALGORITHMS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/ICPRegistration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LeastSquaresAlignment.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PointToPlaneICP.h
)

# Create Algorithms static library
add_library(Algorithms STATIC ${ALGORITHMS_SOURCES} ${ALGORITHMS_HEADERS})

target_include_directories(Algorithms PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/algorithms>
)

# Algorithms library dependencies: Core, Qt components, and optionally Eigen3
target_link_libraries(Algorithms PUBLIC
    Core
    Qt6::Core
    Qt6::Gui
)

# Link Eigen3 only if available
if(EIGEN3_AVAILABLE)
    target_link_libraries(Algorithms PUBLIC Eigen3::Eigen)
endif()

message(STATUS "Configuring Algorithms library...")
