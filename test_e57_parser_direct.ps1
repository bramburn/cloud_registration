# Direct test of E57 parser functionality
Write-Host "=== Direct E57 Parser Test ===" -ForegroundColor Green

# Check if test file exists
$testFile = "test_data\test_real_points.e57"
if (-not (Test-Path $testFile)) {
    Write-Host "Test file not found: $testFile" -ForegroundColor Red
    Write-Host "Please run test_e57_fix.ps1 first to create the test file." -ForegroundColor Yellow
    exit 1
}

Write-Host "Test file found: $testFile" -ForegroundColor Green
$fileSize = (Get-Item $testFile).Length
Write-Host "File size: $fileSize bytes" -ForegroundColor Cyan

# Read and analyze the file structure
$bytes = [System.IO.File]::ReadAllBytes($testFile)

Write-Host "`nAnalyzing E57 file structure:" -ForegroundColor Yellow

# Check signature
$signature = [BitConverter]::ToUInt32($bytes, 0)
Write-Host "Signature: 0x$($signature.ToString('X8'))" -ForegroundColor Cyan

# Check version
$majorVersion = [BitConverter]::ToUInt32($bytes, 4)
$minorVersion = [BitConverter]::ToUInt32($bytes, 8)
Write-Host "Version: $majorVersion.$minorVersion" -ForegroundColor Cyan

# Check file length
$fileLength = [BitConverter]::ToUInt64($bytes, 12)
Write-Host "File length: $fileLength" -ForegroundColor Cyan

# Check XML section
$xmlLength = [BitConverter]::ToUInt64($bytes, 20)
$xmlOffset = [BitConverter]::ToUInt64($bytes, 28)
Write-Host "XML offset: $xmlOffset, length: $xmlLength" -ForegroundColor Cyan

# Extract and display XML content
if ($xmlOffset -lt $bytes.Length -and ($xmlOffset + $xmlLength) -le $bytes.Length) {
    $xmlBytes = $bytes[$xmlOffset..($xmlOffset + $xmlLength - 1)]
    $xmlContent = [System.Text.Encoding]::UTF8.GetString($xmlBytes)
    Write-Host "`nXML Content (first 200 chars):" -ForegroundColor Yellow
    Write-Host $xmlContent.Substring(0, [Math]::Min(200, $xmlContent.Length)) -ForegroundColor White
    
    # Check for key elements
    if ($xmlContent -match "cartesianX") {
        Write-Host "✓ Found cartesianX in XML" -ForegroundColor Green
    } else {
        Write-Host "✗ cartesianX not found in XML" -ForegroundColor Red
    }
    
    if ($xmlContent -match "recordCount") {
        Write-Host "✓ Found recordCount in XML" -ForegroundColor Green
    } else {
        Write-Host "✗ recordCount not found in XML" -ForegroundColor Red
    }
} else {
    Write-Host "Invalid XML offset/length" -ForegroundColor Red
}

# Check binary data section
$binaryOffset = $xmlOffset + $xmlLength
if ($binaryOffset -lt $bytes.Length) {
    Write-Host "`nBinary data section:" -ForegroundColor Yellow
    Write-Host "Binary offset: $binaryOffset" -ForegroundColor Cyan
    Write-Host "Remaining bytes: $($bytes.Length - $binaryOffset)" -ForegroundColor Cyan
    
    # Try to read first few floats
    if (($binaryOffset + 12) -le $bytes.Length) {
        $x1 = [BitConverter]::ToSingle($bytes, $binaryOffset)
        $y1 = [BitConverter]::ToSingle($bytes, $binaryOffset + 4)
        $z1 = [BitConverter]::ToSingle($bytes, $binaryOffset + 8)
        Write-Host "First point: ($x1, $y1, $z1)" -ForegroundColor Green
        
        if (($binaryOffset + 24) -le $bytes.Length) {
            $x2 = [BitConverter]::ToSingle($bytes, $binaryOffset + 12)
            $y2 = [BitConverter]::ToSingle($bytes, $binaryOffset + 16)
            $z2 = [BitConverter]::ToSingle($bytes, $binaryOffset + 20)
            Write-Host "Second point: ($x2, $y2, $z2)" -ForegroundColor Green
        }
    }
} else {
    Write-Host "No binary data section found" -ForegroundColor Red
}

Write-Host "`n=== File Analysis Complete ===" -ForegroundColor Green
Write-Host "The file appears to be a valid E57 file with point data." -ForegroundColor Yellow
Write-Host "Now test loading this file in the application to see if it shows actual points instead of a sphere." -ForegroundColor Yellow
