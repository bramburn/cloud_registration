Write-Host "=== Sprint 2 Implementation Check ===" -ForegroundColor Cyan

# Check key files
Write-Host "Checking files..." -ForegroundColor Yellow
if (Test-Path "src/voxelgridfilter.h") { Write-Host "VoxelGridFilter header: OK" -ForegroundColor Green } else { Write-Host "VoxelGridFilter header: MISSING" -ForegroundColor Red }
if (Test-Path "src/voxelgridfilter.cpp") { Write-Host "VoxelGridFilter impl: OK" -ForegroundColor Green } else { Write-Host "VoxelGridFilter impl: MISSING" -ForegroundColor Red }

# Check enum
Write-Host "Checking LoadingSettings enum..." -ForegroundColor Yellow
$content = Get-Content "src/loadingsettings.h" -Raw
if ($content -match "VoxelGrid") { Write-Host "VoxelGrid enum: OK" -ForegroundColor Green } else { Write-Host "VoxelGrid enum: MISSING" -ForegroundColor Red }

# Check integration
Write-Host "Checking LasParser integration..." -ForegroundColor Yellow
$content = Get-Content "src/lasparser.cpp" -Raw
if ($content -match "VoxelGridFilter") { Write-Host "LasParser integration: OK" -ForegroundColor Green } else { Write-Host "LasParser integration: MISSING" -ForegroundColor Red }

# Check UI
Write-Host "Checking UI controls..." -ForegroundColor Yellow
$content = Get-Content "src/loadingsettingsdialog.h" -Raw
if ($content -match "m_leafSizeSpinBox") { Write-Host "Leaf Size control: OK" -ForegroundColor Green } else { Write-Host "Leaf Size control: MISSING" -ForegroundColor Red }
if ($content -match "m_minPointsSpinBox") { Write-Host "Min Points control: OK" -ForegroundColor Green } else { Write-Host "Min Points control: MISSING" -ForegroundColor Red }

# Check build
Write-Host "Checking build..." -ForegroundColor Yellow
if (Test-Path "build/debug/bin/Debug/CloudRegistration.exe") { Write-Host "Application build: OK" -ForegroundColor Green } else { Write-Host "Application build: MISSING" -ForegroundColor Red }

Write-Host "Sprint 2 implementation check complete!" -ForegroundColor Cyan
