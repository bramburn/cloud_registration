# Simple Phase 1 Implementation Validation Script

Write-Host "=== Phase 1 Implementation Validator ===" -ForegroundColor Green
Write-Host ""

$passedChecks = 0
$totalChecks = 0

function Check-Item {
    param([string]$Name, [bool]$Result, [string]$Success, [string]$Failure)
    $script:totalChecks++
    Write-Host "Checking $Name..." -ForegroundColor Cyan
    if ($Result) {
        Write-Host "  ✓ $Success" -ForegroundColor Green
        $script:passedChecks++
    } else {
        Write-Host "  ✗ $Failure" -ForegroundColor Red
    }
}

# 1. Check CMakeLists.txt enhancements
$cmakeContent = Get-Content "CMakeLists.txt" -Raw
$cmakeOk = ($cmakeContent -match "enable_testing" -and $cmakeContent -match "ENABLE_COVERAGE")
Check-Item "CMakeLists.txt Integration" $cmakeOk "CMake integration configured" "CMake integration missing"

# 2. Check test files
$testFiles = @("tests/test_e57parser.cpp", "tests/test_lasparser.cpp", "tests/test_voxelgridfilter.cpp", "tests/test_sprint1_functionality.cpp")
foreach ($file in $testFiles) {
    $exists = Test-Path $file
    Check-Item "Test File: $file" $exists "File exists" "File missing"
}

# 3. Check documentation
$docFiles = @("docs/testing-best-practices.md", "docs/phase1-implementation-summary.md", "docs/phase1-setup-guide.md")
foreach ($file in $docFiles) {
    $exists = Test-Path $file
    Check-Item "Documentation: $file" $exists "File exists" "File missing"
}

# 4. Check scripts
$scriptFiles = @("scripts/run-tests.ps1", "scripts/run-tests.sh")
foreach ($file in $scriptFiles) {
    $exists = Test-Path $file
    Check-Item "Script: $file" $exists "File exists" "File missing"
}

# 5. Check build directory
$buildExists = Test-Path "build"
Check-Item "Build Directory" $buildExists "Build directory exists" "Build directory missing"

if ($buildExists) {
    $cmakeConfigured = Test-Path "build/CMakeCache.txt"
    Check-Item "CMake Configuration" $cmakeConfigured "CMake configured" "CMake not configured"

    $ctestExists = Test-Path "build/CTestTestfile.cmake"
    Check-Item "CTest Integration" $ctestExists "CTest integrated" "CTest missing"
}

# 6. Check Google Test
$gtestFound = $false
if (Test-Path "build/CMakeCache.txt") {
    $cacheContent = Get-Content "build/CMakeCache.txt" -Raw
    $gtestFound = ($cacheContent -match "GTest_FOUND:BOOL=TRUE")
}
Check-Item "Google Test" $gtestFound "Google Test found" "Google Test not installed"

# Summary
Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Green
Write-Host "Passed: $passedChecks / $totalChecks" -ForegroundColor White
$percentage = [math]::Round(($passedChecks / $totalChecks) * 100, 1)
Write-Host "Success Rate: $percentage%" -ForegroundColor $(if ($percentage -ge 90) { "Green" } else { "Yellow" })

Write-Host ""
if ($passedChecks -eq $totalChecks) {
    Write-Host "Phase 1 implementation is complete!" -ForegroundColor Green
    Write-Host "Next: Install Google Test and run tests" -ForegroundColor Cyan
} elseif ($percentage -ge 80) {
    Write-Host "Phase 1 implementation is mostly complete!" -ForegroundColor Green
    Write-Host "Only Google Test installation remaining" -ForegroundColor Yellow
} else {
    Write-Host "Phase 1 implementation needs attention" -ForegroundColor Yellow
    Write-Host "See docs/phase1-setup-guide.md for instructions" -ForegroundColor Cyan
}

exit $(if ($passedChecks -eq $totalChecks) { 0 } else { 1 })
