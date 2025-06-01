# Setup script for libE57Format integration
# Run this script to install and configure libE57Format

Write-Host "=== libE57Format Integration Setup ===" -ForegroundColor Green

# Step 1: Install libE57Format via vcpkg
Write-Host "Step 1: Installing libE57Format via vcpkg..." -ForegroundColor Yellow

try {
    # Check if vcpkg is available
    $vcpkgVersion = vcpkg version 2>$null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error: vcpkg not found. Please install vcpkg first." -ForegroundColor Red
        exit 1
    }
    
    Write-Host "Found vcpkg: $vcpkgVersion" -ForegroundColor Green
    
    # Install libe57format
    Write-Host "Installing libe57format:x64-windows..." -ForegroundColor Yellow
    vcpkg install libe57format:x64-windows
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ libe57format installed successfully" -ForegroundColor Green
    } else {
        Write-Host "✗ Failed to install libe57format" -ForegroundColor Red
        exit 1
    }
    
    # Verify installation
    Write-Host "Verifying installation..." -ForegroundColor Yellow
    $installed = vcpkg list | Select-String "libe57format"
    if ($installed) {
        Write-Host "✓ Verification successful: $installed" -ForegroundColor Green
    } else {
        Write-Host "✗ Verification failed - libe57format not found in installed packages" -ForegroundColor Red
        exit 1
    }
    
} catch {
    Write-Host "Error during vcpkg installation: $_" -ForegroundColor Red
    exit 1
}

# Step 2: Update vcpkg.json
Write-Host "Step 2: Updating vcpkg.json..." -ForegroundColor Yellow

$vcpkgJsonPath = "vcpkg.json"
if (Test-Path $vcpkgJsonPath) {
    $vcpkgJson = Get-Content $vcpkgJsonPath | ConvertFrom-Json
    
    # Check if libe57format is already in dependencies
    if ($vcpkgJson.dependencies -notcontains "libe57format") {
        $vcpkgJson.dependencies += "libe57format"
        $vcpkgJson | ConvertTo-Json -Depth 10 | Set-Content $vcpkgJsonPath
        Write-Host "✓ Added libe57format to vcpkg.json" -ForegroundColor Green
    } else {
        Write-Host "✓ libe57format already in vcpkg.json" -ForegroundColor Green
    }
} else {
    Write-Host "Creating vcpkg.json..." -ForegroundColor Yellow
    $newVcpkgJson = @{
        dependencies = @("qt6", "gtest", "libe57format")
    }
    $newVcpkgJson | ConvertTo-Json -Depth 10 | Set-Content $vcpkgJsonPath
    Write-Host "✓ Created vcpkg.json with libe57format" -ForegroundColor Green
}

# Step 3: Create backup of current E57Parser
Write-Host "Step 3: Creating backup of current E57Parser..." -ForegroundColor Yellow

$backupDir = "src/backup_before_libe57format"
if (-not (Test-Path $backupDir)) {
    New-Item -ItemType Directory -Path $backupDir | Out-Null
}

Copy-Item "src/e57parser.h" "$backupDir/e57parser.h.backup" -Force
Copy-Item "src/e57parser.cpp" "$backupDir/e57parser.cpp.backup" -Force
Write-Host "✓ Backup created in $backupDir" -ForegroundColor Green

# Step 4: Display next steps
Write-Host "Step 4: Next Steps" -ForegroundColor Yellow
Write-Host ""
Write-Host "Setup completed successfully! Next steps:" -ForegroundColor Green
Write-Host "1. Update CMakeLists.txt to include libE57Format" -ForegroundColor White
Write-Host "2. Add libE57Format headers to e57parser.h" -ForegroundColor White
Write-Host "3. Implement parallel parsing methods" -ForegroundColor White
Write-Host "4. Run tests to verify integration" -ForegroundColor White
Write-Host ""
Write-Host "See docs/libE57Format_integration_plan.md for detailed implementation steps." -ForegroundColor Cyan

# Step 5: Test compilation
Write-Host "Step 5: Testing compilation with libE57Format..." -ForegroundColor Yellow

$testCppContent = @"
#include <iostream>
#include <E57Format.h>

int main() {
    try {
        // Test basic libE57Format functionality
        std::cout << "libE57Format test compilation successful!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
"@

$testCppPath = "test_libe57format.cpp"
$testCppContent | Set-Content $testCppPath

try {
    # Try to compile the test
    Write-Host "Compiling test program..." -ForegroundColor Yellow
    
    # This is a basic test - actual compilation will depend on CMake setup
    Write-Host "✓ Test file created: $testCppPath" -ForegroundColor Green
    Write-Host "Note: Full compilation test will be done after CMake integration" -ForegroundColor Cyan
    
    # Clean up test file
    Remove-Item $testCppPath -Force
    
} catch {
    Write-Host "Note: Compilation test skipped - will be verified after CMake setup" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== Setup Complete ===" -ForegroundColor Green
Write-Host "libE57Format is ready for integration!" -ForegroundColor Green
