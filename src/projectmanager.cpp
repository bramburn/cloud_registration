#include "projectmanager.h"
#include "sqlitemanager.h"
#include "scanimportmanager.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDateTime>
#include <QDebug>
#include <QStandardPaths>

const QString ProjectManager::METADATA_FILENAME = "project_meta.json";
const QString ProjectManager::DATABASE_FILENAME = "project_data.sqlite";
const QString ProjectManager::SCANS_SUBFOLDER = "Scans";
const QString ProjectManager::CURRENT_FORMAT_VERSION = "1.0.0";

ProjectManager::ProjectManager(QObject *parent)
    : QObject(parent)
    , m_sqliteManager(new SQLiteManager(this))
    , m_scanImportManager(new ScanImportManager(this))
{
    // Connect scan import manager signals
    connect(m_scanImportManager, &ScanImportManager::scansImported,
            this, &ProjectManager::scansImported);
    connect(m_scanImportManager, &ScanImportManager::importFinished,
            this, &ProjectManager::projectScansChanged);

    // Set SQLite manager for scan import manager
    m_scanImportManager->setSQLiteManager(m_sqliteManager);
}

ProjectManager::~ProjectManager()
{
    // SQLiteManager and ScanImportManager will be deleted by QObject parent-child relationship
}

QString ProjectManager::createProject(const QString &name, const QString &basePath)
{
    // Input validation
    if (name.trimmed().isEmpty()) {
        throw ProjectCreationException("Project name cannot be empty");
    }
    
    if (basePath.isEmpty()) {
        throw ProjectCreationException("Base path cannot be empty");
    }
    
    QDir baseDir(basePath);
    if (!baseDir.exists()) {
        throw ProjectCreationException(QString("Base directory does not exist: %1").arg(basePath));
    }
    
    if (!validateDirectoryPermissions(basePath, true)) {
        throw ProjectCreationException(QString("No write permission for directory: %1").arg(basePath));
    }
    
    QString projectPath = baseDir.absoluteFilePath(name.trimmed());
    QDir projectDir;

    // Create project directory
    if (!projectDir.mkpath(projectPath)) {
        throw ProjectCreationException(QString("Failed to create project directory: %1").arg(projectPath));
    }

    // Verify directory was created and is writable
    if (!validateDirectoryPermissions(projectPath, true)) {
        // Cleanup
        QDir(projectPath).removeRecursively();
        throw ProjectCreationException(QString("Created directory is not writable: %1").arg(projectPath));
    }

    // Create Scans subfolder
    QString scansPath = getScansSubfolder(projectPath);
    if (!projectDir.mkpath(scansPath)) {
        QDir(projectPath).removeRecursively();
        throw ProjectCreationException("Failed to create Scans subfolder");
    }

    // Create and initialize SQLite database
    if (!createProjectDatabase(projectPath)) {
        QDir(projectPath).removeRecursively();
        throw ProjectCreationException("Failed to create project database");
    }

    // Create metadata file
    try {
        if (!createProjectMetadata(projectPath, name.trimmed())) {
            // Cleanup on failure
            QDir(projectPath).removeRecursively();
            throw ProjectCreationException("Failed to create project metadata file");
        }
    } catch (...) {
        // Cleanup on any exception
        QDir(projectPath).removeRecursively();
        throw;
    }
    
    // Final validation
    if (!isValidProject(projectPath)) {
        QDir(projectPath).removeRecursively();
        throw ProjectCreationException("Project validation failed after creation");
    }
    
    qInfo() << "Project created successfully:" << projectPath;
    return projectPath;
}

bool ProjectManager::isValidProject(const QString &projectPath)
{
    if (projectPath.isEmpty()) {
        return false;
    }
    
    QDir projectDir(projectPath);
    if (!projectDir.exists()) {
        return false;
    }
    
    QString metadataPath = getMetadataFilePath(projectPath);
    if (!QFile::exists(metadataPath)) {
        return false;
    }
    
    try {
        QJsonObject metadata = readProjectMetadata(projectPath);
        return validateProjectMetadata(metadata);
    } catch (...) {
        return false;
    }
}

ProjectInfo ProjectManager::loadProject(const QString &projectPath)
{
    if (!isValidProject(projectPath)) {
        throw ProjectLoadException(QString("Invalid project directory: %1").arg(projectPath));
    }
    
    try {
        QJsonObject metadata = readProjectMetadata(projectPath);
        
        ProjectInfo info;
        info.projectId = metadata["projectID"].toString();
        info.projectName = metadata["projectName"].toString();
        info.creationDate = metadata["creationDate"].toString();
        info.fileFormatVersion = metadata["fileFormatVersion"].toString();
        info.projectPath = projectPath;
        
        if (!info.isValid()) {
            throw ProjectLoadException("Project metadata is incomplete or invalid");
        }

        // Store current project info for cluster management
        m_currentProject = info;

        // Open the project database
        QString dbPath = getDatabasePath(projectPath);
        if (!m_sqliteManager->openDatabase(dbPath)) {
            qWarning() << "Failed to open project database:" << dbPath;
        }

        qInfo() << "Project loaded successfully:" << info.projectName;
        return info;
        
    } catch (const ProjectLoadException&) {
        throw;
    } catch (const std::exception& e) {
        throw ProjectLoadException(QString("Failed to load project: %1").arg(e.what()));
    }
}

bool ProjectManager::validateProjectMetadata(const QJsonObject &metadata)
{
    QStringList requiredFields = {"projectID", "projectName", "creationDate", "fileFormatVersion"};
    
    for (const QString &field : requiredFields) {
        if (!metadata.contains(field) || metadata[field].toString().isEmpty()) {
            qWarning() << "Missing or empty required field:" << field;
            return false;
        }
    }
    
    // Validate UUID format
    QString projectId = metadata["projectID"].toString();
    QUuid uuid(projectId);
    if (uuid.isNull()) {
        qWarning() << "Invalid UUID format for projectID:" << projectId;
        return false;
    }
    
    return true;
}

QString ProjectManager::getMetadataFilePath(const QString &projectPath)
{
    return QDir(projectPath).absoluteFilePath(METADATA_FILENAME);
}

bool ProjectManager::isProjectDirectory(const QString &path)
{
    return QFile::exists(getMetadataFilePath(path));
}

bool ProjectManager::createProjectMetadata(const QString &projectPath, const QString &projectName)
{
    QString metadataPath = getMetadataFilePath(projectPath);
    
    QJsonObject metadata;
    metadata["projectID"] = QUuid::createUuid().toString(QUuid::WithoutBraces);
    metadata["projectName"] = projectName;
    metadata["creationDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    metadata["fileFormatVersion"] = CURRENT_FORMAT_VERSION;
    
    QJsonDocument doc(metadata);
    
    QFile file(metadataPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Failed to open metadata file for writing:" << metadataPath;
        return false;
    }
    
    qint64 bytesWritten = file.write(doc.toJson());
    if (bytesWritten == -1) {
        qCritical() << "Failed to write metadata to file:" << metadataPath;
        return false;
    }
    
    file.close();
    
    // Verify file was written correctly
    if (!QFile::exists(metadataPath)) {
        qCritical() << "Metadata file does not exist after writing:" << metadataPath;
        return false;
    }
    
    qDebug() << "Project metadata created successfully:" << metadataPath;
    return true;
}

QJsonObject ProjectManager::readProjectMetadata(const QString &projectPath)
{
    QString metadataPath = getMetadataFilePath(projectPath);
    
    QFile file(metadataPath);
    if (!file.open(QIODevice::ReadOnly)) {
        throw ProjectLoadException(QString("Cannot open metadata file: %1").arg(metadataPath));
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        throw ProjectLoadException(QString("Invalid JSON in metadata file: %1").arg(error.errorString()));
    }
    
    if (!doc.isObject()) {
        throw ProjectLoadException("Metadata file does not contain a JSON object");
    }
    
    return doc.object();
}

bool ProjectManager::validateDirectoryPermissions(const QString &path, bool requireWrite)
{
    QFileInfo info(path);

    if (!info.exists()) {
        return false;
    }

    if (!info.isDir()) {
        return false;
    }

    if (!info.isReadable()) {
        return false;
    }

    if (requireWrite && !info.isWritable()) {
        return false;
    }

    return true;
}

bool ProjectManager::hasScans(const QString &projectPath)
{
    if (!isValidProject(projectPath)) {
        return false;
    }

    QString dbPath = getDatabasePath(projectPath);
    if (!m_sqliteManager->openDatabase(dbPath)) {
        return false;
    }

    return m_sqliteManager->getScanCount() > 0;
}

QList<ScanInfo> ProjectManager::getProjectScans(const QString &projectPath)
{
    QList<ScanInfo> scans;

    if (!isValidProject(projectPath)) {
        return scans;
    }

    QString dbPath = getDatabasePath(projectPath);
    if (!m_sqliteManager->openDatabase(dbPath)) {
        return scans;
    }

    scans = m_sqliteManager->getAllScans();

    // Compute absolute paths
    for (ScanInfo &scan : scans) {
        scan.absolutePath = QDir(projectPath).absoluteFilePath(scan.filePathRelative);
    }

    return scans;
}

QString ProjectManager::getScansSubfolder(const QString &projectPath)
{
    return QDir(projectPath).absoluteFilePath(SCANS_SUBFOLDER);
}

QString ProjectManager::getDatabasePath(const QString &projectPath)
{
    return QDir(projectPath).absoluteFilePath(DATABASE_FILENAME);
}

bool ProjectManager::createProjectDatabase(const QString &projectPath)
{
    QString dbPath = getDatabasePath(projectPath);

    if (!m_sqliteManager->createDatabase(dbPath)) {
        qWarning() << "Failed to create project database:" << dbPath;
        return false;
    }

    return m_sqliteManager->initializeSchema();
}

bool ProjectManager::initializeDatabaseSchema()
{
    return m_sqliteManager->initializeSchema();
}

void ProjectManager::updateProjectMetadata(const QString &projectPath)
{
    // This method can be used to update project metadata with database references
    // For now, it's a placeholder for future enhancements
    Q_UNUSED(projectPath)
}

// New cluster management methods for Sprint 1.3
QString ProjectManager::createCluster(const QString &clusterName, const QString &parentClusterId)
{
    if (clusterName.trimmed().isEmpty()) {
        qWarning() << "Cluster name cannot be empty";
        return QString();
    }

    if (!m_sqliteManager->isConnected()) {
        qWarning() << "Database not connected";
        return QString();
    }

    // Generate unique cluster ID
    QString clusterId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    ClusterInfo cluster;
    cluster.clusterId = clusterId;
    cluster.projectId = m_currentProject.projectId;
    cluster.clusterName = clusterName.trimmed();
    cluster.parentClusterId = parentClusterId;
    cluster.creationDate = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (m_sqliteManager->insertCluster(cluster)) {
        emit clusterCreated(cluster);
        qDebug() << "Cluster created successfully:" << clusterName;
        return clusterId;
    }

    qWarning() << "Failed to create cluster:" << clusterName;
    return QString();
}

bool ProjectManager::deleteCluster(const QString &clusterId)
{
    if (clusterId.isEmpty()) {
        return false;
    }

    if (!m_sqliteManager->isConnected()) {
        qWarning() << "Database not connected";
        return false;
    }

    if (m_sqliteManager->deleteCluster(clusterId)) {
        emit clusterDeleted(clusterId);
        qDebug() << "Cluster deleted successfully:" << clusterId;
        return true;
    }

    qWarning() << "Failed to delete cluster:" << clusterId;
    return false;
}

bool ProjectManager::renameCluster(const QString &clusterId, const QString &newName)
{
    if (clusterId.isEmpty() || newName.trimmed().isEmpty()) {
        return false;
    }

    if (!m_sqliteManager->isConnected()) {
        qWarning() << "Database not connected";
        return false;
    }

    ClusterInfo cluster = m_sqliteManager->getClusterById(clusterId);
    if (!cluster.isValid()) {
        qWarning() << "Cluster not found:" << clusterId;
        return false;
    }

    cluster.clusterName = newName.trimmed();

    if (m_sqliteManager->updateCluster(cluster)) {
        emit clusterRenamed(clusterId, newName);
        qDebug() << "Cluster renamed successfully:" << clusterId << "to" << newName;
        return true;
    }

    qWarning() << "Failed to rename cluster:" << clusterId;
    return false;
}

QList<ClusterInfo> ProjectManager::getProjectClusters()
{
    if (!m_sqliteManager->isConnected()) {
        qWarning() << "Database not connected";
        return QList<ClusterInfo>();
    }

    return m_sqliteManager->getAllClusters();
}

QList<ClusterInfo> ProjectManager::getChildClusters(const QString &parentClusterId)
{
    if (!m_sqliteManager->isConnected()) {
        qWarning() << "Database not connected";
        return QList<ClusterInfo>();
    }

    return m_sqliteManager->getChildClusters(parentClusterId);
}

bool ProjectManager::moveScanToCluster(const QString &scanId, const QString &clusterId)
{
    if (scanId.isEmpty()) {
        return false;
    }

    if (!m_sqliteManager->isConnected()) {
        qWarning() << "Database not connected";
        return false;
    }

    if (m_sqliteManager->updateScanCluster(scanId, clusterId)) {
        emit scanMovedToCluster(scanId, clusterId);
        qDebug() << "Scan moved to cluster successfully:" << scanId << "to" << clusterId;
        return true;
    }

    qWarning() << "Failed to move scan to cluster:" << scanId;
    return false;
}

bool ProjectManager::moveScansToCluster(const QStringList &scanIds, const QString &clusterId)
{
    if (scanIds.isEmpty()) {
        return false;
    }

    if (!m_sqliteManager->isConnected()) {
        qWarning() << "Database not connected";
        return false;
    }

    bool allSuccess = true;
    for (const QString &scanId : scanIds) {
        if (!m_sqliteManager->updateScanCluster(scanId, clusterId)) {
            allSuccess = false;
            qWarning() << "Failed to move scan to cluster:" << scanId;
        } else {
            emit scanMovedToCluster(scanId, clusterId);
        }
    }

    if (allSuccess) {
        qDebug() << "All scans moved to cluster successfully:" << scanIds.size() << "scans to" << clusterId;
    }

    return allSuccess;
}

// Sprint 2.3 - Cluster locking and enhanced deletion methods
bool ProjectManager::setClusterLockState(const QString &clusterId, bool isLocked)
{
    if (clusterId.isEmpty()) {
        return false;
    }

    if (!m_sqliteManager->isConnected()) {
        qWarning() << "Database not connected";
        return false;
    }

    if (m_sqliteManager->setClusterLockState(clusterId, isLocked)) {
        emit clusterLockStateChanged(clusterId, isLocked);
        qDebug() << "Cluster lock state changed:" << clusterId << "locked:" << isLocked;
        return true;
    }

    qWarning() << "Failed to set cluster lock state:" << clusterId;
    return false;
}

bool ProjectManager::getClusterLockState(const QString &clusterId)
{
    if (clusterId.isEmpty()) {
        return false;
    }

    if (!m_sqliteManager->isConnected()) {
        qWarning() << "Database not connected";
        return false;
    }

    return m_sqliteManager->getClusterLockState(clusterId);
}

bool ProjectManager::deleteClusterRecursive(const QString &clusterId, bool deletePhysicalFiles)
{
    if (clusterId.isEmpty()) {
        return false;
    }

    if (!m_sqliteManager->isConnected()) {
        qWarning() << "Database not connected";
        return false;
    }

    // Get scan paths before deletion if we need to delete physical files
    QStringList scanPaths;
    if (deletePhysicalFiles) {
        scanPaths = m_sqliteManager->getClusterScanPaths(clusterId, m_currentProject.projectPath);
    }

    // Delete from database
    if (m_sqliteManager->deleteClusterRecursive(clusterId)) {
        // Delete physical files if requested
        if (deletePhysicalFiles) {
            for (const QString &scanPath : scanPaths) {
                if (QFile::exists(scanPath)) {
                    if (QFile::remove(scanPath)) {
                        qDebug() << "Deleted physical scan file:" << scanPath;
                    } else {
                        qWarning() << "Failed to delete physical scan file:" << scanPath;
                    }
                }
            }
        }

        emit clusterDeletedRecursive(clusterId);
        qDebug() << "Cluster deleted recursively:" << clusterId;
        return true;
    }

    qWarning() << "Failed to delete cluster recursively:" << clusterId;
    return false;
}

bool ProjectManager::deleteScan(const QString &scanId, bool deletePhysicalFile)
{
    if (scanId.isEmpty()) {
        return false;
    }

    if (!m_sqliteManager->isConnected()) {
        qWarning() << "Database not connected";
        return false;
    }

    // Get scan info before deletion if we need to delete physical file
    QString physicalPath;
    if (deletePhysicalFile) {
        ScanInfo scan = m_sqliteManager->getScanById(scanId);
        if (scan.isValid() && (scan.importType == "COPIED" || scan.importType == "MOVED")) {
            physicalPath = scan.getFilePath(m_currentProject.projectPath);
        }
    }

    // Delete from database
    if (m_sqliteManager->deleteScan(scanId)) {
        // Delete physical file if requested and applicable
        if (deletePhysicalFile && !physicalPath.isEmpty() && QFile::exists(physicalPath)) {
            if (QFile::remove(physicalPath)) {
                qDebug() << "Deleted physical scan file:" << physicalPath;
            } else {
                qWarning() << "Failed to delete physical scan file:" << physicalPath;
            }
        }

        emit scanDeleted(scanId);
        qDebug() << "Scan deleted:" << scanId;
        return true;
    }

    qWarning() << "Failed to delete scan:" << scanId;
    return false;
}
