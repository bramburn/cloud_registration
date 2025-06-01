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

### Automated Tests

Run all tests using the test script:

```powershell
# Windows - Run all tests
.\scripts\run-tests.ps1

# Run tests with coverage and verbose output
.\scripts\run-tests.ps1 -Coverage -Verbose
```

### Individual Test Executables

You can also run individual test executables directly:

```powershell
# Windows
.\build\bin\E57ParserTests.exe
.\build\bin\LasParserTests.exe
.\build\bin\ProjectManagerTests.exe

# Linux/macOS
./build/bin/E57ParserTests
./build/bin/LasParserTests
./build/bin/ProjectManagerTests
```

### Manual Testing

1. **Application Launch**: Verify the application opens with an empty 3D viewport
2. **File Loading**: Test with various file types (E57, LAS, invalid files)
3. **Camera Controls**: Test mouse interactions for smooth navigation
4. **Error Handling**: Try loading non-existent or invalid files
5. **Project Management**: Test creating and opening projects

## Project Structure

```
cloud_registration/
├── CMakeLists.txt              # Build configuration
├── README.md                   # This file
├── src/                        # Main application source code
│   ├── main.cpp               # Application entry point
│   ├── mainwindow.h/cpp       # Main application window
│   ├── pointcloudviewerwidget.h/cpp  # OpenGL 3D viewer
│   ├── e57parserlib.h/cpp     # E57 file parser library
│   ├── lasparser.h/cpp        # LAS file parser
│   ├── projectmanager.h/cpp   # Project management
│   ├── projecthubwidget.h/cpp # Project hub interface
│   ├── sidebarwidget.h/cpp    # Sidebar interface
│   └── ...                    # Other source files
├── tests/                      # Unit and integration tests
│   ├── test_e57parser.cpp     # E57 parser tests
│   ├── test_lasparser.cpp     # LAS parser tests
│   ├── test_projectmanager.cpp # Project manager tests
│   ├── demos/                 # Simple test and demo programs
│   │   ├── test_e57_simple.cpp
│   │   ├── test_las_parser.cpp
│   │   ├── test_sprint1_implementation.cpp
│   │   └── ...                # Other demo files
│   └── ...                    # Other test files
├── scripts/                    # Build and utility scripts
│   ├── build-clean.ps1        # Build cleanup script
│   ├── run-tests.ps1          # Test execution script
│   ├── setup_libe57format.ps1 # E57 library setup
│   ├── tests/                 # Test-specific scripts
│   │   ├── test_app.ps1
│   │   ├── verify_sprint1_implementation.ps1
│   │   └── ...                # Other test scripts
│   └── ...                    # Other utility scripts
├── docs/                       # General documentation
│   ├── build-instructions.md  # Build instructions
│   ├── testing-best-practices.md # Testing guidelines
│   ├── sprints/               # Sprint-specific documentation
│   │   ├── SPRINT_1_IMPLEMENTATION_COMPLETE.md
│   │   ├── E57_PARSING_FIX_SUMMARY.md
│   │   └── ...                # Other sprint documents
│   └── ...                    # Other documentation
├── shaders/                    # OpenGL shaders
│   ├── point.vert            # Vertex shader
│   └── point.frag            # Fragment shader
├── sample/                     # Sample point cloud files
│   ├── bunnyDouble.e57
│   ├── bunnyInt32.e57
│   └── S2max-Power line202503.las
└── test_data/                  # Test data files
    ├── test_3_points_line.e57
    ├── test_triangle.e57
    └── ...                     # Other test files
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
