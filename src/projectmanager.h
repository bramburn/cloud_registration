#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QList>
#include <stdexcept>
#include "project.h"

// Forward declarations
class SQLiteManager;
class ScanImportManager;

// Scan metadata structure for database storage
struct ScanInfo {
    QString scanId;
    QString projectId;
    QString scanName;
    QString filePathRelative;
    QString importType;  // "COPIED" or "MOVED"
    QString dateAdded;
    QString parentClusterId; // New: ID of parent cluster (NULL if at project root)
    QString absolutePath; // Computed field

    bool isValid() const {
        return !scanId.isEmpty() && !scanName.isEmpty() && !filePathRelative.isEmpty();
    }
};

// Cluster metadata structure for database storage
struct ClusterInfo {
    QString clusterId;
    QString projectId;
    QString clusterName;
    QString parentClusterId; // NULL if top-level cluster
    QString creationDate;

    bool isValid() const {
        return !clusterId.isEmpty() && !clusterName.isEmpty() && !projectId.isEmpty();
    }
};

class ProjectCreationException : public std::runtime_error {
public:
    explicit ProjectCreationException(const QString &message) 
        : std::runtime_error(message.toStdString()) {}
};

class ProjectLoadException : public std::runtime_error {
public:
    explicit ProjectLoadException(const QString &message) 
        : std::runtime_error(message.toStdString()) {}
};

class ProjectManager : public QObject
{
    Q_OBJECT

public:
    explicit ProjectManager(QObject *parent = nullptr);
    ~ProjectManager();

    // Enhanced from Sprint 1.1
    QString createProject(const QString &name, const QString &basePath);
    bool isValidProject(const QString &projectPath);
    ProjectInfo loadProject(const QString &projectPath);
    bool validateProjectMetadata(const QJsonObject &metadata);

    // New for Sprint 1.2
    bool hasScans(const QString &projectPath);
    QList<ScanInfo> getProjectScans(const QString &projectPath);
    SQLiteManager* getSQLiteManager() const { return m_sqliteManager; }
    ScanImportManager* getScanImportManager() const { return m_scanImportManager; }

    // New for Sprint 1.3 - Cluster Management
    QString createCluster(const QString &clusterName, const QString &parentClusterId = QString());
    bool deleteCluster(const QString &clusterId);
    bool renameCluster(const QString &clusterId, const QString &newName);
    QList<ClusterInfo> getProjectClusters();
    QList<ClusterInfo> getChildClusters(const QString &parentClusterId);
    bool moveScanToCluster(const QString &scanId, const QString &clusterId);
    bool moveScansToCluster(const QStringList &scanIds, const QString &clusterId);

    static QString getMetadataFilePath(const QString &projectPath);
    static bool isProjectDirectory(const QString &path);
    static QString getScansSubfolder(const QString &projectPath);
    static QString getDatabasePath(const QString &projectPath);

signals:
    void scansImported(const QList<ScanInfo> &scans);
    void projectScansChanged();
    void clusterCreated(const ClusterInfo &cluster);
    void clusterDeleted(const QString &clusterId);
    void clusterRenamed(const QString &clusterId, const QString &newName);
    void scanMovedToCluster(const QString &scanId, const QString &clusterId);

private:
    bool createProjectMetadata(const QString &projectPath, const QString &projectName);
    QJsonObject readProjectMetadata(const QString &projectPath);
    bool validateDirectoryPermissions(const QString &path, bool requireWrite = false);

    // New for Sprint 1.2
    bool createProjectDatabase(const QString &projectPath);
    bool initializeDatabaseSchema();
    void updateProjectMetadata(const QString &projectPath);

    SQLiteManager *m_sqliteManager;
    ScanImportManager *m_scanImportManager;
    ProjectInfo m_currentProject;

    static const QString METADATA_FILENAME;
    static const QString DATABASE_FILENAME;
    static const QString SCANS_SUBFOLDER;
    static const QString CURRENT_FORMAT_VERSION;
};

#endif // PROJECTMANAGER_H
