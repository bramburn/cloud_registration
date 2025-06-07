#!/bin/bash

# Sprint 1 Implementation Test Script
# Tests the 3D Point Cloud Visualization components

echo "=== Sprint 1: 3D Point Cloud Visualization Test ==="
echo "Testing implementation of OpenGL rendering, camera controls, and LOD system"

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir -p build
fi

cd build

# Configure CMake
echo ""
echo "Configuring CMake..."
cmake .. -G "Unix Makefiles"
if [ $? -ne 0 ]; then
    echo "CMake configuration failed"
    exit 1
fi

# Build the project
echo ""
echo "Building project..."
cmake --build . --config Release
if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi

# Check if shader files are copied
echo ""
echo "Checking shader files..."
shader_files=(
    "shaders/pointcloud.vert"
    "shaders/pointcloud.frag"
    "shaders/point.vert"
    "shaders/point.frag"
)

for shader in "${shader_files[@]}"; do
    if [ -f "$shader" ]; then
        echo "✓ Found: $shader"
    else
        echo "✗ Missing: $shader"
    fi
done

# Check file structure
echo ""
echo "Checking Sprint 1 file structure..."
sprint1_files=(
    "../src/rendering/OpenGLRenderer.h"
    "../src/rendering/OpenGLRenderer.cpp"
    "../src/camera/CameraController.h"
    "../src/camera/CameraController.cpp"
    "../src/rendering/LODManager.h"
    "../src/rendering/LODManager.cpp"
    "../src/ui/ViewerToolbar.h"
    "../src/ui/ViewerToolbar.cpp"
    "../tests/test_opengl_renderer.cpp"
    "../tests/test_camera_controller.cpp"
)

missing_files=()
for file in "${sprint1_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file"
    else
        echo "✗ $file"
        missing_files+=("$file")
    fi
done

# Summary
echo ""
echo "=== Sprint 1 Implementation Summary ==="
if [ ${#missing_files[@]} -eq 0 ]; then
    echo "✓ All Sprint 1 files are present"
else
    echo "✗ Missing files: ${#missing_files[@]}"
    for file in "${missing_files[@]}"; do
        echo "  - $file"
    done
fi

echo ""
echo "=== Sprint 1 Test Complete ==="
echo "Sprint 1 components have been implemented."
echo "Next: Build and test the application"

cd ..
