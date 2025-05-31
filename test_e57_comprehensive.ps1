# Comprehensive E57 Parsing Fix Test Script
# This script tests the E57 parsing implementation according to the requirements in docs/2025-05-27 21-54-46 Fix.md

Write-Host "=== Comprehensive E57 Parsing Fix Test ===" -ForegroundColor Green
Write-Host "Testing implementation based on User Story 1 requirements" -ForegroundColor Yellow

# Function to kill any running CloudRegistration processes
function Stop-CloudRegistration {
    $processes = Get-Process -Name "CloudRegistration" -ErrorAction SilentlyContinue
    if ($processes) {
        Write-Host "Stopping existing CloudRegistration processes..." -ForegroundColor Yellow
        $processes | Stop-Process -Force
        Start-Sleep -Seconds 2
    }
}

# Function to build the project
function Build-Project {
    Write-Host "Building project..." -ForegroundColor Yellow
    
    # Stop any running instances first
    Stop-CloudRegistration
    
    $buildResult = cmake --build build --config Debug 2>&1
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed!" -ForegroundColor Red
        Write-Host $buildResult -ForegroundColor Red
        return $false
    }
    Write-Host "Build successful!" -ForegroundColor Green
    return $true
}

# Function to create test E57 files
function Create-TestFiles {
    Write-Host "Creating test E57 files..." -ForegroundColor Yellow
    
    # Test file 1: Simple valid E57 with 3 points
    $testFile1 = "test_data\test_simple_3points.e57"
    
    # Create binary content for E57 file
    $bytes = @()
    
    # E57 Header (48 bytes)
    $bytes += 0x46, 0x54, 0x53, 0x41  # "ASTF" signature
    $bytes += 0x01, 0x00, 0x00, 0x00  # Major version 1
    $bytes += 0x00, 0x00, 0x00, 0x00  # Minor version 0
    
    # File length placeholder (will be updated)
    $fileLengthPos = 12
    $bytes += 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    
    # XML length placeholder (will be updated)
    $xmlLengthPos = 20
    $bytes += 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    
    # XML offset placeholder (will be updated)
    $xmlOffsetPos = 28
    $bytes += 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    
    # Page size (1024)
    $bytes += 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    
    # Reserved (8 bytes)
    $bytes += 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    
    # XML content with proper E57 structure
    $xmlContent = @"
<?xml version="1.0" encoding="UTF-8"?>
<e57Root type="Structure" xmlns="http://www.astm.org/COMMIT/E57/2010-e57-v1.0">
    <formatName type="String">ASTM E57 3D Imaging Data File</formatName>
    <guid type="String">{12345678-1234-1234-1234-123456789012}</guid>
    <versionMajor type="Integer">1</versionMajor>
    <versionMinor type="Integer">0</versionMinor>
    <data3D type="Vector" allowHeterogeneousChildren="1">
        <vectorChild type="Structure">
            <guid type="String">{87654321-4321-4321-4321-210987654321}</guid>
            <name type="String">Test 3 Points</name>
            <points type="CompressedVector" recordCount="3">
                <prototype type="Structure">
                    <cartesianX type="Float" precision="single"/>
                    <cartesianY type="Float" precision="single"/>
                    <cartesianZ type="Float" precision="single"/>
                </prototype>
                <codecs type="Vector">
                    <vectorChild type="CompressedVectorNode">
                        <recordCount type="Integer">3</recordCount>
                        <binarySection type="String">test_binary_section</binarySection>
                    </vectorChild>
                </codecs>
            </points>
        </vectorChild>
    </data3D>
</e57Root>
"@
    
    $xmlBytes = [System.Text.Encoding]::UTF8.GetBytes($xmlContent)
    
    # Calculate positions
    $xmlOffset = 48
    $xmlLength = $xmlBytes.Length
    $binaryOffset = $xmlOffset + $xmlLength
    $fileLength = $binaryOffset + 36  # 3 points * 3 coords * 4 bytes
    
    # Update header with actual values
    $fileLengthBytes = [BitConverter]::GetBytes([uint64]$fileLength)
    for ($i = 0; $i -lt 8; $i++) { $bytes[$fileLengthPos + $i] = $fileLengthBytes[$i] }
    
    $xmlLengthBytes = [BitConverter]::GetBytes([uint64]$xmlLength)
    for ($i = 0; $i -lt 8; $i++) { $bytes[$xmlLengthPos + $i] = $xmlLengthBytes[$i] }
    
    $xmlOffsetBytes = [BitConverter]::GetBytes([uint64]$xmlOffset)
    for ($i = 0; $i -lt 8; $i++) { $bytes[$xmlOffsetPos + $i] = $xmlOffsetBytes[$i] }
    
    # Combine header + XML + binary data
    $allBytes = $bytes + $xmlBytes
    
    # Add test points: (1,2,3), (4,5,6), (7,8,9)
    $allBytes += [BitConverter]::GetBytes([float]1.0)
    $allBytes += [BitConverter]::GetBytes([float]2.0)
    $allBytes += [BitConverter]::GetBytes([float]3.0)
    $allBytes += [BitConverter]::GetBytes([float]4.0)
    $allBytes += [BitConverter]::GetBytes([float]5.0)
    $allBytes += [BitConverter]::GetBytes([float]6.0)
    $allBytes += [BitConverter]::GetBytes([float]7.0)
    $allBytes += [BitConverter]::GetBytes([float]8.0)
    $allBytes += [BitConverter]::GetBytes([float]9.0)
    
    # Write test file
    [System.IO.File]::WriteAllBytes($testFile1, $allBytes)
    
    Write-Host "Created test file: $testFile1" -ForegroundColor Green
    Write-Host "File size: $([System.IO.File]::ReadAllBytes($testFile1).Length) bytes" -ForegroundColor Cyan
    Write-Host "Expected points: 3 points at (1,2,3), (4,5,6), (7,8,9)" -ForegroundColor Cyan
    
    return $testFile1
}

# Function to analyze E57 file structure
function Analyze-E57File {
    param($filePath)
    
    Write-Host "`nAnalyzing E57 file: $filePath" -ForegroundColor Yellow
    
    if (-not (Test-Path $filePath)) {
        Write-Host "File not found: $filePath" -ForegroundColor Red
        return $false
    }
    
    $bytes = [System.IO.File]::ReadAllBytes($filePath)
    
    # Check signature
    $signature = [BitConverter]::ToUInt32($bytes, 0)
    Write-Host "Signature: 0x$($signature.ToString('X8'))" -ForegroundColor Cyan
    
    if ($signature -eq 0x41535446) {
        Write-Host "✓ Valid E57 signature detected" -ForegroundColor Green
    } else {
        Write-Host "✗ Invalid E57 signature" -ForegroundColor Red
        return $false
    }
    
    # Check version
    $majorVersion = [BitConverter]::ToUInt32($bytes, 4)
    $minorVersion = [BitConverter]::ToUInt32($bytes, 8)
    Write-Host "Version: $majorVersion.$minorVersion" -ForegroundColor Cyan
    
    # Check XML section
    $xmlLength = [BitConverter]::ToUInt64($bytes, 20)
    $xmlOffset = [BitConverter]::ToUInt64($bytes, 28)
    Write-Host "XML offset: $xmlOffset, length: $xmlLength" -ForegroundColor Cyan
    
    if ($xmlOffset -lt $bytes.Length -and ($xmlOffset + $xmlLength) -le $bytes.Length) {
        $xmlBytes = $bytes[$xmlOffset..($xmlOffset + $xmlLength - 1)]
        $xmlContent = [System.Text.Encoding]::UTF8.GetString($xmlBytes)
        
        if ($xmlContent -match "cartesianX" -and $xmlContent -match "cartesianY" -and $xmlContent -match "cartesianZ") {
            Write-Host "✓ Valid E57 XML structure with XYZ coordinates" -ForegroundColor Green
        } else {
            Write-Host "✗ Missing XYZ coordinate structure in XML" -ForegroundColor Red
        }
        
        if ($xmlContent -match "recordCount") {
            Write-Host "✓ Found recordCount in XML" -ForegroundColor Green
        } else {
            Write-Host "✗ Missing recordCount in XML" -ForegroundColor Red
        }
    } else {
        Write-Host "✗ Invalid XML section parameters" -ForegroundColor Red
        return $false
    }
    
    return $true
}

# Main test execution
Write-Host "Starting comprehensive E57 parsing test..." -ForegroundColor Green

# Step 1: Build project
if (-not (Build-Project)) {
    Write-Host "Cannot proceed - build failed" -ForegroundColor Red
    exit 1
}

# Step 2: Create test files
$testFile = Create-TestFiles

# Step 3: Analyze test file
if (-not (Analyze-E57File $testFile)) {
    Write-Host "Test file analysis failed" -ForegroundColor Red
    exit 1
}

Write-Host "`n=== Test File Ready ===" -ForegroundColor Green
Write-Host "Test file created: $testFile" -ForegroundColor Yellow
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Run the CloudRegistration application" -ForegroundColor White
Write-Host "2. Load the test file: $testFile" -ForegroundColor White
Write-Host "3. Check console output for E57 parsing messages" -ForegroundColor White
Write-Host "4. Verify if actual points (1,2,3), (4,5,6), (7,8,9) are displayed instead of mock sphere" -ForegroundColor White

Write-Host "`nExpected behavior:" -ForegroundColor Green
Write-Host "- Should see '=== ATTEMPTING REAL E57 POINT EXTRACTION ===' in console" -ForegroundColor Cyan
Write-Host "- Should see '=== SUCCESS: REAL E57 DATA EXTRACTED ===' if parsing works" -ForegroundColor Cyan
Write-Host "- Should display 3 points in a line instead of 10,000 point sphere" -ForegroundColor Cyan
Write-Host "- Status should show 'Successfully loaded 3 points from E57 file'" -ForegroundColor Cyan
