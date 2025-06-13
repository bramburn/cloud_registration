#include "core/sqlitemanager.h"

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
