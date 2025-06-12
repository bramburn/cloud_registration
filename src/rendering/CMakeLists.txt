# Sprint 1: Rendering Library - Placeholder CMakeLists.txt
# This file defines the Rendering static library containing 3D visualization components

# Define source files for the Rendering library
set(RENDERING_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GpuCuller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/LODManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../camera/CameraController.cpp  # From camera module
)

set(RENDERING_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/OpenGLRenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GpuCuller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/LODManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/../camera/CameraController.h  # From camera module
)

# Create Rendering static library
add_library(Rendering STATIC ${RENDERING_SOURCES} ${RENDERING_HEADERS})

target_include_directories(Rendering PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>  # Temporary for files at current location
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../camera>  # For CameraController
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration/rendering>
)

# Rendering library dependencies: Core, Qt OpenGL components
target_link_libraries(Rendering PUBLIC
    Core
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGLWidgets
)

message(STATUS "Configuring Rendering library...")
