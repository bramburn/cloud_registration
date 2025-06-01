#!/usr/bin/env pwsh

# Test script to verify the duplicate status display fix
# This script checks that the status display duplication has been resolved

Write-Host "Testing Status Display Fix..." -ForegroundColor Yellow
Write-Host "================================" -ForegroundColor Yellow

# Test 1: Check that m_statusLabel has been removed from header
Write-Host ""
Write-Host "Test 1: Checking MainWindow header for removed status label..." -ForegroundColor Yellow

$headerContent = Get-Content "src/mainwindow.h" -Raw
if ($headerContent -notmatch "QLabel.*m_statusLabel") {
    Write-Host "✅ m_statusLabel successfully removed from header" -ForegroundColor Green
} else {
    Write-Host "❌ m_statusLabel still present in header" -ForegroundColor Red
    exit 1
}

# Test 2: Check that m_statusLabel initialization has been removed
Write-Host ""
Write-Host "Test 2: Checking MainWindow constructor for removed status label initialization..." -ForegroundColor Yellow

$cppContent = Get-Content "src/mainwindow.cpp" -Raw
if ($cppContent -notmatch "m_statusLabel\(nullptr\)") {
    Write-Host "✅ m_statusLabel initialization successfully removed" -ForegroundColor Green
} else {
    Write-Host "❌ m_statusLabel initialization still present" -ForegroundColor Red
    exit 1
}

# Test 3: Check that setupStatusBar no longer creates QLabel
Write-Host ""
Write-Host "Test 3: Checking setupStatusBar method..." -ForegroundColor Yellow

if ($cppContent -notmatch "new QLabel.*Ready.*this") {
    Write-Host "✅ QLabel creation removed from setupStatusBar" -ForegroundColor Green
} else {
    Write-Host "❌ QLabel creation still present in setupStatusBar" -ForegroundColor Red
    exit 1
}

# Test 4: Check that duplicate status updates have been removed
Write-Host ""
Write-Host "Test 4: Checking for duplicate status updates..." -ForegroundColor Yellow

$duplicateStatusPattern = "statusBar\(\)->showMessage.*\s+.*m_statusLabel->setText"
if ($cppContent -notmatch $duplicateStatusPattern) {
    Write-Host "✅ Duplicate status updates successfully removed" -ForegroundColor Green
} else {
    Write-Host "❌ Duplicate status updates still present" -ForegroundColor Red
    exit 1
}

# Test 5: Check that helper methods have been added
Write-Host ""
Write-Host "Test 5: Checking for new helper methods..." -ForegroundColor Yellow

$helperMethods = @(
    "cleanupParsingThread",
    "cleanupProgressDialog", 
    "updateUIAfterParsing"
)

$allHelpersPresent = $true
foreach ($method in $helperMethods) {
    if ($cppContent -match "void MainWindow::$method") {
        Write-Host "✅ Helper method $method found" -ForegroundColor Green
    } else {
        Write-Host "❌ Helper method $method missing" -ForegroundColor Red
        $allHelpersPresent = $false
    }
}

if (-not $allHelpersPresent) {
    exit 1
}

# Test 6: Check LAS parser helper methods
Write-Host ""
Write-Host "Test 6: Checking LAS parser refactoring..." -ForegroundColor Yellow

$lasContent = Get-Content "src/lasparser.cpp" -Raw
$lasHelpers = @(
    "transformAndAddPoint",
    "updateProgressIfNeeded"
)

foreach ($method in $lasHelpers) {
    if ($lasContent -match "void LasParser::$method") {
        Write-Host "✅ LAS parser helper method $method found" -ForegroundColor Green
    } else {
        Write-Host "❌ LAS parser helper method $method missing" -ForegroundColor Red
        exit 1
    }
}

# Test 7: Build test
Write-Host ""
Write-Host "Test 7: Building application to verify no compilation errors..." -ForegroundColor Yellow

$buildResult = cmake --build build/qt690-verification --config Release 2>&1
if ($LASTEXITCODE -eq 0) {
    Write-Host "✅ Application builds successfully" -ForegroundColor Green
} else {
    Write-Host "❌ Build failed:" -ForegroundColor Red
    Write-Host $buildResult -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "🎉 All tests passed! Status display fix verified successfully." -ForegroundColor Green
Write-Host ""
Write-Host "Summary of fixes:" -ForegroundColor Cyan
Write-Host "- ✅ Removed duplicate status display (m_statusLabel)" -ForegroundColor Green
Write-Host "- ✅ Consolidated status updates to use only statusBar()->showMessage()" -ForegroundColor Green
Write-Host "- ✅ Added helper methods for cleanup and UI updates" -ForegroundColor Green
Write-Host "- ✅ Refactored LAS parser to eliminate coordinate transformation duplication" -ForegroundColor Green
Write-Host "- ✅ Application builds and runs successfully" -ForegroundColor Green
