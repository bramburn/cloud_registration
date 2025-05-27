#!/usr/bin/env pwsh
# Build Management Script for Qt6/C++ Cloud Registration Project
# This script provides clean build management with proper folder organization

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("Debug", "Release", "Both", "Clean")]
    [string]$BuildType = "Release",

    [Parameter(Mandatory=$false)]
    [switch]$CleanFirst = $false,

    [Parameter(Mandatory=$false)]
    [switch]$RunTests = $false,

    [Parameter(Mandatory=$false)]
    [switch]$VerboseOutput = $false
)

# Set error handling
$ErrorActionPreference = "Stop"

# Get script directory and project root
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir

Write-Host "=== Qt6/C++ Cloud Registration Build Script ===" -ForegroundColor Cyan
Write-Host "Project Root: $ProjectRoot" -ForegroundColor Gray
Write-Host "Build Type: $BuildType" -ForegroundColor Gray

# Change to project root
Push-Location $ProjectRoot

try {
    # Function to clean build directories
    function Clean-BuildDirectories {
        Write-Host "`nCleaning build directories..." -ForegroundColor Yellow

        $buildDirs = @(
            "build",
            "build-debug",
            "build-release",
            "cmake-build-debug",
            "cmake-build-debug-visual-studio",
            "cmake-build-release",
            "out"
        )

        foreach ($dir in $buildDirs) {
            if (Test-Path $dir) {
                Write-Host "Removing: $dir" -ForegroundColor Gray
                Remove-Item -Recurse -Force $dir -ErrorAction SilentlyContinue
            }
        }

        Write-Host "Build directories cleaned." -ForegroundColor Green
    }

    # Function to configure and build
    function Build-Configuration {
        param([string]$Config)

        Write-Host "`n=== Building $Config Configuration ===" -ForegroundColor Cyan

        # Set preset name
        $presetName = "msvc-" + $Config.ToLower()

        Write-Host "1. Configuring with preset: $presetName" -ForegroundColor White
        $configResult = & cmake --preset $presetName 2>&1

        if ($LASTEXITCODE -ne 0) {
            Write-Host "ERROR: Configuration failed for $Config" -ForegroundColor Red
            Write-Host $configResult
            throw "Configuration failed"
        }

        if ($VerboseOutput) {
            Write-Host $configResult -ForegroundColor Gray
        }

        Write-Host "2. Building with preset: $presetName" -ForegroundColor White
        $buildResult = & cmake --build --preset $presetName 2>&1

        if ($LASTEXITCODE -ne 0) {
            Write-Host "ERROR: Build failed for $Config" -ForegroundColor Red
            Write-Host $buildResult
            throw "Build failed"
        }

        if ($VerboseOutput) {
            Write-Host $buildResult -ForegroundColor Gray
        }

        # Check if executable was created
        $buildDir = "build-" + $Config.ToLower()
        $exePath = Join-Path $buildDir "bin\$Config\CloudRegistration.exe"

        if (Test-Path $exePath) {
            $fileInfo = Get-Item $exePath
            Write-Host "✅ Build successful: $exePath" -ForegroundColor Green
            Write-Host "   Size: $([math]::Round($fileInfo.Length / 1KB, 2)) KB" -ForegroundColor Gray
            Write-Host "   Modified: $($fileInfo.LastWriteTime)" -ForegroundColor Gray
        } else {
            Write-Host "⚠️  Warning: Executable not found at expected location" -ForegroundColor Yellow
        }
    }

    # Function to run tests
    function Run-Tests {
        param([string]$Config)

        Write-Host "`n=== Running Tests for $Config ===" -ForegroundColor Cyan

        $buildDir = "build-" + $Config.ToLower()

        if (-not (Test-Path $buildDir)) {
            Write-Host "Build directory not found: $buildDir" -ForegroundColor Red
            return
        }

        Push-Location $buildDir

        try {
            Write-Host "Running CTest..." -ForegroundColor White
            $testResult = & ctest --output-on-failure --build-config $Config 2>&1

            if ($LASTEXITCODE -eq 0) {
                Write-Host "✅ All tests passed!" -ForegroundColor Green
            } else {
                Write-Host "❌ Some tests failed" -ForegroundColor Red
                Write-Host $testResult
            }
        }
        finally {
            Pop-Location
        }
    }

    # Main execution
    if ($BuildType -eq "Clean" -or $CleanFirst) {
        Clean-BuildDirectories
    }

    if ($BuildType -ne "Clean") {
        # Ensure environment variables are set
        if (-not $env:VCPKG_ROOT) {
            $env:VCPKG_ROOT = "c:\dev\vcpkg"
            Write-Host "Set VCPKG_ROOT to: $env:VCPKG_ROOT" -ForegroundColor Yellow
        }

        # Build configurations
        switch ($BuildType) {
            "Debug" {
                Build-Configuration "Debug"
                if ($RunTests) { Run-Tests "Debug" }
            }
            "Release" {
                Build-Configuration "Release"
                if ($RunTests) { Run-Tests "Release" }
            }
            "Both" {
                Build-Configuration "Debug"
                Build-Configuration "Release"
                if ($RunTests) {
                    Run-Tests "Debug"
                    Run-Tests "Release"
                }
            }
        }
    }

    Write-Host "`n=== Build Script Completed Successfully ===" -ForegroundColor Green

} catch {
    Write-Host "`n❌ Build script failed: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
} finally {
    Pop-Location
}
