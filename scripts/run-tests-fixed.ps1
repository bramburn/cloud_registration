# Cloud Registration Test Runner Script
# Implements Phase 1 testing infrastructure from PRD

param(
    [string]$BuildDir = "build",
    [string]$Config = "Debug",
    [switch]$Coverage,
    [switch]$Verbose,
    [string]$Filter = "",
    [switch]$Help
)

function Show-Help {
    Write-Host "Cloud Registration Test Runner" -ForegroundColor Green
    Write-Host ""
    Write-Host "Usage: .\scripts\run-tests-fixed.ps1 [options]" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Options:" -ForegroundColor Cyan
    Write-Host "  -BuildDir <path>    Build directory (default: build)"
    Write-Host "  -Config <config>    Build configuration (default: Debug)"
    Write-Host "  -Coverage           Generate code coverage report"
    Write-Host "  -Verbose            Show detailed test output"
    Write-Host "  -Filter <pattern>   Run only tests matching pattern"
    Write-Host "  -Help               Show this help message"
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor Yellow
    Write-Host "  .\scripts\run-tests-fixed.ps1                    # Run all tests"
    Write-Host "  .\scripts\run-tests-fixed.ps1 -Coverage          # Run tests with coverage"
    Write-Host "  .\scripts\run-tests-fixed.ps1 -Filter '*E57*'    # Run only E57 tests"
    Write-Host "  .\scripts\run-tests-fixed.ps1 -Verbose           # Show detailed output"
}

if ($Help) {
    Show-Help
    exit 0
}

Write-Host "=== Cloud Registration Test Runner ===" -ForegroundColor Green
Write-Host "Build Directory: $BuildDir" -ForegroundColor Cyan
Write-Host "Configuration: $Config" -ForegroundColor Cyan

# Check if build directory exists
if (-not (Test-Path $BuildDir)) {
    Write-Host "ERROR: Build directory '$BuildDir' does not exist" -ForegroundColor Red
    Write-Host "Please run cmake to configure the project first" -ForegroundColor Yellow
    exit 1
}

# Change to build directory
$originalLocation = Get-Location
Set-Location $BuildDir

try {
    # Check if Google Test was found during configuration
    $cmakeCache = Get-Content "CMakeCache.txt" -ErrorAction SilentlyContinue
    $gtestFound = $cmakeCache | Select-String "GTest_FOUND:BOOL=TRUE"
    $gtestDirFound = $cmakeCache | Select-String "GTest_DIR.*gtest"

    if (-not $gtestFound -and -not $gtestDirFound) {
        Write-Host "WARNING: Google Test not found during configuration" -ForegroundColor Yellow
        Write-Host "Tests will not be available. Please install Google Test:" -ForegroundColor Yellow
        Write-Host "  - Windows: vcpkg install gtest:x64-windows" -ForegroundColor White
        Write-Host "  - Ubuntu/Debian: sudo apt-get install libgtest-dev" -ForegroundColor White
        Write-Host "See docs/google-test-installation.md for detailed instructions" -ForegroundColor Cyan
        exit 1
    }

    Write-Host "`n1. Building test executables..." -ForegroundColor Cyan

    # Build all test targets
    $testTargets = @("E57ParserTests", "LasParserTests", "VoxelGridFilterTests", "Sprint1FunctionalityTests")

    foreach ($target in $testTargets) {
        Write-Host "Building $target..." -ForegroundColor White
        $buildResult = & cmake --build . --target $target --config $Config 2>&1

        if ($LASTEXITCODE -ne 0) {
            Write-Host "ERROR: Failed to build $target" -ForegroundColor Red
            Write-Host $buildResult
            exit 1
        }
    }

    Write-Host "[PASS] All test executables built successfully" -ForegroundColor Green

    Write-Host "`n2. Running unit tests..." -ForegroundColor Cyan

    # Prepare CTest arguments
    $ctestArgs = @("--output-on-failure", "--config", $Config)

    if ($Verbose) {
        $ctestArgs += "--verbose"
    }

    if ($Filter) {
        $ctestArgs += "-R"
        $ctestArgs += $Filter
    }

    # Run tests using CTest
    Write-Host "Executing: ctest $($ctestArgs -join ' ')" -ForegroundColor White
    $testResult = & ctest @ctestArgs
    $testExitCode = $LASTEXITCODE

    if ($testExitCode -eq 0) {
        Write-Host "[PASS] All tests passed!" -ForegroundColor Green
    } else {
        Write-Host "[FAIL] Some tests failed (exit code: $testExitCode)" -ForegroundColor Red
    }

    # Show test summary
    Write-Host "`n3. Test Summary:" -ForegroundColor Cyan
    $testOutput = $testResult | Out-String

    # Parse test results
    $passedTests = ($testOutput | Select-String "Test #\d+: \w+ \.+ Passed").Count
    $failedTests = ($testOutput | Select-String "Test #\d+: \w+ \.+ Failed").Count
    $totalTests = $passedTests + $failedTests

    Write-Host "Total Tests: $totalTests" -ForegroundColor White
    Write-Host "Passed: $passedTests" -ForegroundColor Green
    Write-Host "Failed: $failedTests" -ForegroundColor $(if ($failedTests -gt 0) { "Red" } else { "Green" })

    # Generate coverage report if requested
    if ($Coverage) {
        Write-Host "`n4. Generating code coverage report..." -ForegroundColor Cyan

        # Check if coverage tools are available
        $lcovPath = Get-Command lcov -ErrorAction SilentlyContinue
        $genhtmlPath = Get-Command genhtml -ErrorAction SilentlyContinue

        if (-not $lcovPath -or -not $genhtmlPath) {
            Write-Host "WARNING: lcov/genhtml not found. Coverage report not available." -ForegroundColor Yellow
            Write-Host "To install on Ubuntu/Debian: sudo apt-get install lcov" -ForegroundColor White
            Write-Host "To install on Windows: Use WSL or install via MSYS2" -ForegroundColor White
        } else {
            # Run coverage target if available
            $coverageResult = & cmake --build . --target coverage --config $Config 2>&1

            if ($LASTEXITCODE -eq 0) {
                Write-Host "[PASS] Coverage report generated successfully" -ForegroundColor Green

                if (Test-Path "coverage_html/index.html") {
                    $coverageFile = Resolve-Path "coverage_html/index.html"
                    Write-Host "Coverage report available at: $coverageFile" -ForegroundColor Cyan

                    # Try to open coverage report in default browser
                    try {
                        Start-Process $coverageFile
                        Write-Host "[PASS] Coverage report opened in browser" -ForegroundColor Green
                    } catch {
                        Write-Host "Note: Could not auto-open coverage report" -ForegroundColor Yellow
                    }
                }
            } else {
                Write-Host "[FAIL] Failed to generate coverage report" -ForegroundColor Red
                Write-Host $coverageResult
            }
        }
    }

    # Individual test execution (for detailed output)
    if ($Verbose -and $testExitCode -ne 0) {
        Write-Host "`n5. Running individual tests for detailed output..." -ForegroundColor Cyan

        foreach ($target in $testTargets) {
            $exePath = ""

            # Find the executable
            $possiblePaths = @(
                "bin/$Config/$target.exe",
                "bin/$target.exe",
                "$Config/$target.exe",
                "$target.exe"
            )

            foreach ($path in $possiblePaths) {
                if (Test-Path $path) {
                    $exePath = $path
                    break
                }
            }

            if ($exePath) {
                Write-Host "`nRunning $target individually:" -ForegroundColor Yellow

                $filterArg = if ($Filter) { "--gtest_filter=$Filter" } else { "" }
                $args = @("--gtest_color=yes")
                if ($filterArg) { $args += $filterArg }

                & $exePath @args
            } else {
                Write-Host "Could not find executable for $target" -ForegroundColor Yellow
            }
        }
    }

    Write-Host "`n=== Test Execution Complete ===" -ForegroundColor Green

    # Set final exit code
    $finalExitCode = $testExitCode

} catch {
    Write-Host "ERROR: An unexpected error occurred: $($_.Exception.Message)" -ForegroundColor Red
    $finalExitCode = 1
} finally {
    # Return to original directory
    Set-Location $originalLocation
}

# Exit with the final exit code
exit $finalExitCode
