# Sprint 1: Detection Library - Placeholder CMakeLists.txt
# This file defines the Detection static library containing target detection components

# Define source files for the Detection library
set(DETECTION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/NaturalPointSelector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/SphereDetector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TargetDetectionBase.cpp
)

set(DETECTION_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/NaturalPointSelector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/SphereDetector.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TargetDetectionBase.h
)

# Create Detection static library
add_library(Detection STATIC ${DETECTION_SOURCES} ${DETECTION_HEADERS})

target_include_directories(Detection PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/detection>
)

# Detection library dependencies: Core, Registration, Qt components
target_link_libraries(Detection PUBLIC
    Core
    Registration
    Qt6::Core
    Qt6::Gui
)

message(STATUS "Configuring Detection library...")
