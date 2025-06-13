#include "core/ProjectStateService.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QJsonParseError>

// Stub implementations for classes moved to UI library
class SQLiteManager : public QObject {
public:
    explicit SQLiteManager(QObject* parent = nullptr) : QObject(parent) {}
    bool openDatabase(const QString&) { return true; }
    void closeDatabase() {}
    bool updateScanFilePath(const QString&, const QString&) { return true; }
    bool deleteScan(const QString&) { return true; }
    bool beginTransaction() { return true; }
    bool commitTransaction() { return true; }
    bool rollbackTransaction() { return true; }
    bool saveAllClusters(const QList<ClusterInfo>&) { return true; }
    bool saveAllScans(const QList<ScanInfo>&) { return true; }
    bool validateReferentialIntegrity() { return true; }
    bool insertCluster(const ClusterInfo&) { return true; }
    bool deleteCluster(const QString&) { return true; }
    bool updateClusterName(const QString&, const QString&) { return true; }
    bool updateScanCluster(const QString&, const QString&) { return true; }
    bool updateClusterLockState(const QString&, bool) { return true; }
    QStringList getScansInCluster(const QString&) { return QStringList(); }
    struct Error { QString text() const { return ""; } };
    Error lastError() const { return Error(); }
};

class ScanImportManager : public QObject {
public:
    explicit ScanImportManager(QObject* parent = nullptr) : QObject(parent) {}
    // Note: signals removed for stub implementation
};

class ProjectTreeModel : public QObject {
public:
    explicit ProjectTreeModel(QObject* parent = nullptr) : QObject(parent) {}
    void clear() {}
    QList<ScanInfo> getAllScans() const { return {}; }
    QList<ClusterInfo> getAllClusters() const { return {}; }
    void updateScanFilePath(const QString&, const QString&) {}
    void clearScanMissingFlag(const QString&) {}
    void removeScan(const QString&) {}
    bool loadFromDatabase(SQLiteManager*) { return true; }
    void setScanMissingFlag(const QString&, bool) {}
    void addCluster(const ClusterInfo&) {}
    void removeCluster(const QString&) {}
    void updateClusterName(const QString&, const QString&) {}
    QList<ClusterInfo> getChildClusters(const QString&) { return QList<ClusterInfo>(); }
    void moveScanToCluster(const QString&, const QString&) {}
    void setClusterLockState(const QString&, bool) {}
    bool getClusterLockState(const QString&) { return false; }
    QStringList getScansInCluster(const QString&) { return QStringList(); }
    QList<ScanInfo> getScansInClusterDetailed(const QString&) const { return QList<ScanInfo>(); }
    ScanInfo getScanInfo(const QString&) { return ScanInfo(); }
};

class RecentProjectsManager : public QObject {
public:
    explicit RecentProjectsManager(QObject* parent = nullptr) : QObject(parent) {}
    QStringList getRecentProjects() const { return QStringList(); }
    void addProject(const QString&) {}
    void removeProject(const QString&) {}
    void clearRecentProjects() {}
};

const QString ProjectStateService::METADATA_FILENAME = "project_meta.json";
const QString ProjectStateService::DATABASE_FILENAME = "project_data.sqlite";
const QString ProjectStateService::SCANS_SUBFOLDER = "Scans";
const QString ProjectStateService::CURRENT_FORMAT_VERSION = "1.0.0";
const QString ProjectStateService::BACKUP_SUFFIX = ".bak";

ProjectStateService::ProjectStateService(QObject *parent)
    : QObject(parent)
    , m_sqliteManager(new SQLiteManager(this))
    , m_scanImportManager(new ScanImportManager(this))
    , m_treeModel(std::make_unique<ProjectTreeModel>())
    , m_validationTimer(std::make_unique<QTimer>())
{
    // Note: Signal connections removed for stub implementations in Sprint 7
    // These will be restored when the actual UI library implementations are integrated

    // Set up validation timer
    m_validationTimer->setInterval(VALIDATION_INTERVAL_MS);
    m_validationTimer->setSingleShot(false);
    connect(m_validationTimer.get(), &QTimer::timeout,
            this, &ProjectStateService::onValidationTimerTimeout);
}

ProjectStateService::~ProjectStateService() {
    closeProject();
}

SaveResult ProjectStateService::loadProject(const QString& projectPath) {
    try {
        clearError();
        
        SaveResult result = loadProjectInternal(projectPath);
        
        if (result == SaveResult::Success) {
            // Start periodic validation of linked files
            m_validationTimer->start();
        }
        
        emit projectLoaded(static_cast<ProjectLoadResult>(result));
        return result;
        
    } catch (const std::exception& ex) {
        setError("Exception during project load", ex.what());
        return SaveResult::UnknownError;
    }
}

SaveResult ProjectStateService::saveProject() {
    if (!hasActiveProject()) {
        setError("No active project to save");
        return SaveResult::NoActiveProject;
    }
    
    try {
        SaveResult result = saveProjectInternal();
        emit projectSaved(result);
        return result;
        
    } catch (const std::exception& ex) {
        setError("Exception during project save", ex.what());
        return SaveResult::UnknownError;
    }
}

void ProjectStateService::closeProject() {
    closeProjectInternal();
    emit projectClosed();
}

bool ProjectStateService::hasActiveProject() const {
    return !m_currentProjectPath.isEmpty() && QDir(m_currentProjectPath).exists();
}

QString ProjectStateService::currentProjectPath() const {
    return m_currentProjectPath;
}

ProjectMetadata ProjectStateService::currentMetadata() const {
    return m_metadata;
}

ProjectInfo ProjectStateService::currentProjectInfo() const {
    return m_currentProject;
}

QString ProjectStateService::lastError() const {
    return m_lastError;
}

QString ProjectStateService::lastDetailedError() const {
    return m_detailedError;
}

SQLiteManager* ProjectStateService::getSQLiteManager() const {
    return m_sqliteManager;
}

ScanImportManager* ProjectStateService::getScanImportManager() const {
    return m_scanImportManager;
}

ProjectTreeModel* ProjectStateService::getTreeModel() const {
    return m_treeModel.get();
}

void ProjectStateService::validateAllLinkedFiles() {
    if (!hasActiveProject()) {
        return;
    }
    
    try {
        auto scans = m_treeModel->getAllScans();
        for (const auto& scan : scans) {
            validateLinkedScanFile(scan.scanId, scan.filePath, scan.scanName);
        }
    } catch (const std::exception& ex) {
        qWarning() << "Exception during file validation:" << ex.what();
    }
}

bool ProjectStateService::relinkScanFile(const QString& scanId, const QString& newFilePath) {
    try {
        if (!isFileAccessible(newFilePath)) {
            setError("New file path is not accessible", newFilePath);
            return false;
        }
        
        if (!m_sqliteManager->updateScanFilePath(scanId, newFilePath)) {
            setError("Failed to update scan file path in database",
                    m_sqliteManager->lastError().text());
            return false;
        }
        
        m_treeModel->updateScanFilePath(scanId, newFilePath);
        m_treeModel->clearScanMissingFlag(scanId);
        
        emit scanFileRelinked(scanId, newFilePath);
        return true;
        
    } catch (const std::exception& ex) {
        setError("Exception during file relink", ex.what());
        return false;
    }
}

bool ProjectStateService::removeMissingScanReference(const QString& scanId) {
    try {
        if (!m_sqliteManager->deleteScan(scanId)) {
            setError("Failed to remove scan from database",
                    m_sqliteManager->lastError().text());
            return false;
        }
        
        m_treeModel->removeScan(scanId);
        emit projectScansChanged();
        return true;
        
    } catch (const std::exception& ex) {
        setError("Exception during scan removal", ex.what());
        return false;
    }
}

bool ProjectStateService::hasScans() const {
    if (!hasActiveProject()) {
        return false;
    }
    
    return m_treeModel->getAllScans().size() > 0;
}

QList<ScanInfo> ProjectStateService::getProjectScans() const {
    if (!hasActiveProject()) {
        return {};
    }
    
    return m_treeModel->getAllScans();
}

// Private implementation methods
SaveResult ProjectStateService::loadProjectInternal(const QString& projectPath) {
    m_currentProjectPath = projectPath;
    
    // Validate project directory exists
    if (!validateProjectDirectory(projectPath)) {
        setError("Project directory does not exist or is not accessible", projectPath);
        return SaveResult::UnknownError;
    }
    
    // Check and load metadata
    if (!validateJsonStructure(getMetadataFilePath())) {
        setError("Project metadata file (project_meta.json) is corrupted or unreadable");
        return SaveResult::MetadataCorrupted;
    }
    
    if (!loadProjectMetadataWithValidation()) {
        return SaveResult::MetadataCorrupted;
    }
    
    // Check and load database
    QString dbPath = getDatabaseFilePath();
    if (!QFileInfo::exists(dbPath)) {
        setError("Project database (project_data.sqlite) is missing");
        return SaveResult::DatabaseMissing;
    }
    
    if (!validateDatabaseIntegrity(dbPath)) {
        setError("Project database (project_data.sqlite) is corrupted or inaccessible");
        return SaveResult::DatabaseCorrupted;
    }
    
    if (!loadProjectDatabaseWithValidation()) {
        return SaveResult::DatabaseCorrupted;
    }
    
    return SaveResult::Success;
}

SaveResult ProjectStateService::saveProjectInternal() {
    try {
        // Update last modified timestamp
        m_metadata.last_modified_date = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        // Create backup before saving
        if (!createBackupFiles()) {
            qWarning() << "Failed to create backup files, continuing with save...";
        }
        
        // Save metadata first (faster operation)
        if (!saveProjectMetadataTransactional()) {
            return SaveResult::MetadataWriteFailed;
        }
        
        // Save database with transaction
        SaveResult dbResult = saveProjectDatabaseTransactional();
        if (dbResult != SaveResult::Success) {
            return dbResult;
        }
        
        return SaveResult::Success;
        
    } catch (const std::exception& ex) {
        setError("Exception during save", ex.what());
        return SaveResult::UnknownError;
    }
}

void ProjectStateService::closeProjectInternal() {
    // Stop validation timer
    m_validationTimer->stop();
    
    // Close database connection
    if (m_sqliteManager) {
        m_sqliteManager->closeDatabase();
    }
    
    // Clear tree model
    if (m_treeModel) {
        m_treeModel->clear();
    }
    
    // Clear state
    m_currentProjectPath.clear();
    m_currentProject = ProjectInfo();
    m_metadata = ProjectMetadata();
    clearError();
}

void ProjectStateService::onValidationTimerTimeout() {
    validateAllLinkedFiles();
}

bool ProjectStateService::validateProjectDirectory(const QString& projectPath) {
    QDir projectDir(projectPath);
    return projectDir.exists() && projectDir.isReadable();
}

bool ProjectStateService::validateJsonStructure(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    return parseError.error == QJsonParseError::NoError && doc.isObject();
}

bool ProjectStateService::validateDatabaseIntegrity(const QString& dbPath) {
    // Basic file existence and readability check
    QFileInfo dbInfo(dbPath);
    if (!dbInfo.exists() || !dbInfo.isReadable() || dbInfo.size() == 0) {
        return false;
    }

    // Try to open the database
    SQLiteManager testManager;
    bool canOpen = testManager.openDatabase(dbPath);
    testManager.closeDatabase();

    return canOpen;
}

bool ProjectStateService::loadProjectMetadataWithValidation() {
    try {
        QJsonObject metadata = readProjectMetadata(m_currentProjectPath);

        // Validate required fields
        if (!metadata.contains("projectID") || !metadata.contains("projectName") ||
            !metadata.contains("creationDate") || !metadata.contains("fileFormatVersion")) {
            setError("Project metadata is missing required fields");
            return false;
        }

        // Load metadata into structure
        m_metadata.project_id = metadata["projectID"].toString();
        m_metadata.project_name = metadata["projectName"].toString();
        m_metadata.creation_date = metadata["creationDate"].toString();
        m_metadata.last_modified_date = metadata.value("lastModifiedDate").toString();
        m_metadata.file_format_version = metadata["fileFormatVersion"].toString();
        m_metadata.description = metadata.value("description").toString();

        // Load into ProjectInfo for compatibility
        m_currentProject.projectId = m_metadata.project_id;
        m_currentProject.projectName = m_metadata.project_name;
        m_currentProject.creationDate = m_metadata.creation_date;
        m_currentProject.fileFormatVersion = m_metadata.file_format_version;
        m_currentProject.projectPath = m_currentProjectPath;

        if (!m_currentProject.isValid()) {
            setError("Project metadata validation failed");
            return false;
        }

        return true;

    } catch (const std::exception& ex) {
        setError("Exception loading project metadata", ex.what());
        return false;
    }
}

bool ProjectStateService::saveProjectMetadataTransactional() {
    try {
        QString metadataPath = getMetadataFilePath();
        QString tempPath = metadataPath + ".tmp";

        // Create metadata JSON
        QJsonObject metadata;
        metadata["projectID"] = m_metadata.project_id;
        metadata["projectName"] = m_metadata.project_name;
        metadata["creationDate"] = m_metadata.creation_date;
        metadata["lastModifiedDate"] = m_metadata.last_modified_date;
        metadata["fileFormatVersion"] = m_metadata.file_format_version;
        metadata["description"] = m_metadata.description;

        // Write to temporary file first
        QFile tempFile(tempPath);
        if (!tempFile.open(QIODevice::WriteOnly)) {
            setError("Failed to create temporary metadata file", tempPath);
            return false;
        }

        QJsonDocument doc(metadata);
        qint64 bytesWritten = tempFile.write(doc.toJson());
        tempFile.close();

        if (bytesWritten == -1) {
            QFile::remove(tempPath);
            setError("Failed to write metadata to temporary file");
            return false;
        }

        // Atomic move from temp to final location
        if (!QFile::rename(tempPath, metadataPath)) {
            QFile::remove(tempPath);
            setError("Failed to move temporary metadata file to final location");
            return false;
        }

        return true;

    } catch (const std::exception& ex) {
        setError("Exception saving project metadata", ex.what());
        return false;
    }
}

QJsonObject ProjectStateService::readProjectMetadata(const QString& projectPath) {
    QString metadataPath = getMetadataFilePath(projectPath);

    QFile file(metadataPath);
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Cannot open project metadata file: " + metadataPath.toStdString());
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError) {
        throw std::runtime_error("JSON parse error in metadata file: " + parseError.errorString().toStdString());
    }

    return doc.object();
}

bool ProjectStateService::loadProjectDatabaseWithValidation() {
    try {
        QString dbPath = getDatabaseFilePath();

        if (!m_sqliteManager->openDatabase(dbPath)) {
            setError("Failed to open project database", m_sqliteManager->lastError().text());
            return false;
        }

        // Load data into tree model
        if (!m_treeModel->loadFromDatabase(m_sqliteManager)) {
            setError("Failed to load project data into tree model");
            return false;
        }

        return true;

    } catch (const std::exception& ex) {
        setError("Exception loading project database", ex.what());
        return false;
    }
}

SaveResult ProjectStateService::saveProjectDatabaseTransactional() {
    try {
        // Begin comprehensive transaction
        if (!m_sqliteManager->beginTransaction()) {
            setError("Failed to begin database transaction", m_sqliteManager->lastError().text());
            return SaveResult::TransactionFailed;
        }

        // Save all clusters with validation
        auto clusters = m_treeModel->getAllClusters();
        if (!m_sqliteManager->saveAllClusters(clusters)) {
            m_sqliteManager->rollbackTransaction();
            setError("Failed to save clusters", m_sqliteManager->lastError().text());
            return SaveResult::DatabaseWriteFailed;
        }

        // Save all scans with validation
        auto scans = m_treeModel->getAllScans();
        if (!m_sqliteManager->saveAllScans(scans)) {
            m_sqliteManager->rollbackTransaction();
            setError("Failed to save scans", m_sqliteManager->lastError().text());
            return SaveResult::DatabaseWriteFailed;
        }

        // Validate referential integrity before commit
        if (!m_sqliteManager->validateReferentialIntegrity()) {
            m_sqliteManager->rollbackTransaction();
            setError("Referential integrity validation failed",
                    "Database relationships are inconsistent");
            return SaveResult::DatabaseWriteFailed;
        }

        // Commit transaction
        if (!m_sqliteManager->commitTransaction()) {
            setError("Failed to commit database transaction", m_sqliteManager->lastError().text());
            return SaveResult::TransactionFailed;
        }

        return SaveResult::Success;

    } catch (const std::exception& ex) {
        if (m_sqliteManager) {
            m_sqliteManager->rollbackTransaction();
        }
        setError("Exception during database save", ex.what());
        return SaveResult::UnknownError;
    }
}

void ProjectStateService::validateLinkedScanFile(const QString& scanId, const QString& filePath, const QString& scanName) {
    if (!isFileAccessible(filePath)) {
        m_treeModel->setScanMissingFlag(scanId, true);
        emit scanFileMissing(scanId, filePath, scanName);
    } else {
        m_treeModel->clearScanMissingFlag(scanId);
    }
}

bool ProjectStateService::isFileAccessible(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isReadable();
}

bool ProjectStateService::createBackupFiles() {
    try {
        QString metadataPath = getMetadataFilePath();
        QString dbPath = getDatabaseFilePath();

        // Backup metadata file
        if (QFileInfo::exists(metadataPath)) {
            QString backupMetadataPath = getBackupMetadataPath();
            QFile::remove(backupMetadataPath); // Remove old backup
            if (!QFile::copy(metadataPath, backupMetadataPath)) {
                qWarning() << "Failed to backup metadata file";
                return false;
            }
        }

        // Backup database file
        if (QFileInfo::exists(dbPath)) {
            QString backupDbPath = getBackupDatabasePath();
            QFile::remove(backupDbPath); // Remove old backup
            if (!QFile::copy(dbPath, backupDbPath)) {
                qWarning() << "Failed to backup database file";
                return false;
            }
        }

        return true;

    } catch (const std::exception& ex) {
        qWarning() << "Exception creating backup files:" << ex.what();
        return false;
    }
}

bool ProjectStateService::restoreFromBackup() {
    try {
        QString metadataPath = getMetadataFilePath();
        QString dbPath = getDatabaseFilePath();
        QString backupMetadataPath = getBackupMetadataPath();
        QString backupDbPath = getBackupDatabasePath();

        bool restored = false;

        // Restore metadata if backup exists
        if (QFileInfo::exists(backupMetadataPath)) {
            QFile::remove(metadataPath);
            if (QFile::copy(backupMetadataPath, metadataPath)) {
                restored = true;
            }
        }

        // Restore database if backup exists
        if (QFileInfo::exists(backupDbPath)) {
            QFile::remove(dbPath);
            if (QFile::copy(backupDbPath, dbPath)) {
                restored = true;
            }
        }

        return restored;

    } catch (const std::exception& ex) {
        qWarning() << "Exception restoring from backup:" << ex.what();
        return false;
    }
}

void ProjectStateService::setError(const QString& error, const QString& details) {
    m_lastError = error;
    m_detailedError = details;
    qWarning() << "ProjectStateService error:" << error;
    if (!details.isEmpty()) {
        qWarning() << "Details:" << details;
    }
}

void ProjectStateService::clearError() {
    m_lastError.clear();
    m_detailedError.clear();
}

// Path utility methods
QString ProjectStateService::getMetadataFilePath() const {
    return getMetadataFilePath(m_currentProjectPath);
}

QString ProjectStateService::getDatabaseFilePath() const {
    return getDatabasePath(m_currentProjectPath);
}

QString ProjectStateService::getBackupMetadataPath() const {
    return getMetadataFilePath() + BACKUP_SUFFIX;
}

QString ProjectStateService::getBackupDatabasePath() const {
    return getDatabaseFilePath() + BACKUP_SUFFIX;
}

// Static utility methods
QString ProjectStateService::getMetadataFilePath(const QString& projectPath) {
    return QDir(projectPath).absoluteFilePath(METADATA_FILENAME);
}

QString ProjectStateService::getDatabasePath(const QString& projectPath) {
    return QDir(projectPath).absoluteFilePath(DATABASE_FILENAME);
}

QString ProjectStateService::getScansSubfolder(const QString& projectPath) {
    return QDir(projectPath).absoluteFilePath(SCANS_SUBFOLDER);
}

// Static method moved to header as inline or implemented elsewhere

// Cluster management methods (delegated from ProjectManager)
QString ProjectStateService::createCluster(const QString& clusterName, const QString& parentClusterId) {
    if (!hasActiveProject()) {
        setError("No active project");
        return QString();
    }

    try {
        QString clusterId = QUuid::createUuid().toString(QUuid::WithoutBraces);

        ClusterInfo cluster;
        cluster.clusterId = clusterId;
        cluster.clusterName = clusterName;
        cluster.parentClusterId = parentClusterId;
        cluster.creationDate = QDateTime::currentDateTime().toString(Qt::ISODate);
        cluster.isLocked = false;

        if (!m_sqliteManager->insertCluster(cluster)) {
            setError("Failed to create cluster in database", m_sqliteManager->lastError().text());
            return QString();
        }

        m_treeModel->addCluster(cluster);
        emit clusterCreated(clusterId, clusterName);

        return clusterId;

    } catch (const std::exception& ex) {
        setError("Exception creating cluster", ex.what());
        return QString();
    }
}

bool ProjectStateService::deleteCluster(const QString& clusterId) {
    if (!hasActiveProject()) {
        setError("No active project");
        return false;
    }

    try {
        if (!m_sqliteManager->deleteCluster(clusterId)) {
            setError("Failed to delete cluster from database", m_sqliteManager->lastError().text());
            return false;
        }

        m_treeModel->removeCluster(clusterId);
        emit clusterDeleted(clusterId);

        return true;

    } catch (const std::exception& ex) {
        setError("Exception deleting cluster", ex.what());
        return false;
    }
}

bool ProjectStateService::renameCluster(const QString& clusterId, const QString& newName) {
    if (!hasActiveProject()) {
        setError("No active project");
        return false;
    }

    try {
        if (!m_sqliteManager->updateClusterName(clusterId, newName)) {
            setError("Failed to rename cluster in database", m_sqliteManager->lastError().text());
            return false;
        }

        m_treeModel->updateClusterName(clusterId, newName);
        emit clusterRenamed(clusterId, newName);

        return true;

    } catch (const std::exception& ex) {
        setError("Exception renaming cluster", ex.what());
        return false;
    }
}

QList<ClusterInfo> ProjectStateService::getProjectClusters() {
    if (!hasActiveProject()) {
        return {};
    }

    return m_treeModel->getAllClusters();
}

QList<ClusterInfo> ProjectStateService::getChildClusters(const QString& parentClusterId) {
    if (!hasActiveProject()) {
        return {};
    }

    return m_treeModel->getChildClusters(parentClusterId);
}

bool ProjectStateService::moveScanToCluster(const QString& scanId, const QString& clusterId) {
    if (!hasActiveProject()) {
        setError("No active project");
        return false;
    }

    try {
        if (!m_sqliteManager->updateScanCluster(scanId, clusterId)) {
            setError("Failed to move scan to cluster in database", m_sqliteManager->lastError().text());
            return false;
        }

        m_treeModel->moveScanToCluster(scanId, clusterId);
        emit scanMovedToCluster(scanId, clusterId);

        return true;

    } catch (const std::exception& ex) {
        setError("Exception moving scan to cluster", ex.what());
        return false;
    }
}

bool ProjectStateService::moveScansToCluster(const QStringList& scanIds, const QString& clusterId) {
    if (!hasActiveProject()) {
        setError("No active project");
        return false;
    }

    try {
        for (const QString& scanId : scanIds) {
            if (!moveScanToCluster(scanId, clusterId)) {
                return false; // Error already set by moveScanToCluster
            }
        }

        return true;

    } catch (const std::exception& ex) {
        setError("Exception moving scans to cluster", ex.what());
        return false;
    }
}

bool ProjectStateService::setClusterLockState(const QString& clusterId, bool isLocked) {
    if (!hasActiveProject()) {
        setError("No active project");
        return false;
    }

    try {
        if (!m_sqliteManager->updateClusterLockState(clusterId, isLocked)) {
            setError("Failed to update cluster lock state in database", m_sqliteManager->lastError().text());
            return false;
        }

        m_treeModel->setClusterLockState(clusterId, isLocked);

        return true;

    } catch (const std::exception& ex) {
        setError("Exception setting cluster lock state", ex.what());
        return false;
    }
}

bool ProjectStateService::getClusterLockState(const QString& clusterId) {
    if (!hasActiveProject()) {
        return false;
    }

    return m_treeModel->getClusterLockState(clusterId);
}

bool ProjectStateService::deleteClusterRecursive(const QString& clusterId, bool deletePhysicalFiles) {
    if (!hasActiveProject()) {
        setError("No active project");
        return false;
    }

    try {
        // Get all child clusters and scans before deletion
        QList<ClusterInfo> childClusters = getChildClusters(clusterId);
        QList<ScanInfo> clusterScans = m_treeModel->getScansInClusterDetailed(clusterId);

        // Recursively delete child clusters
        for (const auto& childCluster : childClusters) {
            if (!deleteClusterRecursive(childCluster.clusterId, deletePhysicalFiles)) {
                return false; // Error already set
            }
        }

        // Delete scans in this cluster
        for (const auto& scan : clusterScans) {
            if (!deleteScan(scan.scanId, deletePhysicalFiles)) {
                return false; // Error already set
            }
        }

        // Finally delete the cluster itself
        return deleteCluster(clusterId);

    } catch (const std::exception& ex) {
        setError("Exception deleting cluster recursively", ex.what());
        return false;
    }
}

bool ProjectStateService::deleteScan(const QString& scanId, bool deletePhysicalFile) {
    if (!hasActiveProject()) {
        setError("No active project");
        return false;
    }

    try {
        // Get scan info before deletion
        ScanInfo scanInfo = m_treeModel->getScanInfo(scanId);

        // Delete physical file if requested
        if (deletePhysicalFile && !scanInfo.filePath.isEmpty()) {
            QFile physicalFile(scanInfo.filePath);
            if (physicalFile.exists() && !physicalFile.remove()) {
                qWarning() << "Failed to delete physical scan file:" << scanInfo.filePath;
                // Continue with database deletion even if physical file deletion fails
            }
        }

        // Delete from database
        if (!m_sqliteManager->deleteScan(scanId)) {
            setError("Failed to delete scan from database", m_sqliteManager->lastError().text());
            return false;
        }

        // Remove from tree model
        m_treeModel->removeScan(scanId);
        emit projectScansChanged();

        return true;

    } catch (const std::exception& ex) {
        setError("Exception deleting scan", ex.what());
        return false;
    }
}

// Additional methods for compatibility with ProjectManager
QString ProjectStateService::createProject(const QString& projectPath, const QString& name)
{
    // Stub implementation for Sprint 7 - will be properly implemented in future sprints
    Q_UNUSED(projectPath)
    Q_UNUSED(name)
    setError("Project creation not yet implemented in Sprint 7");
    return QString();
}



bool ProjectStateService::isProjectOpen() const
{
    return hasActiveProject();
}

ProjectInfo ProjectStateService::currentProject() const
{
    return currentProjectInfo();
}



QString ProjectStateService::detailedError() const
{
    return m_detailedError;
}

bool ProjectStateService::isValidProject(const QString& projectPath)
{
    QDir dir(projectPath);
    return dir.exists() &&
           QFileInfo::exists(getMetadataFilePath(projectPath)) &&
           QFileInfo::exists(getDatabasePath(projectPath));
}

// Static method implementation
bool ProjectStateService::isProjectDirectory(const QString& path)
{
    QDir dir(path);
    return dir.exists() &&
           QFileInfo::exists(ProjectStateService::getMetadataFilePath(path)) &&
           QFileInfo::exists(ProjectStateService::getDatabasePath(path));
}

QStringList ProjectStateService::getScansInCluster(const QString& clusterId)
{
    if (!m_treeModel) {
        return QStringList();
    }
    return m_treeModel->getScansInCluster(clusterId);
}
