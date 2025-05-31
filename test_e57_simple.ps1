# Simple E57 Parsing Test
Write-Host "=== Simple E57 Parsing Test ===" -ForegroundColor Green

# Stop any running CloudRegistration processes
$processes = Get-Process -Name "CloudRegistration" -ErrorAction SilentlyContinue
if ($processes) {
    Write-Host "Stopping existing processes..." -ForegroundColor Yellow
    $processes | Stop-Process -Force
    Start-Sleep -Seconds 2
}

# Build the project
Write-Host "Building project..." -ForegroundColor Yellow
$buildResult = cmake --build build --config Debug 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}
Write-Host "Build successful!" -ForegroundColor Green

# Check if our test file exists
$testFile = "test_data\test_real_points.e57"
if (Test-Path $testFile) {
    Write-Host "Test file found: $testFile" -ForegroundColor Green
    $fileSize = (Get-Item $testFile).Length
    Write-Host "File size: $fileSize bytes" -ForegroundColor Cyan
} else {
    Write-Host "Test file not found: $testFile" -ForegroundColor Red
    Write-Host "Creating a simple test file..." -ForegroundColor Yellow
    
    # Create a minimal E57 file
    $bytes = @()
    # E57 signature "ASTF"
    $bytes += 0x46, 0x54, 0x53, 0x41
    # Version 1.0
    $bytes += 0x01, 0x00, 0x00, 0x00
    $bytes += 0x00, 0x00, 0x00, 0x00
    # File length (48 bytes header only for now)
    $bytes += 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    # XML length (0 for now)
    $bytes += 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    # XML offset (48)
    $bytes += 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    # Page size (1024)
    $bytes += 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    # Reserved
    $bytes += 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    
    [System.IO.File]::WriteAllBytes($testFile, $bytes)
    Write-Host "Created minimal test file: $testFile" -ForegroundColor Green
}

Write-Host "`n=== Test Instructions ===" -ForegroundColor Yellow
Write-Host "1. Run: .\build\bin\Debug\CloudRegistration.exe" -ForegroundColor White
Write-Host "2. Load file: $testFile" -ForegroundColor White
Write-Host "3. Watch console output for E57 parsing messages" -ForegroundColor White
Write-Host "`nLook for these messages:" -ForegroundColor Green
Write-Host "- 'Detected valid E57 file, attempting to parse...'" -ForegroundColor Cyan
Write-Host "- '=== ATTEMPTING REAL E57 POINT EXTRACTION ===' (if XML parsing succeeds)" -ForegroundColor Cyan
Write-Host "- '=== SUCCESS: REAL E57 DATA EXTRACTED ===' (if binary extraction works)" -ForegroundColor Cyan
Write-Host "- OR fallback messages indicating mock data generation" -ForegroundColor Cyan

Write-Host "`n=== Expected Outcome ===" -ForegroundColor Yellow
Write-Host "If E57 parsing works: Display actual point cloud data instead of sphere" -ForegroundColor Green
Write-Host "If E57 parsing fails: Display mock sphere with clear debug messages explaining why" -ForegroundColor Yellow

Write-Host "`nStarting application..." -ForegroundColor Green
Start-Process -FilePath ".\build\bin\Debug\CloudRegistration.exe" -WorkingDirectory "."
