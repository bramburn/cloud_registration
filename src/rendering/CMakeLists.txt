# Sprint 5: Rendering Library - Updated CMakeLists.txt
# This file defines the Rendering static library containing 3D visualization components

# Define source files for the Rendering library
set(RENDERING_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OpenGLRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GpuCuller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/LODManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CameraController.cpp  # Moved from src/camera/
)

set(RENDERING_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/OpenGLRenderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/GpuCuller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/LODManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rendering/CameraController.h  # Moved from src/camera/
)

# Create Rendering static library
add_library(Rendering STATIC ${RENDERING_SOURCES} ${RENDERING_HEADERS})

target_include_directories(Rendering PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/CloudRegistration>
)

# Rendering library dependencies: Core, Qt OpenGL components
target_link_libraries(Rendering PUBLIC
    Core
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGLWidgets
)

message(STATUS "Configuring Rendering library...")
