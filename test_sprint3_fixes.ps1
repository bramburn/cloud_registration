# Sprint 3 Source Code Fixes Validation Script
# This script tests the fixes applied to resolve compilation issues

Write-Host "=== Sprint 3 Source Code Fixes Validation ===" -ForegroundColor Green

# Check if build directory exists
if (!(Test-Path "build")) {
    Write-Host "Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Name "build"
}

Set-Location "build"

try {
    # Configure CMake
    Write-Host "`nConfiguring CMake..." -ForegroundColor Yellow
    cmake .. -G "Visual Studio 17 2022" -A x64
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }

    # Build the project
    Write-Host "`nBuilding project..." -ForegroundColor Yellow
    cmake --build . --config Release
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }

    Write-Host "`n✅ Build successful!" -ForegroundColor Green

    # Run tests
    Write-Host "`nRunning tests..." -ForegroundColor Yellow
    ctest --output-on-failure -C Release
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "`n✅ All tests passed!" -ForegroundColor Green
    } else {
        Write-Host "`n⚠️ Some tests failed, but build system is working" -ForegroundColor Yellow
    }

    # List available test executables
    Write-Host "`nAvailable test executables:" -ForegroundColor Cyan
    Get-ChildItem -Path "Release" -Filter "*Test*.exe" | ForEach-Object {
        Write-Host "  - $($_.Name)" -ForegroundColor White
    }

} catch {
    Write-Host "`n❌ Error: $($_.Exception.Message)" -ForegroundColor Red
    exit 1
} finally {
    Set-Location ".."
}

Write-Host "`n=== Sprint 3 Fixes Validation Complete ===" -ForegroundColor Green
