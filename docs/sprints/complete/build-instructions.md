# Qt6/C++ Cloud Registration Project - Build Instructions

## 🏗️ ORGANIZED BUILD STRUCTURE

### New Clean Build Organization
```
cloud_registration/
├── build-debug/          # Debug builds (gitignored)
├── build-release/        # Release builds (gitignored)
├── src/                  # Source code (tracked)
├── tests/                # Unit tests (tracked)
├── shaders/              # OpenGL shaders (tracked)
├── scripts/              # Build scripts (tracked)
│   └── build-clean.ps1   # New organized build script
├── CMakeLists.txt        # Build configuration (tracked)
├── CMakePresets.json     # Updated presets (tracked)
└── .gitignore           # Updated ignore rules (tracked)
```

## ✅ SOLUTION: Correct CMake Build Workflow

### Method 1: Using New Build Script (Recommended)

```powershell
# Clean build (removes all build directories)
.\scripts\build-clean.ps1 -BuildType Clean

# Build Release version
.\scripts\build-clean.ps1 -BuildType Release

# Build Debug version
.\scripts\build-clean.ps1 -BuildType Debug

# Build both configurations
.\scripts\build-clean.ps1 -BuildType Both

# Clean build with tests
.\scripts\build-clean.ps1 -BuildType Release -CleanFirst -RunTests
```

### Method 2: Using Updated CMake Presets

```powershell
# Configure and build Release (creates build-release/)
cmake --preset msvc-release
cmake --build --preset msvc-release

# Configure and build Debug (creates build-debug/)
cmake --preset msvc-debug
cmake --build --preset msvc-debug
```

### Method 2: Manual Configuration

```powershell
# 1. Create and configure build directory
mkdir build-manual
cd build-manual

# 2. Configure with vcpkg toolchain
cmake .. -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_TOOLCHAIN_FILE="c:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake" `
  -DCMAKE_PREFIX_PATH="C:\Qt\6.9.0\msvc2022_64" `
  -DCMAKE_BUILD_TYPE=Release

# 3. Build the project
cmake --build . --config Release
```

### Method 3: Using Existing Build Directory

```powershell
# Navigate to existing configured build directory
cd build\release

# Build directly from the configured directory
cmake --build . --config Release
```

## ✅ Current Status

Your project is now successfully built! The executable is located at:
```
build\release\bin\Release\CloudRegistration.exe
```

## Environment Setup

Ensure these environment variables are set:
```powershell
$env:VCPKG_ROOT="c:\dev\vcpkg"
$env:PATH="$env:VCPKG_ROOT;$env:PATH"
```

## Project Configuration

- **Qt Version**: 6.9.0 (msvc2022_64)
- **Generator**: Visual Studio 17 2022
- **Architecture**: x64
- **Toolchain**: vcpkg
- **Build Type**: Release/Debug

## Testing

To run tests (requires Google Test via vcpkg):
```powershell
# Install Google Test first
vcpkg install gtest

# Reconfigure and build with tests
cmake --preset msvc-release
cmake --build --preset msvc-release

# Run tests
cd build\release
ctest --output-on-failure
```

## Common Issues and Solutions

### "could not load cache" Error
- **Cause**: Running `cmake --build .` from wrong directory
- **Solution**: Always run from configured build directory or use presets

### Qt6 Not Found
- **Cause**: Qt6 path not in CMAKE_PREFIX_PATH
- **Solution**: Update CMakePresets.json or set Qt6_DIR environment variable

### vcpkg Dependencies Missing
- **Cause**: VCPKG_ROOT not set or toolchain file not specified
- **Solution**: Set environment variables and use toolchain file

## Build Outputs (New Organization)

### Release Build
- **Executable**: `build-release\bin\Release\CloudRegistration.exe`
- **Libraries**: `build-release\lib\Release\`
- **Shaders**: `build-release\shaders\`
- **Qt DLLs**: Automatically copied to executable directory

### Debug Build
- **Executable**: `build-debug\bin\Debug\CloudRegistration.exe`
- **Libraries**: `build-debug\lib\Debug\`
- **Shaders**: `build-debug\shaders\`
- **Qt DLLs**: Automatically copied to executable directory

## Git Repository Status

### Tracked Files (Committed to Git)
- Source code (`src/`)
- Tests (`tests/`)
- Shaders (`shaders/`)
- Build scripts (`scripts/`)
- CMake configuration (`CMakeLists.txt`, `CMakePresets.json`)
- Documentation (`docs/`, `README.md`)

### Ignored Files (Not in Git)
- All build directories (`build-*`, `cmake-build-*`, `out/`)
- Compiled binaries (`*.exe`, `*.dll`, `*.obj`)
- CMake generated files (`CMakeCache.txt`, `CMakeFiles/`)
- IDE files (`*.sln`, `*.vcxproj`, `.vs/`)
