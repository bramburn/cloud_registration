#include "scanimportmanager.h"
#include "sqlitemanager.h"
#include "projectmanager.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QUuid>
#include <QDateTime>
#include <QDebug>
#include <QProgressDialog>
#include <QApplication>
#include <QMimeDatabase>
#include <QMimeType>

const QStringList ScanImportManager::SUPPORTED_EXTENSIONS = {".las", ".e57"};

ScanImportManager::ScanImportManager(QObject *parent)
    : QObject(parent)
    , m_sqliteManager(nullptr)
{
}

void ScanImportManager::setSQLiteManager(SQLiteManager *manager)
{
    m_sqliteManager = manager;
}

ImportResult ScanImportManager::importScans(const QStringList &filePaths,
                                           const QString &projectPath,
                                           const QString &projectId,
                                           ImportMode mode,
                                           QWidget *parent)
{
    ImportResult result;
    result.success = true;
    
    if (filePaths.isEmpty()) {
        result.success = false;
        result.errorMessage = "No files selected for import";
        return result;
    }
    
    // Get SQLite manager from project manager
    if (!m_sqliteManager) {
        result.success = false;
        result.errorMessage = "Database manager not available";
        return result;
    }
    
    // Ensure Scans directory exists (only needed for Copy/Move modes)
    QString scansDir;
    if (mode != ImportMode::Link) {
        scansDir = ProjectManager::getScansSubfolder(projectPath);
        QDir().mkpath(scansDir);
    }

    // Setup progress dialog
    QProgressDialog progress(parent);
    progress.setWindowTitle("Importing Scans");
    progress.setLabelText("Preparing import...");
    progress.setRange(0, filePaths.size());
    progress.setModal(true);
    progress.show();

    QList<ScanInfo> importedScans;

    for (int i = 0; i < filePaths.size(); ++i) {
        const QString &filePath = filePaths[i];

        if (progress.wasCanceled()) {
            result.success = false;
            result.errorMessage = "Import cancelled by user";
            break;
        }

        QFileInfo fileInfo(filePath);
        progress.setLabelText(QString("Importing: %1").arg(fileInfo.fileName()));
        progress.setValue(i);
        QApplication::processEvents();

        emit importProgress(i + 1, filePaths.size(), fileInfo.fileName());

        // Validate file
        if (!isValidScanFile(filePath)) {
            result.failedFiles.append(filePath);
            continue;
        }

        QString targetPath = filePath; // Default for Link mode

        // For Copy/Move modes, create target path and handle conflicts
        if (mode != ImportMode::Link) {
            targetPath = QDir(scansDir).absoluteFilePath(fileInfo.fileName());

            // Handle file name conflicts
            int counter = 1;
            QString originalTargetPath = targetPath;
            while (QFileInfo::exists(targetPath)) {
                QString baseName = fileInfo.completeBaseName();
                QString extension = fileInfo.suffix();
                targetPath = QDir(scansDir).absoluteFilePath(
                    QString("%1_%2.%3").arg(baseName).arg(counter).arg(extension));
                counter++;
            }
        }

        // Perform file operation (or skip for Link mode)
        bool fileOpSuccess = true;
        if (mode != ImportMode::Link) {
            fileOpSuccess = performFileOperation(filePath, targetPath, mode);
        }

        if (fileOpSuccess) {
            // Create scan info and insert into database
            ScanInfo scanInfo = createScanInfo(filePath, targetPath, projectPath, projectId, mode);

            if (m_sqliteManager->insertScan(scanInfo)) {
                importedScans.append(scanInfo);
                result.successfulFiles.append(filePath);
            } else {
                // Rollback file operation if database insert fails (only for Copy/Move)
                if (mode == ImportMode::Copy) {
                    QFile::remove(targetPath);
                } else if (mode == ImportMode::Move) {
                    // For move operations, try to restore original file
                    QFile::rename(targetPath, filePath);
                }
                result.failedFiles.append(filePath);
            }
        } else {
            result.failedFiles.append(filePath);
        }
    }
    
    progress.setValue(filePaths.size());
    
    if (!importedScans.isEmpty()) {
        emit scansImported(importedScans);
    }
    
    emit importFinished();
    
    if (result.hasErrors()) {
        result.success = false;
        result.errorMessage = QString("Failed to import %1 of %2 files")
                             .arg(result.failedFiles.size())
                             .arg(result.totalFiles());
    }
    
    return result;
}

bool ScanImportManager::performFileOperation(const QString &sourcePath,
                                           const QString &targetPath,
                                           ImportMode mode)
{
    QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists() || !sourceInfo.isReadable()) {
        qWarning() << "Source file not accessible:" << sourcePath;
        return false;
    }

    // For Link mode, no file operation is needed - just verify source exists
    if (mode == ImportMode::Link) {
        return true;
    }

    // Ensure target directory exists
    QFileInfo targetInfo(targetPath);
    QDir().mkpath(targetInfo.absolutePath());

    bool success = false;

    if (mode == ImportMode::Copy) {
        success = QFile::copy(sourcePath, targetPath);
        if (!success) {
            qWarning() << "Failed to copy file from" << sourcePath << "to" << targetPath;
        }
    } else if (mode == ImportMode::Move) {
        success = QFile::rename(sourcePath, targetPath);
        if (!success) {
            qWarning() << "Failed to move file from" << sourcePath << "to" << targetPath;
        }
    }

    return success;
}

ScanInfo ScanImportManager::createScanInfo(const QString &sourcePath,
                                          const QString &targetPath,
                                          const QString &projectPath,
                                          const QString &projectId,
                                          ImportMode mode)
{
    ScanInfo scan;
    scan.scanId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    scan.projectId = projectId;
    scan.scanName = getFileBaseName(sourcePath);
    scan.dateAdded = QDateTime::currentDateTime().toString(Qt::ISODate);

    // Get file modification time
    QFileInfo sourceInfo(sourcePath);
    scan.scanFileLastModified = sourceInfo.lastModified().toString(Qt::ISODate);

    // Set import type and paths based on mode
    if (mode == ImportMode::Copy) {
        scan.importType = "COPIED";
        scan.filePathRelative = getRelativePath(targetPath, projectPath);
        scan.originalSourcePath = sourcePath;
        scan.absolutePath = targetPath;
    } else if (mode == ImportMode::Move) {
        scan.importType = "MOVED";
        scan.filePathRelative = getRelativePath(targetPath, projectPath);
        scan.originalSourcePath = sourcePath;
        scan.absolutePath = targetPath;
    } else { // ImportMode::Link
        scan.importType = "LINKED";
        scan.filePathAbsoluteLinked = sourcePath;
        scan.absolutePath = sourcePath;
    }

    // TODO: Extract metadata from scan file headers (point count, bounding box)
    // For now, use default values as specified in Sprint 2.2 requirements
    scan.pointCountEstimate = 0;
    scan.boundingBoxMinX = 0.0;
    scan.boundingBoxMinY = 0.0;
    scan.boundingBoxMinZ = 0.0;
    scan.boundingBoxMaxX = 0.0;
    scan.boundingBoxMaxY = 0.0;
    scan.boundingBoxMaxZ = 0.0;

    return scan;
}

QString ScanImportManager::getRelativePath(const QString &filePath, const QString &projectPath)
{
    QDir projectDir(projectPath);
    return projectDir.relativeFilePath(filePath);
}

bool ScanImportManager::isValidScanFile(const QString &filePath)
{
    if (!QFileInfo::exists(filePath)) {
        return false;
    }
    
    QFileInfo fileInfo(filePath);
    QString extension = "." + fileInfo.suffix().toLower();
    
    return SUPPORTED_EXTENSIONS.contains(extension);
}

QStringList ScanImportManager::getSupportedExtensions()
{
    return SUPPORTED_EXTENSIONS;
}

QString ScanImportManager::getFileBaseName(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.completeBaseName();
}
