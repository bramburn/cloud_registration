# Cloud Registration - Point Cloud Viewer

An open-source point cloud registration application built with Qt6 and OpenGL.

## Phase 1 Sprint 1 - Basic File Loading & 3D Visualization

This sprint implements the foundational application structure with basic E57 file loading and 3D visualization capabilities.

## Features

- **Qt6 Desktop Application**: Modern cross-platform GUI framework
- **OpenGL 3D Viewer**: Hardware-accelerated point cloud rendering
- **E57 File Support**: Basic E57 file format detection (mock data generation for testing)
- **Interactive Camera**: Mouse-based orbit, pan, and zoom controls
- **Error Handling**: Graceful handling of invalid files and loading errors

## Requirements

- **Qt6**: Version 6.5 or later
  - QtCore
  - QtWidgets
  - QtGui
  - QtOpenGLWidgets
- **CMake**: Version 3.16 or later
- **C++17** compatible compiler
- **OpenGL 3.3** or later
- **Google Test** (optional, for unit tests)

## Building

### Windows

1. **Using Build Scripts** (Recommended):
   ```powershell
   # Clean all build directories
   .\scripts\build-clean.ps1 -BuildType Clean

   # Build Release version
   .\scripts\build-clean.ps1 -BuildType Release
   ```

2. **Using CMake Presets Directly**:
   ```powershell
   # Configure and build Release version
   cmake --preset msvc-release
   cmake --build --preset msvc-release
   ```

3. **Run the Application**:
   ```powershell
   .\build\release\bin\CloudRegistration.exe
   ```

### Linux/macOS

1. **Install Qt6**:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install qt6-base-dev qt6-opengl-dev
   
   # macOS with Homebrew
   brew install qt6
   ```

2. **Build**:
   ```bash
   # Clean build
   rm -rf build/
   
   # Configure and build
   cmake --preset gcc-release
   cmake --build --preset gcc-release
   ```

3. **Run**:
   ```bash
   ./build/release/bin/CloudRegistration
   ```

## Usage

1. **Launch Application**: Run the CloudRegistration executable
2. **Load Point Cloud**: Click "Open E57 File" button or use File → Open menu
3. **Navigate 3D View**:
   - **Left Mouse + Drag**: Orbit camera around point cloud
   - **Right Mouse + Drag**: Pan camera
   - **Mouse Wheel**: Zoom in/out

## Testing

### Unit Tests

Run the E57 parser unit tests:

```powershell
# Windows
.\CloudRegistrationTests.exe

# Linux/macOS
./CloudRegistrationTests
```

### Manual Testing

1. **Application Launch**: Verify the application opens with an empty 3D viewport
2. **File Loading**: Test with various file types (E57, invalid files)
3. **Camera Controls**: Test mouse interactions for smooth navigation
4. **Error Handling**: Try loading non-existent or invalid files

## Project Structure

```
cloud_registration/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── src/                        # Source files
│   ├── main.cpp               # Application entry point
│   ├── mainwindow.h/cpp       # Main application window
│   ├── pointcloudviewerwidget.h/cpp  # OpenGL 3D viewer
│   └── e57parser.h/cpp        # E57 file parser
├── shaders/                    # OpenGL shaders
│   ├── point.vert            # Vertex shader
│   └── point.frag            # Fragment shader
├── tests/                      # Unit tests
│   └── test_e57parser.cpp     # E57 parser tests
└── docs/                       # Documentation
    └── phase 1-sprint 1.md    # Sprint requirements
```

## Current Limitations

- **E57 Parsing**: Currently generates mock data for testing. Full E57 binary parsing will be implemented in future sprints.
- **File Formats**: Only E57 format detection is implemented
- **Performance**: Not optimized for very large point clouds yet
- **Features**: Basic viewing only - no registration or advanced features

## Next Steps (Future Sprints)

- Implement full E57 binary parsing
- Add support for additional file formats (RCP, PLY, etc.)
- Implement scan/cluster tree view
- Add point cloud registration tools
- Performance optimizations for large datasets

## Contributing

This is Phase 1 Sprint 1 of a larger point cloud registration software project. See the documentation in `docs/` for detailed requirements and development plans.

## License

Open source - license to be determined.
