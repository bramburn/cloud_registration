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
    file_path_project_relative TEXT,
    file_path_absolute_linked TEXT,
    import_type TEXT NOT NULL CHECK (import_type IN ('COPIED', 'MOVED', 'LINKED')),
    original_source_path TEXT,
    point_count_estimate INTEGER DEFAULT 0,
    bounding_box_min_x REAL,
    bounding_box_min_y REAL,
    bounding_box_min_z REAL,
    bounding_box_max_x REAL,
    bounding_box_max_y REAL,
    bounding_box_max_z REAL,
    date_added TEXT NOT NULL,
    scan_file_last_modified TEXT,
    parent_cluster_id TEXT,
    FOREIGN KEY (parent_cluster_id) REFERENCES clusters(cluster_id) ON DELETE SET NULL,
    CHECK (
        (import_type = 'LINKED' AND file_path_absolute_linked IS NOT NULL AND file_path_project_relative IS NULL) OR
        (import_type IN ('COPIED', 'MOVED') AND file_path_project_relative IS NOT NULL AND file_path_absolute_linked IS NULL)
    )
)
)";

const QString SQLiteManager::CLUSTERS_TABLE_SCHEMA = R"(
CREATE TABLE IF NOT EXISTS clusters (
    cluster_id TEXT PRIMARY KEY,
    project_id TEXT NOT NULL,
    cluster_name TEXT NOT NULL,
    parent_cluster_id TEXT,
    creation_date TEXT NOT NULL,
    is_locked BOOLEAN DEFAULT 0 NOT NULL,
    FOREIGN KEY (parent_cluster_id) REFERENCES clusters(cluster_id) ON DELETE CASCADE
)
)";

// Sprint 3.4: Registration data table schemas
const QString SQLiteManager::REGISTRATION_STATUS_TABLE_SCHEMA = R"(
CREATE TABLE IF NOT EXISTS registration_status (
    item_id TEXT PRIMARY KEY,
    item_type TEXT NOT NULL CHECK (item_type IN ('SCAN', 'CLUSTER')),
    status TEXT NOT NULL CHECK (status IN (
        'UNREGISTERED', 'PROCESSING', 'REGISTERED_MANUAL',
        'REGISTERED_AUTO', 'FAILED_REGISTRATION', 'NEEDS_REVIEW'
    )),
    error_metric_value REAL,
    error_metric_type TEXT,
    last_registration_date TEXT,
    FOREIGN KEY (item_id) REFERENCES scans(scan_id) ON DELETE CASCADE
)
)";

const QString SQLiteManager::TRANSFORMATION_MATRICES_TABLE_SCHEMA = R"(
CREATE TABLE IF NOT EXISTS transformation_matrices (
    item_id TEXT PRIMARY KEY,
    item_type TEXT NOT NULL CHECK (item_type IN ('SCAN', 'CLUSTER')),
    matrix_data BLOB NOT NULL,
    relative_to_item_id TEXT,
    last_transform_date TEXT,
    FOREIGN KEY (item_id) REFERENCES scans(scan_id) ON DELETE CASCADE
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

    // Check current schema version and migrate if needed
    int currentVersion = getCurrentSchemaVersion();
    if (currentVersion < 3) {
        if (!migrateToVersion3()) {
            return false;
        }
    }

    // Sprint 3.4: Check for version 4 migration (registration tables)
    if (currentVersion < 4) {
        if (!migrateToVersion4()) {
            return false;
        }
    }

    // Create clusters table first due to foreign key dependency
    if (!createClustersTable()) {
        return false;
    }

    if (!createScansTable()) {
        return false;
    }

    // Add parent_cluster_id column to existing scans table if it doesn't exist
    if (!addParentClusterIdToScans()) {
        return false;
    }

    // Sprint 3.4: Create registration tables
    return createRegistrationTables();
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
        INSERT INTO scans (
            scan_id, project_id, scan_name,
            file_path_project_relative, file_path_absolute_linked,
            import_type, original_source_path,
            point_count_estimate,
            bounding_box_min_x, bounding_box_min_y, bounding_box_min_z,
            bounding_box_max_x, bounding_box_max_y, bounding_box_max_z,
            date_added, scan_file_last_modified, parent_cluster_id
        )
        VALUES (
            :scan_id, :project_id, :scan_name,
            :file_path_project_relative, :file_path_absolute_linked,
            :import_type, :original_source_path,
            :point_count_estimate,
            :bounding_box_min_x, :bounding_box_min_y, :bounding_box_min_z,
            :bounding_box_max_x, :bounding_box_max_y, :bounding_box_max_z,
            :date_added, :scan_file_last_modified, :parent_cluster_id
        )
    )");

    query.bindValue(":scan_id", scan.scanId);
    query.bindValue(":project_id", scan.projectId);
    query.bindValue(":scan_name", scan.scanName);
    query.bindValue(":file_path_project_relative", scan.filePathRelative.isEmpty() ? QVariant() : scan.filePathRelative);
    query.bindValue(":file_path_absolute_linked", scan.filePathAbsoluteLinked.isEmpty() ? QVariant() : scan.filePathAbsoluteLinked);
    query.bindValue(":import_type", scan.importType);
    query.bindValue(":original_source_path", scan.originalSourcePath.isEmpty() ? QVariant() : scan.originalSourcePath);
    query.bindValue(":point_count_estimate", scan.pointCountEstimate);
    query.bindValue(":bounding_box_min_x", scan.boundingBoxMinX);
    query.bindValue(":bounding_box_min_y", scan.boundingBoxMinY);
    query.bindValue(":bounding_box_min_z", scan.boundingBoxMinZ);
    query.bindValue(":bounding_box_max_x", scan.boundingBoxMaxX);
    query.bindValue(":bounding_box_max_y", scan.boundingBoxMaxY);
    query.bindValue(":bounding_box_max_z", scan.boundingBoxMaxZ);
    query.bindValue(":date_added", scan.dateAdded);
    query.bindValue(":scan_file_last_modified", scan.scanFileLastModified.isEmpty() ? QVariant() : scan.scanFileLastModified);
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
        scan.filePathRelative = query.value("file_path_project_relative").toString();
        scan.filePathAbsoluteLinked = query.value("file_path_absolute_linked").toString();
        scan.importType = query.value("import_type").toString();
        scan.originalSourcePath = query.value("original_source_path").toString();
        scan.pointCountEstimate = query.value("point_count_estimate").toInt();
        scan.boundingBoxMinX = query.value("bounding_box_min_x").toDouble();
        scan.boundingBoxMinY = query.value("bounding_box_min_y").toDouble();
        scan.boundingBoxMinZ = query.value("bounding_box_min_z").toDouble();
        scan.boundingBoxMaxX = query.value("bounding_box_max_x").toDouble();
        scan.boundingBoxMaxY = query.value("bounding_box_max_y").toDouble();
        scan.boundingBoxMaxZ = query.value("bounding_box_max_z").toDouble();
        scan.dateAdded = query.value("date_added").toString();
        scan.scanFileLastModified = query.value("scan_file_last_modified").toString();
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
        scan.filePathRelative = query.value("file_path_project_relative").toString();
        scan.filePathAbsoluteLinked = query.value("file_path_absolute_linked").toString();
        scan.importType = query.value("import_type").toString();
        scan.originalSourcePath = query.value("original_source_path").toString();
        scan.pointCountEstimate = query.value("point_count_estimate").toInt();
        scan.boundingBoxMinX = query.value("bounding_box_min_x").toDouble();
        scan.boundingBoxMinY = query.value("bounding_box_min_y").toDouble();
        scan.boundingBoxMinZ = query.value("bounding_box_min_z").toDouble();
        scan.boundingBoxMaxX = query.value("bounding_box_max_x").toDouble();
        scan.boundingBoxMaxY = query.value("bounding_box_max_y").toDouble();
        scan.boundingBoxMaxZ = query.value("bounding_box_max_z").toDouble();
        scan.dateAdded = query.value("date_added").toString();
        scan.scanFileLastModified = query.value("scan_file_last_modified").toString();
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
            scan.filePathRelative = query.value("file_path_project_relative").toString();
            scan.filePathAbsoluteLinked = query.value("file_path_absolute_linked").toString();
            scan.importType = query.value("import_type").toString();
            scan.originalSourcePath = query.value("original_source_path").toString();
            scan.pointCountEstimate = query.value("point_count_estimate").toInt();
            scan.boundingBoxMinX = query.value("bounding_box_min_x").toDouble();
            scan.boundingBoxMinY = query.value("bounding_box_min_y").toDouble();
            scan.boundingBoxMinZ = query.value("bounding_box_min_z").toDouble();
            scan.boundingBoxMaxX = query.value("bounding_box_max_x").toDouble();
            scan.boundingBoxMaxY = query.value("bounding_box_max_y").toDouble();
            scan.boundingBoxMaxZ = query.value("bounding_box_max_z").toDouble();
            scan.dateAdded = query.value("date_added").toString();
            scan.scanFileLastModified = query.value("scan_file_last_modified").toString();
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
        cluster.isLocked = query.value("is_locked").toBool();

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
            cluster.isLocked = query.value("is_locked").toBool();

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
        cluster.isLocked = query.value("is_locked").toBool();
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

// Sprint 2.3 - Schema migration methods
int SQLiteManager::getCurrentSchemaVersion()
{
    if (!m_database.isOpen()) {
        return 0;
    }

    // Check if schema_version table exists
    QSqlQuery checkQuery(m_database);
    checkQuery.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='schema_version'");

    if (!checkQuery.next()) {
        // Create schema_version table if it doesn't exist
        QSqlQuery createQuery(m_database);
        if (!createQuery.exec("CREATE TABLE schema_version (version INTEGER)")) {
            qWarning() << "Failed to create schema_version table:" << createQuery.lastError().text();
            return 0;
        }

        // Insert initial version
        QSqlQuery insertQuery(m_database);
        if (!insertQuery.exec("INSERT INTO schema_version (version) VALUES (2)")) {
            qWarning() << "Failed to insert initial schema version:" << insertQuery.lastError().text();
            return 0;
        }
        return 2;
    }

    // Get current version
    QSqlQuery versionQuery(m_database);
    if (versionQuery.exec("SELECT version FROM schema_version") && versionQuery.next()) {
        return versionQuery.value(0).toInt();
    }

    return 0;
}

bool SQLiteManager::updateSchemaVersion(int version)
{
    if (!m_database.isOpen()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE schema_version SET version = ?");
    query.addBindValue(version);

    if (!query.exec()) {
        qWarning() << "Failed to update schema version:" << query.lastError().text();
        return false;
    }

    return true;
}

bool SQLiteManager::migrateToVersion3()
{
    if (!m_database.isOpen()) {
        return false;
    }

    qInfo() << "Migrating database schema to version 3...";

    // Start transaction
    m_database.transaction();

    try {
        // Check if is_locked column already exists
        QSqlQuery checkQuery(m_database);
        checkQuery.exec("PRAGMA table_info(clusters)");

        bool columnExists = false;
        while (checkQuery.next()) {
            if (checkQuery.value("name").toString() == "is_locked") {
                columnExists = true;
                break;
            }
        }

        if (!columnExists) {
            // Add is_locked column to clusters table
            QSqlQuery alterQuery(m_database);
            if (!alterQuery.exec("ALTER TABLE clusters ADD COLUMN is_locked BOOLEAN DEFAULT 0 NOT NULL")) {
                throw std::runtime_error("Failed to add is_locked column: " + alterQuery.lastError().text().toStdString());
            }
            qDebug() << "Added is_locked column to clusters table";
        }

        // Update schema version
        if (!updateSchemaVersion(3)) {
            throw std::runtime_error("Failed to update schema version");
        }

        m_database.commit();
        qInfo() << "Database migration to version 3 completed successfully";
        return true;

    } catch (const std::exception& e) {
        m_database.rollback();
        qCritical() << "Database migration failed:" << e.what();
        return false;
    }
}

// Sprint 2.3 - Cluster locking operations
bool SQLiteManager::setClusterLockState(const QString &clusterId, bool isLocked)
{
    if (!m_database.isOpen()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE clusters SET is_locked = ? WHERE cluster_id = ?");
    query.addBindValue(isLocked);
    query.addBindValue(clusterId);

    if (!query.exec()) {
        qWarning() << "Failed to set cluster lock state:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool SQLiteManager::getClusterLockState(const QString &clusterId)
{
    if (!m_database.isOpen()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT is_locked FROM clusters WHERE cluster_id = ?");
    query.addBindValue(clusterId);

    if (query.exec() && query.next()) {
        return query.value(0).toBool();
    }

    return false;
}

// Sprint 2.3 - Enhanced deletion operations
QStringList SQLiteManager::getChildClusterIds(const QString &clusterId)
{
    QStringList childIds;

    if (!m_database.isOpen()) {
        return childIds;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT cluster_id FROM clusters WHERE parent_cluster_id = ?");
    query.addBindValue(clusterId);

    if (query.exec()) {
        while (query.next()) {
            childIds.append(query.value(0).toString());
        }
    }

    return childIds;
}

QStringList SQLiteManager::getClusterScanPaths(const QString &clusterId, const QString &projectPath)
{
    QStringList scanPaths;

    if (!m_database.isOpen()) {
        return scanPaths;
    }

    QSqlQuery query(m_database);
    query.prepare(R"(
        SELECT file_path_project_relative, file_path_absolute_linked, import_type
        FROM scans
        WHERE parent_cluster_id = ?
    )");
    query.addBindValue(clusterId);

    if (query.exec()) {
        while (query.next()) {
            QString importType = query.value("import_type").toString();
            QString filePath;

            if (importType == "LINKED") {
                filePath = query.value("file_path_absolute_linked").toString();
            } else if (importType == "COPIED" || importType == "MOVED") {
                QString relativePath = query.value("file_path_project_relative").toString();
                if (!relativePath.isEmpty() && !projectPath.isEmpty()) {
                    filePath = QDir(projectPath).absoluteFilePath(relativePath);
                }
            }

            if (!filePath.isEmpty()) {
                scanPaths.append(filePath);
            }
        }
    }

    return scanPaths;
}

bool SQLiteManager::deleteClusterRecursive(const QString &clusterId)
{
    if (!m_database.isOpen()) {
        return false;
    }

    // Start transaction for atomicity
    m_database.transaction();

    try {
        // Get all child clusters recursively
        QStringList allClusters;
        QStringList toProcess;
        toProcess.append(clusterId);

        while (!toProcess.isEmpty()) {
            QString currentId = toProcess.takeFirst();
            allClusters.append(currentId);

            // Get child clusters
            QStringList children = getChildClusterIds(currentId);
            toProcess.append(children);
        }

        // Delete all scans in all clusters (from deepest to shallowest)
        for (int i = allClusters.size() - 1; i >= 0; --i) {
            const QString &clusterIdToDelete = allClusters[i];

            // Delete scans in this cluster
            QSqlQuery deleteScanQuery(m_database);
            deleteScanQuery.prepare("DELETE FROM scans WHERE parent_cluster_id = ?");
            deleteScanQuery.bindValue(0, clusterIdToDelete);

            if (!deleteScanQuery.exec()) {
                throw std::runtime_error("Failed to delete scans in cluster: " + deleteScanQuery.lastError().text().toStdString());
            }
        }

        // Delete all clusters (from deepest to shallowest)
        for (int i = allClusters.size() - 1; i >= 0; --i) {
            const QString &clusterIdToDelete = allClusters[i];

            QSqlQuery deleteClusterQuery(m_database);
            deleteClusterQuery.prepare("DELETE FROM clusters WHERE cluster_id = ?");
            deleteClusterQuery.bindValue(0, clusterIdToDelete);

            if (!deleteClusterQuery.exec()) {
                throw std::runtime_error("Failed to delete cluster: " + deleteClusterQuery.lastError().text().toStdString());
            }
        }

        m_database.commit();
        qDebug() << "Recursively deleted cluster and all children:" << clusterId;
        return true;

    } catch (const std::exception& e) {
        m_database.rollback();
        qWarning() << "Failed to recursively delete cluster:" << e.what();
        return false;
    }
}

// Sprint 3.1 - Transactional operations and integrity checks
bool SQLiteManager::beginTransaction() {
    if (!m_database.isOpen()) {
        return false;
    }
    return m_database.transaction();
}

bool SQLiteManager::commitTransaction() {
    if (!m_database.isOpen()) {
        return false;
    }
    return m_database.commit();
}

bool SQLiteManager::rollbackTransaction() {
    if (!m_database.isOpen()) {
        return false;
    }
    return m_database.rollback();
}

bool SQLiteManager::saveAllClusters(const QList<ClusterInfo> &clusters) {
    if (!m_database.isOpen()) {
        return false;
    }

    // Clear existing clusters and insert new ones
    QSqlQuery deleteQuery(m_database);
    if (!deleteQuery.exec("DELETE FROM clusters")) {
        qWarning() << "Failed to clear clusters table:" << deleteQuery.lastError().text();
        return false;
    }

    // Insert all clusters
    for (const ClusterInfo &cluster : clusters) {
        if (!insertCluster(cluster)) {
            return false;
        }
    }

    return true;
}

bool SQLiteManager::saveAllScans(const QList<ScanInfo> &scans) {
    if (!m_database.isOpen()) {
        return false;
    }

    // Clear existing scans and insert new ones
    QSqlQuery deleteQuery(m_database);
    if (!deleteQuery.exec("DELETE FROM scans")) {
        qWarning() << "Failed to clear scans table:" << deleteQuery.lastError().text();
        return false;
    }

    // Insert all scans
    for (const ScanInfo &scan : scans) {
        if (!insertScan(scan)) {
            return false;
        }
    }

    return true;
}

QList<ClusterInfo> SQLiteManager::loadAllClusters() {
    return getAllClusters();
}

QList<ScanInfo> SQLiteManager::loadAllScans() {
    return getAllScans();
}

bool SQLiteManager::validateReferentialIntegrity() {
    if (!m_database.isOpen()) {
        return false;
    }

    try {
        QSqlQuery query(m_database);

        // Check for orphaned scans (scans with non-existent parent clusters)
        query.prepare(R"(
            SELECT COUNT(*) FROM scans s
            LEFT JOIN clusters c ON s.parent_cluster_id = c.cluster_id
            WHERE s.parent_cluster_id IS NOT NULL AND c.cluster_id IS NULL
        )");

        if (!query.exec()) {
            qWarning() << "Failed to check orphaned scans:" << query.lastError().text();
            return false;
        }

        if (query.next() && query.value(0).toInt() > 0) {
            qWarning() << "Found orphaned scans with invalid parent cluster references";
            return false;
        }

        // Check for circular cluster references (simplified check)
        query.prepare(R"(
            SELECT COUNT(*) FROM clusters c1
            JOIN clusters c2 ON c1.cluster_id = c2.parent_cluster_id
            WHERE c1.parent_cluster_id = c2.cluster_id
        )");

        if (!query.exec()) {
            qWarning() << "Failed to check circular references:" << query.lastError().text();
            return false;
        }

        if (query.next() && query.value(0).toInt() > 0) {
            qWarning() << "Found circular references in cluster hierarchy";
            return false;
        }

        return true;

    } catch (const std::exception& ex) {
        qWarning() << "Exception during integrity validation:" << ex.what();
        return false;
    }
}

bool SQLiteManager::updateScanFilePath(const QString &scanId, const QString &newPath) {
    if (!m_database.isOpen()) {
        return false;
    }

    QSqlQuery query(m_database);
    query.prepare("UPDATE scans SET file_path_absolute_linked = :new_path WHERE scan_id = :scan_id");
    query.bindValue(":new_path", newPath);
    query.bindValue(":scan_id", scanId);

    if (!query.exec()) {
        qWarning() << "Failed to update scan file path:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool SQLiteManager::createDatabaseBackup(const QString &backupPath) {
    if (!m_database.isOpen()) {
        return false;
    }

    try {
        QSqlQuery query(m_database);
        QString backupSql = QString("VACUUM INTO '%1'").arg(backupPath);

        if (!query.exec(backupSql)) {
            qWarning() << "Failed to create database backup:" << query.lastError().text();
            return false;
        }

        return true;

    } catch (const std::exception& ex) {
        qWarning() << "Exception creating backup:" << ex.what();
        return false;
    }
}

// Sprint 3.4: Registration data table implementation
bool SQLiteManager::createRegistrationTables()
{
    if (!m_database.isOpen()) {
        return false;
    }

    // Create registration status table
    QSqlQuery query(m_database);
    if (!query.exec(REGISTRATION_STATUS_TABLE_SCHEMA)) {
        qCritical() << "Failed to create registration_status table:" << query.lastError().text();
        return false;
    }
    qDebug() << "Registration status table created successfully";

    // Create transformation matrices table
    if (!query.exec(TRANSFORMATION_MATRICES_TABLE_SCHEMA)) {
        qCritical() << "Failed to create transformation_matrices table:" << query.lastError().text();
        return false;
    }
    qDebug() << "Transformation matrices table created successfully";

    return true;
}

bool SQLiteManager::migrateToVersion4()
{
    if (!m_database.isOpen()) {
        return false;
    }

    qInfo() << "Migrating database schema to version 4 (registration tables)...";

    // Start transaction
    m_database.transaction();

    try {
        // Create registration tables
        if (!createRegistrationTables()) {
            throw std::runtime_error("Failed to create registration tables");
        }

        // Update schema version
        if (!updateSchemaVersion(4)) {
            throw std::runtime_error("Failed to update schema version to 4");
        }

        // Commit transaction
        m_database.commit();
        qInfo() << "Successfully migrated to schema version 4";
        return true;

    } catch (const std::exception& ex) {
        m_database.rollback();
        qCritical() << "Failed to migrate to version 4:" << ex.what();
        return false;
    }
}
