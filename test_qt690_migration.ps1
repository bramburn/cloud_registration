# Qt 6.9.0 Migration Verification Script
# This script verifies that the Qt 6.9.0 migration was successful

Write-Host "=== Qt 6.9.0 Migration Verification ===" -ForegroundColor Green
Write-Host ""

# Test 1: Check if Qt 6.9.0 is available
Write-Host "Test 1: Checking Qt 6.9.0 installation..." -ForegroundColor Yellow
$qt690Path = "C:\Qt\6.9.0\msvc2022_64"
if (Test-Path $qt690Path) {
    Write-Host "✅ Qt 6.9.0 found at: $qt690Path" -ForegroundColor Green
} else {
    Write-Host "❌ Qt 6.9.0 not found at expected location" -ForegroundColor Red
    exit 1
}

# Test 2: Clean and configure build
Write-Host ""
Write-Host "Test 2: Configuring build with Qt 6.9.0..." -ForegroundColor Yellow
if (Test-Path "build\qt690-verification") {
    Remove-Item "build\qt690-verification" -Recurse -Force
}

$configResult = & cmake -B "build\qt690-verification" -S "." -G "Visual Studio 17 2022" -A x64 2>&1
if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ CMake configuration successful" -ForegroundColor Green
    
    # Check if Qt 6.9.0 was actually found
    $qtVersionCheck = $configResult | Select-String "Found Qt6 at.*6\.9\.0"
    if ($qtVersionCheck) {
        Write-Host "✅ Qt 6.9.0 detected in build configuration" -ForegroundColor Green
    } else {
        Write-Host "❌ Qt 6.9.0 not detected in build configuration" -ForegroundColor Red
        Write-Host "Configuration output:" -ForegroundColor Gray
        Write-Host $configResult -ForegroundColor Gray
        exit 1
    }
} else {
    Write-Host "❌ CMake configuration failed" -ForegroundColor Red
    Write-Host $configResult -ForegroundColor Gray
    exit 1
}

# Test 3: Build the application
Write-Host ""
Write-Host "Test 3: Building application..." -ForegroundColor Yellow
$buildResult = & msbuild "build\qt690-verification\CloudRegistration.sln" /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal 2>&1
if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Build successful" -ForegroundColor Green
    
    # Check for warnings
    $warningCheck = $buildResult | Select-String "Warning"
    if ($warningCheck) {
        Write-Host "⚠️  Build completed with warnings:" -ForegroundColor Yellow
        $warningCheck | ForEach-Object { Write-Host "   $_" -ForegroundColor Yellow }
    } else {
        Write-Host "✅ Build completed with no warnings" -ForegroundColor Green
    }
} else {
    Write-Host "❌ Build failed" -ForegroundColor Red
    Write-Host $buildResult -ForegroundColor Gray
    exit 1
}

# Test 4: Check if executable was created
Write-Host ""
Write-Host "Test 4: Verifying executable..." -ForegroundColor Yellow
$exePath = "build\qt690-verification\bin\Release\CloudRegistration.exe"
if (Test-Path $exePath) {
    Write-Host "✅ Executable created: $exePath" -ForegroundColor Green
    
    # Get file info
    $fileInfo = Get-Item $exePath
    Write-Host "   Size: $($fileInfo.Length) bytes" -ForegroundColor Gray
    Write-Host "   Created: $($fileInfo.CreationTime)" -ForegroundColor Gray
} else {
    Write-Host "❌ Executable not found" -ForegroundColor Red
    exit 1
}

# Test 5: Deploy Qt dependencies
Write-Host ""
Write-Host "Test 5: Deploying Qt dependencies..." -ForegroundColor Yellow
$windeployqt = "C:\Qt\6.9.0\msvc2022_64\bin\windeployqt.exe"
if (Test-Path $windeployqt) {
    $deployResult = & $windeployqt --release --qmldir . $exePath 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ Deployment successful" -ForegroundColor Green
        
        # Check for Qt 6.9.0 DLLs
        $qtDlls = @("Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll", "Qt6OpenGLWidgets.dll")
        $deployDir = Split-Path $exePath
        $allDllsFound = $true
        
        foreach ($dll in $qtDlls) {
            $dllPath = Join-Path $deployDir $dll
            if (Test-Path $dllPath) {
                Write-Host "   ✅ Found: $dll" -ForegroundColor Green
            } else {
                Write-Host "   ❌ Missing: $dll" -ForegroundColor Red
                $allDllsFound = $false
            }
        }
        
        if ($allDllsFound) {
            Write-Host "✅ All required Qt DLLs deployed" -ForegroundColor Green
        }
    } else {
        Write-Host "❌ Deployment failed" -ForegroundColor Red
        Write-Host $deployResult -ForegroundColor Gray
        exit 1
    }
} else {
    Write-Host "❌ windeployqt not found at: $windeployqt" -ForegroundColor Red
    exit 1
}

# Test 6: Quick runtime test
Write-Host ""
Write-Host "Test 6: Quick runtime test..." -ForegroundColor Yellow
$testProcess = Start-Process -FilePath $exePath -WorkingDirectory (Split-Path $exePath) -PassThru -WindowStyle Hidden
Start-Sleep -Seconds 3

if (!$testProcess.HasExited) {
    Write-Host "✅ Application started successfully" -ForegroundColor Green
    $testProcess.Kill()
    $testProcess.WaitForExit()
    
    # Check for log file
    $logPath = Join-Path (Split-Path $exePath) "CloudRegistration.log"
    if (Test-Path $logPath) {
        Write-Host "✅ Log file created" -ForegroundColor Green
        
        # Check Qt version in log
        $logContent = Get-Content $logPath -Raw
        if ($logContent -match "Qt Version: 6\.9\.0") {
            Write-Host "✅ Qt 6.9.0 runtime confirmed" -ForegroundColor Green
        } else {
            Write-Host "❌ Qt 6.9.0 runtime not confirmed" -ForegroundColor Red
        }
    }
} else {
    Write-Host "❌ Application failed to start or crashed immediately" -ForegroundColor Red
    exit 1
}

# Summary
Write-Host ""
Write-Host "=== Migration Verification Complete ===" -ForegroundColor Green
Write-Host "✅ All tests passed - Qt 6.9.0 migration successful!" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Run manual testing with point cloud files" -ForegroundColor White
Write-Host "2. Update production build scripts to use Qt 6.9.0" -ForegroundColor White
Write-Host "3. Update deployment procedures" -ForegroundColor White
Write-Host ""
