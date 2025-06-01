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
    UNIQUE(file_path_relative)
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
    
    return createScansTable();
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
        INSERT INTO scans (scan_id, project_id, scan_name, file_path_relative, import_type, date_added)
        VALUES (:scan_id, :project_id, :scan_name, :file_path_relative, :import_type, :date_added)
    )");
    
    query.bindValue(":scan_id", scan.scanId);
    query.bindValue(":project_id", scan.projectId);
    query.bindValue(":scan_name", scan.scanName);
    query.bindValue(":file_path_relative", scan.filePathRelative);
    query.bindValue(":import_type", scan.importType);
    query.bindValue(":date_added", scan.dateAdded);
    
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
