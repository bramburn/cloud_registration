# Phase 1 Setup Guide: Google Test Implementation

## Overview

This guide provides step-by-step instructions to complete the Phase 1 implementation of Google Test in the Cloud Registration project. All the infrastructure has been implemented - you just need to install Google Test to activate it.

## Current Status

✅ **CMake Integration**: Enhanced with CTest, coverage support, and custom targets  
✅ **Test Files**: All test files exist and are comprehensive  
✅ **Documentation**: Best practices and implementation guides created  
✅ **Scripts**: Cross-platform test execution scripts ready  
⚠️ **Google Test**: Needs to be installed to activate testing

## Quick Start

### Step 1: Install Google Test

#### Option A: Using vcpkg (Recommended for Windows)
```powershell
# Install vcpkg if not already installed
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install Google Test
.\vcpkg install gtest:x64-windows

# Integrate with Visual Studio
.\vcpkg integrate install
```

#### Option B: Using Conan
```bash
# Install Conan if not already installed
pip install conan

# Create conanfile.txt in project root
echo "[requires]" > conanfile.txt
echo "gtest/1.14.0" >> conanfile.txt
echo "[generators]" >> conanfile.txt
echo "CMakeDeps" >> conanfile.txt
echo "CMakeToolchain" >> conanfile.txt

# Install dependencies
conan install . --build=missing
```

#### Option C: Manual Installation
```bash
# Clone and build Google Test
git clone https://github.com/google/googletest.git
cd googletest
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make && sudo make install
```

### Step 2: Configure Project with Google Test

```powershell
# Clean previous configuration
Remove-Item -Recurse -Force build
mkdir build

# Configure with Google Test
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake"

# Or if using Conan
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
```

### Step 3: Build and Run Tests

```powershell
# Build all test targets
cmake --build . --config Debug

# Run tests using our script
..\scripts\run-tests.ps1

# Or run tests directly with CTest
ctest --output-on-failure
```

## Detailed Implementation Verification

### 1. Verify CMake Configuration

After installing Google Test, reconfigure the project:

```powershell
cmake -B build -S .
```

You should see:
```
-- Google Test found - building unit tests
```

Instead of:
```
-- Google Test not found - unit tests will not be built
```

### 2. Verify Test Targets

Check that test targets are created:

```powershell
cmake --build build --target help | Select-String "Test"
```

Expected output:
```
... E57ParserTests
... LasParserTests  
... VoxelGridFilterTests
... Sprint1FunctionalityTests
... AllTests
... run_tests
```

### 3. Run Individual Tests

```powershell
# Build and run E57 parser tests
cmake --build build --target E57ParserTests
.\build\Debug\E57ParserTests.exe

# Build and run LAS parser tests  
cmake --build build --target LasParserTests
.\build\Debug\LasParserTests.exe

# Build and run VoxelGrid filter tests
cmake --build build --target VoxelGridFilterTests
.\build\Debug\VoxelGridFilterTests.exe
```

### 4. Run All Tests with CTest

```powershell
cd build
ctest --output-on-failure --verbose
```

### 5. Generate Code Coverage (Optional)

If you have GCOV/LCOV installed:

```powershell
# Configure with coverage
cmake .. -DENABLE_COVERAGE=ON

# Build and run tests with coverage
cmake --build . --target coverage
```

## Expected Test Results

### Test Summary
When all tests pass, you should see:

```
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

Total Test time (real) =   6.36 sec
```

### Individual Test Details

#### E57ParserTests (6 test cases):
- ValidE57FileDetection
- InvalidFileDetection  
- NonExistentFileHandling
- MockDataGeneration
- ValidE57FileParsing
- ErrorHandling

#### LasParserTests (6 test cases):
- ValidLasFileDetection
- InvalidFileDetection
- NonExistentFileHandling
- Format0Parsing
- Format1Parsing
- ErrorHandling
- InvalidHeaderHandling

#### VoxelGridFilterTests (8 test cases):
- EmptyInput
- SinglePoint
- PointsInSameVoxel
- PointsInDifferentVoxels
- MinPointsPerVoxelFiltering
- InvalidLeafSize
- InvalidInputSize
- LargePointCloudPerformance

#### Sprint1FunctionalityTests (8 test cases):
- LoadingSettingsStructure
- LoadingMethodEnum
- LoadingSettingsDialogCreation
- SettingsPersistence
- LasHeaderMetadataStructure
- LasParserSignalEmission
- LoadingMethodEnum
- SettingsWorkflow

## Using the Test Scripts

### Windows PowerShell Script

```powershell
# Basic usage
.\scripts\run-tests.ps1

# With coverage
.\scripts\run-tests.ps1 -Coverage

# Verbose output
.\scripts\run-tests.ps1 -Verbose

# Filter specific tests
.\scripts\run-tests.ps1 -Filter "*E57*"

# Help
.\scripts\run-tests.ps1 -Help
```

### Linux/macOS Bash Script

```bash
# Make executable (Linux/macOS only)
chmod +x scripts/run-tests.sh

# Basic usage
./scripts/run-tests.sh

# With coverage
./scripts/run-tests.sh --coverage

# Verbose output
./scripts/run-tests.sh --verbose

# Filter specific tests
./scripts/run-tests.sh --filter "*E57*"

# Help
./scripts/run-tests.sh --help
```

## Troubleshooting

### Google Test Not Found

**Problem**: CMake says "Google Test not found"

**Solutions**:
1. Ensure Google Test is properly installed
2. Set `CMAKE_PREFIX_PATH` to Google Test installation
3. Use vcpkg toolchain file
4. Check `GTest_DIR` environment variable

### Build Errors

**Problem**: Test targets fail to build

**Solutions**:
1. Ensure Qt6 is properly configured
2. Check that all source files exist
3. Verify include paths are correct
4. Clean and reconfigure build directory

### Test Failures

**Problem**: Tests fail during execution

**Solutions**:
1. Check test output for specific failures
2. Verify temporary file permissions
3. Ensure Qt application can initialize
4. Check file path separators (Windows vs Unix)

## Integration with Development Workflow

### Pre-commit Testing

Add to your development workflow:

```powershell
# Before committing changes
.\scripts\run-tests.ps1
if ($LASTEXITCODE -eq 0) {
    git commit -m "Your commit message"
} else {
    Write-Host "Tests failed - fix before committing" -ForegroundColor Red
}
```

### Continuous Integration

For CI/CD pipelines, use:

```yaml
# Example GitHub Actions step
- name: Run Tests
  run: |
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
    cmake --build build --config Release
    cd build && ctest --output-on-failure
```

## Next Steps

Once Phase 1 is complete:

1. **Verify Coverage**: Aim for >70% code coverage
2. **Performance Baseline**: Document test execution times
3. **CI Integration**: Set up automated testing
4. **Phase 2 Planning**: Prepare for advanced Google Test features

## Success Criteria

Phase 1 is complete when:

- ✅ All 28 test cases pass
- ✅ CTest integration works
- ✅ Test scripts execute successfully  
- ✅ Coverage reporting functions (if enabled)
- ✅ Documentation is accessible and clear

## Support

For issues or questions:

1. Check the troubleshooting section above
2. Review `docs/testing-best-practices.md`
3. Examine existing test files for patterns
4. Consult Google Test documentation
