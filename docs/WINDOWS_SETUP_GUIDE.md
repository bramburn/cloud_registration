# Windows Setup Guide for CloudRegistration

This guide provides detailed instructions for setting up the CloudRegistration development environment on Windows.

## Prerequisites

### System Requirements
- **Windows 10/11** (64-bit)
- **8GB RAM** minimum, 16GB+ recommended
- **Visual Studio 2022** Community or Professional
- **Git** for version control
- **Administrator privileges** for software installation

## Step 1: Install Visual Studio 2022

1. **Download Visual Studio 2022**
   - Visit [Visual Studio Downloads](https://visualstudio.microsoft.com/downloads/)
   - Choose Community (free) or Professional edition

2. **Installation Configuration**
   - Select "Desktop development with C++" workload
   - Ensure these components are included:
     - MSVC v143 - VS 2022 C++ x64/x86 build tools
     - Windows 10/11 SDK (latest version)
     - CMake tools for Visual Studio
     - Git for Windows (if not already installed)

3. **Verify Installation**
   ```powershell
   # Check compiler version
   cl.exe
   # Should show: Microsoft (R) C/C++ Optimizing Compiler Version 19.xx
   ```

## Step 2: Install Qt6

### Option A: Qt Online Installer (Recommended)

1. **Download Qt Installer**
   - Visit [Qt Downloads](https://www.qt.io/download)
   - Download the Qt Online Installer

2. **Installation Process**
   - Run the installer as Administrator
   - Create a Qt account (free)
   - Choose installation directory: `C:\Qt`
   - Select Qt version: **6.9.0** (recommended) or **6.5.3+**
   - Select components:
     - Qt 6.x.x → MSVC 2022 64-bit
     - Qt 6.x.x → Sources (optional, for debugging)
     - Developer and Designer Tools → CMake
     - Developer and Designer Tools → Ninja

3. **Environment Setup**
   ```powershell
   # Add Qt to PATH (replace with your Qt version)
   $env:PATH += ";C:\Qt\6.9.0\msvc2022_64\bin"
   
   # Set Qt6_DIR for CMake
   $env:Qt6_DIR = "C:\Qt\6.9.0\msvc2022_64\lib\cmake\Qt6"
   ```

### Option B: vcpkg Installation

```powershell
# Install Qt6 via vcpkg (alternative method)
.\vcpkg install qt6-base:x64-windows
.\vcpkg install qt6-opengl:x64-windows
```

## Step 3: Install vcpkg

1. **Clone vcpkg Repository**
   ```powershell
   # Create development directory
   mkdir C:\dev
   cd C:\dev
   
   # Clone vcpkg
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   ```

2. **Bootstrap vcpkg**
   ```powershell
   # Run bootstrap script
   .\bootstrap-vcpkg.bat
   
   # Integrate with Visual Studio (optional)
   .\vcpkg integrate install
   ```

3. **Install Required Dependencies**
   ```powershell
   # Install E57 format library
   .\vcpkg install libe57format:x64-windows
   
   # Install XML parser
   .\vcpkg install xerces-c:x64-windows
   
   # Install testing framework
   .\vcpkg install gtest:x64-windows
   
   # Optional: Install Vulkan SDK
   .\vcpkg install vulkan:x64-windows
   ```

4. **Verify Installation**
   ```powershell
   # List installed packages
   .\vcpkg list
   
   # Should show: libe57format, xerces-c, gtest
   ```

## Step 4: Install CMake

### Option A: Standalone Installation

1. **Download CMake**
   - Visit [CMake Downloads](https://cmake.org/download/)
   - Download Windows x64 Installer

2. **Installation**
   - Run installer as Administrator
   - Choose "Add CMake to system PATH for all users"
   - Install to default location

### Option B: Via Visual Studio

CMake is included with Visual Studio 2022 C++ workload.

3. **Verify Installation**
   ```powershell
   cmake --version
   # Should show: cmake version 3.xx.x
   ```

## Step 5: Configure Environment Variables

### System Environment Variables

1. **Open Environment Variables**
   - Press `Win + R`, type `sysdm.cpl`
   - Click "Environment Variables..."

2. **Add/Modify System Variables**
   ```
   CMAKE_TOOLCHAIN_FILE = C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake
   Qt6_DIR = C:\Qt\6.9.0\msvc2022_64\lib\cmake\Qt6
   QTDIR = C:\Qt\6.9.0\msvc2022_64
   ```

3. **Update PATH Variable**
   Add these paths to your system PATH:
   ```
   C:\Qt\6.9.0\msvc2022_64\bin
   C:\dev\vcpkg
   ```

### PowerShell Profile (Alternative)

Create a PowerShell profile for development:

```powershell
# Create profile directory
mkdir $env:USERPROFILE\Documents\PowerShell -Force

# Create profile file
@"
# CloudRegistration Development Environment
`$env:CMAKE_TOOLCHAIN_FILE = "C:\dev\vcpkg\scripts\buildsystems\vcpkg.cmake"
`$env:Qt6_DIR = "C:\Qt\6.9.0\msvc2022_64\lib\cmake\Qt6"
`$env:QTDIR = "C:\Qt\6.9.0\msvc2022_64"
`$env:PATH += ";C:\Qt\6.9.0\msvc2022_64\bin;C:\dev\vcpkg"

Write-Host "CloudRegistration development environment loaded" -ForegroundColor Green
"@ | Out-File $env:USERPROFILE\Documents\PowerShell\Microsoft.PowerShell_profile.ps1
```

## Step 6: Clone and Build Project

1. **Clone Repository**
   ```powershell
   # Navigate to your projects directory
   cd C:\dev
   
   # Clone the repository
   git clone https://github.com/bramburn/cloud_registration.git
   cd cloud_registration
   ```

2. **Build Project**
   ```powershell
   # Using build scripts (recommended)
   .\scripts\build-clean.ps1 -BuildType Release
   
   # Or using CMake directly
   cmake --preset msvc-release
   cmake --build --preset msvc-release
   ```

3. **Run Application**
   ```powershell
   .\build-release\bin\Release\CloudRegistration.exe
   ```

## Troubleshooting

### Common Issues

1. **Qt6 Not Found**
   ```
   CMake Error: Could not find Qt6
   ```
   **Solution**: Verify Qt6_DIR environment variable points to correct CMake directory

2. **vcpkg Dependencies Missing**
   ```
   CMake Error: Could not find libE57Format
   ```
   **Solution**: Ensure CMAKE_TOOLCHAIN_FILE points to vcpkg CMake file

3. **MSVC Compiler Not Found**
   ```
   CMake Error: Could not find MSVC
   ```
   **Solution**: Install Visual Studio 2022 with C++ workload

4. **Permission Denied Errors**
   **Solution**: Run PowerShell as Administrator for installation steps

### Verification Commands

```powershell
# Verify all tools are installed
cmake --version
cl.exe
qmake.exe -version
.\vcpkg list | findstr e57

# Test build environment
cd cloud_registration
cmake --preset msvc-release --dry-run
```

## IDE Setup (Optional)

### Visual Studio 2022

1. **Open Project**
   - File → Open → CMake...
   - Select `CMakeLists.txt` in project root

2. **Configure CMake Settings**
   - Project → CMake Settings
   - Verify toolchain file path
   - Set build configurations (Debug/Release)

### VS Code

1. **Install Extensions**
   - C/C++ Extension Pack
   - CMake Tools
   - Qt tools (optional)

2. **Configure Workspace**
   - Open project folder in VS Code
   - Configure CMake kit (MSVC 2022)
   - Set CMake toolchain file in settings

## Next Steps

After completing this setup:

1. **Run Tests**: `.\scripts\run-tests.ps1`
2. **Build Documentation**: Follow development guide
3. **Start Development**: See contributing guidelines in main README

## Support

If you encounter issues:
- Check the troubleshooting section above
- Review GitHub Issues for similar problems
- Create a new issue with your system configuration
