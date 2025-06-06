# CloudRegistration Deployment Guide

This guide covers deployment strategies for distributing the CloudRegistration application to end users.

## Overview

CloudRegistration is a Qt6-based desktop application that requires specific runtime dependencies. This guide covers:

- Standalone deployment for individual users
- Enterprise deployment strategies
- Installer creation
- Troubleshooting deployment issues

## Prerequisites

Before deployment, ensure you have:
- Successfully built CloudRegistration in Release mode
- Access to Qt6 installation directory
- vcpkg dependencies installed
- Windows deployment tools

## Standalone Deployment

### Method 1: windeployqt (Recommended)

The Qt deployment tool automatically copies required Qt libraries and dependencies.

1. **Prepare Build Directory**
   ```powershell
   # Navigate to release build
   cd build-release\bin\Release
   
   # Verify executable exists
   dir CloudRegistration.exe
   ```

2. **Run windeployqt**
   ```powershell
   # Basic deployment
   windeployqt.exe CloudRegistration.exe --release --no-translations
   
   # Advanced deployment with debug info
   windeployqt.exe CloudRegistration.exe --release --debug-info --no-translations --no-system-d3d-compiler
   
   # Include QML if used (currently not needed)
   # windeployqt.exe CloudRegistration.exe --qmldir ..\..\..\..\src
   ```

3. **Copy vcpkg Dependencies**
   ```powershell
   # Copy E57 and Xerces libraries
   copy "C:\dev\vcpkg\installed\x64-windows\bin\E57Format.dll" .
   copy "C:\dev\vcpkg\installed\x64-windows\bin\xerces-c_3_2.dll" .
   
   # Copy any other vcpkg DLLs
   copy "C:\dev\vcpkg\installed\x64-windows\bin\*.dll" .
   ```

4. **Copy Application Resources**
   ```powershell
   # Copy shaders directory
   xcopy /E /I "..\..\..\shaders" "shaders"
   
   # Copy sample data (optional)
   xcopy /E /I "..\..\..\sample" "sample"
   
   # Copy icons
   xcopy /E /I "..\..\..\icons" "icons"
   ```

5. **Test Deployment**
   ```powershell
   # Test on clean system or VM
   .\CloudRegistration.exe
   ```

### Method 2: Manual Deployment

For more control over the deployment process:

1. **Create Deployment Directory**
   ```powershell
   mkdir CloudRegistration-Deployment
   cd CloudRegistration-Deployment
   ```

2. **Copy Application Files**
   ```powershell
   # Main executable
   copy "..\build-release\bin\Release\CloudRegistration.exe" .
   
   # Application resources
   xcopy /E /I "..\shaders" "shaders"
   xcopy /E /I "..\icons" "icons"
   xcopy /E /I "..\sample" "sample"
   ```

3. **Copy Qt6 Dependencies**
   ```powershell
   # Core Qt libraries
   copy "C:\Qt\6.9.0\msvc2022_64\bin\Qt6Core.dll" .
   copy "C:\Qt\6.9.0\msvc2022_64\bin\Qt6Gui.dll" .
   copy "C:\Qt\6.9.0\msvc2022_64\bin\Qt6Widgets.dll" .
   copy "C:\Qt\6.9.0\msvc2022_64\bin\Qt6OpenGL.dll" .
   copy "C:\Qt\6.9.0\msvc2022_64\bin\Qt6OpenGLWidgets.dll" .
   copy "C:\Qt\6.9.0\msvc2022_64\bin\Qt6Xml.dll" .
   copy "C:\Qt\6.9.0\msvc2022_64\bin\Qt6Sql.dll" .
   copy "C:\Qt\6.9.0\msvc2022_64\bin\Qt6Concurrent.dll" .
   ```

4. **Copy Platform Plugins**
   ```powershell
   mkdir platforms
   copy "C:\Qt\6.9.0\msvc2022_64\plugins\platforms\qwindows.dll" "platforms\"
   ```

5. **Copy Visual C++ Runtime**
   ```powershell
   # These are usually already installed on target systems
   # Include if targeting older systems
   copy "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.xx.xxxxx\x64\Microsoft.VC143.CRT\*.dll" .
   ```

## Enterprise Deployment

### Group Policy Deployment

1. **Create MSI Package** (see Installer Creation section)
2. **Deploy via Group Policy**
   - Computer Configuration → Software Settings → Software Installation
   - Add CloudRegistration.msi
   - Configure deployment options

### SCCM Deployment

1. **Package Application**
   - Create application package with all dependencies
   - Include detection rules for existing installations
   - Configure deployment requirements

2. **Distribution Points**
   - Copy package to distribution points
   - Configure deployment schedules
   - Monitor deployment status

## Installer Creation

### NSIS Installer

1. **Install NSIS**
   - Download from [NSIS Website](https://nsis.sourceforge.io/)
   - Install with modern UI plugin

2. **Create Installer Script**
   ```nsis
   ; CloudRegistration Installer Script
   !include "MUI2.nsh"
   
   Name "CloudRegistration"
   OutFile "CloudRegistration-Setup.exe"
   InstallDir "$PROGRAMFILES64\CloudRegistration"
   
   !insertmacro MUI_PAGE_WELCOME
   !insertmacro MUI_PAGE_LICENSE "LICENSE"
   !insertmacro MUI_PAGE_DIRECTORY
   !insertmacro MUI_PAGE_INSTFILES
   !insertmacro MUI_PAGE_FINISH
   
   Section "Main Application"
     SetOutPath "$INSTDIR"
     File "CloudRegistration.exe"
     File "*.dll"
     
     SetOutPath "$INSTDIR\platforms"
     File "platforms\*.dll"
     
     SetOutPath "$INSTDIR\shaders"
     File /r "shaders\*"
     
     CreateShortcut "$DESKTOP\CloudRegistration.lnk" "$INSTDIR\CloudRegistration.exe"
     CreateShortcut "$SMPROGRAMS\CloudRegistration.lnk" "$INSTDIR\CloudRegistration.exe"
   SectionEnd
   ```

3. **Build Installer**
   ```powershell
   makensis.exe CloudRegistration.nsi
   ```

### WiX Toolset

1. **Install WiX Toolset**
   - Download from [WiX Toolset](https://wixtoolset.org/)
   - Install WiX v3 or v4

2. **Create Product Definition**
   ```xml
   <?xml version="1.0" encoding="UTF-8"?>
   <Wix xmlns="http://schemas.microsoft.com/wix/2006/wi">
     <Product Id="*" Name="CloudRegistration" Language="1033" Version="1.0.0.0" 
              Manufacturer="Your Company" UpgradeCode="PUT-GUID-HERE">
       <Package InstallerVersion="200" Compressed="yes" InstallScope="perMachine" />
       
       <MediaTemplate EmbedCab="yes" />
       
       <Feature Id="ProductFeature" Title="CloudRegistration" Level="1">
         <ComponentGroupRef Id="ProductComponents" />
       </Feature>
     </Product>
     
     <Fragment>
       <Directory Id="TARGETDIR" Name="SourceDir">
         <Directory Id="ProgramFiles64Folder">
           <Directory Id="INSTALLFOLDER" Name="CloudRegistration" />
         </Directory>
       </Directory>
     </Fragment>
     
     <Fragment>
       <ComponentGroup Id="ProductComponents" Directory="INSTALLFOLDER">
         <Component Id="MainExecutable">
           <File Source="CloudRegistration.exe" />
         </Component>
         <!-- Add more components for DLLs, resources, etc. -->
       </ComponentGroup>
     </Fragment>
   </Wix>
   ```

3. **Build MSI Package**
   ```powershell
   candle.exe Product.wxs
   light.exe Product.wixobj -o CloudRegistration.msi
   ```

## Deployment Verification

### Automated Testing

Create a deployment test script:

```powershell
# deployment-test.ps1
param(
    [string]$DeploymentPath = ".\CloudRegistration-Deployment"
)

Write-Host "Testing CloudRegistration deployment..." -ForegroundColor Yellow

# Test 1: Executable exists
if (Test-Path "$DeploymentPath\CloudRegistration.exe") {
    Write-Host "✓ Main executable found" -ForegroundColor Green
} else {
    Write-Host "✗ Main executable missing" -ForegroundColor Red
    exit 1
}

# Test 2: Required DLLs
$requiredDlls = @(
    "Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll", 
    "Qt6OpenGL.dll", "Qt6OpenGLWidgets.dll",
    "E57Format.dll", "xerces-c_3_2.dll"
)

foreach ($dll in $requiredDlls) {
    if (Test-Path "$DeploymentPath\$dll") {
        Write-Host "✓ $dll found" -ForegroundColor Green
    } else {
        Write-Host "✗ $dll missing" -ForegroundColor Red
    }
}

# Test 3: Resources
$requiredDirs = @("platforms", "shaders", "icons")
foreach ($dir in $requiredDirs) {
    if (Test-Path "$DeploymentPath\$dir") {
        Write-Host "✓ $dir directory found" -ForegroundColor Green
    } else {
        Write-Host "✗ $dir directory missing" -ForegroundColor Red
    }
}

# Test 4: Application launch (optional)
Write-Host "Testing application launch..." -ForegroundColor Yellow
$process = Start-Process "$DeploymentPath\CloudRegistration.exe" -PassThru -WindowStyle Hidden
Start-Sleep 5
if (!$process.HasExited) {
    Write-Host "✓ Application launched successfully" -ForegroundColor Green
    $process.Kill()
} else {
    Write-Host "✗ Application failed to launch" -ForegroundColor Red
}

Write-Host "Deployment test completed." -ForegroundColor Yellow
```

### Manual Testing Checklist

Test on a clean Windows system:

1. **Installation**
   - ✅ Installer runs without errors
   - ✅ Files copied to correct locations
   - ✅ Shortcuts created properly
   - ✅ Start menu entries added

2. **Application Launch**
   - ✅ Application starts without DLL errors
   - ✅ Main window displays correctly
   - ✅ Menus and toolbars functional

3. **Core Functionality**
   - ✅ Create new project
   - ✅ Import sample E57/LAS files
   - ✅ 3D visualization works
   - ✅ File operations complete successfully

4. **System Integration**
   - ✅ File associations work (if configured)
   - ✅ Application appears in Add/Remove Programs
   - ✅ Uninstaller works correctly

## Troubleshooting

### Common Deployment Issues

1. **Missing DLL Errors**
   ```
   The program can't start because Qt6Core.dll is missing
   ```
   **Solution**: Run windeployqt or manually copy missing DLLs

2. **Platform Plugin Errors**
   ```
   This application failed to start because no Qt platform plugin could be initialized
   ```
   **Solution**: Ensure platforms/qwindows.dll is present

3. **E57 Library Errors**
   ```
   The procedure entry point could not be located in E57Format.dll
   ```
   **Solution**: Verify correct version of E57Format.dll from vcpkg

4. **OpenGL Context Errors**
   ```
   Failed to create OpenGL context
   ```
   **Solution**: Install latest graphics drivers on target system

### Dependency Analysis Tools

Use these tools to identify missing dependencies:

```powershell
# Dependency Walker (legacy but useful)
depends.exe CloudRegistration.exe

# Process Monitor (monitor file access)
procmon.exe

# PowerShell dependency check
Get-ChildItem *.dll | ForEach-Object { 
    Write-Host $_.Name; 
    dumpbin /dependents $_.FullName 
}
```

## Best Practices

1. **Version Management**
   - Include version information in executable
   - Maintain deployment manifests
   - Track dependency versions

2. **Testing**
   - Test on multiple Windows versions
   - Use virtual machines for clean testing
   - Automate deployment verification

3. **Documentation**
   - Provide user installation guide
   - Document system requirements
   - Include troubleshooting steps

4. **Updates**
   - Plan for application updates
   - Consider auto-update mechanisms
   - Maintain backward compatibility

## Support

For deployment issues:
- Check Windows Event Viewer for error details
- Use Process Monitor to track file access issues
- Review Qt deployment documentation
- Contact support with system configuration details
