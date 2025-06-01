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
    
    // Ensure Scans directory exists
    QString scansDir = ProjectManager::getScansSubfolder(projectPath);
    QDir().mkpath(scansDir);
    
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
        
        // Create target path
        QString targetPath = QDir(scansDir).absoluteFilePath(fileInfo.fileName());
        
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
        
        // Perform file operation
        if (performFileOperation(filePath, targetPath, mode)) {
            // Create scan info and insert into database
            ScanInfo scanInfo = createScanInfo(targetPath, projectPath, projectId, mode);
            
            if (m_sqliteManager->insertScan(scanInfo)) {
                importedScans.append(scanInfo);
                result.successfulFiles.append(filePath);
            } else {
                // Rollback file operation if database insert fails
                if (mode == ImportMode::Copy) {
                    QFile::remove(targetPath);
                } else {
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
    
    // Ensure target directory exists
    QFileInfo targetInfo(targetPath);
    QDir().mkpath(targetInfo.absolutePath());
    
    bool success = false;
    
    if (mode == ImportMode::Copy) {
        success = QFile::copy(sourcePath, targetPath);
        if (!success) {
            qWarning() << "Failed to copy file from" << sourcePath << "to" << targetPath;
        }
    } else { // ImportMode::Move
        success = QFile::rename(sourcePath, targetPath);
        if (!success) {
            qWarning() << "Failed to move file from" << sourcePath << "to" << targetPath;
        }
    }
    
    return success;
}

ScanInfo ScanImportManager::createScanInfo(const QString &filePath,
                                          const QString &projectPath,
                                          const QString &projectId,
                                          ImportMode mode)
{
    ScanInfo scan;
    scan.scanId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    scan.projectId = projectId;
    scan.scanName = getFileBaseName(filePath);
    scan.filePathRelative = getRelativePath(filePath, projectPath);
    scan.importType = (mode == ImportMode::Copy) ? "COPIED" : "MOVED";
    scan.dateAdded = QDateTime::currentDateTime().toString(Qt::ISODate);
    scan.absolutePath = filePath;
    
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
