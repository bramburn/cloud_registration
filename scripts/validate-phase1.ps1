# Phase 1 Implementation Validation Script
# Verifies that all Phase 1 components are properly implemented

param(
    [switch]$Help
)

function Show-Help {
    Write-Host "Phase 1 Implementation Validator" -ForegroundColor Green
    Write-Host ""
    Write-Host "This script validates that Phase 1 of the Google Test PRD" -ForegroundColor Yellow
    Write-Host "has been properly implemented in the Cloud Registration project." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Usage: .\scripts\validate-phase1.ps1" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "The script checks:" -ForegroundColor White
    Write-Host "  ✓ CMake integration and configuration" -ForegroundColor White
    Write-Host "  ✓ Test file existence and structure" -ForegroundColor White
    Write-Host "  ✓ Documentation completeness" -ForegroundColor White
    Write-Host "  ✓ Script availability and permissions" -ForegroundColor White
    Write-Host "  ✓ Google Test installation status" -ForegroundColor White
}

if ($Help) {
    Show-Help
    exit 0
}

Write-Host "=== Phase 1 Implementation Validator ===" -ForegroundColor Green
Write-Host ""

$allChecks = @()
$passedChecks = 0
$totalChecks = 0

function Test-Component {
    param(
        [string]$Name,
        [scriptblock]$Test,
        [string]$SuccessMessage,
        [string]$FailureMessage
    )
    
    $global:totalChecks++
    Write-Host "Checking $Name..." -ForegroundColor Cyan
    
    try {
        $result = & $Test
        if ($result) {
            Write-Host "  ✓ $SuccessMessage" -ForegroundColor Green
            $global:passedChecks++
            $global:allChecks += @{Name=$Name; Status="PASS"; Message=$SuccessMessage}
        } else {
            Write-Host "  ✗ $FailureMessage" -ForegroundColor Red
            $global:allChecks += @{Name=$Name; Status="FAIL"; Message=$FailureMessage}
        }
    } catch {
        Write-Host "  ✗ $FailureMessage (Exception: $($_.Exception.Message))" -ForegroundColor Red
        $global:allChecks += @{Name=$Name; Status="ERROR"; Message="$FailureMessage (Exception: $($_.Exception.Message))"}
    }
}

# 1. Check CMakeLists.txt enhancements
Test-Component "CMakeLists.txt Integration" {
    $cmakeContent = Get-Content "CMakeLists.txt" -Raw
    return ($cmakeContent -match "enable_testing\(\)" -and 
            $cmakeContent -match "ENABLE_COVERAGE" -and
            $cmakeContent -match "add_test\(NAME.*COMMAND" -and
            $cmakeContent -match "run_tests")
} "CMake integration properly configured" "CMake integration missing or incomplete"

# 2. Check test files exist
$testFiles = @(
    "tests/test_e57parser.cpp",
    "tests/test_lasparser.cpp", 
    "tests/test_voxelgridfilter.cpp",
    "tests/test_sprint1_functionality.cpp"
)

foreach ($testFile in $testFiles) {
    Test-Component "Test File: $testFile" {
        return (Test-Path $testFile)
    } "Test file exists" "Test file missing"
}

# 3. Check test file structure and content
Test-Component "E57Parser Test Structure" {
    $content = Get-Content "tests/test_e57parser.cpp" -Raw
    return ($content -match "class E57ParserTest.*::testing::Test" -and
            $content -match "TEST_F\(E57ParserTest" -and
            $content -match "SetUp\(\)" -and
            $content -match "TearDown\(\)")
} "E57Parser tests properly structured" "E57Parser test structure incomplete"

Test-Component "LasParser Test Structure" {
    $content = Get-Content "tests/test_lasparser.cpp" -Raw
    return ($content -match "class LasParserTest.*::testing::Test" -and
            $content -match "TEST_F\(LasParserTest" -and
            $content -match "createMockLasFile")
} "LasParser tests properly structured" "LasParser test structure incomplete"

Test-Component "VoxelGridFilter Test Structure" {
    $content = Get-Content "tests/test_voxelgridfilter.cpp" -Raw
    return ($content -match "class VoxelGridFilterTest.*::testing::Test" -and
            $content -match "TEST_F\(VoxelGridFilterTest" -and
            $content -match "EmptyInput\|SinglePoint\|Performance")
} "VoxelGridFilter tests properly structured" "VoxelGridFilter test structure incomplete"

# 4. Check documentation files
$docFiles = @(
    "docs/testing-best-practices.md",
    "docs/phase1-implementation-summary.md",
    "docs/phase1-setup-guide.md"
)

foreach ($docFile in $docFiles) {
    Test-Component "Documentation: $docFile" {
        return (Test-Path $docFile)
    } "Documentation file exists" "Documentation file missing"
}

# 5. Check script files
$scriptFiles = @(
    "scripts/run-tests.ps1",
    "scripts/run-tests.sh"
)

foreach ($scriptFile in $scriptFiles) {
    Test-Component "Script: $scriptFile" {
        return (Test-Path $scriptFile)
    } "Script file exists" "Script file missing"
}

# 6. Check script functionality
Test-Component "PowerShell Script Functionality" {
    $content = Get-Content "scripts/run-tests.ps1" -Raw
    return ($content -match "param\(" -and
            $content -match "ctest" -and
            $content -match "Coverage" -and
            $content -match "Show-Help")
} "PowerShell script properly implemented" "PowerShell script incomplete"

Test-Component "Bash Script Functionality" {
    $content = Get-Content "scripts/run-tests.sh" -Raw
    return ($content -match "#!/bin/bash" -and
            $content -match "ctest" -and
            $content -match "coverage" -and
            $content -match "show_help")
} "Bash script properly implemented" "Bash script incomplete"

# 7. Check build directory and configuration
Test-Component "Build Directory" {
    return (Test-Path "build")
} "Build directory exists" "Build directory missing - run cmake configuration"

if (Test-Path "build") {
    Test-Component "CMake Configuration" {
        return (Test-Path "build/CMakeCache.txt")
    } "CMake has been configured" "CMake not configured - run cmake -B build -S ."
    
    Test-Component "CTest Integration" {
        return (Test-Path "build/CTestTestfile.cmake")
    } "CTest integration active" "CTest integration missing"
}

# 8. Check Google Test availability
Test-Component "Google Test Installation" {
    if (Test-Path "build/CMakeCache.txt") {
        $cacheContent = Get-Content "build/CMakeCache.txt" -Raw
        return ($cacheContent -match "GTest_FOUND:BOOL=TRUE")
    }
    return $false
} "Google Test found and configured" "Google Test not installed - see setup guide"

# 9. Check source file integration
Test-Component "Source File Integration" {
    $cmakeContent = Get-Content "CMakeLists.txt" -Raw
    return ($cmakeContent -match "src/e57parser.cpp" -and
            $cmakeContent -match "src/lasparser.cpp" -and
            $cmakeContent -match "src/voxelgridfilter.cpp")
} "Source files properly integrated" "Source file integration incomplete"

# 10. Check test coverage setup
Test-Component "Coverage Configuration" {
    $cmakeContent = Get-Content "CMakeLists.txt" -Raw
    return ($cmakeContent -match "ENABLE_COVERAGE" -and
            $cmakeContent -match "lcov" -and
            $cmakeContent -match "genhtml")
} "Coverage reporting configured" "Coverage configuration incomplete"

# Summary
Write-Host ""
Write-Host "=== Validation Summary ===" -ForegroundColor Green
Write-Host "Total Checks: $totalChecks" -ForegroundColor White
Write-Host "Passed: $passedChecks" -ForegroundColor Green
Write-Host "Failed: $($totalChecks - $passedChecks)" -ForegroundColor $(if ($passedChecks -eq $totalChecks) { "Green" } else { "Red" })

$percentage = [math]::Round(($passedChecks / $totalChecks) * 100, 1)
Write-Host "Success Rate: $percentage%" -ForegroundColor $(if ($percentage -ge 90) { "Green" } elseif ($percentage -ge 70) { "Yellow" } else { "Red" })

Write-Host ""
Write-Host "=== Detailed Results ===" -ForegroundColor Cyan
foreach ($check in $allChecks) {
    $color = switch ($check.Status) {
        "PASS" { "Green" }
        "FAIL" { "Red" }
        "ERROR" { "Magenta" }
    }
    Write-Host "[$($check.Status)] $($check.Name): $($check.Message)" -ForegroundColor $color
}

# Recommendations
Write-Host ""
Write-Host "=== Next Steps ===" -ForegroundColor Yellow

if ($passedChecks -eq $totalChecks) {
    Write-Host "🎉 Phase 1 implementation is complete!" -ForegroundColor Green
    Write-Host ""
    Write-Host "You can now:" -ForegroundColor White
    Write-Host "  1. Run tests: .\scripts\run-tests.ps1" -ForegroundColor Cyan
    Write-Host "  2. Generate coverage: .\scripts\run-tests.ps1 -Coverage" -ForegroundColor Cyan
    Write-Host "  3. Proceed to Phase 2 implementation" -ForegroundColor Cyan
} else {
    Write-Host "Phase 1 implementation needs attention:" -ForegroundColor Yellow
    Write-Host ""
    
    $failedChecks = $allChecks | Where-Object { $_.Status -ne "PASS" }
    foreach ($failed in $failedChecks) {
        Write-Host "  • Fix: $($failed.Name)" -ForegroundColor Red
    }
    
    Write-Host ""
    Write-Host "Refer to docs/phase1-setup-guide.md for detailed instructions" -ForegroundColor Cyan
}

# Exit with appropriate code
exit $(if ($passedChecks -eq $totalChecks) { 0 } else { 1 })
