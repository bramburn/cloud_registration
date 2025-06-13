#ifndef RECENTPROJECTSMANAGER_H
#define RECENTPROJECTSMANAGER_H

#include <QObject>
#include <QStringList>

class QSettings;

class RecentProjectsManager : public QObject
{
    Q_OBJECT

public:
    explicit RecentProjectsManager(QObject* parent = nullptr);

    void addProject(const QString& projectPath);
    QStringList getRecentProjects() const;
    void removeProject(const QString& projectPath);
    void setRecentProjects(const QStringList& projects);
    void clearRecentProjects();

    static QString getProjectDisplayName(const QString& projectPath);

signals:
    void recentProjectsChanged();

private:
    void saveRecentProjects();
    void loadRecentProjects();
    void ensureUniqueAndLimited();

    QStringList m_recentProjects;
    QSettings* m_settings;
    static const int MAX_RECENT_PROJECTS = 10;
    static const QString SETTINGS_KEY;
};

#endif  // RECENTPROJECTSMANAGER_H
