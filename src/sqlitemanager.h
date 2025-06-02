#ifndef SQLITEMANAGER_H
#define SQLITEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QList>
#include <QUuid>

struct ScanInfo; // Forward declaration
struct ClusterInfo; // Forward declaration

class SQLiteManager : public QObject
{
    Q_OBJECT

public:
    explicit SQLiteManager(QObject *parent = nullptr);
    ~SQLiteManager();
    
    bool createDatabase(const QString &dbPath);
    bool openDatabase(const QString &dbPath);
    void closeDatabase();
    bool initializeSchema();
    
    // Scan operations
    bool insertScan(const ScanInfo &scan);
    bool insertScans(const QList<ScanInfo> &scans);
    QList<ScanInfo> getAllScans();
    QList<ScanInfo> getScansByCluster(const QString &clusterId);
    ScanInfo getScanById(const QString &scanId);
    bool deleteScan(const QString &scanId);
    bool updateScanCluster(const QString &scanId, const QString &clusterId);
    int getScanCount();

    // Cluster operations
    bool insertCluster(const ClusterInfo &cluster);
    QList<ClusterInfo> getAllClusters();
    QList<ClusterInfo> getChildClusters(const QString &parentClusterId);
    ClusterInfo getClusterById(const QString &clusterId);
    bool deleteCluster(const QString &clusterId);
    bool updateCluster(const ClusterInfo &cluster);
    int getClusterCount();

    // Sprint 2.3 - Cluster locking operations
    bool setClusterLockState(const QString &clusterId, bool isLocked);
    bool getClusterLockState(const QString &clusterId);

    // Sprint 2.3 - Enhanced deletion operations
    bool deleteClusterRecursive(const QString &clusterId);
    QStringList getClusterScanPaths(const QString &clusterId, const QString &projectPath);
    QStringList getChildClusterIds(const QString &clusterId);

    // Sprint 3.1 - Transactional operations and integrity checks
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool saveAllClusters(const QList<ClusterInfo> &clusters);
    bool saveAllScans(const QList<ScanInfo> &scans);
    QList<ClusterInfo> loadAllClusters();
    QList<ScanInfo> loadAllScans();
    bool validateReferentialIntegrity();
    bool updateScanFilePath(const QString &scanId, const QString &newPath);
    bool createDatabaseBackup(const QString &backupPath);

    // Utility
    QSqlError lastError() const;
    bool isConnected() const;

private:
    bool createScansTable();
    bool createClustersTable();
    bool addParentClusterIdToScans();
    bool executeQuery(const QString &query);
    QString generateConnectionName();

    // Sprint 2.3 - Schema migration
    bool migrateToVersion3();
    int getCurrentSchemaVersion();
    bool updateSchemaVersion(int version);
    
    QSqlDatabase m_database;
    QString m_connectionName;
    QString m_currentDatabasePath;
    
    static const QString SCANS_TABLE_SCHEMA;
    static const QString CLUSTERS_TABLE_SCHEMA;
};

#endif // SQLITEMANAGER_H
