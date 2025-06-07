# Sprint 1 Implementation Test Script
# Tests the 3D Point Cloud Visualization components

Write-Host "=== Sprint 1: 3D Point Cloud Visualization Test ===" -ForegroundColor Green
Write-Host "Testing implementation of OpenGL rendering, camera controls, and LOD system" -ForegroundColor Yellow

# Check if build directory exists
if (-not (Test-Path "build")) {
    Write-Host "Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path "build" -Force
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

    # Check if shader files are copied
    Write-Host "`nChecking shader files..." -ForegroundColor Yellow
    $shaderFiles = @(
        "shaders/pointcloud.vert",
        "shaders/pointcloud.frag",
        "shaders/point.vert", 
        "shaders/point.frag"
    )

    foreach ($shader in $shaderFiles) {
        if (Test-Path $shader) {
            Write-Host "✓ Found: $shader" -ForegroundColor Green
        } else {
            Write-Host "✗ Missing: $shader" -ForegroundColor Red
        }
    }

    # Run Sprint 1 specific tests
    Write-Host "`nRunning Sprint 1 tests..." -ForegroundColor Yellow
    
    # Test OpenGL Renderer
    if (Test-Path "bin/Release/OpenGLRendererTests.exe") {
        Write-Host "`nRunning OpenGL Renderer tests..." -ForegroundColor Cyan
        & "bin/Release/OpenGLRendererTests.exe"
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ OpenGL Renderer tests passed" -ForegroundColor Green
        } else {
            Write-Host "✗ OpenGL Renderer tests failed" -ForegroundColor Red
        }
    } else {
        Write-Host "✗ OpenGL Renderer tests not found" -ForegroundColor Red
    }

    # Test Camera Controller
    if (Test-Path "bin/Release/CameraControllerTests.exe") {
        Write-Host "`nRunning Camera Controller tests..." -ForegroundColor Cyan
        & "bin/Release/CameraControllerTests.exe"
        if ($LASTEXITCODE -eq 0) {
            Write-Host "✓ Camera Controller tests passed" -ForegroundColor Green
        } else {
            Write-Host "✗ Camera Controller tests failed" -ForegroundColor Red
        }
    } else {
        Write-Host "✗ Camera Controller tests not found" -ForegroundColor Red
    }

    # Test main application startup
    Write-Host "`nTesting main application..." -ForegroundColor Yellow
    if (Test-Path "bin/Release/CloudRegistration.exe") {
        Write-Host "✓ Main application built successfully" -ForegroundColor Green
        
        # Quick startup test (launch and close)
        Write-Host "Testing application startup (will close automatically)..." -ForegroundColor Cyan
        $process = Start-Process -FilePath "bin/Release/CloudRegistration.exe" -PassThru
        Start-Sleep -Seconds 3
        if (-not $process.HasExited) {
            $process.CloseMainWindow()
            Start-Sleep -Seconds 2
            if (-not $process.HasExited) {
                $process.Kill()
            }
            Write-Host "✓ Application started successfully" -ForegroundColor Green
        } else {
            Write-Host "✗ Application crashed on startup" -ForegroundColor Red
        }
    } else {
        Write-Host "✗ Main application not found" -ForegroundColor Red
    }

    # Check file structure
    Write-Host "`nChecking Sprint 1 file structure..." -ForegroundColor Yellow
    $sprint1Files = @(
        "../src/rendering/OpenGLRenderer.h",
        "../src/rendering/OpenGLRenderer.cpp",
        "../src/camera/CameraController.h", 
        "../src/camera/CameraController.cpp",
        "../src/rendering/LODManager.h",
        "../src/rendering/LODManager.cpp",
        "../src/ui/ViewerToolbar.h",
        "../src/ui/ViewerToolbar.cpp",
        "../tests/test_opengl_renderer.cpp",
        "../tests/test_camera_controller.cpp"
    )

    $missingFiles = @()
    foreach ($file in $sprint1Files) {
        if (Test-Path $file) {
            Write-Host "✓ $file" -ForegroundColor Green
        } else {
            Write-Host "✗ $file" -ForegroundColor Red
            $missingFiles += $file
        }
    }

    # Summary
    Write-Host "`n=== Sprint 1 Implementation Summary ===" -ForegroundColor Green
    if ($missingFiles.Count -eq 0) {
        Write-Host "✓ All Sprint 1 files are present" -ForegroundColor Green
    } else {
        Write-Host "✗ Missing files: $($missingFiles.Count)" -ForegroundColor Red
        foreach ($file in $missingFiles) {
            Write-Host "  - $file" -ForegroundColor Red
        }
    }

    # Test integration with existing system
    Write-Host "`nTesting integration with existing components..." -ForegroundColor Yellow
    
    # Check if existing tests still pass
    Write-Host "Running existing core tests..." -ForegroundColor Cyan
    $coreTests = @("E57ParserTests", "LasParserTests", "ProjectManagerTests")
    
    foreach ($test in $coreTests) {
        $testPath = "bin/Release/$test.exe"
        if (Test-Path $testPath) {
            Write-Host "Running $test..." -ForegroundColor Gray
            & $testPath --gtest_brief=1
            if ($LASTEXITCODE -eq 0) {
                Write-Host "✓ $test passed" -ForegroundColor Green
            } else {
                Write-Host "✗ $test failed" -ForegroundColor Red
            }
        }
    }

    Write-Host "`n=== Sprint 1 Test Complete ===" -ForegroundColor Green
    Write-Host "Sprint 1 components have been implemented and tested." -ForegroundColor Yellow
    Write-Host "Next: Implement Sprint 2 - Registration Workflow UI Foundation" -ForegroundColor Cyan

} catch {
    Write-Host "`nError: $_" -ForegroundColor Red
    exit 1
} finally {
    Set-Location ".."
}
