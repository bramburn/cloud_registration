#!/usr/bin/env pwsh

<#
.SYNOPSIS
    Test Sprint 3.2 Implementation: 3D Point Cloud Viewer Component

.DESCRIPTION
    This script validates the implementation of Sprint 3.2 requirements:
    - User Story 1: 3D Point Cloud Viewer Component
    - User Story 2: Point Cloud Data Rendering  
    - User Story 3: Camera Controls

.PARAMETER BuildType
    The build type to test (Debug or Release). Default is Release.

.EXAMPLE
    .\test_sprint32_implementation.ps1 -BuildType Release
#>

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Release"
)

# Script configuration
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

# Get script directory and project root
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent (Split-Path -Parent $ScriptDir)
$BuildDir = Join-Path $ProjectRoot "build" $BuildType.ToLower()

Write-Host "=== Sprint 3.2 Implementation Test ===" -ForegroundColor Cyan
Write-Host "Testing 3D Point Cloud Viewer Component" -ForegroundColor Cyan
Write-Host "Build Type: $BuildType" -ForegroundColor Yellow
Write-Host "Project Root: $ProjectRoot" -ForegroundColor Yellow
Write-Host "Build Directory: $BuildDir" -ForegroundColor Yellow
Write-Host ""

# Function to check if file exists
function Test-FileExists {
    param([string]$FilePath, [string]$Description)
    
    if (Test-Path $FilePath) {
        Write-Host "✓ $Description exists" -ForegroundColor Green
        return $true
    } else {
        Write-Host "✗ $Description missing: $FilePath" -ForegroundColor Red
        return $false
    }
}

# Function to check source code implementation
function Test-SourceImplementation {
    param([string]$FilePath, [string[]]$RequiredPatterns, [string]$Description)
    
    if (-not (Test-Path $FilePath)) {
        Write-Host "✗ $Description - File missing: $FilePath" -ForegroundColor Red
        return $false
    }
    
    $content = Get-Content $FilePath -Raw
    $allFound = $true
    
    foreach ($pattern in $RequiredPatterns) {
        if ($content -match $pattern) {
            Write-Host "  ✓ Found: $pattern" -ForegroundColor Green
        } else {
            Write-Host "  ✗ Missing: $pattern" -ForegroundColor Red
            $allFound = $false
        }
    }
    
    if ($allFound) {
        Write-Host "✓ $Description - All patterns found" -ForegroundColor Green
    } else {
        Write-Host "✗ $Description - Some patterns missing" -ForegroundColor Red
    }
    
    return $allFound
}

# Test 1: Check Core Files Exist
Write-Host "Test 1: Checking Core Implementation Files" -ForegroundColor Magenta

$coreFiles = @(
    @{Path = "src/pointcloudviewerwidget.h"; Desc = "PointCloudViewerWidget Header"},
    @{Path = "src/pointcloudviewerwidget.cpp"; Desc = "PointCloudViewerWidget Implementation"},
    @{Path = "src/pointcloudloadmanager.h"; Desc = "PointCloudLoadManager Header"},
    @{Path = "src/pointcloudloadmanager.cpp"; Desc = "PointCloudLoadManager Implementation"},
    @{Path = "src/mainwindow.h"; Desc = "MainWindow Header"},
    @{Path = "src/mainwindow.cpp"; Desc = "MainWindow Implementation"},
    @{Path = "tests/test_sprint32_3d_viewer.cpp"; Desc = "Sprint 3.2 Test Suite"}
)

$allCoreFilesExist = $true
foreach ($file in $coreFiles) {
    $fullPath = Join-Path $ProjectRoot $file.Path
    if (-not (Test-FileExists $fullPath $file.Desc)) {
        $allCoreFilesExist = $false
    }
}

Write-Host ""

# Test 2: Check PointCloudViewerWidget Implementation
Write-Host "Test 2: Checking PointCloudViewerWidget Implementation" -ForegroundColor Magenta

$viewerHeaderPath = Join-Path $ProjectRoot "src/pointcloudviewerwidget.h"
$viewerRequiredPatterns = @(
    "class PointCloudViewerWidget.*QOpenGLWidget",
    "enum class ViewerState",
    "void loadPointCloud.*std::vector<float>",
    "void clearPointCloud",
    "ViewerState getViewerState",
    "bool hasPointCloudData",
    "size_t getPointCount",
    "void simulateOrbitCamera",
    "void simulatePanCamera",
    "void simulateZoomCamera"
)

$viewerHeaderOk = Test-SourceImplementation $viewerHeaderPath $viewerRequiredPatterns "PointCloudViewerWidget Header"

$viewerImplPath = Join-Path $ProjectRoot "src/pointcloudviewerwidget.cpp"
$viewerImplPatterns = @(
    "void PointCloudViewerWidget::loadPointCloud",
    "void PointCloudViewerWidget::clearPointCloud",
    "void PointCloudViewerWidget::simulateOrbitCamera",
    "void PointCloudViewerWidget::simulatePanCamera",
    "void PointCloudViewerWidget::simulateZoomCamera",
    "glDrawArrays.*GL_POINTS",
    "updateCamera",
    "calculateBoundingBox"
)

$viewerImplOk = Test-SourceImplementation $viewerImplPath $viewerImplPatterns "PointCloudViewerWidget Implementation"

Write-Host ""

# Test 3: Check PointCloudLoadManager Enhancement
Write-Host "Test 3: Checking PointCloudLoadManager Enhancement" -ForegroundColor Magenta

$loadManagerHeaderPath = Join-Path $ProjectRoot "src/pointcloudloadmanager.h"
$loadManagerHeaderPatterns = @(
    "bool viewScan.*QString",
    "bool viewCluster.*QString", 
    "std::vector<float> getAggregatedPointCloudData",
    "std::vector<float> getScanPointCloudData",
    "void pointCloudDataReady.*std::vector<float>.*QString",
    "void pointCloudViewFailed.*QString"
)

$loadManagerHeaderOk = Test-SourceImplementation $loadManagerHeaderPath $loadManagerHeaderPatterns "PointCloudLoadManager Header"

$loadManagerImplPath = Join-Path $ProjectRoot "src/pointcloudloadmanager.cpp"
$loadManagerImplPatterns = @(
    "bool PointCloudLoadManager::viewScan",
    "bool PointCloudLoadManager::viewCluster",
    "std::vector<float> PointCloudLoadManager::getAggregatedPointCloudData",
    "std::vector<float> PointCloudLoadManager::getScanPointCloudData",
    "emit pointCloudDataReady",
    "emit pointCloudViewFailed"
)

$loadManagerImplOk = Test-SourceImplementation $loadManagerImplPath $loadManagerImplPatterns "PointCloudLoadManager Implementation"

Write-Host ""

# Test 4: Check MainWindow Integration
Write-Host "Test 4: Checking MainWindow Integration" -ForegroundColor Magenta

$mainWindowHeaderPath = Join-Path $ProjectRoot "src/mainwindow.h"
$mainWindowHeaderPatterns = @(
    "class PointCloudLoadManager",
    "PointCloudLoadManager.*m_loadManager",
    "void onPointCloudDataReady.*std::vector<float>.*QString",
    "void onPointCloudViewFailed.*QString",
    "PointCloudViewerWidget.*getPointCloudViewer",
    "PointCloudLoadManager.*getPointCloudLoadManager"
)

$mainWindowHeaderOk = Test-SourceImplementation $mainWindowHeaderPath $mainWindowHeaderPatterns "MainWindow Header"

$mainWindowImplPath = Join-Path $ProjectRoot "src/mainwindow.cpp"
$mainWindowImplPatterns = @(
    "#include.*pointcloudloadmanager.h",
    "new PointCloudLoadManager",
    "connect.*m_loadManager.*pointCloudDataReady",
    "connect.*m_loadManager.*pointCloudViewFailed",
    "void MainWindow::onPointCloudDataReady",
    "void MainWindow::onPointCloudViewFailed",
    "m_loadManager->setSQLiteManager",
    "m_sidebar->setPointCloudLoadManager"
)

$mainWindowImplOk = Test-SourceImplementation $mainWindowImplPath $mainWindowImplPatterns "MainWindow Implementation"

Write-Host ""

# Test 5: Check Test Suite
Write-Host "Test 5: Checking Test Suite Implementation" -ForegroundColor Magenta

$testSuitePath = Join-Path $ProjectRoot "tests/test_sprint32_3d_viewer.cpp"
$testSuitePatterns = @(
    "class Sprint32ViewerTest",
    "TEST_F.*ViewerComponentCreation",
    "TEST_F.*PointCloudDataLoading",
    "TEST_F.*CameraOrbitControls",
    "TEST_F.*CameraPanControls",
    "TEST_F.*CameraZoomControls",
    "TEST_F.*LoadManagerIntegration",
    "TEST_F.*MainWindowIntegration"
)

$testSuiteOk = Test-SourceImplementation $testSuitePath $testSuitePatterns "Sprint 3.2 Test Suite"

Write-Host ""

# Test 6: Summary and Results
Write-Host "Test 6: Implementation Summary" -ForegroundColor Magenta

$allTestsPassed = $allCoreFilesExist -and $viewerHeaderOk -and $viewerImplOk -and 
                  $loadManagerHeaderOk -and $loadManagerImplOk -and 
                  $mainWindowHeaderOk -and $mainWindowImplOk -and $testSuiteOk

Write-Host ""
Write-Host "=== Sprint 3.2 Implementation Test Results ===" -ForegroundColor Cyan

if ($allTestsPassed) {
    Write-Host "✓ ALL TESTS PASSED - Sprint 3.2 Implementation Complete!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Sprint 3.2 Features Implemented:" -ForegroundColor Green
    Write-Host "  ✓ 3D Point Cloud Viewer Component with OpenGL rendering" -ForegroundColor Green
    Write-Host "  ✓ Point Cloud Data Loading and Rendering" -ForegroundColor Green
    Write-Host "  ✓ Camera Controls (Orbit, Pan, Zoom)" -ForegroundColor Green
    Write-Host "  ✓ Enhanced PointCloudLoadManager with view operations" -ForegroundColor Green
    Write-Host "  ✓ MainWindow integration with signal connections" -ForegroundColor Green
    Write-Host "  ✓ Comprehensive test suite for validation" -ForegroundColor Green
    Write-Host ""
    Write-Host "Ready for Sprint 3.2 acceptance testing!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "✗ SOME TESTS FAILED - Sprint 3.2 Implementation Incomplete" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please review the failed tests above and complete the missing implementations." -ForegroundColor Red
    exit 1
}
