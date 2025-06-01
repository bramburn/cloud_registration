#!/usr/bin/env pwsh

# Sprint 4 E57 Library Integration Test Runner
# Comprehensive testing, performance profiling, and documentation validation

param(
    [string]$BuildDir = "build",
    [string]$TestDataDir = "test_data",
    [switch]$PerformanceOnly,
    [switch]$ComprehensiveOnly,
    [switch]$GenerateReports,
    [switch]$Verbose
)

Write-Host "=== Sprint 4 E57 Library Integration Test Suite ===" -ForegroundColor Green
Write-Host "Testing: Comprehensive functionality, Performance profiling, Multi-scan support" -ForegroundColor Yellow

# Set working directory
Set-Location "C:\dev\cloud_registration"

# Check if build directory exists
if (-not (Test-Path $BuildDir)) {
    Write-Host "Build directory not found: $BuildDir" -ForegroundColor Red
    Write-Host "Please build the project first using:" -ForegroundColor Yellow
    Write-Host "  cmake --build $BuildDir --config Release" -ForegroundColor White
    exit 1
}

# Check if test executable exists
$TestExecutable = "$BuildDir\bin\Release\E57Sprint4ComprehensiveTests.exe"
if (-not (Test-Path $TestExecutable)) {
    $TestExecutable = "$BuildDir\tests\Release\E57Sprint4ComprehensiveTests.exe"
    if (-not (Test-Path $TestExecutable)) {
        Write-Host "Test executable not found. Expected locations:" -ForegroundColor Red
        Write-Host "  $BuildDir\bin\Release\E57Sprint4ComprehensiveTests.exe" -ForegroundColor Gray
        Write-Host "  $BuildDir\tests\Release\E57Sprint4ComprehensiveTests.exe" -ForegroundColor Gray
        exit 1
    }
}

# Ensure test data directory exists
if (-not (Test-Path $TestDataDir)) {
    Write-Host "Creating test data directory: $TestDataDir" -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $TestDataDir -Force | Out-Null
}

# Check for available test files
$AvailableTestFiles = @()
$SampleFiles = @("sample\bunnyDouble.e57", "sample\bunnyInt32.e57")

foreach ($file in $SampleFiles) {
    if (Test-Path $file) {
        $AvailableTestFiles += $file
        Write-Host "Found test file: $file" -ForegroundColor Green
    } else {
        Write-Host "Test file not found: $file" -ForegroundColor Yellow
    }
}

if ($AvailableTestFiles.Count -eq 0) {
    Write-Host "Warning: No E57 test files found. Some tests will be skipped." -ForegroundColor Yellow
    Write-Host "To get full test coverage, place E57 files in the sample/ directory." -ForegroundColor Gray
}

Write-Host "`nStarting Sprint 4 test execution..." -ForegroundColor Cyan

# Function to run tests with specific filters
function Run-TestCategory {
    param(
        [string]$Category,
        [string]$Filter,
        [string]$Description
    )
    
    Write-Host "`n--- $Description ---" -ForegroundColor Magenta
    
    $Args = @()
    if ($Filter) {
        $Args += "--gtest_filter=$Filter"
    }
    if ($Verbose) {
        $Args += "--gtest_verbose"
    }
    
    $StartTime = Get-Date
    
    try {
        & $TestExecutable $Args
        $ExitCode = $LASTEXITCODE
        
        $EndTime = Get-Date
        $Duration = $EndTime - $StartTime
        
        if ($ExitCode -eq 0) {
            Write-Host "$Category tests completed successfully in $($Duration.TotalSeconds.ToString('F2')) seconds" -ForegroundColor Green
        } else {
            Write-Host "$Category tests failed with exit code: $ExitCode" -ForegroundColor Red
        }
        
        return $ExitCode
    } catch {
        Write-Host "Error running $Category tests: $($_.Exception.Message)" -ForegroundColor Red
        return 1
    }
}

# Test execution based on parameters
$TestResults = @{}

if ($PerformanceOnly) {
    Write-Host "Running performance tests only..." -ForegroundColor Cyan
    $TestResults["Performance"] = Run-TestCategory "Performance" "*Performance*" "Performance Profiling and Optimization Tests"
} elseif ($ComprehensiveOnly) {
    Write-Host "Running comprehensive functionality tests only..." -ForegroundColor Cyan
    $TestResults["Comprehensive"] = Run-TestCategory "Comprehensive" "*Comprehensive*" "Comprehensive E57 Functionality Verification"
} else {
    # Run all test categories
    Write-Host "Running all Sprint 4 test categories..." -ForegroundColor Cyan
    
    # 1. Comprehensive Functionality Verification
    $TestResults["Comprehensive"] = Run-TestCategory "Comprehensive" "*ComprehensiveFunctionalityVerification*" "User Story 1: Comprehensive E57 Functionality Verification"
    
    # 2. Performance Profiling and Optimization
    $TestResults["Performance"] = Run-TestCategory "Performance" "*PerformanceProfilingAndOptimization*" "User Story 2: Profile and Optimize E57 Loading Performance"
    
    # 3. Multi-scan Handling
    $TestResults["MultiScan"] = Run-TestCategory "MultiScan" "*MultiScanHandling*" "User Story 3: Basic Handling of E57 Files with Multiple Scans"
    
    # 4. Enhanced Unit Test Coverage
    $TestResults["UnitTests"] = Run-TestCategory "UnitTests" "*EnhancedUnitTest*" "User Story 4: Adapt and Enhance Unit Test Suite"
    
    # 5. Integration Tests
    $TestResults["Integration"] = Run-TestCategory "Integration" "*Integration*" "Complete Workflow Integration Tests"
    
    # 6. Regression Prevention
    $TestResults["Regression"] = Run-TestCategory "Regression" "*Regression*" "Regression Prevention Tests"
}

# Generate summary report
Write-Host "`n=== Sprint 4 Test Summary ===" -ForegroundColor Green

$TotalTests = $TestResults.Count
$PassedTests = ($TestResults.Values | Where-Object { $_ -eq 0 }).Count
$FailedTests = $TotalTests - $PassedTests

Write-Host "Total Test Categories: $TotalTests" -ForegroundColor White
Write-Host "Passed: $PassedTests" -ForegroundColor Green
Write-Host "Failed: $FailedTests" -ForegroundColor $(if ($FailedTests -eq 0) { "Green" } else { "Red" })

if ($FailedTests -eq 0) {
    Write-Host "`nAll Sprint 4 tests passed successfully! ✅" -ForegroundColor Green
} else {
    Write-Host "`nSome tests failed. Check the output above for details. ❌" -ForegroundColor Red
    
    Write-Host "`nFailed test categories:" -ForegroundColor Yellow
    foreach ($test in $TestResults.GetEnumerator()) {
        if ($test.Value -ne 0) {
            Write-Host "  - $($test.Key)" -ForegroundColor Red
        }
    }
}

# Check for generated reports
if ($GenerateReports -or $TestResults.Count -gt 0) {
    Write-Host "`n--- Generated Reports ---" -ForegroundColor Magenta
    
    $ReportFiles = @(
        "$TestDataDir\comprehensive_test_report.html",
        "$TestDataDir\performance_report.html",
        "$TestDataDir\final_test_report.html",
        "$TestDataDir\final_performance_report.html",
        "$TestDataDir\performance_metrics.json"
    )
    
    foreach ($report in $ReportFiles) {
        if (Test-Path $report) {
            $FileSize = (Get-Item $report).Length
            Write-Host "Generated: $report ($([math]::Round($FileSize/1KB, 2)) KB)" -ForegroundColor Green
        }
    }
    
    # Open main report if available
    $MainReport = "$TestDataDir\final_test_report.html"
    if (Test-Path $MainReport) {
        Write-Host "`nOpening main test report..." -ForegroundColor Cyan
        Start-Process $MainReport
    }
}

Write-Host "`n=== Sprint 4 Test Execution Complete ===" -ForegroundColor Green

# Documentation validation
Write-Host "`n--- Documentation Validation ---" -ForegroundColor Magenta
$DocFiles = @(
    "docs\e57library\Sprint4_Implementation_Guide.md",
    "docs\e57library\s4.md",
    "docs\e57library\s4g.md"
)

foreach ($doc in $DocFiles) {
    if (Test-Path $doc) {
        Write-Host "Documentation found: $doc ✅" -ForegroundColor Green
    } else {
        Write-Host "Documentation missing: $doc ❌" -ForegroundColor Yellow
    }
}

# Exit with appropriate code
if ($FailedTests -eq 0) {
    exit 0
} else {
    exit 1
}
