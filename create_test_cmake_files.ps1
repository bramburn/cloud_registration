# Sprint 1: Create Test CMakeLists.txt Files
Write-Host "Creating test CMakeLists.txt files for Sprint 1..."

$modules = @(
    "analysis",
    "app", 
    "camera",
    "crs",
    "detection",
    "export",
    "features",
    "implementations",
    "interfaces",
    "optimization",
    "parsers",
    "performance",
    "quality",
    "registration",
    "rendering",
    "ui",
    "mocks"
)

foreach ($module in $modules) {
    $content = @"
# Sprint 1: $module Tests - Placeholder CMakeLists.txt
# This file defines test executables for the $module library

# Placeholder for $module library tests
# Tests will be added in subsequent sprints

message(STATUS "Configuring $module tests...")
"@
    
    $filePath = "tests/$module/CMakeLists.txt"
    $content | Out-File -FilePath $filePath -Encoding UTF8
    Write-Host "Created $filePath"
}

Write-Host "All test CMakeLists.txt files created successfully!"
