#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QDir>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>
#include <QTimer>

#include <memory>
#include <stdexcept>

#include "core/ProjectStateService.h"  // Include to get type definitions
#include "core/project.h"

// Forward declarations
class RecentProjectsManager;

// Note: ScanInfo, ClusterInfo, SaveResult, ProjectLoadResult are now defined in ProjectStateService.h

class ProjectCreationException : public std::runtime_error
{
public:
    explicit ProjectCreationException(const QString& message) : std::runtime_error(message.toStdString()) {}
};

class ProjectLoadException : public std::runtime_error
{
public:
    explicit ProjectLoadException(const QString& message) : std::runtime_error(message.toStdString()) {}
};

class ProjectManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjectManager(QObject* parent = nullptr);
    ~ProjectManager();

    // Sprint 3.1 - Enhanced save/load with comprehensive error handling
    SaveResult saveProject();
    ProjectLoadResult loadProject(const QString& projectPath);

    // Enhanced from Sprint 1.1
    QString createProject(const QString& name, const QString& basePath);
    bool isValidProject(const QString& projectPath);
    ProjectInfo loadProjectLegacy(const QString& projectPath);  // Renamed for compatibility
    bool validateProjectMetadata(const QJsonObject& metadata);

    // Sprint 3.1 - File validation and recovery
    void validateAllLinkedFiles();
    bool relinkScanFile(const QString& scanId, const QString& newFilePath);
    bool removeMissingScanReference(const QString& scanId);

    // New for Sprint 1.2
    bool hasScans(const QString& projectPath);
    QList<ScanInfo> getProjectScans(const QString& projectPath);

    // Component access (delegated to ProjectStateService)
    SQLiteManager* getSQLiteManager() const;
    ScanImportManager* getScanImportManager() const;
    ProjectTreeModel* treeModel() const;

    // Sprint 3.1 - Enhanced getters (delegated to ProjectStateService)
    QString currentProjectPath() const;
    ProjectMetadata currentMetadata() const;
    ProjectInfo currentProject() const;
    bool isProjectOpen() const;
    void closeProject();
    QString lastError() const;
    QString detailedError() const;
    QString lastDetailedError() const;

    // Recent projects management (delegated to RecentProjectsManager)
    QStringList getRecentProjects() const;
    void addRecentProject(const QString& projectPath);
    void removeRecentProject(const QString& projectPath);
    void clearRecentProjects();

    // New for Sprint 1.3 - Cluster Management
    QString createCluster(const QString& clusterName, const QString& parentClusterId = QString());
    bool deleteCluster(const QString& clusterId);
    bool renameCluster(const QString& clusterId, const QString& newName);
    QList<ClusterInfo> getProjectClusters();
    QList<ClusterInfo> getChildClusters(const QString& parentClusterId);
    bool moveScanToCluster(const QString& scanId, const QString& clusterId);
    bool moveScansToCluster(const QStringList& scanIds, const QString& clusterId);

    // Sprint 4: Additional cluster methods
    QStringList getScansInCluster(const QString& clusterId);

    // Sprint 2.3 - Cluster locking and enhanced deletion
    bool setClusterLockState(const QString& clusterId, bool isLocked);
    bool getClusterLockState(const QString& clusterId);
    bool deleteClusterRecursive(const QString& clusterId, bool deletePhysicalFiles = false);
    bool deleteScan(const QString& scanId, bool deletePhysicalFile = false);

    static QString getMetadataFilePath(const QString& projectPath);
    static bool isProjectDirectory(const QString& path);
    static QString getScansSubfolder(const QString& projectPath);
    static QString getDatabasePath(const QString& projectPath);

signals:
    void scansImported(const QList<ScanInfo>& scans);
    void projectScansChanged();
    void clusterCreated(const ClusterInfo& cluster);
    void clusterDeleted(const QString& clusterId);
    void clusterRenamed(const QString& clusterId, const QString& newName);
    void scanMovedToCluster(const QString& scanId, const QString& clusterId);

    // Sprint 2.3 - New signals
    void clusterLockStateChanged(const QString& clusterId, bool isLocked);
    void scanDeleted(const QString& scanId);
    void clusterDeletedRecursive(const QString& clusterId);

    // Sprint 3.1 - Enhanced signals for save/load and error handling
    void projectSaved(SaveResult result);
    void projectLoaded(ProjectLoadResult result);
    void scanFileMissing(const QString& scanId, const QString& originalPath, const QString& scanName);
    void scanFileRelinked(const QString& scanId, const QString& newPath);
    void scanReferenceRemoved(const QString& scanId);
    void errorOccurred(const QString& error, const QString& details = QString());

private slots:
    void onValidationTimerTimeout();

private:
    // Sprint 2 Decoupling: Facade coordination methods
    void connectServiceSignals();

    // Legacy methods (kept for compatibility)
    bool createProjectMetadata(const QString& projectPath, const QString& projectName);
    QJsonObject readProjectMetadata(const QString& projectPath);
    bool validateDirectoryPermissions(const QString& path, bool requireWrite = false);

    // Service instances
    ProjectStateService* m_projectStateService;
    RecentProjectsManager* m_recentProjectsManager;

    // Legacy member variables for compatibility
    ProjectInfo m_currentProject;
    QString m_currentProjectPath;
    ProjectMetadata m_metadata;
    QString m_lastError;
    QString m_detailedError;

    static const QString METADATA_FILENAME;
    static const QString DATABASE_FILENAME;
    static const QString SCANS_SUBFOLDER;
    static const QString CURRENT_FORMAT_VERSION;
    static const QString BACKUP_SUFFIX;
    static constexpr int VALIDATION_INTERVAL_MS = 30000;  // 30 seconds
};

#endif  // PROJECTMANAGER_H
