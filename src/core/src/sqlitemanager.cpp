#include "core/sqlitemanager.h"
#include "core/scaninfo.h"
#include "core/clusterinfo.h"

SQLiteManager::SQLiteManager(QObject* parent) : QObject(parent) {}

bool SQLiteManager::openDatabase(const QString& databasePath)
{
    m_currentDatabasePath = databasePath;
    m_isOpen = true;
    emit databaseOpened(databasePath);
    return true;
}

void SQLiteManager::closeDatabase()
{
    if (m_isOpen)
    {
        m_isOpen = false;
        emit databaseClosed();
        m_currentDatabasePath.clear();
    }
}

bool SQLiteManager::createProject(const QString& projectName, const QString& projectPath)
{
    Q_UNUSED(projectName)
    emit projectCreated(projectPath);
    return true;
}

bool SQLiteManager::loadProject(const QString& projectPath)
{
    emit projectLoaded(projectPath);
    return true;
}

QStringList SQLiteManager::getRecentProjects()
{
    // Stub implementation - return empty list
    return QStringList();
}

QList<ScanInfo> SQLiteManager::getAllScans() const
{
    // Stub implementation - return empty list for now
    // In a real implementation, this would query the database
    return QList<ScanInfo>();
}

QList<ClusterInfo> SQLiteManager::getAllClusters() const
{
    // Stub implementation - return empty list for now
    // In a real implementation, this would query the database
    return QList<ClusterInfo>();
}
