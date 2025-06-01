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
    
    // Utility
    QSqlError lastError() const;
    bool isConnected() const;

private:
    bool createScansTable();
    bool createClustersTable();
    bool addParentClusterIdToScans();
    bool executeQuery(const QString &query);
    QString generateConnectionName();
    
    QSqlDatabase m_database;
    QString m_connectionName;
    QString m_currentDatabasePath;
    
    static const QString SCANS_TABLE_SCHEMA;
    static const QString CLUSTERS_TABLE_SCHEMA;
};

#endif // SQLITEMANAGER_H
