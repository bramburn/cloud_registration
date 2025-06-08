#!/bin/bash
# Cloud Registration MVP - Project Backup Script
# Sprint 8: Testing, Documentation, and Deployment
# Linux/macOS Shell Script for Automated Project Backup

set -e  # Exit on any error

# Configuration
SCRIPT_NAME="Cloud Registration Project Backup"
VERSION="1.0"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

echo "========================================"
echo "$SCRIPT_NAME v$VERSION"
echo "========================================"
echo

# Check if project path is provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <project_path> [backup_location]"
    echo
    echo "Examples:"
    echo "  $0 /home/user/Projects/MyRegistration"
    echo "  $0 /home/user/Projects/MyRegistration /backup/location"
    echo
    exit 1
fi

PROJECT_PATH="$1"
BACKUP_ROOT="${2:-$HOME/Documents/CloudRegistration_Backups}"

# Validate project path
if [ ! -d "$PROJECT_PATH" ]; then
    echo "ERROR: Project path does not exist: $PROJECT_PATH"
    exit 1
fi

# Extract project name from path
PROJECT_NAME=$(basename "$PROJECT_PATH")

# Create backup directory structure
BACKUP_DIR="$BACKUP_ROOT/$PROJECT_NAME"
BACKUP_FILE="$BACKUP_DIR/${PROJECT_NAME}_backup_${TIMESTAMP}.tar.gz"

mkdir -p "$BACKUP_ROOT"
mkdir -p "$BACKUP_DIR"

echo "Project Path: $PROJECT_PATH"
echo "Project Name: $PROJECT_NAME"
echo "Backup Location: $BACKUP_FILE"
echo

# Check available disk space
echo "Checking available disk space..."
AVAILABLE_SPACE=$(df "$BACKUP_ROOT" | awk 'NR==2 {print $4}')
echo "Available space: ${AVAILABLE_SPACE}KB"

# Calculate project size
echo "Calculating project size..."
PROJECT_SIZE=$(du -sk "$PROJECT_PATH" | cut -f1)
echo "Project size: ${PROJECT_SIZE}KB"
echo

# Check if we have enough space (with 20% buffer)
REQUIRED_SPACE=$((PROJECT_SIZE * 120 / 100))
if [ "$AVAILABLE_SPACE" -lt "$REQUIRED_SPACE" ]; then
    echo "WARNING: Insufficient disk space for backup"
    echo "Required: ${REQUIRED_SPACE}KB"
    echo "Available: ${AVAILABLE_SPACE}KB"
    echo
    read -p "Continue anyway? (y/N): " CONTINUE
    if [ "$CONTINUE" != "y" ] && [ "$CONTINUE" != "Y" ]; then
        echo "Backup cancelled."
        exit 1
    fi
fi

echo "========================================"
echo "Starting Backup Process"
echo "========================================"
echo

# Create backup using tar with compression
echo "Creating compressed archive..."
cd "$(dirname "$PROJECT_PATH")"
tar -czf "$BACKUP_FILE" "$(basename "$PROJECT_PATH")" 2>/dev/null

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to create backup archive"
    exit 1
fi

echo "Backup archive created successfully."
echo

# Verify backup integrity
echo "Verifying backup integrity..."
ENTRY_COUNT=$(tar -tzf "$BACKUP_FILE" 2>/dev/null | wc -l)
if [ $? -eq 0 ]; then
    echo "Archive contains $ENTRY_COUNT entries"
    echo "Backup verification successful."
else
    echo "WARNING: Backup verification failed"
fi
echo

# Get backup file size
BACKUP_SIZE=$(du -sk "$BACKUP_FILE" | cut -f1)
echo "Backup file size: ${BACKUP_SIZE}KB"

# Calculate compression ratio
COMPRESSION_RATIO=$((BACKUP_SIZE * 100 / PROJECT_SIZE))
echo "Compression ratio: ${COMPRESSION_RATIO}%"
echo

# Cleanup old backups (keep last 5)
echo "Cleaning up old backups..."
cd "$BACKUP_DIR"
BACKUP_COUNT=0
for backup in $(ls -t ${PROJECT_NAME}_backup_*.tar.gz 2>/dev/null); do
    BACKUP_COUNT=$((BACKUP_COUNT + 1))
    if [ $BACKUP_COUNT -gt 5 ]; then
        echo "Deleting old backup: $backup"
        rm -f "$backup"
    fi
done
echo

# Create backup log entry
LOG_FILE="$BACKUP_DIR/backup_log.txt"
{
    echo "$(date) - Backup created: $BACKUP_FILE"
    echo "$(date) - Project size: ${PROJECT_SIZE}KB"
    echo "$(date) - Backup size: ${BACKUP_SIZE}KB"
    echo "$(date) - Compression: ${COMPRESSION_RATIO}%"
    echo
} >> "$LOG_FILE"

echo "========================================"
echo "Backup Summary"
echo "========================================"
echo
echo "Project: $PROJECT_NAME"
echo "Source: $PROJECT_PATH"
echo "Backup: $BACKUP_FILE"
echo
echo "Original Size: ${PROJECT_SIZE}KB"
echo "Backup Size: ${BACKUP_SIZE}KB"
echo "Compression: ${COMPRESSION_RATIO}%"
echo
echo "Backup completed successfully at $(date)"
echo "Log file: $LOG_FILE"
echo

# Optional: Open backup directory (if GUI available)
if command -v xdg-open >/dev/null 2>&1; then
    read -p "Open backup directory? (y/N): " OPEN_DIR
    if [ "$OPEN_DIR" = "y" ] || [ "$OPEN_DIR" = "Y" ]; then
        xdg-open "$BACKUP_DIR"
    fi
elif command -v open >/dev/null 2>&1; then
    # macOS
    read -p "Open backup directory? (y/N): " OPEN_DIR
    if [ "$OPEN_DIR" = "y" ] || [ "$OPEN_DIR" = "Y" ]; then
        open "$BACKUP_DIR"
    fi
fi

echo
echo "Backup process completed."
exit 0
