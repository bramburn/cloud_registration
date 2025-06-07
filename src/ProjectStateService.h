#ifndef PROJECTSTATESERVICE_H
#define PROJECTSTATESERVICE_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <memory>
#include "project.h"

// Forward declarations
class SQLiteManager;
class ScanImportManager;
class ProjectTreeModel;
class QTimer;

/**
 * @brief ProjectStateService - Manages the state of the currently active project
 * 
 * This service is responsible for managing the lifecycle of the currently active project,
 * including loading, saving, and providing access to the current project's data.
 * It encapsulates all operations related to the active project state.
 * 
 * Sprint 2 Decoupling: Extracted from ProjectManager to separate concerns.
 * The ProjectManager now acts as a facade that coordinates this service with
 * the RecentProjectsManager.
 */
class ProjectStateService : public QObject {
    Q_OBJECT

public:
    explicit ProjectStateService(QObject *parent = nullptr);
    ~ProjectStateService();

    // Project state management
    SaveResult loadProject(const QString& projectPath);
    SaveResult saveProject();
    void closeProject();
    
    // Project state queries
    bool hasActiveProject() const;
    QString currentProjectPath() const;
    ProjectMetadata currentMetadata() const;
    ProjectInfo currentProjectInfo() const;
    
    // Error handling
    QString lastError() const;
    QString lastDetailedError() const;
    
    // Component access
    SQLiteManager* getSQLiteManager() const;
    ScanImportManager* getScanImportManager() const;
    ProjectTreeModel* getTreeModel() const;
    
    // Project validation and recovery
    void validateAllLinkedFiles();
    bool relinkScanFile(const QString& scanId, const QString& newFilePath);
    bool removeMissingScanReference(const QString& scanId);
    
    // Scan management
    bool hasScans() const;
    QList<ScanInfo> getProjectScans() const;
    
    // Cluster management
    QString createCluster(const QString& clusterName, const QString& parentClusterId = QString());
    bool deleteCluster(const QString& clusterId);
    bool renameCluster(const QString& clusterId, const QString& newName);
    QList<ClusterInfo> getProjectClusters();
    QList<ClusterInfo> getChildClusters(const QString& parentClusterId);
    bool moveScanToCluster(const QString& scanId, const QString& clusterId);
    bool moveScansToCluster(const QStringList& scanIds, const QString& clusterId);
    
    // Enhanced cluster operations
    bool setClusterLockState(const QString& clusterId, bool isLocked);
    bool getClusterLockState(const QString& clusterId);
    bool deleteClusterRecursive(const QString& clusterId, bool deletePhysicalFiles = false);
    bool deleteScan(const QString& scanId, bool deletePhysicalFile = false);

signals:
    // Project lifecycle signals
    void projectLoaded(ProjectLoadResult result);
    void projectSaved(SaveResult result);
    void projectClosed();
    
    // Project content signals
    void projectScansChanged();
    void scansImported(const QStringList& scanIds);
    void scanFileRelinked(const QString& scanId, const QString& newFilePath);
    void scanFileMissing(const QString& scanId, const QString& filePath, const QString& scanName);
    
    // Cluster management signals
    void clusterCreated(const QString& clusterId, const QString& clusterName);
    void clusterDeleted(const QString& clusterId);
    void clusterRenamed(const QString& clusterId, const QString& newName);
    void scanMovedToCluster(const QString& scanId, const QString& clusterId);

private slots:
    void onValidationTimerTimeout();

private:
    // Project lifecycle operations
    SaveResult loadProjectInternal(const QString& projectPath);
    SaveResult saveProjectInternal();
    void closeProjectInternal();
    
    // Project validation
    bool validateProjectDirectory(const QString& projectPath);
    bool validateJsonStructure(const QString& filePath);
    bool validateDatabaseIntegrity(const QString& dbPath);
    
    // Metadata operations
    bool loadProjectMetadataWithValidation();
    bool saveProjectMetadataTransactional();
    QJsonObject readProjectMetadata(const QString& projectPath);
    bool createProjectMetadata(const QString& projectPath, const QString& projectName);
    
    // Database operations
    bool loadProjectDatabaseWithValidation();
    SaveResult saveProjectDatabaseTransactional();
    bool createProjectDatabase(const QString& projectPath);
    bool initializeDatabaseSchema();
    
    // File validation and recovery
    void validateLinkedScanFile(const QString& scanId, const QString& filePath, const QString& scanName);
    bool isFileAccessible(const QString& filePath);
    
    // Backup and recovery
    bool createBackupFiles();
    bool restoreFromBackup();
    
    // Error handling
    void setError(const QString& error, const QString& details = QString());
    void clearError();
    
    // Path utilities
    QString getMetadataFilePath() const;
    QString getDatabaseFilePath() const;
    QString getBackupMetadataPath() const;
    QString getBackupDatabasePath() const;
    
    // Static utility methods
    static QString getMetadataFilePath(const QString& projectPath);
    static QString getDatabasePath(const QString& projectPath);
    static QString getScansSubfolder(const QString& projectPath);
    static bool isProjectDirectory(const QString& path);
    
    // Data members
    SQLiteManager* m_sqliteManager;
    ScanImportManager* m_scanImportManager;
    std::unique_ptr<ProjectTreeModel> m_treeModel;
    std::unique_ptr<QTimer> m_validationTimer;
    
    // Project state
    ProjectInfo m_currentProject;
    QString m_currentProjectPath;
    ProjectMetadata m_metadata;
    
    // Error tracking
    QString m_lastError;
    QString m_detailedError;
    
    // Constants
    static const QString METADATA_FILENAME;
    static const QString DATABASE_FILENAME;
    static const QString SCANS_SUBFOLDER;
    static const QString CURRENT_FORMAT_VERSION;
    static const QString BACKUP_SUFFIX;
    static constexpr int VALIDATION_INTERVAL_MS = 30000; // 30 seconds
};

#endif // PROJECTSTATESERVICE_H
