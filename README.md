# CloudRegistration - Professional Point Cloud Viewer

A professional-grade point cloud visualization and management application built with Qt6 and OpenGL, designed for surveying, engineering, and 3D data analysis workflows.

## Overview

CloudRegistration is a comprehensive desktop application for loading, visualizing, and managing point cloud data from various sources. It provides robust support for industry-standard file formats, advanced 3D visualization capabilities, and a project-based workflow for organizing and analyzing large datasets.

## Key Features

### **File Format Support**
- **E57 Files**: Complete support for ASTM E2807 standard using libE57Format
  - Multi-scan E57 files with XYZ coordinates, intensity, and RGB color data
  - Metadata preservation and scan organization
  - High-performance reading and writing capabilities
- **LAS Files**: Full LAS/LAZ support (versions 1.2-1.4, point formats 0-3)
  - Header metadata extraction and validation
  - Efficient point data parsing with configurable loading options

### **3D Visualization**
- **Hardware-Accelerated Rendering**: OpenGL-based point cloud viewer
- **Level-of-Detail (LOD) System**: Octree-based rendering for large datasets
- **Advanced Rendering Options**:
  - Color and intensity attribute visualization
  - Point size attenuation and screen-space error calculations
  - Multiple viewing modes and camera controls
- **Interactive Navigation**: Mouse-based orbit, pan, zoom, and preset view angles

### **Project Management**
- **SQLite-Based Projects**: Organized workspace for managing multiple scans
- **Scan Import System**: Batch import and organization of point cloud files
- **Metadata Management**: Comprehensive scan information and properties
- **Recent Projects**: Quick access to recently opened projects

### **Performance & Efficiency**
- **Voxel Grid Filtering**: Configurable point cloud subsampling
- **Chunked Processing**: Memory-efficient handling of large datasets
- **Progress Reporting**: Real-time feedback for long-running operations
- **Multi-threaded Operations**: Non-blocking file loading and processing

## System Requirements

### **Windows (Primary Platform)**
- **Operating System**: Windows 10/11 (64-bit)
- **Qt6**: Version 6.9.0 or later (6.5.3+ supported)
  - QtCore, QtWidgets, QtGui, QtOpenGLWidgets
  - QtXml, QtSql, QtConcurrent, QtTest
- **Visual Studio**: 2022 (MSVC v143 toolset)
- **CMake**: Version 3.16 or later
- **vcpkg**: For dependency management
- **OpenGL**: Version 3.3 or later
- **Memory**: 8GB RAM minimum, 16GB+ recommended for large datasets

### **Linux (Fully Supported)**
- **Operating System**: Ubuntu 20.04+, Fedora 35+, Arch Linux, or compatible (64-bit)
- **Qt6**: Version 6.5.0 or later (6.9.0+ recommended)
  - QtCore, QtWidgets, QtGui, QtOpenGLWidgets
  - QtXml, QtSql, QtConcurrent, QtTest
- **Compilers**: GCC 8+ or Clang 7+
- **CMake**: Version 3.16 or later
- **Build System**: Ninja (recommended) or Make
- **OpenGL**: Version 3.3 or later
- **Memory**: 8GB RAM minimum, 16GB+ recommended for large datasets

### **Dependencies**
- **libE57Format**: E57 file format support
- **Xerces-C**: XML parsing for E57 files
- **Google Test**: Unit testing framework
- **Vulkan SDK**: Advanced graphics support (optional)
- **Boost**: System and filesystem libraries (Linux)
- **Mesa**: OpenGL implementation (Linux)

## Building

For detailed build instructions, dependency setup, and troubleshooting, see **[BUILDING.md](BUILDING.md)**.

### Quick Start

**Prerequisites**: CMake 3.16+, C++17 compiler, vcpkg, Qt6

```bash
# Clone the repository
git clone https://github.com/bramburn/cloud_registration.git
cd cloud_registration

# Configure with vcpkg toolchain
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"

# Build the application
cmake --build build --config Release

# Run tests
cd build && ctest --output-on-failure
```

### Platform-Specific Quick Setup

#### **Linux (Recommended for Development)**
```bash
# Install dependencies
sudo apt-get install -y build-essential cmake ninja-build qt6-base-dev qt6-opengl-dev

# Clone and build
git clone https://github.com/bramburn/cloud_registration.git
cd cloud_registration
./setup.sh
```

#### **Windows**
```powershell
# Prerequisites: Visual Studio 2022, Qt6, vcpkg
git clone https://github.com/bramburn/cloud_registration.git
cd cloud_registration
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
```

For complete setup instructions including dependency installation, see **[BUILDING.md](BUILDING.md)**.

## Platform-Specific Setup Guides

For detailed setup instructions for all platforms, see **[BUILDING.md](BUILDING.md)**.

### **Linux Setup Guide**
For detailed Linux setup instructions, see [Linux Setup Guide](docs/LINUX_SETUP_GUIDE.md).

### **Windows Setup Guide**
For detailed Windows setup instructions, see [Windows Setup Guide](docs/WINDOWS_SETUP_GUIDE.md).

## Getting Started

### **First Launch**

1. **Start the Application**
   ```powershell
   .\CloudRegistration.exe
   ```

2. **Create Your First Project**
   - Click "Create New Project" on the welcome screen
   - Choose a project name and location
   - The project will store scan metadata and settings

### **Loading Point Cloud Data**

1. **Import Scans**
   - Use **File → Import Scans** or click the import button
   - Select E57 or LAS files from your file system
   - The application will parse metadata and add scans to your project

2. **View Point Clouds**
   - Click on any scan in the project tree to load it
   - Use the 3D viewer to explore your data

### **3D Navigation Controls**

- **Orbit**: Left mouse button + drag
- **Pan**: Right mouse button + drag
- **Zoom**: Mouse wheel scroll
- **Preset Views**: Use toolbar buttons for Top, Left, Right, Bottom views
- **Reset View**: Double-click to fit point cloud in view

### **Visualization Options**

- **Color Rendering**: Toggle RGB color display (if available in data)
- **Intensity Rendering**: Toggle intensity-based coloring
- **Point Size**: Adjust point size and attenuation settings
- **Loading Settings**: Configure voxel grid filtering for large datasets

## Testing and Validation

### **Automated Test Suite**

The application includes comprehensive unit and integration tests:

```powershell
# Run all tests
.\scripts\run-tests.ps1

# Run with detailed output and coverage
.\scripts\run-tests.ps1 -Coverage -Verbose

# Run specific test categories
.\scripts\run-tests.ps1 -TestFilter "E57*"
```

### **Individual Test Components**

```powershell
# E57 parsing and writing tests
.\build-release\bin\Release\E57ParserTests.exe
.\build-release\bin\Release\E57WriterLibTests.exe

# LAS file parsing tests
.\build-release\bin\Release\LasParserTests.exe

# Project management tests
.\build-release\bin\Release\ProjectManagerTests.exe

# Performance and integration tests
.\build-release\bin\Release\E57Sprint4ComprehensiveTests.exe
```

### **Sample Data for Testing**

The repository includes sample files for testing:

- **E57 Files**: `sample/bunnyDouble.e57`, `sample/bunnyInt32.e57`
- **LAS Files**: `sample/S2max-Power line202503.las`
- **Test Data**: Various test files in `test_data/` directory

### **Manual Testing Checklist**

1. **Application Startup**
   - ✅ Application launches without errors
   - ✅ Welcome screen displays correctly
   - ✅ Recent projects list functions

2. **Project Management**
   - ✅ Create new project
   - ✅ Open existing project
   - ✅ Import scans (E57 and LAS)
   - ✅ Project tree navigation

3. **Point Cloud Visualization**
   - ✅ Load and display point clouds
   - ✅ 3D navigation controls
   - ✅ Color and intensity rendering
   - ✅ Performance with large datasets

4. **File Format Support**
   - ✅ E57 multi-scan files
   - ✅ LAS files with various point formats
   - ✅ Error handling for invalid files

## Project Structure

```
cloud_registration/
├── CMakeLists.txt              # Main build configuration
├── CMakePresets.json           # CMake preset configurations
├── vcpkg.json                  # Dependency management
├── README.md                   # This documentation
├── LICENSE                     # Project license
├── resources.qrc               # Qt resource file
│
├── src/                        # Application source code
│   ├── main.cpp               # Application entry point
│   ├── mainwindow.h/cpp       # Main application window
│   ├── pointcloudviewerwidget.h/cpp  # OpenGL 3D viewer with LOD
│   ├── projectmanager.h/cpp   # Project and workspace management
│   ├── projecthubwidget.h/cpp # Project selection interface
│   ├── sidebarwidget.h/cpp    # Navigation and scan tree
│   │
│   ├── e57parserlib.h/cpp     # E57 file parser (libE57Format)
│   ├── e57writer_lib.h/cpp    # E57 file writing capabilities
│   ├── E57DataManager.h/cpp   # High-level E57 operations
│   ├── lasparser.h/cpp        # LAS/LAZ file parser
│   │
│   ├── pointcloudloadmanager.h/cpp  # Point cloud loading system
│   ├── scanimportmanager.h/cpp      # Scan import workflows
│   ├── sqlitemanager.h/cpp          # Database operations
│   ├── projecttreemodel.h/cpp       # Project tree data model
│   │
│   ├── octree.h/cpp           # Spatial indexing for LOD
│   ├── screenspaceerror.h/cpp # LOD error calculations
│   ├── voxelgridfilter.h/cpp  # Point cloud subsampling
│   ├── performance_profiler.h/cpp   # Performance monitoring
│   │
│   └── e57_parser/            # E57 parsing components
│       ├── E57HeaderParser.h/cpp
│       ├── E57BinaryReader.h/cpp
│       └── E57XmlParser.h/cpp
│
├── tests/                      # Comprehensive test suite
│   ├── test_e57parser.cpp     # E57 parsing tests
│   ├── test_e57writer_lib.cpp # E57 writing tests
│   ├── test_lasparser.cpp     # LAS parsing tests
│   ├── test_projectmanager.cpp # Project management tests
│   ├── test_pointcloudviewerwidget_lod.cpp # LOD system tests
│   ├── E57TestFramework.h/cpp # Test utilities
│   ├── PerformanceProfiler.h/cpp # Performance testing
│   └── e57_parser/            # E57 component tests
│
├── scripts/                    # Build and utility scripts
│   ├── build-clean.ps1        # Build management
│   ├── run-tests.ps1          # Test execution
│   ├── setup_libe57format.ps1 # Dependency setup
│   └── tests/                 # Specialized test scripts
│
├── docs/                       # Documentation
│   ├── sprints/               # Development sprint documentation
│   ├── mvp/                   # MVP requirements and design
│   ├── e57library2/           # E57 integration documentation
│   └── visualization/         # Rendering and visualization docs
│
├── sample/                     # Sample data files
│   ├── bunnyDouble.e57        # E57 test file (double precision)
│   ├── bunnyInt32.e57         # E57 test file (integer)
│   └── S2max-Power line202503.las # LAS test file
│
├── test_data/                  # Test data and validation files
├── shaders/                    # OpenGL shader programs
│   ├── point.vert            # Point cloud vertex shader
│   └── point.frag            # Point cloud fragment shader
│
└── icons/                      # Application icons and resources
    ├── project.svg
    ├── scan.svg
    └── cluster.svg
```

## Troubleshooting

### **Common Build Issues**

1. **Qt6 Not Found**
   ```
   Error: Could not find Qt6
   ```
   - Verify Qt6 installation path: `C:\Qt\6.9.0\msvc2022_64\`
   - Set environment variable: `Qt6_DIR=C:\Qt\6.9.0\msvc2022_64\lib\cmake\Qt6`

2. **vcpkg Dependencies Missing**
   ```
   Error: Could not find libE57Format
   ```
   - Ensure vcpkg is properly installed and integrated
   - Verify dependencies: `.\vcpkg list | findstr e57`
   - Set toolchain: `CMAKE_TOOLCHAIN_FILE=C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake`

3. **Visual Studio Toolset Issues**
   ```
   Error: MSVC compiler not found
   ```
   - Install Visual Studio 2022 with C++ workload
   - Ensure MSVC v143 toolset is selected

### **Runtime Issues**

1. **Missing DLL Errors**
   - Run `windeployqt.exe` on the executable
   - Copy vcpkg DLLs to application directory
   - Install Visual C++ Redistributable 2022

2. **OpenGL Context Errors**
   - Update graphics drivers
   - Verify OpenGL 3.3+ support
   - Check Windows graphics settings

### **Performance Optimization**

For large point cloud files:
- Use **Voxel Grid** loading method to reduce point count
- Enable **LOD rendering** in visualization settings
- Increase system RAM for better caching
- Use SSD storage for faster file access

## Development and Contributing

### **Development Setup**

1. **IDE Configuration**
   - Visual Studio 2022 or VS Code with C++ extensions
   - Qt Creator (optional, for UI design)
   - Configure IntelliSense with CMake integration

2. **Code Quality Tools**
   - **clang-format**: Automatic code formatting (see [Code Quality Setup](docs/code-quality-setup.md))
   - **clang-tidy**: Static code analysis
   - **Pre-commit hooks**: Automated quality checks
   - **CI/CD integration**: Continuous quality monitoring

3. **Code Style**
   - Follow Qt coding conventions with clang-format enforcement
   - Use meaningful variable and function names
   - Document public APIs with Doxygen comments
   - Include unit tests for new functionality

4. **Testing Requirements**
   - All new features must include unit tests
   - Integration tests for file format support
   - Performance benchmarks for optimization work
   - Manual testing with real-world data

### **Contributing Guidelines**

1. **Fork and Branch**
   - Fork the repository on GitHub
   - Create feature branches from `main`
   - Use descriptive branch names: `feature/e57-export`, `fix/las-parsing`

2. **Pull Request Process**
   - Ensure all tests pass locally
   - Include test data for new file format features
   - Update documentation for user-facing changes
   - Request review from maintainers

3. **Issue Reporting**
   - Use GitHub Issues for bug reports and feature requests
   - Include system information and reproduction steps
   - Attach sample files for file format issues

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- **libE57Format**: ASTM E57 standard implementation
- **Qt Framework**: Cross-platform application framework
- **vcpkg**: C++ package management
- **Contributors**: All developers who have contributed to this project

## Support and Documentation

- **GitHub Issues**: Bug reports and feature requests
- **Documentation**: Comprehensive guides in `docs/` directory
- **Sample Data**: Test files available in `sample/` directory
- **API Reference**: Generated from source code comments

---

**CloudRegistration** - Professional point cloud visualization for surveying and engineering workflows.
