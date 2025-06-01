# Test script to verify E57 parsing fix
Write-Host "=== Testing E57 Parsing Fix ===" -ForegroundColor Green

# Build the project
Write-Host "Building project..." -ForegroundColor Yellow
$buildResult = cmake --build build --config Debug
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    exit 1
}
Write-Host "Build successful!" -ForegroundColor Green

# Create a simple test E57 file with actual point data
Write-Host "Creating test E57 file..." -ForegroundColor Yellow

$testFile = "test_data\test_real_points.e57"

# Create binary content for a minimal E57 file
$bytes = @()

# E57 Header (48 bytes)
# Signature "ASTF" (0x41535446 in little-endian)
$bytes += 0x46, 0x54, 0x53, 0x41  # "ASTF"
# Major version (1)
$bytes += 0x01, 0x00, 0x00, 0x00
# Minor version (0)
$bytes += 0x00, 0x00, 0x00, 0x00
# File physical length (will be calculated)
$bytes += 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  # Placeholder
# XML length (will be calculated)
$bytes += 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  # Placeholder
# XML offset (will be calculated)
$bytes += 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  # Placeholder
# Page size (1024)
$bytes += 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
# Reserved (8 bytes)
$bytes += 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

# Simple XML content
$xmlContent = @"
<?xml version="1.0" encoding="UTF-8"?>
<e57Root type="Structure" xmlns="http://www.astm.org/COMMIT/E57/2010-e57-v1.0">
    <formatName type="String">ASTM E57 3D Imaging Data File</formatName>
    <data3D type="Vector">
        <vectorChild type="Structure">
            <points type="CompressedVector" recordCount="3">
                <prototype type="Structure">
                    <cartesianX type="Float" precision="single"/>
                    <cartesianY type="Float" precision="single"/>
                    <cartesianZ type="Float" precision="single"/>
                </prototype>
                <codecs type="Vector">
                    <vectorChild type="CompressedVectorNode">
                        <recordCount type="Integer">3</recordCount>
                        <binarySection type="String">test_binary</binarySection>
                    </vectorChild>
                </codecs>
            </points>
        </vectorChild>
    </data3D>
</e57Root>
"@

$xmlBytes = [System.Text.Encoding]::UTF8.GetBytes($xmlContent)

# Calculate offsets
$xmlOffset = 48  # After header
$xmlLength = $xmlBytes.Length
$binaryOffset = $xmlOffset + $xmlLength
$fileLength = $binaryOffset + 36  # 3 points * 3 coordinates * 4 bytes

Write-Host "XML offset: $xmlOffset, length: $xmlLength" -ForegroundColor Cyan
Write-Host "Binary offset: $binaryOffset" -ForegroundColor Cyan
Write-Host "File length: $fileLength" -ForegroundColor Cyan

# Update header with actual values
# File length at offset 12
$fileLengthBytes = [BitConverter]::GetBytes([uint64]$fileLength)
for ($i = 0; $i -lt 8; $i++) { $bytes[12 + $i] = $fileLengthBytes[$i] }

# XML length at offset 20
$xmlLengthBytes = [BitConverter]::GetBytes([uint64]$xmlLength)
for ($i = 0; $i -lt 8; $i++) { $bytes[20 + $i] = $xmlLengthBytes[$i] }

# XML offset at offset 28
$xmlOffsetBytes = [BitConverter]::GetBytes([uint64]$xmlOffset)
for ($i = 0; $i -lt 8; $i++) { $bytes[28 + $i] = $xmlOffsetBytes[$i] }

# Combine all data
$allBytes = $bytes + $xmlBytes

# Add test point data (3 points)
# Point 1: (1.0, 2.0, 3.0)
$allBytes += [BitConverter]::GetBytes([float]1.0)
$allBytes += [BitConverter]::GetBytes([float]2.0)
$allBytes += [BitConverter]::GetBytes([float]3.0)

# Point 2: (4.0, 5.0, 6.0)
$allBytes += [BitConverter]::GetBytes([float]4.0)
$allBytes += [BitConverter]::GetBytes([float]5.0)
$allBytes += [BitConverter]::GetBytes([float]6.0)

# Point 3: (7.0, 8.0, 9.0)
$allBytes += [BitConverter]::GetBytes([float]7.0)
$allBytes += [BitConverter]::GetBytes([float]8.0)
$allBytes += [BitConverter]::GetBytes([float]9.0)

# Write to file
[System.IO.File]::WriteAllBytes($testFile, $allBytes)

Write-Host "Created test E57 file: $testFile" -ForegroundColor Green
Write-Host "File size: $([System.IO.File]::ReadAllBytes($testFile).Length) bytes" -ForegroundColor Cyan

Write-Host "Test file created successfully!" -ForegroundColor Green
Write-Host "You can now test the application by loading this file." -ForegroundColor Yellow
Write-Host "Expected result: Should show actual point cloud data instead of a sphere." -ForegroundColor Yellow
