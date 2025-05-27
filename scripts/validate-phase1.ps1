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

function Test-Check {
    param(
        [string]$Name,
        [bool]$Result,
        [string]$SuccessMessage,
        [string]$FailureMessage
    )

    $script:totalChecks++
    Write-Host "Checking $Name..." -ForegroundColor Cyan

    if ($Result) {
        Write-Host "  ✓ $SuccessMessage" -ForegroundColor Green
        $script:passedChecks++
        $script:allChecks += @{Name=$Name; Status="PASS"; Message=$SuccessMessage}
    } else {
        Write-Host "  ✗ $FailureMessage" -ForegroundColor Red
        $script:allChecks += @{Name=$Name; Status="FAIL"; Message=$FailureMessage}
    }
}

# 1. Check CMakeLists.txt enhancements
$cmakeContent = Get-Content "CMakeLists.txt" -Raw
$cmakeIntegration = ($cmakeContent -match "enable_testing\(\)" -and
                    $cmakeContent -match "ENABLE_COVERAGE" -and
                    $cmakeContent -match "add_test\(NAME.*COMMAND" -and
                    $cmakeContent -match "run_tests")
Test-Check "CMakeLists.txt Integration" $cmakeIntegration "CMake integration properly configured" "CMake integration missing or incomplete"

# 2. Check test files exist
$testFiles = @(
    "tests/test_e57parser.cpp",
    "tests/test_lasparser.cpp",
    "tests/test_voxelgridfilter.cpp",
    "tests/test_sprint1_functionality.cpp"
)

foreach ($testFile in $testFiles) {
    $exists = Test-Path $testFile
    Test-Check "Test File: $testFile" $exists "Test file exists" "Test file missing"
}

# 3. Check test file structure and content
$e57Content = Get-Content "tests/test_e57parser.cpp" -Raw
$e57Structure = ($e57Content -match "class E57ParserTest.*::testing::Test" -and
                $e57Content -match "TEST_F\(E57ParserTest" -and
                $e57Content -match "SetUp\(\)" -and
                $e57Content -match "TearDown\(\)")
Test-Check "E57Parser Test Structure" $e57Structure "E57Parser tests properly structured" "E57Parser test structure incomplete"

$lasContent = Get-Content "tests/test_lasparser.cpp" -Raw
$lasStructure = ($lasContent -match "class LasParserTest.*::testing::Test" -and
                $lasContent -match "TEST_F\(LasParserTest" -and
                $lasContent -match "createMockLasFile")
Test-Check "LasParser Test Structure" $lasStructure "LasParser tests properly structured" "LasParser test structure incomplete"

$voxelContent = Get-Content "tests/test_voxelgridfilter.cpp" -Raw
$voxelStructure = ($voxelContent -match "class VoxelGridFilterTest.*::testing::Test" -and
                  $voxelContent -match "TEST_F\(VoxelGridFilterTest" -and
                  $voxelContent -match "EmptyInput|SinglePoint|Performance")
Test-Check "VoxelGridFilter Test Structure" $voxelStructure "VoxelGridFilter tests properly structured" "VoxelGridFilter test structure incomplete"

# 4. Check documentation files
$docFiles = @(
    "docs/testing-best-practices.md",
    "docs/phase1-implementation-summary.md",
    "docs/phase1-setup-guide.md"
)

foreach ($docFile in $docFiles) {
    $exists = Test-Path $docFile
    Test-Check "Documentation: $docFile" $exists "Documentation file exists" "Documentation file missing"
}

# 5. Check script files
$scriptFiles = @(
    "scripts/run-tests.ps1",
    "scripts/run-tests.sh"
)

foreach ($scriptFile in $scriptFiles) {
    $exists = Test-Path $scriptFile
    Test-Check "Script: $scriptFile" $exists "Script file exists" "Script file missing"
}

# 6. Check script functionality
$psContent = Get-Content "scripts/run-tests.ps1" -Raw
$psFunction = ($psContent -match "param\(" -and
              $psContent -match "ctest" -and
              $psContent -match "Coverage" -and
              $psContent -match "Show-Help")
Test-Check "PowerShell Script Functionality" $psFunction "PowerShell script properly implemented" "PowerShell script incomplete"

$bashContent = Get-Content "scripts/run-tests.sh" -Raw
$bashFunction = ($bashContent -match "#!/bin/bash" -and
                $bashContent -match "ctest" -and
                $bashContent -match "coverage" -and
                $bashContent -match "show_help")
Test-Check "Bash Script Functionality" $bashFunction "Bash script properly implemented" "Bash script incomplete"

# 7. Check build directory and configuration
$buildExists = Test-Path "build"
Test-Check "Build Directory" $buildExists "Build directory exists" "Build directory missing - run cmake configuration"

if (Test-Path "build") {
    $cmakeConfigured = Test-Path "build/CMakeCache.txt"
    Test-Check "CMake Configuration" $cmakeConfigured "CMake has been configured" "CMake not configured - run cmake -B build -S ."

    $ctestIntegration = Test-Path "build/CTestTestfile.cmake"
    Test-Check "CTest Integration" $ctestIntegration "CTest integration active" "CTest integration missing"
}

# 8. Check Google Test availability
$gtestFound = $false
if (Test-Path "build/CMakeCache.txt") {
    $cacheContent = Get-Content "build/CMakeCache.txt" -Raw
    $gtestFound = ($cacheContent -match "GTest_FOUND:BOOL=TRUE")
}
Test-Check "Google Test Installation" $gtestFound "Google Test found and configured" "Google Test not installed - see setup guide"

# 9. Check source file integration
$sourceIntegration = ($cmakeContent -match "src/e57parser.cpp" -and
                     $cmakeContent -match "src/lasparser.cpp" -and
                     $cmakeContent -match "src/voxelgridfilter.cpp")
Test-Check "Source File Integration" $sourceIntegration "Source files properly integrated" "Source file integration incomplete"

# 10. Check test coverage setup
$coverageConfig = ($cmakeContent -match "ENABLE_COVERAGE" -and
                  $cmakeContent -match "lcov" -and
                  $cmakeContent -match "genhtml")
Test-Check "Coverage Configuration" $coverageConfig "Coverage reporting configured" "Coverage configuration incomplete"

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
