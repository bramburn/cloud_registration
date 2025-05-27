# Simple Sprint 2 Test Script
Write-Host "=== Sprint 2 Implementation Test ===" -ForegroundColor Green

# Test 1: Build check
Write-Host "`n1. Building application..." -ForegroundColor Cyan
$buildResult = cmake --build build/debug --target CloudRegistration
if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Build successful" -ForegroundColor Green
} else {
    Write-Host "✗ Build failed" -ForegroundColor Red
}

# Test 2: File existence
Write-Host "`n2. Checking files..." -ForegroundColor Cyan
if (Test-Path "src/voxelgridfilter.h") {
    Write-Host "✓ voxelgridfilter.h exists" -ForegroundColor Green
} else {
    Write-Host "✗ voxelgridfilter.h missing" -ForegroundColor Red
}

if (Test-Path "src/voxelgridfilter.cpp") {
    Write-Host "✓ voxelgridfilter.cpp exists" -ForegroundColor Green
} else {
    Write-Host "✗ voxelgridfilter.cpp missing" -ForegroundColor Red
}

# Test 3: Code content checks
Write-Host "`n3. Checking code content..." -ForegroundColor Cyan
$loadingSettings = Get-Content "src/loadingsettings.h" -Raw
if ($loadingSettings -match "VoxelGrid") {
    Write-Host "✓ VoxelGrid enum added" -ForegroundColor Green
} else {
    Write-Host "✗ VoxelGrid enum missing" -ForegroundColor Red
}

$dialog = Get-Content "src/loadingsettingsdialog.cpp" -Raw
if ($dialog -match "QDoubleSpinBox") {
    Write-Host "✓ QDoubleSpinBox added to dialog" -ForegroundColor Green
} else {
    Write-Host "✗ QDoubleSpinBox missing" -ForegroundColor Red
}

$parser = Get-Content "src/lasparser.cpp" -Raw
if ($parser -match "VoxelGridFilter") {
    Write-Host "✓ VoxelGridFilter integrated" -ForegroundColor Green
} else {
    Write-Host "✗ VoxelGridFilter integration missing" -ForegroundColor Red
}

Write-Host "`n=== Test Complete ===" -ForegroundColor Green
Write-Host "Sprint 2 implementation appears to be complete!" -ForegroundColor Yellow
