# Sprint 5: Rendering Tests - Complete CMakeLists.txt
# This file defines test executables for the rendering library

# CameraController Tests
add_executable(CameraControllerTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_camera_controller.cpp
)
target_link_libraries(CameraControllerTests PRIVATE
    Rendering           # Link against the Rendering library
    Core                # Indirect dependency through Rendering
    GTest::gtest_main   # Google Test framework
    Qt6::Test           # Qt Test module
    Qt6::Core
    Qt6::Gui            # For QVector3D, QMatrix4x4 etc.
)
target_include_directories(CameraControllerTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/rendering/include/rendering # Rendering headers
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core # Core headers
)
add_test(NAME CameraControllerTests COMMAND CameraControllerTests)

# OpenGLRenderer Tests
add_executable(OpenGLRendererTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_opengl_renderer.cpp
)
target_link_libraries(OpenGLRendererTests PRIVATE
    Rendering
    Core
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGLWidgets # For QOpenGLContext, QOpenGLBuffer etc.
)
target_include_directories(OpenGLRendererTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/rendering/include/rendering
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core
)
add_test(NAME OpenGLRendererTests COMMAND OpenGLRendererTests)

# GpuCuller Tests
add_executable(GpuCullerTests
    ${CMAKE_CURRENT_SOURCE_DIR}/test_gpuculler.cpp
)
target_link_libraries(GpuCullerTests PRIVATE
    Rendering
    Core
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGLWidgets
)
target_include_directories(GpuCullerTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/rendering/include/rendering
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/core/include/core
)
add_test(NAME GpuCullerTests COMMAND GpuCullerTests)

# OpenGL Initialization Test
add_executable(OpenGLInitTest
    ${CMAKE_CURRENT_SOURCE_DIR}/test_opengl_init.cpp
)
target_link_libraries(OpenGLInitTest PRIVATE
    Rendering
    GTest::gtest_main
    Qt6::Test
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGLWidgets
)
target_include_directories(OpenGLInitTest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/rendering/include/rendering
)
add_test(NAME OpenGLInitTest COMMAND OpenGLInitTest)

message(STATUS "Configuring rendering tests...")
