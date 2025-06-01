#!/usr/bin/env pwsh

# Sprint 1.2 Implementation Test Script
# Tests scan import functionality and SQLite database integration

Write-Host "🚀 Sprint 1.2 Implementation Test" -ForegroundColor Cyan
Write-Host "=================================" -ForegroundColor Cyan

# Function to check if command exists
function Test-Command($cmdname) {
    return [bool](Get-Command -Name $cmdname -ErrorAction SilentlyContinue)
}

# Function to build the project
function Build-Project {
    Write-Host "Building project..." -ForegroundColor Yellow
    
    if (Test-Path "build") {
        Remove-Item -Recurse -Force "build"
    }
    
    New-Item -ItemType Directory -Path "build" | Out-Null
    Set-Location "build"
    
    try {
        $configResult = cmake .. 2>&1
        if ($LASTEXITCODE -ne 0) {
            Write-Host "CMake configuration failed!" -ForegroundColor Red
            Write-Host $configResult -ForegroundColor Red
            return $false
        }
        
        $buildResult = cmake --build . --config Debug 2>&1
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Build failed!" -ForegroundColor Red
            Write-Host $buildResult -ForegroundColor Red
            return $false
        }
        
        Write-Host "✅ Build successful!" -ForegroundColor Green
        return $true
    }
    finally {
        Set-Location ".."
    }
}

# Function to run Sprint 1.2 tests
function Test-Sprint12Implementation {
    Write-Host "Running Sprint 1.2 implementation tests..." -ForegroundColor Yellow
    
    if (!(Test-Path "build/bin/Debug/test_sprint1_2_implementation.exe")) {
        Write-Host "❌ Test executable not found. Building test..." -ForegroundColor Red
        
        # Add test executable to CMakeLists.txt if not already there
        $cmakeContent = Get-Content "CMakeLists.txt" -Raw
        if ($cmakeContent -notmatch "test_sprint1_2_implementation") {
            Write-Host "Adding test executable to CMakeLists.txt..." -ForegroundColor Yellow
            
            $testTarget = @"

# Sprint 1.2 Implementation Test
add_executable(test_sprint1_2_implementation
    test_sprint1_2_implementation.cpp
    src/projectmanager.cpp
    src/sqlitemanager.cpp
    src/scanimportmanager.cpp
    src/projecttreemodel.cpp
    src/project.cpp
)

target_link_libraries(test_sprint1_2_implementation
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)

target_include_directories(test_sprint1_2_implementation PRIVATE .)
"@
            Add-Content "CMakeLists.txt" $testTarget
            
            if (!(Build-Project)) {
                return $false
            }
        }
    }
    
    if (Test-Path "build/bin/Debug/test_sprint1_2_implementation.exe") {
        Write-Host "Running Sprint 1.2 tests..." -ForegroundColor Green
        $testResult = & "build/bin/Debug/test_sprint1_2_implementation.exe" 2>&1
        Write-Host $testResult
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✅ Sprint 1.2 tests passed!" -ForegroundColor Green
            return $true
        } else {
            Write-Host "❌ Sprint 1.2 tests failed!" -ForegroundColor Red
            return $false
        }
    } else {
        Write-Host "❌ Test executable still not found after build" -ForegroundColor Red
        return $false
    }
}

# Function to test main application
function Test-MainApplication {
    Write-Host "Testing main application..." -ForegroundColor Yellow
    
    if (Test-Path "build/bin/Debug/CloudRegistration.exe") {
        Write-Host "✅ Main application executable found" -ForegroundColor Green
        
        Write-Host "Manual testing instructions:" -ForegroundColor Cyan
        Write-Host "1. Run: build/bin/Debug/CloudRegistration.exe" -ForegroundColor White
        Write-Host "2. Create a new project" -ForegroundColor White
        Write-Host "3. Verify import guidance appears" -ForegroundColor White
        Write-Host "4. Try importing scan files (.las or .e57)" -ForegroundColor White
        Write-Host "5. Verify scans appear in sidebar" -ForegroundColor White
        Write-Host "6. Check project folder for:" -ForegroundColor White
        Write-Host "   - Scans/ subfolder" -ForegroundColor White
        Write-Host "   - project_data.sqlite database" -ForegroundColor White
        
        return $true
    } else {
        Write-Host "❌ Main application executable not found" -ForegroundColor Red
        return $false
    }
}

# Function to create test scan files
function Create-TestScanFiles {
    Write-Host "Creating test scan files..." -ForegroundColor Yellow
    
    if (!(Test-Path "test_data")) {
        New-Item -ItemType Directory -Path "test_data" | Out-Null
    }
    
    # Create a simple LAS file header (minimal for testing)
    $lasHeader = [byte[]](
        0x4C, 0x41, 0x53, 0x46,  # "LASF" signature
        0x01, 0x02,              # Version 1.2
        0x00, 0x00,              # Reserved
        0x00, 0x00, 0x00, 0x00,  # GUID
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x01,                    # Version major
        0x02,                    # Version minor
        0x00, 0x00, 0x00, 0x00,  # System identifier (32 bytes)
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    )
    
    # Pad to minimum LAS header size (227 bytes)
    $padding = New-Object byte[] (227 - $lasHeader.Length)
    $fullHeader = $lasHeader + $padding
    
    [System.IO.File]::WriteAllBytes("test_data/test_scan.las", $fullHeader)
    
    # Create a minimal E57 file
    $e57Content = @"
ASTM-E57<?xml version="1.0" encoding="UTF-8"?>
<e57Root type="Structure" xmlns="http://www.astm.org/COMMIT/E57/2010-e57-v1.0">
    <formatName type="String">ASTM E57 3D Imaging Data File</formatName>
    <data3D type="Vector">
        <vectorChild type="Structure">
            <points type="CompressedVector" recordCount="0">
                <prototype type="Structure">
                    <cartesianX type="Float" precision="single"/>
                    <cartesianY type="Float" precision="single"/>
                    <cartesianZ type="Float" precision="single"/>
                </prototype>
            </points>
        </vectorChild>
    </data3D>
</e57Root>
"@
    
    [System.IO.File]::WriteAllText("test_data/test_scan.e57", $e57Content)
    
    Write-Host "✅ Test scan files created in test_data/" -ForegroundColor Green
}

# Main execution
try {
    # Check prerequisites
    if (!(Test-Command "cmake")) {
        Write-Host "❌ CMake not found. Please install CMake." -ForegroundColor Red
        exit 1
    }
    
    if (!(Test-Command "cl") -and !(Test-Command "gcc")) {
        Write-Host "❌ No C++ compiler found. Please install Visual Studio or GCC." -ForegroundColor Red
        exit 1
    }
    
    Write-Host "✅ Prerequisites check passed" -ForegroundColor Green
    
    # Create test files
    Create-TestScanFiles
    
    # Build project
    if (!(Build-Project)) {
        Write-Host "❌ Build failed. Cannot proceed with tests." -ForegroundColor Red
        exit 1
    }
    
    # Run Sprint 1.2 tests
    if (!(Test-Sprint12Implementation)) {
        Write-Host "❌ Sprint 1.2 implementation tests failed." -ForegroundColor Red
        exit 1
    }
    
    # Test main application
    if (!(Test-MainApplication)) {
        Write-Host "❌ Main application test failed." -ForegroundColor Red
        exit 1
    }
    
    Write-Host "`n🎉 Sprint 1.2 Implementation Test Complete!" -ForegroundColor Green
    Write-Host "==========================================" -ForegroundColor Green
    Write-Host "✅ All automated tests passed" -ForegroundColor Green
    Write-Host "✅ Main application builds successfully" -ForegroundColor Green
    Write-Host "✅ Test scan files created" -ForegroundColor Green
    Write-Host "`nNext steps:" -ForegroundColor Cyan
    Write-Host "1. Run manual tests with the main application" -ForegroundColor White
    Write-Host "2. Test scan import with real .las/.e57 files" -ForegroundColor White
    Write-Host "3. Verify database functionality" -ForegroundColor White
    
} catch {
    Write-Host "❌ Test script failed: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
}
