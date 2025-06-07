# Linux Setup Guide for CloudRegistration

This guide provides detailed instructions for setting up the CloudRegistration development environment on Linux systems.

## Prerequisites

### System Requirements
- **Linux Distribution**: Ubuntu 20.04+, Fedora 35+, Arch Linux, or compatible
- **Architecture**: x86_64 (64-bit)
- **Memory**: 8GB RAM minimum, 16GB+ recommended for large datasets
- **Storage**: 5GB free space for development environment
- **Graphics**: OpenGL 3.3+ compatible graphics card

### Supported Distributions
- **Ubuntu/Debian**: 20.04 LTS, 22.04 LTS, 24.04 LTS
- **Fedora/RHEL**: 35+, CentOS Stream 9+
- **Arch Linux**: Rolling release
- **openSUSE**: Leap 15.4+, Tumbleweed

## Step 1: Install System Dependencies

### Ubuntu/Debian
```bash
# Update package lists
sudo apt-get update

# Install essential build tools
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git \
    curl \
    wget

# Install Qt6 development packages
sudo apt-get install -y \
    qt6-base-dev \
    qt6-base-dev-tools \
    libqt6opengl6-dev \
    libqt6openglwidgets6 \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    libgl1-mesa-dev \
    libglu1-mesa-dev

# Install testing frameworks
sudo apt-get install -y \
    libgtest-dev \
    libgmock-dev

# Install additional dependencies
sudo apt-get install -y \
    libxml2-dev \
    libboost-dev \
    libboost-system-dev \
    libboost-filesystem-dev \
    lcov
```

### Fedora/RHEL/CentOS
```bash
# Install development tools
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y cmake ninja-build pkg-config git

# Install Qt6 packages
sudo dnf install -y \
    qt6-qtbase-devel \
    qt6-qttools-devel \
    qt6-qtbase-gui \
    mesa-libGL-devel \
    mesa-libGLU-devel

# Install testing frameworks
sudo dnf install -y gtest-devel gmock-devel

# Install additional dependencies
sudo dnf install -y \
    libxml2-devel \
    boost-devel \
    lcov
```

### Arch Linux
```bash
# Install base development packages
sudo pacman -S base-devel cmake ninja git

# Install Qt6 packages
sudo pacman -S \
    qt6-base \
    qt6-tools \
    qt6-declarative \
    mesa \
    glu

# Install testing frameworks
sudo pacman -S gtest

# Install additional dependencies
sudo pacman -S \
    libxml2 \
    boost \
    lcov
```

## Step 2: Install Dependencies via Package Manager

### Option A: Using System Package Manager (Recommended)

Most dependencies are available through system package managers. The setup script will handle this automatically.

### Option B: Using vcpkg (Advanced)

If you need specific versions or the system packages are outdated:

```bash
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# Install dependencies
./vcpkg install libe57format:x64-linux
./vcpkg install xerces-c:x64-linux
./vcpkg install gtest:x64-linux

# Set environment variable
export CMAKE_TOOLCHAIN_FILE=$(pwd)/scripts/buildsystems/vcpkg.cmake
```

## Step 3: Build the Project

### Option A: Using the Setup Script (Recommended)
```bash
# Clone the repository
git clone https://github.com/bramburn/cloud_registration.git
cd cloud_registration

# Run the automated setup script
./setup.sh

# Or run specific steps
./setup.sh --install-deps    # Install dependencies only
./setup.sh --configure       # Configure project only
./setup.sh --build          # Build project only
./setup.sh --test           # Run tests only
```

### Option B: Using the Linux Build Script
```bash
# Debug build with GCC
./scripts/build-linux.sh

# Release build with Clang
./scripts/build-linux.sh -t Release -c clang

# Clean build with coverage
./scripts/build-linux.sh --clean --coverage

# Show all options
./scripts/build-linux.sh --help
```

### Option C: Manual CMake Build
```bash
# Create build directory
mkdir build-linux-debug
cd build-linux-debug

# Configure with CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++ \
    -G Ninja

# Build
ninja

# Run tests
ctest --output-on-failure
```

### Option D: Using CMake Presets
```bash
# List available presets
cmake --list-presets

# Configure using preset
cmake --preset linux-gcc-debug

# Build using preset
cmake --build --preset linux-gcc-debug

# Test using preset
ctest --preset linux-gcc-debug-test
```

## Step 4: Environment Configuration

### Qt6 Environment Variables
```bash
# Add to ~/.bashrc or ~/.profile
export QT_SELECT=qt6
export CMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6:$CMAKE_PREFIX_PATH

# For custom Qt6 installation
export Qt6_DIR=/opt/Qt/6.9.0/gcc_64/lib/cmake/Qt6
export PATH=/opt/Qt/6.9.0/gcc_64/bin:$PATH
```

### Development Environment
```bash
# Enable core dumps for debugging
ulimit -c unlimited

# Set optimal number of build jobs
export CMAKE_BUILD_PARALLEL_LEVEL=$(nproc)

# Enable colored output
export CMAKE_COLOR_MAKEFILE=ON
export NINJA_STATUS="[%f/%t] "
```

## Step 5: Verify Installation

### Check Build Output
```bash
# Verify executable exists
ls -la build*/bin/CloudRegistration || ls -la build*/CloudRegistration

# Check dependencies
ldd build*/bin/CloudRegistration || ldd build*/CloudRegistration

# Test basic functionality
./build*/bin/CloudRegistration --help || ./build*/CloudRegistration --help
```

### Run Tests
```bash
cd build*
ctest --output-on-failure --verbose

# Or run individual test suites
./E57ParserTests
./LasParserTests
./VoxelGridFilterTests
```

## Troubleshooting

### Common Issues

1. **Qt6 Not Found**
   ```
   CMake Error: Could not find Qt6
   ```
   **Solution**: Install qt6-base-dev or set Qt6_DIR environment variable

2. **Missing OpenGL Headers**
   ```
   fatal error: GL/gl.h: No such file or directory
   ```
   **Solution**: Install mesa-libGL-devel or libgl1-mesa-dev

3. **E57Format Library Not Found**
   ```
   CMake Error: Could not find E57Format
   ```
   **Solution**: Install via system package manager or build from source

4. **Ninja Not Found**
   ```
   CMake Error: CMAKE_MAKE_PROGRAM is set to "ninja" but ninja was not found
   ```
   **Solution**: Install ninja-build package

5. **Compiler Version Too Old**
   ```
   error: 'std::filesystem' is not a member of 'std'
   ```
   **Solution**: Update to GCC 8+ or Clang 7+

### Debug Build Issues
```bash
# Enable verbose CMake output
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON

# Enable verbose Ninja output
ninja -v

# Check CMake configuration
cmake .. --debug-output

# Verify compiler versions
gcc --version
g++ --version
cmake --version
```

### Performance Optimization
```bash
# Use all CPU cores for building
ninja -j$(nproc)

# Enable link-time optimization for Release builds
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON

# Use ccache for faster rebuilds
sudo apt-get install ccache  # Ubuntu/Debian
export CMAKE_CXX_COMPILER_LAUNCHER=ccache
```

## Development Workflow

### Daily Development
```bash
# Quick build after changes
cd build*
ninja

# Run specific tests
ctest -R "E57Parser"

# Debug with GDB
gdb ./bin/CloudRegistration
```

### Code Coverage
```bash
# Build with coverage
./scripts/build-linux.sh --coverage

# Generate coverage report
cd build*
ninja coverage

# View coverage report
xdg-open coverage_html/index.html
```

### Static Analysis
```bash
# Install clang-tidy
sudo apt-get install clang-tidy

# Run static analysis
clang-tidy src/*.cpp -- -I src -I /usr/include/qt6
```

## IDE Integration

### VS Code
1. Install C/C++ extension
2. Install CMake Tools extension
3. Open project folder
4. Select CMake preset: `linux-gcc-debug`
5. Build and debug using VS Code interface

### CLion
1. Open project folder
2. CLion will automatically detect CMakeLists.txt
3. Configure CMake settings in Settings → Build → CMake
4. Select appropriate toolchain and build type

### Qt Creator
1. Open CMakeLists.txt as project
2. Configure kit with appropriate Qt6 version
3. Set build directory and configuration
4. Build and run from Qt Creator

## Next Steps

After successful setup:
1. **Run the application**: `./build*/bin/CloudRegistration`
2. **Load sample data**: Use files in the `sample/` directory
3. **Run tests**: `cd build* && ctest --output-on-failure`
4. **Read documentation**: Check `docs/` directory for development guides
5. **Contribute**: Follow the development workflow in `docs/DEVELOPMENT_GUIDE.md`

For additional help, see:
- [Development Guide](DEVELOPMENT_GUIDE.md)
- [Deployment Guide](DEPLOYMENT_GUIDE.md)
- [Windows Setup Guide](WINDOWS_SETUP_GUIDE.md)
