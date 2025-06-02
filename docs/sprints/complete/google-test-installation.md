# Google Test Installation Guide

## Overview

This guide provides step-by-step instructions for installing Google Test on different platforms to enable the Phase 1 testing infrastructure.

## Windows Installation

### Option 1: Using vcpkg (Recommended)

#### Step 1: Install vcpkg
```powershell
# Clone vcpkg repository
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# Add vcpkg to PATH (optional but recommended)
$env:PATH += ";C:\vcpkg"
```

#### Step 2: Install Google Test
```powershell
# Install Google Test for x64 Windows
.\vcpkg install gtest:x64-windows

# Integrate with Visual Studio (optional)
.\vcpkg integrate install
```

#### Step 3: Configure Project
```powershell
# Navigate to project directory
cd C:\dev\cloud_registration

# Configure with vcpkg toolchain
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake"

# Verify Google Test is found
# Look for: "Google Test found - building unit tests"
```

### Option 2: Manual Installation

#### Step 1: Download and Build Google Test
```powershell
# Create a directory for Google Test
mkdir C:\googletest
cd C:\googletest

# Clone Google Test repository
git clone https://github.com/google/googletest.git
cd googletest

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_INSTALL_PREFIX="C:\googletest\install"

# Build and install
cmake --build . --config Release
cmake --install . --config Release
```

#### Step 2: Configure Project
```powershell
# Navigate to project directory
cd C:\dev\cloud_registration

# Configure with Google Test path
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:\googletest\install"
```

### Option 3: Using Conan

#### Step 1: Install Conan
```powershell
pip install conan
```

#### Step 2: Create conanfile.txt
```ini
[requires]
gtest/1.14.0

[generators]
CMakeDeps
CMakeToolchain

[options]
gtest:shared=False
```

#### Step 3: Install Dependencies
```powershell
# Create build directory
mkdir build && cd build

# Install dependencies
conan install .. --build=missing

# Configure with Conan
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
```

## Linux Installation

### Ubuntu/Debian
```bash
# Install Google Test development package
sudo apt-get update
sudo apt-get install libgtest-dev

# Build and install Google Test
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp lib/*.a /usr/lib

# Configure project
cd /path/to/cloud_registration
cmake -B build -S .
```

### CentOS/RHEL/Fedora
```bash
# Install development tools
sudo yum groupinstall "Development Tools"
sudo yum install cmake

# Clone and build Google Test
git clone https://github.com/google/googletest.git
cd googletest
mkdir build && cd build
cmake ..
make
sudo make install

# Configure project
cd /path/to/cloud_registration
cmake -B build -S .
```

## macOS Installation

### Using Homebrew
```bash
# Install Google Test
brew install googletest

# Configure project
cd /path/to/cloud_registration
cmake -B build -S .
```

### Manual Installation
```bash
# Clone and build Google Test
git clone https://github.com/google/googletest.git
cd googletest
mkdir build && cd build
cmake ..
make
sudo make install

# Configure project
cd /path/to/cloud_registration
cmake -B build -S .
```

## Verification

### Check Installation
After installation, verify Google Test is properly configured:

```powershell
# Windows
cd C:\dev\cloud_registration
cmake -B build -S . [additional-options]

# Look for this message in the output:
# "Google Test found - building unit tests"

# Instead of:
# "Google Test not found - unit tests will not be built"
```

### Run Validation Script
```powershell
# Windows
.\scripts\validate-phase1-clean.ps1

# Linux/macOS
./scripts/validate-phase1-clean.ps1
```

You should see:
```
Checking Google Test...
  [PASS] Google Test found
```

## Troubleshooting

### Common Issues

#### 1. vcpkg not found
**Problem**: `vcpkg : The term 'vcpkg' is not recognized`

**Solution**:
```powershell
# Add vcpkg to PATH
$env:PATH += ";C:\vcpkg"

# Or use full path
C:\vcpkg\vcpkg install gtest:x64-windows
```

#### 2. Google Test not found after installation
**Problem**: CMake still reports "Google Test not found"

**Solutions**:
```powershell
# Option 1: Use toolchain file
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake"

# Option 2: Set prefix path
cmake -B build -S . -DCMAKE_PREFIX_PATH="C:\path\to\gtest\install"

# Option 3: Set GTest_DIR
cmake -B build -S . -DGTest_DIR="C:\path\to\gtest\lib\cmake\GTest"
```

#### 3. Build errors
**Problem**: Test targets fail to build

**Solutions**:
1. Ensure Qt6 is properly configured
2. Clean and reconfigure build directory:
   ```powershell
   Remove-Item -Recurse -Force build
   cmake -B build -S . [options]
   ```
3. Check compiler compatibility

#### 4. Test execution fails
**Problem**: Tests fail to run

**Solutions**:
1. Ensure all dependencies are in PATH
2. Check test executable permissions
3. Verify Qt application can initialize

## Testing the Installation

### Run Tests
Once Google Test is installed and configured:

```powershell
# Build and run all tests
.\scripts\run-tests.ps1

# Run with verbose output
.\scripts\run-tests.ps1 -Verbose

# Run specific tests
.\scripts\run-tests.ps1 -Filter "*E57*"

# Generate coverage report (if lcov available)
.\scripts\run-tests.ps1 -Coverage
```

### Expected Output
```
=== Cloud Registration Test Runner ===
Build Directory: build
Configuration: Debug

1. Building test executables...
Building E57ParserTests...
Building LasParserTests...
Building VoxelGridFilterTests...
Building Sprint1FunctionalityTests...
✓ All test executables built successfully

2. Running unit tests...
Executing: ctest --output-on-failure --config Debug
Test project C:/dev/cloud_registration/build
    Start 1: E57ParserTests
1/4 Test #1: E57ParserTests ...................   Passed    2.34 sec
    Start 2: LasParserTests
2/4 Test #2: LasParserTests ...................   Passed    1.87 sec
    Start 3: VoxelGridFilterTests
3/4 Test #3: VoxelGridFilterTests .............   Passed    0.92 sec
    Start 4: Sprint1FunctionalityTests
4/4 Test #4: Sprint1FunctionalityTests .......   Passed    1.23 sec

100% tests passed, 0 tests failed out of 4

✓ All tests passed!

3. Test Summary:
Total Tests: 4
Passed: 4
Failed: 0

=== Test Execution Complete ===
```

## Next Steps

Once Google Test is successfully installed:

1. **Validate Installation**: Run `.\scripts\validate-phase1-clean.ps1`
2. **Run Tests**: Execute `.\scripts\run-tests.ps1`
3. **Check Coverage**: Use `.\scripts\run-tests.ps1 -Coverage` (if lcov available)
4. **Proceed to Phase 2**: Begin implementing advanced Google Test features

## Support

If you encounter issues:

1. Check the troubleshooting section above
2. Verify your CMake version (3.16+ required)
3. Ensure Qt6 is properly installed and configured
4. Review the error messages carefully
5. Consult the Google Test documentation

For project-specific issues, refer to:
- `docs/testing-best-practices.md`
- `docs/phase1-setup-guide.md`
- `docs/phase1-implementation-summary.md`
