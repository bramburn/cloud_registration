#ifndef SQLITEMANAGER_H
#define SQLITEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @brief SQLiteManager - Manages SQLite database operations
 * 
 * This is a stub implementation for Sprint 1 to resolve compilation issues.
 * Full implementation will be added in future sprints.
 */
class SQLiteManager : public QObject {
    Q_OBJECT

public:
    explicit SQLiteManager(QObject *parent = nullptr);
    virtual ~SQLiteManager() = default;

    // Basic database operations
    bool openDatabase(const QString& databasePath);
    void closeDatabase();
    bool isOpen() const { return m_isOpen; }
    
    // Project operations
    bool createProject(const QString& projectName, const QString& projectPath);
    bool loadProject(const QString& projectPath);
    QStringList getRecentProjects();

signals:
    void databaseOpened(const QString& path);
    void databaseClosed();
    void projectCreated(const QString& projectPath);
    void projectLoaded(const QString& projectPath);

private:
    bool m_isOpen = false;
    QString m_currentDatabasePath;
};

#endif // SQLITEMANAGER_H
