# CloudRegistration Modular Library Architecture Guide

## Overview

The CloudRegistration project has been refactored into a modular library architecture consisting of five specialized static libraries. This guide explains the structure, dependencies, and usage of each library.

## Library Hierarchy

```
CloudRegistration (Main Executable)
├── Core Library (Foundation)
├── Algorithms Library (depends on Core)
├── Parsers Library (depends on Core) 
├── Rendering Library (depends on Core)
└── UI Library (depends on Core)
```

## Library Details

### 1. Core Library ✅ **FUNCTIONAL**

**Purpose**: Foundational data structures and utilities shared across all components

**Files**:
- `project.cpp/h` - Project management core functionality
- `octree.cpp/h` - Spatial data structure for point clouds
- `voxelgridfilter.cpp/h` - Point cloud filtering algorithms
- `performance_profiler.cpp/h` - Performance monitoring utilities
- `screenspaceerror.cpp/h` - Screen space error calculations
- `pointdata.h` - Point cloud data structures (header-only)
- `loadingsettings.h` - Loading configuration (header-only)
- `lasheadermetadata.h` - LAS file metadata (header-only)

**Dependencies**:
- Qt6::Core (basic Qt functionality)
- Qt6::Gui (for QVector3D and matrix operations)

**CMake Target**: `Core`

**Usage Example**:
```cmake
target_link_libraries(MyTarget PRIVATE Core)
```

### 2. Algorithms Library ✅ **FUNCTIONAL**

**Purpose**: Mathematical algorithms and point cloud registration logic

**Files**:
- `algorithms/LeastSquaresAlignment.cpp/h` - Least squares alignment algorithm
- `algorithms/ICPRegistration.cpp/h` - Iterative Closest Point base implementation
- `algorithms/PointToPlaneICP.cpp/h` - Point-to-plane ICP variant

**Dependencies**:
- Core (foundational functionality)
- Qt6::Core (basic Qt functionality)
- Qt6::Gui (for geometric operations)
- Eigen3::Eigen (linear algebra operations)

**CMake Target**: `Algorithms`

**Usage Example**:
```cmake
target_link_libraries(MyTarget PRIVATE Algorithms)
# Note: Algorithms automatically brings in Core dependency
```

### 3. UI Library ✅ **FUNCTIONAL**

**Purpose**: User interface components and project management

**Files**:
- `mainwindow_simple.cpp/h` - Main application window
- `recentprojectsmanager.cpp/h` - Recent projects management
- `sqlitemanager.cpp/h` - Database management
- `projecttreemodel.cpp/h` - Project tree view model
- `progressmanager.cpp/h` - Progress tracking
- `scanimportmanager.cpp/h` - Scan import functionality
- `createprojectdialog.cpp/h` - Project creation dialog
- `loadingsettingsdialog.cpp/h` - Loading settings dialog
- `scanimportdialog.cpp/h` - Scan import dialog
- `pointcloudloadmanager.cpp/h` - Point cloud loading management

**Dependencies**:
- Core (foundational functionality)
- Qt6::Core (basic Qt functionality)
- Qt6::Gui (GUI components)
- Qt6::Widgets (widget components)
- Qt6::Sql (database functionality)

**CMake Target**: `UI`

**Usage Example**:
```cmake
target_link_libraries(MyTarget PRIVATE UI)
# Note: UI automatically brings in Core dependency
```

### 4. Parsers Library ⚠️ **NEEDS WORK**

**Purpose**: File format parsers for point cloud data

**Files**:
- `e57parserlib.cpp/h` - E57 format parser
- `lasparser.cpp/h` - LAS format parser

**Dependencies**:
- Core (foundational functionality)
- Qt6::Core (basic Qt functionality)
- E57Format (E57 file format library)
- XercesC::XercesC (XML parsing for E57)

**CMake Target**: `Parsers`

**Current Issues**:
- E57 parser has missing includes and implementation issues
- Needs refactoring to use proper libE57Format API

**Usage Example** (when fixed):
```cmake
target_link_libraries(MyTarget PRIVATE Parsers)
```

### 5. Rendering Library ⚠️ **NEEDS WORK**

**Purpose**: OpenGL rendering and camera control for 3D visualization

**Files**:
- `rendering/OpenGLRenderer.cpp/h` - Main OpenGL rendering engine
- `rendering/GpuCuller.cpp/h` - GPU-based frustum culling
- `rendering/LODManager.cpp/h` - Level-of-detail management
- `camera/CameraController.cpp/h` - Camera control and navigation

**Dependencies**:
- Core (foundational functionality)
- Qt6::Core (basic Qt functionality)
- Qt6::Gui (GUI components)
- Qt6::OpenGLWidgets (OpenGL integration)

**CMake Target**: `Rendering`

**Current Issues**:
- OpenGL function signatures incompatible with Qt6
- Buffer type mismatches need resolution

**Usage Example** (when fixed):
```cmake
target_link_libraries(MyTarget PRIVATE Rendering)
```

## Building Individual Libraries

You can build individual libraries for testing:

```bash
# Build Core library
cmake --build build --target Core

# Build Algorithms library
cmake --build build --target Algorithms

# Build UI library
cmake --build build --target UI

# Build all working libraries
cmake --build build --target Core Algorithms UI
```

## Running Tests

The test suite has been updated to use the modular libraries:

```bash
# Run all tests
ctest --output-on-failure -C Debug

# Or use the custom target
cmake --build build --target run_all_tests
```

## Installation

Install all libraries and headers for development:

```bash
cmake --build build --target install
```

This installs:
- Library files to `lib/`
- Executables to `bin/`
- Headers to `include/CloudRegistration/`
- Resources to `share/CloudRegistration/resources/`

## Development Guidelines

### Adding New Files

1. **Determine the appropriate library** based on functionality and dependencies
2. **Add source files** to the appropriate `*_SOURCES` variable in CMakeLists.txt
3. **Add header files** to the appropriate `*_HEADERS` variable
4. **Update dependencies** if new external libraries are required

### Creating New Libraries

1. **Define source and header lists**
2. **Create static library target** with `add_library(LibraryName STATIC ...)`
3. **Set include directories** with `target_include_directories`
4. **Link dependencies** with `target_link_libraries`
5. **Update main executable** to link against the new library

### Dependency Management

- **Keep Core library minimal** - only essential, widely-used functionality
- **Avoid circular dependencies** - libraries should only depend on Core, not each other
- **Use PUBLIC linkage** for dependencies that are part of the library's interface
- **Use PRIVATE linkage** for internal implementation dependencies

## Troubleshooting

### Common Build Issues

1. **Missing includes**: Ensure all required headers are included in source files
2. **Undefined symbols**: Check that all dependencies are properly linked
3. **Circular dependencies**: Verify library dependency hierarchy
4. **Qt6 compatibility**: Use Qt6-compatible APIs and function signatures

### Library-Specific Issues

- **Core**: Usually related to Qt6::Gui dependency for QVector3D
- **Algorithms**: May need Eigen3 compatibility updates
- **UI**: Check Qt6::Widgets and Qt6::Sql availability
- **Parsers**: E57Format and XercesC integration issues
- **Rendering**: Qt6::OpenGLWidgets compatibility problems

## Future Improvements

1. **Fix Parsers Library**: Resolve E57Format integration
2. **Fix Rendering Library**: Update OpenGL code for Qt6
3. **Add Plugin System**: Support for dynamically loaded parsers/renderers
4. **Optimize Dependencies**: Reduce library coupling where possible
5. **Add Library Versioning**: Support for API versioning and compatibility
