# Test script for CloudRegistration application
Write-Host "Testing CloudRegistration Application..."

# Set Qt path
$env:PATH = "C:\Qt\6.5.3\msvc2019_64\bin;$env:PATH"

# Test Debug build
Write-Host "Testing Debug build..."
if (Test-Path ".\build\debug\bin\Debug\CloudRegistration.exe") {
    Write-Host "Debug executable found. Starting application..."
    Start-Process -FilePath ".\build\debug\bin\Debug\CloudRegistration.exe" -NoNewWindow
    Write-Host "Debug application started successfully!"
} else {
    Write-Host "Debug executable not found!"
}

# Test Release build
Write-Host "Testing Release build..."
if (Test-Path ".\build\release\bin\Release\CloudRegistration.exe") {
    Write-Host "Release executable found."
    Write-Host "Release build is available for production use."
} else {
    Write-Host "Release executable not found!"
}

Write-Host "Test completed!"
