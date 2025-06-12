# Sprint 1: Create Directory Structure for Modular Restructure
Write-Host "Creating directory structure for Sprint 1..."

# Create core module directories
New-Item -ItemType Directory -Path "src/core/include/core" -Force | Out-Null
New-Item -ItemType Directory -Path "src/core/src" -Force | Out-Null
Write-Host "✓ Created core module directories"

# Create algorithms module directories  
New-Item -ItemType Directory -Path "src/algorithms/include/algorithms" -Force | Out-Null
New-Item -ItemType Directory -Path "src/algorithms/src" -Force | Out-Null
Write-Host "✓ Created algorithms module directories"

# Create analysis module directories
New-Item -ItemType Directory -Path "src/analysis/include/analysis" -Force | Out-Null
New-Item -ItemType Directory -Path "src/analysis/src" -Force | Out-Null
Write-Host "✓ Created analysis module directories"

# Create app directory
New-Item -ItemType Directory -Path "src/app" -Force | Out-Null
Write-Host "✓ Created app directory"

# Create camera module directories
New-Item -ItemType Directory -Path "src/camera/include/camera" -Force | Out-Null
New-Item -ItemType Directory -Path "src/camera/src" -Force | Out-Null
Write-Host "✓ Created camera module directories"

# Create crs module directories
New-Item -ItemType Directory -Path "src/crs/include/crs" -Force | Out-Null
New-Item -ItemType Directory -Path "src/crs/src" -Force | Out-Null
Write-Host "✓ Created crs module directories"

# Create detection module directories
New-Item -ItemType Directory -Path "src/detection/include/detection" -Force | Out-Null
New-Item -ItemType Directory -Path "src/detection/src" -Force | Out-Null
Write-Host "✓ Created detection module directories"

# Create export module directories
New-Item -ItemType Directory -Path "src/export/include/export" -Force | Out-Null
New-Item -ItemType Directory -Path "src/export/src" -Force | Out-Null
New-Item -ItemType Directory -Path "src/export/FormatWriters/include/FormatWriters" -Force | Out-Null
New-Item -ItemType Directory -Path "src/export/FormatWriters/src" -Force | Out-Null
Write-Host "✓ Created export module directories"

# Create features module directories
New-Item -ItemType Directory -Path "src/features/include/features" -Force | Out-Null
New-Item -ItemType Directory -Path "src/features/src" -Force | Out-Null
Write-Host "✓ Created features module directories"

# Create implementations module directories
New-Item -ItemType Directory -Path "src/implementations/include/implementations" -Force | Out-Null
New-Item -ItemType Directory -Path "src/implementations/src" -Force | Out-Null
Write-Host "✓ Created implementations module directories"

# Create interfaces module directories
New-Item -ItemType Directory -Path "src/interfaces/include/interfaces" -Force | Out-Null
New-Item -ItemType Directory -Path "src/interfaces/src" -Force | Out-Null
Write-Host "✓ Created interfaces module directories"

# Create optimization module directories
New-Item -ItemType Directory -Path "src/optimization/include/optimization" -Force | Out-Null
New-Item -ItemType Directory -Path "src/optimization/src" -Force | Out-Null
Write-Host "✓ Created optimization module directories"

# Create performance module directories
New-Item -ItemType Directory -Path "src/performance/include/performance" -Force | Out-Null
New-Item -ItemType Directory -Path "src/performance/src" -Force | Out-Null
Write-Host "✓ Created performance module directories"

# Create quality module directories
New-Item -ItemType Directory -Path "src/quality/include/quality" -Force | Out-Null
New-Item -ItemType Directory -Path "src/quality/src" -Force | Out-Null
Write-Host "✓ Created quality module directories"

# Create registration module directories
New-Item -ItemType Directory -Path "src/registration/include/registration" -Force | Out-Null
New-Item -ItemType Directory -Path "src/registration/src" -Force | Out-Null
Write-Host "✓ Created registration module directories"

# Create rendering module directories
New-Item -ItemType Directory -Path "src/rendering/include/rendering" -Force | Out-Null
New-Item -ItemType Directory -Path "src/rendering/src" -Force | Out-Null
Write-Host "✓ Created rendering module directories"

# Create ui module directories
New-Item -ItemType Directory -Path "src/ui/include/ui" -Force | Out-Null
New-Item -ItemType Directory -Path "src/ui/src" -Force | Out-Null
Write-Host "✓ Created ui module directories"

# Create parsers module directories (if not already exist)
New-Item -ItemType Directory -Path "src/parsers/include/parsers" -Force | Out-Null
New-Item -ItemType Directory -Path "src/parsers/src" -Force | Out-Null
Write-Host "✓ Created parsers module directories"

Write-Host "✅ All module directories created successfully!"
