@echo off
REM Cloud Registration MVP - Project Backup Script
REM Sprint 8: Testing, Documentation, and Deployment
REM Windows Batch Script for Automated Project Backup

setlocal enabledelayedexpansion

REM Configuration
set "SCRIPT_NAME=Cloud Registration Project Backup"
set "VERSION=1.0"
set "TIMESTAMP=%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%"
set "TIMESTAMP=%TIMESTAMP: =0%"

echo ========================================
echo %SCRIPT_NAME% v%VERSION%
echo ========================================
echo.

REM Check if project path is provided
if "%~1"=="" (
    echo Usage: backup_project.bat ^<project_path^> [backup_location]
    echo.
    echo Examples:
    echo   backup_project.bat "C:\Projects\MyRegistration"
    echo   backup_project.bat "C:\Projects\MyRegistration" "D:\Backups"
    echo.
    pause
    exit /b 1
)

set "PROJECT_PATH=%~1"
set "BACKUP_ROOT=%~2"

REM Set default backup location if not provided
if "%BACKUP_ROOT%"=="" (
    set "BACKUP_ROOT=%USERPROFILE%\Documents\CloudRegistration_Backups"
)

REM Validate project path
if not exist "%PROJECT_PATH%" (
    echo ERROR: Project path does not exist: %PROJECT_PATH%
    pause
    exit /b 1
)

REM Extract project name from path
for %%f in ("%PROJECT_PATH%") do set "PROJECT_NAME=%%~nxf"

REM Create backup directory structure
set "BACKUP_DIR=%BACKUP_ROOT%\%PROJECT_NAME%"
set "BACKUP_FILE=%BACKUP_DIR%\%PROJECT_NAME%_backup_%TIMESTAMP%.zip"

if not exist "%BACKUP_ROOT%" (
    mkdir "%BACKUP_ROOT%"
    echo Created backup root directory: %BACKUP_ROOT%
)

if not exist "%BACKUP_DIR%" (
    mkdir "%BACKUP_DIR%"
    echo Created project backup directory: %BACKUP_DIR%
)

echo Project Path: %PROJECT_PATH%
echo Project Name: %PROJECT_NAME%
echo Backup Location: %BACKUP_FILE%
echo.

REM Check available disk space (simplified check)
echo Checking available disk space...
for /f "tokens=3" %%a in ('dir /-c "%BACKUP_ROOT%" ^| find "bytes free"') do set "FREE_SPACE=%%a"
echo Available space: %FREE_SPACE% bytes
echo.

REM Calculate project size
echo Calculating project size...
set "PROJECT_SIZE=0"
for /r "%PROJECT_PATH%" %%f in (*) do (
    set /a PROJECT_SIZE+=%%~zf
)
echo Project size: %PROJECT_SIZE% bytes
echo.

REM Check if we have enough space (with 20% buffer)
set /a REQUIRED_SPACE=%PROJECT_SIZE% * 120 / 100
if %FREE_SPACE% lss %REQUIRED_SPACE% (
    echo WARNING: Insufficient disk space for backup
    echo Required: %REQUIRED_SPACE% bytes
    echo Available: %FREE_SPACE% bytes
    echo.
    set /p CONTINUE="Continue anyway? (y/N): "
    if /i not "!CONTINUE!"=="y" (
        echo Backup cancelled.
        pause
        exit /b 1
    )
)

echo ========================================
echo Starting Backup Process
echo ========================================
echo.

REM Create backup using PowerShell (more reliable than built-in tools)
echo Creating compressed archive...
powershell -Command "& {Add-Type -AssemblyName System.IO.Compression.FileSystem; [System.IO.Compression.ZipFile]::CreateFromDirectory('%PROJECT_PATH%', '%BACKUP_FILE%', 'Optimal', $true)}"

if !errorlevel! neq 0 (
    echo ERROR: Failed to create backup archive
    pause
    exit /b 1
)

echo Backup archive created successfully.
echo.

REM Verify backup integrity
echo Verifying backup integrity...
powershell -Command "& {Add-Type -AssemblyName System.IO.Compression.FileSystem; $zip = [System.IO.Compression.ZipFile]::OpenRead('%BACKUP_FILE%'); $entryCount = $zip.Entries.Count; $zip.Dispose(); Write-Host 'Archive contains' $entryCount 'entries'}"

if !errorlevel! neq 0 (
    echo WARNING: Backup verification failed
) else (
    echo Backup verification successful.
)
echo.

REM Get backup file size
for %%f in ("%BACKUP_FILE%") do set "BACKUP_SIZE=%%~zf"
echo Backup file size: %BACKUP_SIZE% bytes
echo.

REM Calculate compression ratio
set /a COMPRESSION_RATIO=%BACKUP_SIZE% * 100 / %PROJECT_SIZE%
echo Compression ratio: %COMPRESSION_RATIO%%%
echo.

REM Cleanup old backups (keep last 5)
echo Cleaning up old backups...
set "BACKUP_COUNT=0"
for /f "delims=" %%f in ('dir /b /o-d "%BACKUP_DIR%\%PROJECT_NAME%_backup_*.zip" 2^>nul') do (
    set /a BACKUP_COUNT+=1
    if !BACKUP_COUNT! gtr 5 (
        echo Deleting old backup: %%f
        del "%BACKUP_DIR%\%%f"
    )
)
echo.

REM Create backup log entry
set "LOG_FILE=%BACKUP_DIR%\backup_log.txt"
echo %date% %time% - Backup created: %BACKUP_FILE% >> "%LOG_FILE%"
echo %date% %time% - Project size: %PROJECT_SIZE% bytes >> "%LOG_FILE%"
echo %date% %time% - Backup size: %BACKUP_SIZE% bytes >> "%LOG_FILE%"
echo %date% %time% - Compression: %COMPRESSION_RATIO%%% >> "%LOG_FILE%"
echo. >> "%LOG_FILE%"

echo ========================================
echo Backup Summary
echo ========================================
echo.
echo Project: %PROJECT_NAME%
echo Source: %PROJECT_PATH%
echo Backup: %BACKUP_FILE%
echo.
echo Original Size: %PROJECT_SIZE% bytes
echo Backup Size: %BACKUP_SIZE% bytes
echo Compression: %COMPRESSION_RATIO%%%
echo.
echo Backup completed successfully at %date% %time%
echo Log file: %LOG_FILE%
echo.

REM Optional: Open backup directory
set /p OPEN_DIR="Open backup directory? (y/N): "
if /i "%OPEN_DIR%"=="y" (
    explorer "%BACKUP_DIR%"
)

echo.
echo Backup process completed.
pause
exit /b 0
