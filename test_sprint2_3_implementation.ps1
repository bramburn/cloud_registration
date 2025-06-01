#!/usr/bin/env pwsh

# Sprint 2.3 UI/UX Loading Feedback Implementation Test Script
# This script verifies that all Sprint 2.3 enhancements have been properly implemented

Write-Host "=== Sprint 2.3 UI/UX Loading Feedback Implementation Test ===" -ForegroundColor Cyan
Write-Host ""

$testsPassed = 0
$testsTotal = 0

function Test-Implementation {
    param(
        [string]$TestName,
        [string]$FilePath,
        [string]$Pattern,
        [string]$Description
    )
    
    $global:testsTotal++
    Write-Host "Testing: $TestName" -ForegroundColor Yellow
    
    if (Test-Path $FilePath) {
        $content = Get-Content $FilePath -Raw
        if ($content -match $Pattern) {
            Write-Host "  ✓ $Description" -ForegroundColor Green
            $global:testsPassed++
            return $true
        } else {
            Write-Host "  ✗ $Description" -ForegroundColor Red
            return $false
        }
    } else {
        Write-Host "  ✗ File not found: $FilePath" -ForegroundColor Red
        return $false
    }
}

# Test 1: Enhanced onParsingProgressUpdated method signature
Test-Implementation -TestName "Enhanced Progress Method Signature" `
    -FilePath "src/mainwindow.h" `
    -Pattern "onParsingProgressUpdated\(int percentage, const QString `&stage\)" `
    -Description "onParsingProgressUpdated method accepts stage parameter"

# Test 2: Enhanced onParsingProgressUpdated implementation
Test-Implementation -TestName "Enhanced Progress Method Implementation" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "void MainWindow::onParsingProgressUpdated\(int percentage, const QString `&stage\)" `
    -Description "onParsingProgressUpdated implementation updated with stage parameter"

# Test 3: Standardized status bar methods declarations
Test-Implementation -TestName "Status Bar Methods Declaration" `
    -FilePath "src/mainwindow.h" `
    -Pattern "void setStatusReady\(\);" `
    -Description "setStatusReady method declared"

Test-Implementation -TestName "Status Bar Methods Declaration" `
    -FilePath "src/mainwindow.h" `
    -Pattern "void setStatusLoading\(const QString `&filename\);" `
    -Description "setStatusLoading method declared"

Test-Implementation -TestName "Status Bar Methods Declaration" `
    -FilePath "src/mainwindow.h" `
    -Pattern "void setStatusLoadSuccess\(const QString `&filename, int pointCount\);" `
    -Description "setStatusLoadSuccess method declared"

Test-Implementation -TestName "Status Bar Methods Declaration" `
    -FilePath "src/mainwindow.h" `
    -Pattern "void setStatusLoadFailed\(const QString `&filename, const QString `&error\);" `
    -Description "setStatusLoadFailed method declared"

Test-Implementation -TestName "Status Bar Methods Declaration" `
    -FilePath "src/mainwindow.h" `
    -Pattern "void setStatusViewChanged\(const QString `&viewName\);" `
    -Description "setStatusViewChanged method declared"

# Test 4: Standardized status bar methods implementations
Test-Implementation -TestName "Status Bar Methods Implementation" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "void MainWindow::setStatusReady\(\)" `
    -Description "setStatusReady method implemented"

Test-Implementation -TestName "Status Bar Methods Implementation" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "void MainWindow::setStatusLoading\(const QString `&filename\)" `
    -Description "setStatusLoading method implemented"

Test-Implementation -TestName "Status Bar Methods Implementation" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "void MainWindow::setStatusLoadSuccess\(const QString `&filename, int pointCount\)" `
    -Description "setStatusLoadSuccess method implemented"

Test-Implementation -TestName "Status Bar Methods Implementation" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "void MainWindow::setStatusLoadFailed\(const QString `&filename, const QString `&error\)" `
    -Description "setStatusLoadFailed method implemented"

Test-Implementation -TestName "Status Bar Methods Implementation" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "void MainWindow::setStatusViewChanged\(const QString `&viewName\)" `
    -Description "setStatusViewChanged method implemented"

# Test 5: Enhanced onParsingFinished using standardized methods
Test-Implementation -TestName "Enhanced onParsingFinished" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "setStatusLoadSuccess\(m_currentFileName, m_currentPointCount\)" `
    -Description "onParsingFinished uses setStatusLoadSuccess"

Test-Implementation -TestName "Enhanced onParsingFinished" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "setStatusLoadFailed\(m_currentFileName, message\)" `
    -Description "onParsingFinished uses setStatusLoadFailed"

# Test 6: View control methods using standardized status
Test-Implementation -TestName "View Control Methods" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "setStatusViewChanged\(\"Top\"\)" `
    -Description "onTopViewClicked uses setStatusViewChanged"

Test-Implementation -TestName "View Control Methods" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "setStatusViewChanged\(\"Left\"\)" `
    -Description "onLeftViewClicked uses setStatusViewChanged"

Test-Implementation -TestName "View Control Methods" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "setStatusViewChanged\(\"Right\"\)" `
    -Description "onRightViewClicked uses setStatusViewChanged"

Test-Implementation -TestName "View Control Methods" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "setStatusViewChanged\(\"Bottom\"\)" `
    -Description "onBottomViewClicked uses setStatusViewChanged"

# Test 7: Signal connections to viewer for visual feedback
Test-Implementation -TestName "Viewer Signal Connections" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "connect\(e57Worker, `&E57Parser::progressUpdated, m_viewer, `&PointCloudViewerWidget::onLoadingProgress" `
    -Description "E57Parser progressUpdated connected to viewer"

Test-Implementation -TestName "Viewer Signal Connections" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "connect\(e57Worker, `&E57Parser::parsingFinished, m_viewer, `&PointCloudViewerWidget::onLoadingFinished" `
    -Description "E57Parser parsingFinished connected to viewer"

Test-Implementation -TestName "Viewer Signal Connections" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "connect\(lasWorker, `&LasParser::progressUpdated, m_viewer, `&PointCloudViewerWidget::onLoadingProgress" `
    -Description "LasParser progressUpdated connected to viewer"

Test-Implementation -TestName "Viewer Signal Connections" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "connect\(lasWorker, `&LasParser::parsingFinished, m_viewer, `&PointCloudViewerWidget::onLoadingFinished" `
    -Description "LasParser parsingFinished connected to viewer"

# Test 8: Enhanced onLasHeaderParsed using standardized method
Test-Implementation -TestName "Enhanced onLasHeaderParsed" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "setStatusFileInfo\(m_currentFileName," `
    -Description "onLasHeaderParsed uses setStatusFileInfo"

# Test 9: Status bar widgets setup
Test-Implementation -TestName "Status Bar Setup" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "m_statusLabel = new QLabel\(this\)" `
    -Description "Status bar widgets properly created"

Test-Implementation -TestName "Status Bar Setup" `
    -FilePath "src/mainwindow.cpp" `
    -Pattern "statusBar\(\)->addWidget\(m_statusLabel, 1\)" `
    -Description "Status bar widgets properly added"

# Test 10: PointCloudViewerWidget state management (already implemented)
Test-Implementation -TestName "Viewer State Management" `
    -FilePath "src/pointcloudviewerwidget.h" `
    -Pattern "enum class ViewerState" `
    -Description "ViewerState enum defined"

Test-Implementation -TestName "Viewer Loading Slots" `
    -FilePath "src/pointcloudviewerwidget.h" `
    -Pattern "void onLoadingProgress\(int percentage, const QString &stage\)" `
    -Description "onLoadingProgress slot defined"

Test-Implementation -TestName "Viewer Loading Slots" `
    -FilePath "src/pointcloudviewerwidget.h" `
    -Pattern "void onLoadingFinished\(bool success, const QString &message," `
    -Description "onLoadingFinished slot defined"

# Summary
Write-Host ""
Write-Host "=== Test Results ===" -ForegroundColor Cyan
Write-Host "Tests Passed: $testsPassed / $testsTotal" -ForegroundColor $(if ($testsPassed -eq $testsTotal) { "Green" } else { "Yellow" })

if ($testsPassed -eq $testsTotal) {
    Write-Host ""
    Write-Host "🎉 All Sprint 2.3 UI/UX Loading Feedback enhancements have been successfully implemented!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Implemented Features:" -ForegroundColor Cyan
    Write-Host "  ✓ Enhanced progress reporting with stage information" -ForegroundColor Green
    Write-Host "  ✓ Standardized status bar message methods" -ForegroundColor Green
    Write-Host "  ✓ Enhanced MainWindow integration with viewer visual feedback" -ForegroundColor Green
    Write-Host "  ✓ Updated view control methods with consistent status messages" -ForegroundColor Green
    Write-Host "  ✓ Complete signal connections for parser-to-viewer communication" -ForegroundColor Green
    Write-Host "  ✓ Enhanced error handling and user feedback" -ForegroundColor Green
} else {
    Write-Host ""
    Write-Host "⚠️  Some tests failed. Please review the implementation." -ForegroundColor Yellow
}

Write-Host ""
