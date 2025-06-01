# Complete E57 Fix Verification Script
# This script provides step-by-step verification of the E57 parsing fix

Write-Host "=== E57 Parsing Fix - Complete Verification ===" -ForegroundColor Green
Write-Host "This script will guide you through verifying the E57 parsing fix" -ForegroundColor Yellow

# Function to display section headers
function Write-Section {
    param($title)
    Write-Host "`n" + "="*60 -ForegroundColor Blue
    Write-Host $title -ForegroundColor Blue
    Write-Host "="*60 -ForegroundColor Blue
}

# Function to wait for user input
function Wait-ForUser {
    param($message = "Press Enter to continue...")
    Write-Host $message -ForegroundColor Yellow
    Read-Host
}

Write-Section "STEP 1: BUILD VERIFICATION"

Write-Host "Building the project with E57 parsing fixes..." -ForegroundColor White

# Stop any running processes
$processes = Get-Process -Name "CloudRegistration" -ErrorAction SilentlyContinue
if ($processes) {
    Write-Host "Stopping existing CloudRegistration processes..." -ForegroundColor Yellow
    $processes | Stop-Process -Force
    Start-Sleep -Seconds 2
}

# Build project
$buildResult = cmake --build build --config Debug 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ BUILD FAILED!" -ForegroundColor Red
    Write-Host $buildResult -ForegroundColor Red
    exit 1
}

Write-Host "✅ Build successful!" -ForegroundColor Green

Write-Section "STEP 2: TEST FILE VERIFICATION"

$testFile = "test_data\test_real_points.e57"
if (Test-Path $testFile) {
    Write-Host "✅ Test file found: $testFile" -ForegroundColor Green
    $fileSize = (Get-Item $testFile).Length
    Write-Host "   File size: $fileSize bytes" -ForegroundColor Cyan
    
    # Analyze file structure
    $bytes = [System.IO.File]::ReadAllBytes($testFile)
    $signature = [BitConverter]::ToUInt32($bytes, 0)
    
    if ($signature -eq 0x41535446) {
        Write-Host "✅ Valid E57 signature detected (0x$($signature.ToString('X8')))" -ForegroundColor Green
    } else {
        Write-Host "❌ Invalid E57 signature" -ForegroundColor Red
    }
} else {
    Write-Host "❌ Test file not found: $testFile" -ForegroundColor Red
    Write-Host "Please run test_e57_fix.ps1 first to create the test file" -ForegroundColor Yellow
    exit 1
}

Write-Section "STEP 3: MANUAL TESTING INSTRUCTIONS"

Write-Host "Now we'll test the E57 parsing fix manually:" -ForegroundColor White
Write-Host ""
Write-Host "1. The application will start automatically" -ForegroundColor Cyan
Write-Host "2. Click 'Open File' button" -ForegroundColor Cyan
Write-Host "3. Navigate to and select: $testFile" -ForegroundColor Cyan
Write-Host "4. Watch the console output carefully" -ForegroundColor Cyan
Write-Host "5. Observe what is displayed in the viewer" -ForegroundColor Cyan

Write-Host "`nWhat to look for in the CONSOLE OUTPUT:" -ForegroundColor Yellow
Write-Host "✅ 'Detected valid E57 file, attempting to parse...'" -ForegroundColor Green
Write-Host "✅ '=== E57Parser::parseXmlSection ==='" -ForegroundColor Green
Write-Host "✅ 'XML parsed successfully'" -ForegroundColor Green
Write-Host "✅ 'Found 1 data3D elements'" -ForegroundColor Green
Write-Host "✅ '=== ATTEMPTING REAL E57 POINT EXTRACTION ==='" -ForegroundColor Green
Write-Host "✅ '=== SUCCESS: REAL E57 DATA EXTRACTED ===' (if successful)" -ForegroundColor Green
Write-Host "✅ 'Sample real E57 coordinates - First point: 1 2 3'" -ForegroundColor Green
Write-Host "✅ 'Successfully extracted 3 points from E57 file'" -ForegroundColor Green

Write-Host "`nWhat to look for in the VIEWER:" -ForegroundColor Yellow
Write-Host "✅ Should see 3 points in a line formation" -ForegroundColor Green
Write-Host "✅ NOT a sphere with 10,000 points" -ForegroundColor Green
Write-Host "✅ Status bar should show 'Successfully loaded 3 points from E57 file'" -ForegroundColor Green

Write-Host "`nIf you see FALLBACK MESSAGES:" -ForegroundColor Yellow
Write-Host "⚠️  '=== FAILED: E57 extraction returned empty, falling back to mock data ==='" -ForegroundColor Red
Write-Host "⚠️  This means the parsing didn't work and mock data is being used" -ForegroundColor Red

Wait-ForUser "Ready to start the application? Press Enter to continue..."

Write-Section "STEP 4: LAUNCHING APPLICATION"

Write-Host "Starting CloudRegistration application..." -ForegroundColor White
Write-Host "Watch the console window that appears for debug output!" -ForegroundColor Yellow

try {
    Start-Process -FilePath ".\build\bin\Debug\CloudRegistration.exe" -WorkingDirectory "."
    Write-Host "✅ Application started successfully" -ForegroundColor Green
} catch {
    Write-Host "❌ Failed to start application: $_" -ForegroundColor Red
    exit 1
}

Write-Section "STEP 5: VERIFICATION CHECKLIST"

Write-Host "Please complete the following verification steps:" -ForegroundColor White
Write-Host ""
Write-Host "□ 1. Application launched without errors" -ForegroundColor Cyan
Write-Host "□ 2. Loaded test file: $testFile" -ForegroundColor Cyan
Write-Host "□ 3. Console shows E57 parsing attempt messages" -ForegroundColor Cyan
Write-Host "□ 4. Console shows either SUCCESS or FAILED messages" -ForegroundColor Cyan
Write-Host "□ 5. Viewer displays appropriate content:" -ForegroundColor Cyan
Write-Host "     - If SUCCESS: 3 points in line (coordinates 1,2,3 to 7,8,9)" -ForegroundColor Green
Write-Host "     - If FAILED: Mock sphere with clear explanation" -ForegroundColor Yellow
Write-Host "□ 6. Status bar shows correct point count" -ForegroundColor Cyan

Write-Host "`n" + "="*60 -ForegroundColor Blue
Write-Host "EXPECTED RESULTS SUMMARY" -ForegroundColor Blue
Write-Host "="*60 -ForegroundColor Blue

Write-Host "`nSUCCESS SCENARIO (E57 parsing works):" -ForegroundColor Green
Write-Host "- Console: '=== SUCCESS: REAL E57 DATA EXTRACTED ==='" -ForegroundColor White
Write-Host "- Viewer: 3 points in a line" -ForegroundColor White
Write-Host "- Status: 'Successfully loaded 3 points from E57 file'" -ForegroundColor White
Write-Host "- This means the fix is working correctly!" -ForegroundColor Green

Write-Host "`nFALLBACK SCENARIO (E57 parsing fails gracefully):" -ForegroundColor Yellow
Write-Host "- Console: '=== FAILED: ..., falling back to mock data ==='" -ForegroundColor White
Write-Host "- Viewer: Sphere with 10,000 points (mock data)" -ForegroundColor White
Write-Host "- Status: 'Generated 10000 mock points'" -ForegroundColor White
Write-Host "- This means parsing failed but fallback works" -ForegroundColor Yellow

Write-Host "`nThe key improvement is that we now ATTEMPT real parsing" -ForegroundColor Cyan
Write-Host "instead of ALWAYS using mock data!" -ForegroundColor Cyan

Write-Host "`n✅ Verification script complete!" -ForegroundColor Green
Write-Host "Please test the application and report your findings." -ForegroundColor Yellow
