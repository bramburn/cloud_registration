# Sprint 1: Camera Library - Placeholder CMakeLists.txt
# This file defines the Camera static library containing camera control components

# Define source files for the Camera library
set(CAMERA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/CameraController.cpp
)

set(CAMERA_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/CameraController.h
)

# Create Camera static library
add_library(Camera STATIC ${CAMERA_SOURCES} ${CAMERA_HEADERS})

target_include_directories(Camera PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/camera>
)

# Camera library dependencies: Core, Qt components
target_link_libraries(Camera PUBLIC
    Core
    Qt6::Core
    Qt6::Gui
)

message(STATUS "Configuring Camera library...")
