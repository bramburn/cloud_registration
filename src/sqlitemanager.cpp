#include "sqlitemanager.h"
#include "projectmanager.h" // For ScanInfo struct
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QDir>
#include <QUuid>
#include <QDateTime>
#include <QDebug>
#include <QVariant>

const QString SQLiteManager::SCANS_TABLE_SCHEMA = R"(
CREATE TABLE IF NOT EXISTS scans (
    scan_id TEXT PRIMARY KEY,
    project_id TEXT NOT NULL,
    scan_name TEXT NOT NULL,
    file_path_relative TEXT NOT NULL,
    import_type TEXT NOT NULL CHECK (import_type IN ('COPIED', 'MOVED')),
    date_added TEXT NOT NULL,
    parent_cluster_id TEXT,
    UNIQUE(file_path_relative),
    FOREIGN KEY (parent_cluster_id) REFERENCES clusters(cluster_id) ON DELETE SET NULL
)
)";

const QString SQLiteManager::CLUSTERS_TABLE_SCHEMA = R"(
CREATE TABLE IF NOT EXISTS clusters (
    cluster_id TEXT PRIMARY KEY,
    project_id TEXT NOT NULL,
    cluster_name TEXT NOT NULL,
    parent_cluster_id TEXT,
    creation_date TEXT NOT NULL,
    FOREIGN KEY (parent_cluster_id) REFERENCES clusters(cluster_id) ON DELETE CASCADE
)
)";

SQLiteManager::SQLiteManager(QObject *parent)
    : QObject(parent)
    , m_connectionName(generateConnectionName())
{
}

SQLiteManager::~SQLiteManager()
{
    closeDatabase();
}

bool SQLiteManager::createDatabase(const QString &dbPath)
{
    // Ensure directory exists
    QFileInfo dbInfo(dbPath);
    QDir().mkpath(dbInfo.absolutePath());
    
    m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_database.setDatabaseName(dbPath);
    
    if (!m_database.open()) {
        qWarning() << "Failed to create database:" << m_database.lastError().text();
        return false;
    }
    
    m_currentDatabasePath = dbPath;
    
    if (!initializeSchema()) {
        qWarning() << "Failed to initialize database schema";
        closeDatabase();
        return false;
    }
    
    qInfo() << "Database created successfully:" << dbPath;
    return true;
}

bool SQLiteManager::openDatabase(const QString &dbPath)
{
    if (m_database.isOpen() && m_currentDatabasePath == dbPath) {
        return true; // Already open
    }
    
    closeDatabase(); // Close any existing connection
    
    if (!QFileInfo::exists(dbPath)) {
        qWarning() << "Database file does not exist:" << dbPath;
        return false;
    }
    
    m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_database.setDatabaseName(dbPath);
    
    if (!m_database.open()) {
        qWarning() << "Failed to open database:" << m_database.lastError().text();
        return false;
    }
    
    m_currentDatabasePath = dbPath;
    qDebug() << "Database opened successfully:" << dbPath;
    return true;
}

void SQLiteManager::closeDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
        qDebug() << "Database closed:" << m_currentDatabasePath;
    }
    
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }
    
    m_currentDatabasePath.clear();
}

bool SQLiteManager::initializeSchema()
{
    if (!m_database.isOpen()) {
        return false;
    }

    // Create clusters table first due to foreign key dependency
    if (!createClustersTable()) {
        return false;
    }

    if (!createScansTable()) {
        return false;
    }

    // Add parent_cluster_id column to existing scans table if it doesn't exist
    return addParentClusterIdToScans();
}

bool SQLiteManager::createScansTable()
{
    QSqlQuery query(m_database);

    if (!query.exec(SCANS_TABLE_SCHEMA)) {
        qCritical() << "Failed to create scans table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Scans table created successfully";
    return true;
}

bool SQLiteManager::createClustersTable()
{
    QSqlQuery query(m_database);

    if (!query.exec(CLUSTERS_TABLE_SCHEMA)) {
        qCritical() << "Failed to create clusters table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Clusters table created successfully";
    return true;
}

bool SQLiteManager::addParentClusterIdToScans()
{
    // Check if parent_cluster_id column already exists
    QSqlQuery checkQuery(m_database);
    checkQuery.exec("PRAGMA table_info(scans)");

    bool columnExists = false;
    while (checkQuery.next()) {
        if (checkQuery.value("name").toString() == "parent_cluster_id") {
            columnExists = true;
            break;
        }
    }

    if (!columnExists) {
        QSqlQuery alterQuery(m_database);
        if (!alterQuery.exec("ALTER TABLE scans ADD COLUMN parent_cluster_id TEXT")) {
            qWarning() << "Failed to add parent_cluster_id column:" << alterQuery.lastError().text();
            return false;
        }
        qDebug() << "Added parent_cluster_id column to scans table";
    }

    return true;
}

bool SQLiteManager::insertScan(const ScanInfo &scan)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database not open";
        return false;
    }
    
    if (!scan.isValid()) {
        qWarning() << "Invalid scan info provided";
        return false;
    }
    
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO scans (scan_id, project_id, scan_name, file_path_relative, import_type, date_added, parent_cluster_id)
        VALUES (:scan_id, :project_id, :scan_name, :file_path_relative, :import_type, :date_added, :parent_cluster_id)
    )");

    query.bindValue(":scan_id", scan.scanId);
    query.bindValue(":project_id", scan.projectId);
    query.bindValue(":scan_name", scan.scanName);
    query.bindValue(":file_path_relative", scan.filePathRelative);
    query.bindValue(":import_type", scan.importType);
    query.bindValue(":date_added", scan.dateAdded);
    query.bindValue(":parent_cluster_id", scan.parentClusterId.isEmpty() ? QVariant() : scan.parentClusterId);
    
    if (!query.exec()) {
        qWarning() << "Failed to insert scan:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Scan inserted successfully:" << scan.scanName;
    return true;
}

bool SQLiteManager::insertScans(const QList<ScanInfo> &scans)
{
    if (!m_database.isOpen()) {
        return false;
    }
    
    // Use transaction for atomicity
    m_database.transaction();
    
    bool allSuccess = true;
    for (const ScanInfo &scan : scans) {
        if (!insertScan(scan)) {
            allSuccess = false;
            break;
        }
    }
    
    if (allSuccess) {
        m_database.commit();
    } else {
        m_database.rollback();
    }
    
    return allSuccess;
}

QList<ScanInfo> SQLiteManager::getAllScans()
{
    QList<ScanInfo> scans;
    
    if (!m_database.isOpen()) {
        return scans;
    }
    
    QSqlQuery query("SELECT * FROM scans ORDER BY date_added", m_database);
    
    while (query.next()) {
        ScanInfo scan;
        scan.scanId = query.value("scan_id").toString();
        scan.projectId = query.value("project_id").toString();
        scan.scanName = query.value("scan_name").toString();
        scan.filePathRelative = query.value("file_path_relative").toString();
        scan.importType = query.value("import_type").toString();
        scan.dateAdded = query.value("date_added").toString();
        scan.parentClusterId = query.value("parent_cluster_id").toString();

        scans.append(scan);
    }
    
    return scans;
}

ScanInfo SQLiteManager::getScanById(const QString &scanId)
{
    ScanInfo scan;
    
    if (!m_database.isOpen()) {
        return scan;
    }
    
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM scans WHERE scan_id = :scan_id");
    query.bindValue(":scan_id", scanId);
    
    if (query.exec() && query.next()) {
        scan.scanId = query.value("scan_id").toString();
        scan.projectId = query.value("project_id").toString();
        scan.scanName = query.value("scan_name").toString();
        scan.filePathRelative = query.value("file_path_relative").toString();
        scan.importType = query.value("import_type").toString();
        scan.dateAdded = query.value("date_added").toString();
        scan.parentClusterId = query.value("parent_cluster_id").toString();
    }
    
    return scan;
}

bool SQLiteManager::deleteScan(const QString &scanId)
{
    if (!m_database.isOpen()) {
        return false;
    }
    
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM scans WHERE scan_id = :scan_id");
    query.bindValue(":scan_id", scanId);
    
    if (!query.exec()) {
        qWarning() << "Failed to delete scan:" << query.lastError().text();
        return false;
    }
    
    return query.numRowsAffected() > 0;
}

int SQLiteManager::getScanCount()
{
    if (!m_database.isOpen()) {
        return 0;
    }
    
    QSqlQuery query("SELECT COUNT(*) FROM scans", m_database);
    if (query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

QSqlError SQLiteManager::lastError() const
{
    return m_database.lastError();
}

bool SQLiteManager::isConnected() const
{
    return m_database.isOpen();
}

bool SQLiteManager::executeQuery(const QString &query)
{
    if (!m_database.isOpen()) {
        return false;
    }
    
    QSqlQuery sqlQuery(m_database);
    return sqlQuery.exec(query);
}

QString SQLiteManager::generateConnectionName()
{
    return QString("SQLiteManager_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}

// New cluster-related methods for Sprint 1.3
QList<ScanInfo> SQLiteManager::getScansByCluster(const QString &clusterId)
{
    QList<ScanInfo> scans;

    if (!m_database.isOpen()) {
        return scans;
    }

    QSqlQuery query(m_database);
    if (clusterId.isEmpty()) {
        // Get scans at project root (parent_cluster_id is NULL)
        query.prepare("SELECT * FROM scans WHERE parent_cluster_id IS NULL ORDER BY date_added");
    } else {
        query.prepare("SELECT * FROM scans WHERE parent_cluster_id = :cluster_id ORDER BY date_added");
        query.bindValue(":cluster_id", clusterId);
    }

    if (query.exec()) {
        while (query.next()) {
            ScanInfo scan;
            scan.scanId = query.value("scan_id").toString();
            scan.projectId = query.value("project_id").toString();
            scan.scanName = query.value("scan_name").toString();
            scan.filePathRelative = query.value("file_path_relative").toString();
            scan.importType = query.value("import_type").toString();
            scan.dateAdded = query.value("date_added").toString();
            scan.parentClusterId = query.value("parent_cluster_id").toString();

            scans.append(scan);
        }
    }

    return scans;
}

bool SQLiteManager::updateScanCluster(const QString &scanId, const QString &clusterId)
{
    if (!m_database.isOpen()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE scans SET parent_cluster_id = :cluster_id WHERE scan_id = :scan_id");
    query.bindValue(":cluster_id", clusterId.isEmpty() ? QVariant() : clusterId);
    query.bindValue(":scan_id", scanId);

    if (!query.exec()) {
        qWarning() << "Failed to update scan cluster:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool SQLiteManager::insertCluster(const ClusterInfo &cluster)
{
    if (!m_database.isOpen()) {
        qWarning() << "Database not open";
        return false;
    }

    if (!cluster.isValid()) {
        qWarning() << "Invalid cluster info provided";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO clusters (cluster_id, project_id, cluster_name, parent_cluster_id, creation_date)
        VALUES (:cluster_id, :project_id, :cluster_name, :parent_cluster_id, :creation_date)
    )");

    query.bindValue(":cluster_id", cluster.clusterId);
    query.bindValue(":project_id", cluster.projectId);
    query.bindValue(":cluster_name", cluster.clusterName);
    query.bindValue(":parent_cluster_id", cluster.parentClusterId.isEmpty() ? QVariant() : cluster.parentClusterId);
    query.bindValue(":creation_date", cluster.creationDate);

    if (!query.exec()) {
        qWarning() << "Failed to insert cluster:" << query.lastError().text();
        return false;
    }

    qDebug() << "Cluster inserted successfully:" << cluster.clusterName;
    return true;
}

QList<ClusterInfo> SQLiteManager::getAllClusters()
{
    QList<ClusterInfo> clusters;

    if (!m_database.isOpen()) {
        return clusters;
    }

    QSqlQuery query("SELECT * FROM clusters ORDER BY creation_date", m_database);

    while (query.next()) {
        ClusterInfo cluster;
        cluster.clusterId = query.value("cluster_id").toString();
        cluster.projectId = query.value("project_id").toString();
        cluster.clusterName = query.value("cluster_name").toString();
        cluster.parentClusterId = query.value("parent_cluster_id").toString();
        cluster.creationDate = query.value("creation_date").toString();

        clusters.append(cluster);
    }

    return clusters;
}

QList<ClusterInfo> SQLiteManager::getChildClusters(const QString &parentClusterId)
{
    QList<ClusterInfo> clusters;

    if (!m_database.isOpen()) {
        return clusters;
    }

    QSqlQuery query(m_database);
    if (parentClusterId.isEmpty()) {
        // Get top-level clusters (parent_cluster_id is NULL)
        query.prepare("SELECT * FROM clusters WHERE parent_cluster_id IS NULL ORDER BY creation_date");
    } else {
        query.prepare("SELECT * FROM clusters WHERE parent_cluster_id = :parent_id ORDER BY creation_date");
        query.bindValue(":parent_id", parentClusterId);
    }

    if (query.exec()) {
        while (query.next()) {
            ClusterInfo cluster;
            cluster.clusterId = query.value("cluster_id").toString();
            cluster.projectId = query.value("project_id").toString();
            cluster.clusterName = query.value("cluster_name").toString();
            cluster.parentClusterId = query.value("parent_cluster_id").toString();
            cluster.creationDate = query.value("creation_date").toString();

            clusters.append(cluster);
        }
    }

    return clusters;
}

ClusterInfo SQLiteManager::getClusterById(const QString &clusterId)
{
    ClusterInfo cluster;

    if (!m_database.isOpen()) {
        return cluster;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM clusters WHERE cluster_id = :cluster_id");
    query.bindValue(":cluster_id", clusterId);

    if (query.exec() && query.next()) {
        cluster.clusterId = query.value("cluster_id").toString();
        cluster.projectId = query.value("project_id").toString();
        cluster.clusterName = query.value("cluster_name").toString();
        cluster.parentClusterId = query.value("parent_cluster_id").toString();
        cluster.creationDate = query.value("creation_date").toString();
    }

    return cluster;
}

bool SQLiteManager::deleteCluster(const QString &clusterId)
{
    if (!m_database.isOpen()) {
        return false;
    }

    // Start transaction for atomicity
    m_database.transaction();

    // First, move all scans in this cluster to project root
    QSqlQuery updateScansQuery(m_database);
    updateScansQuery.prepare("UPDATE scans SET parent_cluster_id = NULL WHERE parent_cluster_id = :cluster_id");
    updateScansQuery.bindValue(":cluster_id", clusterId);

    if (!updateScansQuery.exec()) {
        qWarning() << "Failed to update scans when deleting cluster:" << updateScansQuery.lastError().text();
        m_database.rollback();
        return false;
    }

    // Delete the cluster (CASCADE will handle child clusters)
    QSqlQuery deleteQuery(m_database);
    deleteQuery.prepare("DELETE FROM clusters WHERE cluster_id = :cluster_id");
    deleteQuery.bindValue(":cluster_id", clusterId);

    if (!deleteQuery.exec()) {
        qWarning() << "Failed to delete cluster:" << deleteQuery.lastError().text();
        m_database.rollback();
        return false;
    }

    bool success = deleteQuery.numRowsAffected() > 0;
    if (success) {
        m_database.commit();
    } else {
        m_database.rollback();
    }

    return success;
}

bool SQLiteManager::updateCluster(const ClusterInfo &cluster)
{
    if (!m_database.isOpen()) {
        return false;
    }

    if (!cluster.isValid()) {
        qWarning() << "Invalid cluster info provided";
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE clusters
        SET cluster_name = :cluster_name, parent_cluster_id = :parent_cluster_id
        WHERE cluster_id = :cluster_id
    )");

    query.bindValue(":cluster_name", cluster.clusterName);
    query.bindValue(":parent_cluster_id", cluster.parentClusterId.isEmpty() ? QVariant() : cluster.parentClusterId);
    query.bindValue(":cluster_id", cluster.clusterId);

    if (!query.exec()) {
        qWarning() << "Failed to update cluster:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

int SQLiteManager::getClusterCount()
{
    if (!m_database.isOpen()) {
        return 0;
    }

    QSqlQuery query("SELECT COUNT(*) FROM clusters", m_database);
    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}
