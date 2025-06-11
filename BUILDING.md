# Building CloudRegistration

This document provides comprehensive instructions for setting up, building, testing, and packaging the CloudRegistration application. Follow these steps to get the project running on your development environment.

## Prerequisites

Before building CloudRegistration, ensure you have the following tools installed:

### Required Tools

- **CMake**: Version 3.16 or later
  - Download from [cmake.org](https://cmake.org/download/)
  - Ensure CMake is added to your system PATH
- **C++17 Compiler**: 
  - **Windows**: Visual Studio 2022 with MSVC v143 toolset
  - **Linux**: GCC 8+ or Clang 7+
  - **macOS**: Xcode 10+ or Clang 7+
- **vcpkg**: C++ package manager for dependency management
  - Installation guide: [vcpkg Getting Started](https://vcpkg.io/en/getting-started.html)

### Platform-Specific Requirements

#### Windows
- **Visual Studio 2022** (Community, Professional, or Enterprise)
  - Install "Desktop development with C++" workload
  - Ensure MSVC v143 toolset and Windows 10/11 SDK are included
- **Qt6**: Version 6.5.3 or later (6.9.0 recommended)
  - Install to `C:\Qt\6.9.0\msvc2022_64\` (or adjust paths accordingly)
  - Select MSVC 2022 64-bit component during installation

#### Linux
- **Build Tools**: `build-essential` (Ubuntu/Debian) or `Development Tools` (Fedora/RHEL)
- **Qt6**: Version 6.5.0 or later
  - Install via package manager or Qt installer
- **Additional Libraries**: OpenGL development libraries, X11 development libraries

#### macOS
- **Xcode**: Latest version with command line tools
- **Qt6**: Version 6.5.0 or later
- **Homebrew**: Recommended for installing dependencies

## Dependency Configuration

CloudRegistration uses vcpkg for C++ dependency management and Qt6 for the user interface framework.

### vcpkg Setup

1. **Install vcpkg** (if not already installed):
   ```bash
   # Windows (PowerShell)
   git clone https://github.com/Microsoft/vcpkg.git C:\dev\vcpkg
   cd C:\dev\vcpkg
   .\bootstrap-vcpkg.bat
   
   # Linux/macOS
   git clone https://github.com/Microsoft/vcpkg.git ~/vcpkg
   cd ~/vcpkg
   ./bootstrap-vcpkg.sh
   ```

2. **Set Environment Variable** (optional but recommended):
   ```bash
   # Windows
   $env:VCPKG_ROOT = "C:\dev\vcpkg"
   
   # Linux/macOS
   export VCPKG_ROOT=~/vcpkg
   ```

### Dependencies Managed by vcpkg

The project's `vcpkg.json` file automatically manages the following dependencies:

- **eigen3**: Linear algebra library for mathematical computations
- **libe57format**: ASTM E57 standard implementation for E57 file support
- **xerces-c**: XML parsing library required by libE57Format
- **gtest**: Google Test framework for unit testing

These dependencies are automatically installed when you configure the project with CMake using the vcpkg toolchain.

### Adding New Dependencies

To add a new C++ library dependency:

1. **Search for the package**:
   ```bash
   vcpkg search <package-name>
   ```

2. **Add to vcpkg.json**:
   ```json
   {
     "dependencies": [
       "eigen3",
       "libe57format",
       "xerces-c",
       "gtest",
       "your-new-package"
     ]
   }
   ```

3. **Reconfigure CMake** to install the new dependency automatically.

## Building the Application

### Windows Build Instructions

1. **Clone the Repository**:
   ```powershell
   git clone https://github.com/bramburn/cloud_registration.git
   cd cloud_registration
   ```

2. **Configure the Build**:
   ```powershell
   # Using vcpkg toolchain file
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake"
   
   # Alternative: If VCPKG_ROOT environment variable is set
   cmake -B build -S .
   ```

3. **Build the Application**:
   ```powershell
   # Build Release configuration (recommended)
   cmake --build build --config Release
   
   # Build Debug configuration (for development)
   cmake --build build --config Debug
   ```

4. **Locate the Executable**:
   ```powershell
   # Release build
   .\build\bin\Release\CloudRegistration.exe
   
   # Debug build
   .\build\bin\Debug\CloudRegistration.exe
   ```

### Linux Build Instructions

1. **Install System Dependencies**:
   ```bash
   # Ubuntu/Debian
   sudo apt-get update
   sudo apt-get install -y build-essential cmake ninja-build qt6-base-dev qt6-opengl-dev libgl1-mesa-dev
   
   # Fedora/RHEL
   sudo dnf groupinstall -y "Development Tools"
   sudo dnf install -y cmake ninja-build qt6-qtbase-devel qt6-qtopengl-devel mesa-libGL-devel
   
   # Arch Linux
   sudo pacman -S base-devel cmake ninja qt6-base qt6-opengl mesa
   ```

2. **Clone and Build**:
   ```bash
   git clone https://github.com/bramburn/cloud_registration.git
   cd cloud_registration
   
   # Configure with vcpkg toolchain
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -G Ninja
   
   # Build the application
   cmake --build build
   ```

3. **Run the Application**:
   ```bash
   ./build/bin/CloudRegistration
   ```

### macOS Build Instructions

1. **Install Dependencies**:
   ```bash
   # Install Xcode command line tools
   xcode-select --install
   
   # Install Qt6 via Homebrew
   brew install qt@6 cmake ninja
   ```

2. **Build the Application**:
   ```bash
   git clone https://github.com/bramburn/cloud_registration.git
   cd cloud_registration
   
   # Configure with Qt6 path
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6)"
   
   # Build
   cmake --build build
   ```

## Running Tests

CloudRegistration includes a comprehensive test suite to verify functionality and catch regressions.

### Running All Tests

```bash
# Navigate to build directory
cd build

# Run all tests with detailed output
ctest --output-on-failure

# Run tests in parallel (faster)
ctest --output-on-failure --parallel 4

# Run tests with verbose output
ctest --output-on-failure --verbose
```

### Running Specific Test Categories

```bash
# Run only E57 parser tests
ctest --output-on-failure -R "E57.*"

# Run only algorithm tests
ctest --output-on-failure -R ".*Algorithm.*"

# Run only rendering tests
ctest --output-on-failure -R ".*Rendering.*"
```

### Custom Test Target

The project includes a custom `run_all_tests` target for convenience:

```bash
# Windows
cmake --build build --target run_all_tests

# Linux/macOS
cmake --build build --target run_all_tests
```

### Individual Test Executables

You can also run individual test executables directly:

```bash
# Windows
.\build\bin\Release\E57ParserTests.exe
.\build\bin\Release\LeastSquaresAlignmentTests.exe
.\build\bin\Release\VoxelGridFilterTests.exe

# Linux/macOS
./build/bin/E57ParserTests
./build/bin/LeastSquaresAlignmentTests
./build/bin/VoxelGridFilterTests
```

## Code Formatting

The project uses automated code formatting to maintain consistent code style.

### Format Code

```bash
# Format all source files
cmake --build build --target format

# Check formatting without making changes
cmake --build build --target check-format
```

### Formatting Tools

- **clang-format**: Used for C++ code formatting
- **Configuration**: Formatting rules are defined in `.clang-format` file
- **Integration**: Most IDEs can be configured to use the project's formatting rules

## Creating an Installer

CloudRegistration supports creating platform-native installers using CPack.

### Generate Installer

```bash
# Navigate to build directory
cd build

# Create installer package
cpack

# Create specific package type (Windows)
cpack -G NSIS

# Create specific package type (Linux)
cpack -G DEB    # Debian package
cpack -G RPM    # RPM package

# Create specific package type (macOS)
cpack -G DragNDrop  # DMG package
```

### Installer Configuration

- **Windows**: Creates NSIS-based installer (.exe)
- **Linux**: Creates DEB or RPM packages
- **macOS**: Creates DMG disk image
- **Configuration**: Installer settings are defined in CMakeLists.txt CPack section

### Deployment Dependencies

The installer automatically includes:
- Application executable
- Required Qt6 libraries
- vcpkg dependencies (E57Format, Xerces-C, etc.)
- Application resources and shaders
- Sample data files

## Troubleshooting

### Common Build Issues

1. **Qt6 Not Found**:
   ```
   Error: Could not find Qt6
   ```
   **Solution**: Set `CMAKE_PREFIX_PATH` to your Qt6 installation:
   ```bash
   cmake -B build -S . -DCMAKE_PREFIX_PATH="C:/Qt/6.9.0/msvc2022_64"
   ```

2. **vcpkg Dependencies Not Found**:
   ```
   Error: Could not find libE57Format
   ```
   **Solution**: Ensure vcpkg toolchain file is specified:
   ```bash
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake"
   ```

3. **Compiler Not Found (Windows)**:
   ```
   Error: MSVC compiler not found
   ```
   **Solution**:
   - Install Visual Studio 2022 with C++ workload
   - Use "Developer Command Prompt for VS 2022"
   - Or use CMake with Visual Studio generator:
     ```bash
     cmake -B build -S . -G "Visual Studio 17 2022" -A x64
     ```

4. **OpenGL Errors (Linux)**:
   ```
   Error: OpenGL context creation failed
   ```
   **Solution**: Install OpenGL development libraries:
   ```bash
   sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev
   ```

### Performance Issues

1. **Slow Build Times**:
   - Use Ninja generator: `-G Ninja`
   - Enable parallel builds: `cmake --build build --parallel`
   - Use ccache (Linux/macOS): `export CMAKE_CXX_COMPILER_LAUNCHER=ccache`

2. **Large Memory Usage**:
   - Build in Release mode for smaller binaries
   - Use incremental builds instead of clean rebuilds
   - Close unnecessary applications during build

### Runtime Issues

1. **Missing DLL Errors (Windows)**:
   - Use `windeployqt.exe` to copy Qt dependencies
   - Copy vcpkg DLLs from `vcpkg_installed/x64-windows/bin/`
   - Install Visual C++ Redistributable 2022

2. **Library Loading Errors (Linux)**:
   - Check `LD_LIBRARY_PATH` includes Qt6 libraries
   - Verify all dependencies are installed: `ldd ./build/bin/CloudRegistration`

## Development Workflow

### Recommended Development Setup

1. **IDE Configuration**:
   - **Visual Studio 2022**: Use CMake integration
   - **VS Code**: Install C++ and CMake extensions
   - **Qt Creator**: Import CMakeLists.txt as project
   - **CLion**: Native CMake support

2. **Build Configuration**:
   - Use Debug builds for development
   - Enable all warnings and treat warnings as errors
   - Use address sanitizer for memory debugging (GCC/Clang)

3. **Testing Workflow**:
   - Run tests after every change: `ctest --output-on-failure`
   - Use test-driven development for new features
   - Maintain test coverage above 80%

### Contributing Guidelines

1. **Code Style**:
   - Run `cmake --build build --target format` before committing
   - Follow Qt coding conventions
   - Use meaningful variable and function names

2. **Testing Requirements**:
   - Add unit tests for all new functionality
   - Ensure all tests pass before submitting pull requests
   - Include integration tests for file format support

3. **Documentation**:
   - Update this BUILDING.md for build process changes
   - Document public APIs with Doxygen comments
   - Include usage examples for new features

---

For additional help and project information, see the main [README.md](README.md) file.
