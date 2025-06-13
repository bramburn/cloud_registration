# Sprint 1: Create Test Directory Structure for Modular Restructure
Write-Host "Creating test directory structure for Sprint 1..."

# Create test directories for each module
New-Item -ItemType Directory -Path "tests/algorithms" -Force | Out-Null
Write-Host "Created tests/algorithms directory"

New-Item -ItemType Directory -Path "tests/analysis" -Force | Out-Null
Write-Host "Created tests/analysis directory"

New-Item -ItemType Directory -Path "tests/app" -Force | Out-Null
Write-Host "Created tests/app directory"

New-Item -ItemType Directory -Path "tests/camera" -Force | Out-Null
Write-Host "Created tests/camera directory"

New-Item -ItemType Directory -Path "tests/core" -Force | Out-Null
Write-Host "Created tests/core directory"

New-Item -ItemType Directory -Path "tests/crs" -Force | Out-Null
Write-Host "Created tests/crs directory"

New-Item -ItemType Directory -Path "tests/detection" -Force | Out-Null
Write-Host "Created tests/detection directory"

New-Item -ItemType Directory -Path "tests/export" -Force | Out-Null
Write-Host "Created tests/export directory"

New-Item -ItemType Directory -Path "tests/features" -Force | Out-Null
Write-Host "Created tests/features directory"

New-Item -ItemType Directory -Path "tests/implementations" -Force | Out-Null
Write-Host "Created tests/implementations directory"

New-Item -ItemType Directory -Path "tests/interfaces" -Force | Out-Null
Write-Host "Created tests/interfaces directory"

New-Item -ItemType Directory -Path "tests/optimization" -Force | Out-Null
Write-Host "Created tests/optimization directory"

New-Item -ItemType Directory -Path "tests/parsers" -Force | Out-Null
Write-Host "Created tests/parsers directory"

New-Item -ItemType Directory -Path "tests/performance" -Force | Out-Null
Write-Host "Created tests/performance directory"

New-Item -ItemType Directory -Path "tests/quality" -Force | Out-Null
Write-Host "Created tests/quality directory"

New-Item -ItemType Directory -Path "tests/registration" -Force | Out-Null
Write-Host "Created tests/registration directory"

New-Item -ItemType Directory -Path "tests/rendering" -Force | Out-Null
Write-Host "Created tests/rendering directory"

New-Item -ItemType Directory -Path "tests/ui" -Force | Out-Null
Write-Host "Created tests/ui directory"

# Ensure mocks directory exists (already present but confirm)
New-Item -ItemType Directory -Path "tests/mocks" -Force | Out-Null
Write-Host "Confirmed tests/mocks directory"

Write-Host "All test directories created successfully!"
