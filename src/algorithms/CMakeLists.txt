# Sprint 3: Algorithms Library - Updated CMakeLists.txt
# This file defines the Algorithms static library containing mathematical algorithms

# Define source files for the Algorithms library (located in current directory)
set(ALGORITHMS_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/ICPRegistration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LeastSquaresAlignment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PointToPlaneICP.cpp
)

# Define header files for the Algorithms library (located in current directory)
set(ALGORITHMS_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/ICPRegistration.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LeastSquaresAlignment.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PointToPlaneICP.h
)

# Create Algorithms static library
add_library(Algorithms STATIC ${ALGORITHMS_SOURCES} ${ALGORITHMS_HEADERS})

# Set up include directories for the Algorithms library
target_include_directories(Algorithms PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # For current location of headers
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/algorithms>
)

# Algorithms library dependencies: Core, Qt components, and Eigen3
target_link_libraries(Algorithms PUBLIC
    Core
    Qt6::Core
    Qt6::Gui
)

# Link Eigen3 (required for LeastSquaresAlignment)
find_package(Eigen3 REQUIRED)
target_link_libraries(Algorithms PUBLIC Eigen3::Eigen)

message(STATUS "Configuring Algorithms library...")
