#!/usr/bin/env pwsh

<#
.SYNOPSIS
    Sprint 1.4 Integration Testing Script
    
.DESCRIPTION
    Comprehensive testing script for Sprint 1.4 integration testing framework.
    Executes all integration tests, generates reports, and validates Sprint 1.4 
    acceptance criteria.
    
.PARAMETER BuildType
    Build configuration to test (Debug or Release)
    
.PARAMETER GenerateReports
    Whether to generate comprehensive test reports
    
.PARAMETER TestTimeout
    Timeout for individual tests in seconds
    
.EXAMPLE
    .\test_sprint1_4_integration.ps1 -BuildType Debug -GenerateReports
#>

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Debug",
    
    [Parameter(Mandatory=$false)]
    [switch]$GenerateReports = $true,
    
    [Parameter(Mandatory=$false)]
    [int]$TestTimeout = 300
)

# Script configuration
$ErrorActionPreference = "Stop"
$ProgressPreference = "Continue"

# Paths
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$BuildDir = Join-Path $ScriptDir "build"
$TestDataDir = Join-Path $ScriptDir "test_data"
$SampleDir = Join-Path $ScriptDir "sample"
$ReportsDir = Join-Path $ScriptDir "test_reports"

Write-Host "=== Sprint 1.4 Integration Testing ===" -ForegroundColor Cyan
Write-Host "Build Type: $BuildType" -ForegroundColor Green
Write-Host "Test Timeout: $TestTimeout seconds" -ForegroundColor Green
Write-Host "Generate Reports: $GenerateReports" -ForegroundColor Green
Write-Host ""

# Function to check if test data exists
function Test-TestDataAvailability {
    Write-Host "Checking test data availability..." -ForegroundColor Yellow
    
    $testFiles = @(
        "test_data/simple_uncompressed.e57",
        "test_data/compressedvector_uncompressed_data.e57",
        "test_data/test_large_coords.e57",
        "test_data/test_3_points_line.e57",
        "test_data/malformed_compressedvector.e57"
    )
    
    $realWorldFiles = @(
        "sample/bunnyDouble.e57",
        "sample/S2max-Power line202503.las"
    )
    
    $availableTestFiles = 0
    $availableRealWorldFiles = 0
    
    foreach ($file in $testFiles) {
        $fullPath = Join-Path $ScriptDir $file
        if (Test-Path $fullPath) {
            Write-Host "  ✓ $file" -ForegroundColor Green
            $availableTestFiles++
        } else {
            Write-Host "  ✗ $file (missing)" -ForegroundColor Red
        }
    }
    
    foreach ($file in $realWorldFiles) {
        $fullPath = Join-Path $ScriptDir $file
        if (Test-Path $fullPath) {
            Write-Host "  ✓ $file" -ForegroundColor Green
            $availableRealWorldFiles++
        } else {
            Write-Host "  ✗ $file (missing)" -ForegroundColor Yellow
        }
    }
    
    Write-Host "Test files available: $availableTestFiles/$($testFiles.Count)" -ForegroundColor Cyan
    Write-Host "Real-world files available: $availableRealWorldFiles/$($realWorldFiles.Count)" -ForegroundColor Cyan
    Write-Host ""
    
    return @{
        TestFiles = $availableTestFiles
        RealWorldFiles = $availableRealWorldFiles
        TotalTestFiles = $testFiles.Count
        TotalRealWorldFiles = $realWorldFiles.Count
    }
}

# Function to run integration tests
function Invoke-IntegrationTests {
    param(
        [string]$TestExecutable,
        [int]$TimeoutSeconds
    )
    
    Write-Host "Running Sprint 1.4 Integration Tests..." -ForegroundColor Yellow
    Write-Host "Executable: $TestExecutable" -ForegroundColor Gray
    Write-Host "Timeout: $TimeoutSeconds seconds" -ForegroundColor Gray
    Write-Host ""
    
    if (-not (Test-Path $TestExecutable)) {
        throw "Test executable not found: $TestExecutable"
    }
    
    try {
        # Run the integration tests with timeout
        $process = Start-Process -FilePath $TestExecutable -NoNewWindow -PassThru -RedirectStandardOutput "test_output.txt" -RedirectStandardError "test_errors.txt"
        
        if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
            $process.Kill()
            throw "Integration tests timed out after $TimeoutSeconds seconds"
        }
        
        $exitCode = $process.ExitCode
        
        # Display test output
        if (Test-Path "test_output.txt") {
            $output = Get-Content "test_output.txt" -Raw
            Write-Host $output
        }
        
        if (Test-Path "test_errors.txt") {
            $errors = Get-Content "test_errors.txt" -Raw
            if ($errors.Trim()) {
                Write-Host "Test Errors:" -ForegroundColor Red
                Write-Host $errors -ForegroundColor Red
            }
        }
        
        # Clean up temporary files
        Remove-Item "test_output.txt" -ErrorAction SilentlyContinue
        Remove-Item "test_errors.txt" -ErrorAction SilentlyContinue
        
        return $exitCode
        
    } catch {
        Write-Host "Error running integration tests: $($_.Exception.Message)" -ForegroundColor Red
        throw
    }
}

# Function to validate Sprint 1.4 acceptance criteria
function Test-Sprint14AcceptanceCriteria {
    param(
        [int]$TestExitCode,
        [hashtable]$TestDataInfo
    )
    
    Write-Host "Validating Sprint 1.4 Acceptance Criteria..." -ForegroundColor Yellow
    
    $criteria = @()
    
    # Criterion 1: Integration tests pass
    if ($TestExitCode -eq 0) {
        $criteria += @{ Name = "Integration Tests Pass"; Status = "PASS"; Details = "All integration tests completed successfully" }
    } else {
        $criteria += @{ Name = "Integration Tests Pass"; Status = "FAIL"; Details = "Integration tests failed with exit code $TestExitCode" }
    }
    
    # Criterion 2: Test data coverage
    $testDataCoverage = ($TestDataInfo.TestFiles / $TestDataInfo.TotalTestFiles) * 100
    if ($testDataCoverage -ge 80) {
        $criteria += @{ Name = "Test Data Coverage"; Status = "PASS"; Details = "$($testDataCoverage.ToString('F1'))% test files available" }
    } else {
        $criteria += @{ Name = "Test Data Coverage"; Status = "FAIL"; Details = "Only $($testDataCoverage.ToString('F1'))% test files available (minimum 80%)" }
    }
    
    # Criterion 3: Real-world file testing
    if ($TestDataInfo.RealWorldFiles -gt 0) {
        $criteria += @{ Name = "Real-World File Testing"; Status = "PASS"; Details = "$($TestDataInfo.RealWorldFiles) real-world files available for testing" }
    } else {
        $criteria += @{ Name = "Real-World File Testing"; Status = "WARN"; Details = "No real-world files available for testing" }
    }
    
    # Criterion 4: Test reports generated
    if ($GenerateReports -and (Test-Path $ReportsDir)) {
        $reportFiles = Get-ChildItem $ReportsDir -Filter "*.txt", "*.json", "*.md" | Measure-Object
        if ($reportFiles.Count -gt 0) {
            $criteria += @{ Name = "Test Reports Generated"; Status = "PASS"; Details = "$($reportFiles.Count) report files generated" }
        } else {
            $criteria += @{ Name = "Test Reports Generated"; Status = "FAIL"; Details = "No report files found in $ReportsDir" }
        }
    } else {
        $criteria += @{ Name = "Test Reports Generated"; Status = "SKIP"; Details = "Report generation disabled or reports directory not found" }
    }
    
    # Display results
    Write-Host ""
    Write-Host "Sprint 1.4 Acceptance Criteria Results:" -ForegroundColor Cyan
    Write-Host "=======================================" -ForegroundColor Cyan
    
    $passCount = 0
    $failCount = 0
    $warnCount = 0
    
    foreach ($criterion in $criteria) {
        $color = switch ($criterion.Status) {
            "PASS" { "Green"; $passCount++ }
            "FAIL" { "Red"; $failCount++ }
            "WARN" { "Yellow"; $warnCount++ }
            "SKIP" { "Gray" }
        }
        
        Write-Host "[$($criterion.Status)] $($criterion.Name)" -ForegroundColor $color
        Write-Host "    $($criterion.Details)" -ForegroundColor Gray
    }
    
    Write-Host ""
    Write-Host "Summary: $passCount passed, $failCount failed, $warnCount warnings" -ForegroundColor Cyan
    
    return ($failCount -eq 0)
}

# Main execution
try {
    # Check test data availability
    $testDataInfo = Test-TestDataAvailability
    
    # Determine test executable path
    $testExecutable = Join-Path $BuildDir "bin\$BuildType\Sprint14IntegrationTests.exe"
    if (-not (Test-Path $testExecutable)) {
        $testExecutable = Join-Path $BuildDir "Sprint14IntegrationTests.exe"
    }
    
    # Run integration tests
    $testExitCode = Invoke-IntegrationTests -TestExecutable $testExecutable -TimeoutSeconds $TestTimeout
    
    # Validate acceptance criteria
    $criteriaPass = Test-Sprint14AcceptanceCriteria -TestExitCode $testExitCode -TestDataInfo $testDataInfo
    
    # Final result
    Write-Host ""
    if ($criteriaPass) {
        Write-Host "🎉 Sprint 1.4 Integration Testing: SUCCESS" -ForegroundColor Green
        Write-Host "All acceptance criteria met!" -ForegroundColor Green
        exit 0
    } else {
        Write-Host "❌ Sprint 1.4 Integration Testing: FAILED" -ForegroundColor Red
        Write-Host "Some acceptance criteria not met. See details above." -ForegroundColor Red
        exit 1
    }
    
} catch {
    Write-Host ""
    Write-Host "❌ Sprint 1.4 Integration Testing: ERROR" -ForegroundColor Red
    Write-Host "Error: $($_.Exception.Message)" -ForegroundColor Red
    exit 2
}
