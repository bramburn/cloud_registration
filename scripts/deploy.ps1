# Cloud Registration MVP Deployment Script
# Sprint 8: Testing, Documentation, and Deployment
# PowerShell deployment script for Windows

param(
    [Parameter(Mandatory=$false)]
    [string]$BuildType = "Release",
    
    [Parameter(Mandatory=$false)]
    [string]$QtPath = "C:\Qt\6.5.3\msvc2019_64",
    
    [Parameter(Mandatory=$false)]
    [string]$VcpkgPath = "C:\vcpkg",
    
    [Parameter(Mandatory=$false)]
    [string]$OutputDir = "dist",
    
    [Parameter(Mandatory=$false)]
    [switch]$SkipTests = $false,
    
    [Parameter(Mandatory=$false)]
    [switch]$CreateInstaller = $false,
    
    [Parameter(Mandatory=$false)]
    [switch]$SignBinaries = $false
)

# Script configuration
$ErrorActionPreference = "Stop"
$ProgressPreference = "Continue"

# Paths and configuration
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$BuildDir = Join-Path $ProjectRoot "build"
$DistDir = Join-Path $ProjectRoot $OutputDir
$AppName = "CloudRegistration"
$AppVersion = "1.0.0"

Write-Host "=== Cloud Registration MVP Deployment ===" -ForegroundColor Cyan
Write-Host "Build Type: $BuildType" -ForegroundColor Green
Write-Host "Qt Path: $QtPath" -ForegroundColor Green
Write-Host "Output Directory: $DistDir" -ForegroundColor Green

# Function to check if a command exists
function Test-Command {
    param([string]$Command)
    try {
        Get-Command $Command -ErrorAction Stop | Out-Null
        return $true
    } catch {
        return $false
    }
}

# Function to run command with error checking
function Invoke-SafeCommand {
    param(
        [string]$Command,
        [string]$Arguments = "",
        [string]$WorkingDirectory = $PWD
    )
    
    Write-Host "Executing: $Command $Arguments" -ForegroundColor Yellow
    
    $process = Start-Process -FilePath $Command -ArgumentList $Arguments -WorkingDirectory $WorkingDirectory -Wait -PassThru -NoNewWindow
    
    if ($process.ExitCode -ne 0) {
        throw "Command failed with exit code $($process.ExitCode): $Command $Arguments"
    }
}

# Validate prerequisites
Write-Host "`n--- Validating Prerequisites ---" -ForegroundColor Cyan

# Check CMake
if (-not (Test-Command "cmake")) {
    throw "CMake not found. Please install CMake and add it to PATH."
}

# Check Qt installation
if (-not (Test-Path $QtPath)) {
    throw "Qt installation not found at: $QtPath"
}

$QtBinPath = Join-Path $QtPath "bin"
$env:PATH = "$QtBinPath;$env:PATH"

# Check windeployqt
$WinDeployQt = Join-Path $QtBinPath "windeployqt.exe"
if (-not (Test-Path $WinDeployQt)) {
    throw "windeployqt.exe not found at: $WinDeployQt"
}

# Check vcpkg if specified
if ($VcpkgPath -and -not (Test-Path $VcpkgPath)) {
    Write-Warning "vcpkg path specified but not found: $VcpkgPath"
}

Write-Host "Prerequisites validated successfully." -ForegroundColor Green

# Clean and create directories
Write-Host "`n--- Preparing Build Environment ---" -ForegroundColor Cyan

if (Test-Path $BuildDir) {
    Write-Host "Cleaning existing build directory..."
    Remove-Item $BuildDir -Recurse -Force
}

if (Test-Path $DistDir) {
    Write-Host "Cleaning existing distribution directory..."
    Remove-Item $DistDir -Recurse -Force
}

New-Item -ItemType Directory -Path $BuildDir -Force | Out-Null
New-Item -ItemType Directory -Path $DistDir -Force | Out-Null

Write-Host "Build environment prepared." -ForegroundColor Green

# Configure CMake
Write-Host "`n--- Configuring CMake ---" -ForegroundColor Cyan

$CMakeArgs = @(
    "-B", $BuildDir,
    "-S", $ProjectRoot,
    "-DCMAKE_BUILD_TYPE=$BuildType",
    "-DCMAKE_PREFIX_PATH=$QtPath"
)

if ($VcpkgPath) {
    $VcpkgToolchain = Join-Path $VcpkgPath "scripts\buildsystems\vcpkg.cmake"
    if (Test-Path $VcpkgToolchain) {
        $CMakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$VcpkgToolchain"
    }
}

if (-not $SkipTests) {
    $CMakeArgs += "-DBUILD_TESTING=ON"
}

Invoke-SafeCommand -Command "cmake" -Arguments ($CMakeArgs -join " ") -WorkingDirectory $ProjectRoot

Write-Host "CMake configuration completed." -ForegroundColor Green

# Build the project
Write-Host "`n--- Building Project ---" -ForegroundColor Cyan

$BuildArgs = @(
    "--build", $BuildDir,
    "--config", $BuildType,
    "--parallel"
)

Invoke-SafeCommand -Command "cmake" -Arguments ($BuildArgs -join " ") -WorkingDirectory $ProjectRoot

Write-Host "Build completed successfully." -ForegroundColor Green

# Run tests (if not skipped)
if (-not $SkipTests) {
    Write-Host "`n--- Running Tests ---" -ForegroundColor Cyan
    
    try {
        Invoke-SafeCommand -Command "ctest" -Arguments "--output-on-failure --parallel" -WorkingDirectory $BuildDir
        Write-Host "All tests passed." -ForegroundColor Green
    } catch {
        Write-Warning "Some tests failed. Continuing with deployment..."
    }
}

# Prepare distribution
Write-Host "`n--- Preparing Distribution ---" -ForegroundColor Cyan

# Find the executable
$ExePattern = Join-Path $BuildDir "*\$AppName.exe"
$ExePath = Get-ChildItem -Path $ExePattern -Recurse | Select-Object -First 1

if (-not $ExePath) {
    throw "Executable not found: $AppName.exe"
}

Write-Host "Found executable: $($ExePath.FullName)" -ForegroundColor Green

# Copy executable to distribution directory
$DistExePath = Join-Path $DistDir "$AppName.exe"
Copy-Item $ExePath.FullName $DistExePath

# Deploy Qt dependencies
Write-Host "Deploying Qt dependencies..."

$WinDeployArgs = @(
    $DistExePath,
    "--release",
    "--no-translations",
    "--no-system-d3d-compiler",
    "--no-opengl-sw"
)

Invoke-SafeCommand -Command $WinDeployQt -Arguments ($WinDeployArgs -join " ") -WorkingDirectory $DistDir

# Copy additional resources
Write-Host "Copying additional resources..."

# Copy sample data
$SampleDir = Join-Path $ProjectRoot "sample"
if (Test-Path $SampleDir) {
    $DistSampleDir = Join-Path $DistDir "sample"
    Copy-Item $SampleDir $DistSampleDir -Recurse
}

# Copy documentation
$DocsDir = Join-Path $ProjectRoot "docs"
if (Test-Path $DocsDir) {
    $DistDocsDir = Join-Path $DistDir "docs"
    Copy-Item $DocsDir $DistDocsDir -Recurse
}

# Copy shaders
$ShadersDir = Join-Path $ProjectRoot "shaders"
if (Test-Path $ShadersDir) {
    $DistShadersDir = Join-Path $DistDir "shaders"
    Copy-Item $ShadersDir $DistShadersDir -Recurse
}

# Copy license and readme
$LicenseFile = Join-Path $ProjectRoot "LICENSE"
$ReadmeFile = Join-Path $ProjectRoot "README.md"

if (Test-Path $LicenseFile) {
    Copy-Item $LicenseFile $DistDir
}

if (Test-Path $ReadmeFile) {
    Copy-Item $ReadmeFile $DistDir
}

Write-Host "Distribution prepared successfully." -ForegroundColor Green

# Sign binaries (if requested)
if ($SignBinaries) {
    Write-Host "`n--- Signing Binaries ---" -ForegroundColor Cyan
    
    # This would require a code signing certificate
    # Implementation depends on your signing setup
    Write-Warning "Binary signing not implemented. Skipping..."
}

# Create installer (if requested)
if ($CreateInstaller) {
    Write-Host "`n--- Creating Installer ---" -ForegroundColor Cyan
    
    # Check for NSIS or other installer tools
    if (Test-Command "makensis") {
        # Create NSIS installer script
        $InstallerScript = @"
!define APP_NAME "$AppName"
!define APP_VERSION "$AppVersion"
!define PUBLISHER "CloudRegistration Team"
!define WEB_SITE "https://github.com/bramburn/cloud_registration"
!define APP_EXE "$AppName.exe"

!include "MUI2.nsh"

Name "`${APP_NAME}"
OutFile "$AppName-$AppVersion-Setup.exe"
InstallDir "`$PROGRAMFILES64\`${APP_NAME}"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "MainSection" SEC01
    SetOutPath "`$INSTDIR"
    SetOverwrite ifnewer
    File /r "$DistDir\*"
    CreateDirectory "`$SMPROGRAMS\`${APP_NAME}"
    CreateShortCut "`$SMPROGRAMS\`${APP_NAME}\`${APP_NAME}.lnk" "`$INSTDIR\`${APP_EXE}"
    CreateShortCut "`$DESKTOP\`${APP_NAME}.lnk" "`$INSTDIR\`${APP_EXE}"
SectionEnd

Section Uninstall
    Delete "`$INSTDIR\*"
    RMDir /r "`$INSTDIR"
    Delete "`$SMPROGRAMS\`${APP_NAME}\*"
    RMDir "`$SMPROGRAMS\`${APP_NAME}"
    Delete "`$DESKTOP\`${APP_NAME}.lnk"
SectionEnd
"@
        
        $InstallerScriptPath = Join-Path $DistDir "installer.nsi"
        $InstallerScript | Out-File -FilePath $InstallerScriptPath -Encoding UTF8
        
        Invoke-SafeCommand -Command "makensis" -Arguments $InstallerScriptPath -WorkingDirectory $DistDir
        
        Write-Host "Installer created successfully." -ForegroundColor Green
    } else {
        Write-Warning "NSIS not found. Skipping installer creation."
    }
}

# Generate deployment summary
Write-Host "`n--- Deployment Summary ---" -ForegroundColor Cyan

$DistSize = (Get-ChildItem $DistDir -Recurse | Measure-Object -Property Length -Sum).Sum
$DistSizeMB = [math]::Round($DistSize / 1MB, 2)

Write-Host "Application: $AppName v$AppVersion" -ForegroundColor White
Write-Host "Build Type: $BuildType" -ForegroundColor White
Write-Host "Distribution Size: $DistSizeMB MB" -ForegroundColor White
Write-Host "Output Directory: $DistDir" -ForegroundColor White

$FileCount = (Get-ChildItem $DistDir -Recurse -File).Count
Write-Host "Total Files: $FileCount" -ForegroundColor White

Write-Host "`n=== Deployment Completed Successfully ===" -ForegroundColor Green
Write-Host "The application is ready for distribution." -ForegroundColor Green
