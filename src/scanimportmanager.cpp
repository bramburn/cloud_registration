#include "scanimportmanager.h"
#include "sqlitemanager.h"
#include "projectmanager.h"
#include "projecttreemodel.h"
#include "E57DataManager.h"
#include "errordialog.h"
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
#include <QMessageBox>
#include <numeric>

const QStringList ScanImportManager::SUPPORTED_EXTENSIONS = {".las", ".e57"};

ScanImportManager::ScanImportManager(QObject *parent)
    : QObject(parent)
    , m_sqliteManager(nullptr)
    , m_projectTreeModel(nullptr)
    , m_parentWidget(nullptr)
{
}

void ScanImportManager::setSQLiteManager(SQLiteManager *manager)
{
    m_sqliteManager = manager;
}

void ScanImportManager::setProjectTreeModel(ProjectTreeModel *model)
{
    m_projectTreeModel = model;
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

// Sprint 1.3: E57-specific import implementation
void ScanImportManager::handleE57Import(const QString& filePath)
{
    try {
        qDebug() << "ScanImportManager: Starting E57 import for" << filePath;

        // Create progress dialog
        auto* progressDialog = new QProgressDialog(
            QString("Importing E57 file: %1").arg(QFileInfo(filePath).fileName()),
            "Cancel", 0, 100, m_parentWidget);
        progressDialog->setWindowModality(Qt::WindowModal);
        progressDialog->setAutoClose(false);
        progressDialog->show();

        // Initialize E57DataManager
        auto* e57Manager = new E57DataManager(this);

        // Connect progress signals
        connect(e57Manager, &E57DataManager::progress,
                progressDialog, &QProgressDialog::setValue);
        connect(e57Manager, &E57DataManager::operationStarted,
                progressDialog, &QProgressDialog::setLabelText);
        connect(progressDialog, &QProgressDialog::canceled,
                this, [this]() {
                    qDebug() << "E57 import canceled by user";
                });

        // First, validate the file and get metadata
        if (!e57Manager->isValidE57File(filePath)) {
            throw std::runtime_error("Invalid E57 file format");
        }

        QVector<ScanMetadata> scanMetadata = e57Manager->getScanMetadata(filePath);

        if (scanMetadata.isEmpty()) {
            throw std::runtime_error("E57 file contains no valid scans");
        }

        qDebug() << "ScanImportManager: Found" << scanMetadata.size() << "scans in E57 file";

        // Process each scan
        for (int i = 0; i < scanMetadata.size(); ++i) {
            const ScanMetadata& metadata = scanMetadata[i];

            // Create ScanInfo record for database
            ScanInfo scanInfo;
            scanInfo.scanId = QUuid::createUuid().toString(QUuid::WithoutBraces);
            scanInfo.scanName = metadata.name.isEmpty() ?
                QString("E57_Scan_%1").arg(i + 1) : metadata.name;
            scanInfo.filePathRelative = filePath; // Store original path for E57
            scanInfo.importType = "E57";
            scanInfo.pointCountEstimate = static_cast<int>(metadata.pointCount);
            scanInfo.dateAdded = QDateTime::currentDateTime().toString(Qt::ISODate);

            // Store E57-specific metadata
            scanInfo.originalSourcePath = metadata.guid; // Store E57 GUID in this field

            // Set bounding box information
            scanInfo.boundingBoxMinX = metadata.minX;
            scanInfo.boundingBoxMinY = metadata.minY;
            scanInfo.boundingBoxMinZ = metadata.minZ;
            scanInfo.boundingBoxMaxX = metadata.maxX;
            scanInfo.boundingBoxMaxY = metadata.maxY;
            scanInfo.boundingBoxMaxZ = metadata.maxZ;

            // Insert into database
            if (!m_sqliteManager->insertScan(scanInfo)) {
                throw std::runtime_error("Failed to insert scan info into database");
            }

            // Update project tree model if available
            if (m_projectTreeModel) {
                // Note: This may need to be adapted based on ProjectTreeModel's interface
                qDebug() << "ScanImportManager: Would update project tree with scan" << scanInfo.scanName;
            }

            qDebug() << "ScanImportManager: Successfully imported scan" << scanInfo.scanName
                     << "with" << scanInfo.pointCountEstimate << "points";
        }

        progressDialog->close();
        delete progressDialog;
        delete e57Manager;

        // Show success message
        QMessageBox::information(m_parentWidget, "Import Successful",
            QString("Successfully imported %1 scan(s) from E57 file:\n%2\n\nTotal points: %3")
            .arg(scanMetadata.size())
            .arg(QFileInfo(filePath).fileName())
            .arg(std::accumulate(scanMetadata.begin(), scanMetadata.end(), 0ULL,
                [](uint64_t sum, const ScanMetadata& scan) { return sum + scan.pointCount; })));

        emit importCompleted(filePath, scanMetadata.size());

    } catch (const std::exception& ex) {
        handleE57ImportError(filePath, QString("Error: %1").arg(ex.what()));
    }
}

void ScanImportManager::handleE57ImportError(const QString& filePath, const QString& error)
{
    qDebug() << "ScanImportManager: E57 import error:" << error;

    // Use ErrorDialog if available, otherwise fallback to QMessageBox
    QMessageBox::critical(m_parentWidget, "E57 Import Failed",
        QString("Could not import E57 file:\n%1\n\n%2")
        .arg(QFileInfo(filePath).fileName())
        .arg(error));

    emit importFailed(filePath, error);
}
