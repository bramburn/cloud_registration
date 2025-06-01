#include "recentprojectsmanager.h"
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

const QString RecentProjectsManager::SETTINGS_KEY = "recent_projects";

RecentProjectsManager::RecentProjectsManager(QObject *parent)
    : QObject(parent)
{
    // Use application-specific settings
    m_settings = new QSettings(this);
    loadRecentProjects();
}

void RecentProjectsManager::addProject(const QString &projectPath)
{
    if (projectPath.isEmpty()) {
        return;
    }
    
    QString canonicalPath = QDir(projectPath).canonicalPath();
    if (canonicalPath.isEmpty()) {
        qWarning() << "Cannot resolve canonical path for:" << projectPath;
        return;
    }
    
    // Remove if already exists (to move to top)
    m_recentProjects.removeAll(canonicalPath);
    
    // Add to top
    m_recentProjects.prepend(canonicalPath);
    
    ensureUniqueAndLimited();
    
    if (m_recentProjects.size() > 0) {
        saveRecentProjects();
        emit recentProjectsChanged();
    }
}

QStringList RecentProjectsManager::getRecentProjects() const
{
    return m_recentProjects;
}

void RecentProjectsManager::removeProject(const QString &projectPath)
{
    QString canonicalPath = QDir(projectPath).canonicalPath();
    if (canonicalPath.isEmpty()) {
        canonicalPath = projectPath; // Fallback to original path
    }
    
    int removed = m_recentProjects.removeAll(canonicalPath);
    if (removed > 0) {
        saveRecentProjects();
        emit recentProjectsChanged();
    }
}

void RecentProjectsManager::setRecentProjects(const QStringList &projects)
{
    m_recentProjects = projects;
    ensureUniqueAndLimited();
    saveRecentProjects();
    emit recentProjectsChanged();
}

void RecentProjectsManager::clearRecentProjects()
{
    if (!m_recentProjects.isEmpty()) {
        m_recentProjects.clear();
        saveRecentProjects();
        emit recentProjectsChanged();
    }
}

QString RecentProjectsManager::getProjectDisplayName(const QString &projectPath)
{
    QFileInfo info(projectPath);
    return info.baseName();
}

void RecentProjectsManager::ensureUniqueAndLimited()
{
    // Remove duplicates while preserving order
    QStringList uniqueProjects;
    for (const QString &project : m_recentProjects) {
        if (!uniqueProjects.contains(project)) {
            uniqueProjects.append(project);
        }
    }
    m_recentProjects = uniqueProjects;
    
    // Limit size
    while (m_recentProjects.size() > MAX_RECENT_PROJECTS) {
        m_recentProjects.removeLast();
    }
}

void RecentProjectsManager::loadRecentProjects()
{
    m_recentProjects = m_settings->value(SETTINGS_KEY).toStringList();
    ensureUniqueAndLimited();
}

void RecentProjectsManager::saveRecentProjects()
{
    m_settings->setValue(SETTINGS_KEY, m_recentProjects);
    m_settings->sync();
}
