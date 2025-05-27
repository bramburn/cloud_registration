#!/usr/bin/env powershell

# Test script for the debugging implementation from docs/2025-05-27 fix.md
# This script builds the application and tests the debugging features

Write-Host "=== Testing Point Cloud Display Fix Debugging Implementation ===" -ForegroundColor Green

# Check if we're in the right directory
if (-not (Test-Path "CMakeLists.txt")) {
    Write-Host "Error: CMakeLists.txt not found. Please run this script from the project root directory." -ForegroundColor Red
    exit 1
}

# Clean and build the project
Write-Host "Cleaning previous build..." -ForegroundColor Yellow
if (Test-Path "build") {
    Remove-Item -Recurse -Force "build"
}

Write-Host "Creating build directory..." -ForegroundColor Yellow
New-Item -ItemType Directory -Path "build" | Out-Null

Write-Host "Configuring CMake..." -ForegroundColor Yellow
Set-Location "build"
cmake -G "Visual Studio 17 2022" -A x64 ..
if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Set-Location ..
    exit 1
}

Write-Host "Building project..." -ForegroundColor Yellow
cmake --build . --config Debug
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    Set-Location ..
    exit 1
}

Set-Location ..

Write-Host "Build completed successfully!" -ForegroundColor Green

# Check if executable exists
$exePath = "build\bin\Debug\CloudRegistration.exe"
if (-not (Test-Path $exePath)) {
    Write-Host "Error: Executable not found at $exePath" -ForegroundColor Red
    exit 1
}

Write-Host "Executable found at: $exePath" -ForegroundColor Green

# Create a simple test LAS file for testing (mock data)
Write-Host "Creating test data..." -ForegroundColor Yellow
$testDataDir = "test_data"
if (-not (Test-Path $testDataDir)) {
    New-Item -ItemType Directory -Path $testDataDir | Out-Null
}

# Note: We'll test with E57 files since they use mock data generation
# Create a simple test E57 file (just a placeholder - the parser will generate mock data)
$testE57File = "$testDataDir\test_mock.e57"
"E57_MOCK_FILE" | Out-File -FilePath $testE57File -Encoding ASCII

Write-Host "Test data created at: $testE57File" -ForegroundColor Green

Write-Host ""
Write-Host "=== Debugging Implementation Test Summary ===" -ForegroundColor Cyan
Write-Host "1. Enhanced MainWindow::onParsingFinished with comprehensive data flow logging" -ForegroundColor White
Write-Host "2. Enhanced LasParser::parse with detailed parsing and coordinate logging" -ForegroundColor White
Write-Host "3. Enhanced E57Parser::generateMockPointCloud with mock data validation logging" -ForegroundColor White
Write-Host "4. Enhanced PointCloudViewerWidget::loadPointCloud with data reception and GPU upload logging" -ForegroundColor White
Write-Host "5. Enhanced OpenGL methods with comprehensive error checking and state logging" -ForegroundColor White
Write-Host "6. Added detailed uniform location validation in setupShaders" -ForegroundColor White
Write-Host "7. Added comprehensive OpenGL error checking in paintGL method" -ForegroundColor White
Write-Host ""

Write-Host "=== Manual Testing Instructions ===" -ForegroundColor Cyan
Write-Host "1. Run the application: $exePath" -ForegroundColor White
Write-Host "2. Open the test E57 file: $testE57File" -ForegroundColor White
Write-Host "3. Check the console output for detailed debugging information" -ForegroundColor White
Write-Host "4. Look for the following debug sections:" -ForegroundColor White
Write-Host "   - === E57Parser::generateMockPointCloud ===" -ForegroundColor Gray
Write-Host "   - === MainWindow::onParsingFinished ===" -ForegroundColor Gray
Write-Host "   - === PointCloudViewerWidget::loadPointCloud ===" -ForegroundColor Gray
Write-Host "   - OpenGL initialization and error checking logs" -ForegroundColor Gray
Write-Host "   - Bounding box and camera calculation logs" -ForegroundColor Gray
Write-Host "   - paintGL rendering state logs" -ForegroundColor Gray
Write-Host ""

Write-Host "=== Expected Debug Output Sections ===" -ForegroundColor Cyan
Write-Host "When loading an E57 file, you should see:" -ForegroundColor White
Write-Host "1. E57Parser mock data generation with sample coordinates" -ForegroundColor Gray
Write-Host "2. MainWindow data transfer confirmation with point counts" -ForegroundColor Gray
Write-Host "3. PointCloudViewerWidget data reception and bounding box calculation" -ForegroundColor Gray
Write-Host "4. Camera fitting calculations with distance and position" -ForegroundColor Gray
Write-Host "5. OpenGL buffer allocation and vertex attribute setup" -ForegroundColor Gray
Write-Host "6. paintGL rendering calls with point counts and uniform values" -ForegroundColor Gray
Write-Host ""

Write-Host "=== Troubleshooting ===" -ForegroundColor Cyan
Write-Host "If points are still not visible after this debugging implementation:" -ForegroundColor White
Write-Host "1. Check for qCritical() messages indicating OpenGL errors" -ForegroundColor Gray
Write-Host "2. Verify uniform locations are not -1" -ForegroundColor Gray
Write-Host "3. Confirm m_hasData and m_shadersInitialized are both true" -ForegroundColor Gray
Write-Host "4. Check bounding box calculations for valid (non-zero, non-NaN) values" -ForegroundColor Gray
Write-Host "5. Verify camera position is distinct from camera target" -ForegroundColor Gray
Write-Host "6. Ensure point count in glDrawArrays matches expected value" -ForegroundColor Gray
Write-Host ""

Write-Host "Build and setup completed successfully!" -ForegroundColor Green
Write-Host "You can now run the application and test the debugging features." -ForegroundColor Green
