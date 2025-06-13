#include "core/projectmanager.h"

#include <QDebug>

#include "core/ProjectStateService.h"

// Constants
const QString ProjectManager::METADATA_FILENAME = "project.json";
const QString ProjectManager::DATABASE_FILENAME = "project.db";
const QString ProjectManager::SCANS_SUBFOLDER = "Scans";
const QString ProjectManager::CURRENT_FORMAT_VERSION = "1.0";
const QString ProjectManager::BACKUP_SUFFIX = ".backup";

ProjectManager::ProjectManager(QObject* parent)
    : QObject(parent), m_projectStateService(new ProjectStateService(this)), m_recentProjectsManager(nullptr)
{
    // Initialize member variables
    m_currentProject = ProjectInfo();
    m_currentProjectPath = QString();
    m_metadata = ProjectMetadata();
    m_lastError = QString();
    m_detailedError = QString();

    // Create stub for RecentProjectsManager
    m_recentProjectsManager = nullptr;  // Will be properly initialized when UI components are integrated

    // Signal connections will be properly implemented when UI components are integrated
    // For Sprint 7, we focus on the modular structure without signal forwarding
}

ProjectManager::~ProjectManager() = default;

// Delegation methods to ProjectStateService
QString ProjectManager::createProject(const QString& name, const QString& basePath)
{
    return m_projectStateService->createProject(basePath, name);
}

ProjectLoadResult ProjectManager::loadProject(const QString& projectPath)
{
    SaveResult result = m_projectStateService->loadProject(projectPath);
    return static_cast<ProjectLoadResult>(result);
}

SaveResult ProjectManager::saveProject()
{
    return m_projectStateService->saveProject();
}

void ProjectManager::closeProject()
{
    m_projectStateService->closeProject();
}

bool ProjectManager::isProjectOpen() const
{
    return m_projectStateService->isProjectOpen();
}

ProjectInfo ProjectManager::currentProject() const
{
    return m_projectStateService->currentProject();
}

ProjectMetadata ProjectManager::currentMetadata() const
{
    return m_projectStateService->currentMetadata();
}

QString ProjectManager::currentProjectPath() const
{
    return m_projectStateService->currentProjectPath();
}

QString ProjectManager::createCluster(const QString& clusterName, const QString& parentClusterId)
{
    return m_projectStateService->createCluster(clusterName, parentClusterId);
}

bool ProjectManager::deleteCluster(const QString& clusterId)
{
    return m_projectStateService->deleteCluster(clusterId);
}

bool ProjectManager::renameCluster(const QString& clusterId, const QString& newName)
{
    return m_projectStateService->renameCluster(clusterId, newName);
}

QList<ClusterInfo> ProjectManager::getProjectClusters()
{
    return m_projectStateService->getProjectClusters();
}

QList<ClusterInfo> ProjectManager::getChildClusters(const QString& parentClusterId)
{
    return m_projectStateService->getChildClusters(parentClusterId);
}

bool ProjectManager::moveScanToCluster(const QString& scanId, const QString& clusterId)
{
    return m_projectStateService->moveScanToCluster(scanId, clusterId);
}

bool ProjectManager::moveScansToCluster(const QStringList& scanIds, const QString& clusterId)
{
    return m_projectStateService->moveScansToCluster(scanIds, clusterId);
}

QStringList ProjectManager::getScansInCluster(const QString& clusterId)
{
    return m_projectStateService->getScansInCluster(clusterId);
}

bool ProjectManager::setClusterLockState(const QString& clusterId, bool isLocked)
{
    return m_projectStateService->setClusterLockState(clusterId, isLocked);
}

bool ProjectManager::getClusterLockState(const QString& clusterId)
{
    return m_projectStateService->getClusterLockState(clusterId);
}

bool ProjectManager::deleteClusterRecursive(const QString& clusterId, bool deletePhysicalFiles)
{
    return m_projectStateService->deleteClusterRecursive(clusterId, deletePhysicalFiles);
}

bool ProjectManager::deleteScan(const QString& scanId, bool deletePhysicalFile)
{
    return m_projectStateService->deleteScan(scanId, deletePhysicalFile);
}

// Utility methods that can remain in ProjectManager
bool ProjectManager::isValidProject(const QString& projectPath)
{
    return m_projectStateService->isValidProject(projectPath);
}

bool ProjectManager::isProjectDirectory(const QString& path)
{
    return ProjectStateService::isProjectDirectory(path);
}

QString ProjectManager::lastError() const
{
    return m_projectStateService->lastError();
}

QString ProjectManager::detailedError() const
{
    return m_projectStateService->detailedError();
}

// Recent projects management (delegated to stub for now)
QStringList ProjectManager::getRecentProjects() const
{
    // Stub implementation - will be properly implemented when UI components are integrated
    return QStringList();
}

void ProjectManager::addRecentProject(const QString& projectPath)
{
    // Stub implementation - will be properly implemented when UI components are integrated
    Q_UNUSED(projectPath)
}

void ProjectManager::removeRecentProject(const QString& projectPath)
{
    // Stub implementation - will be properly implemented when UI components are integrated
    Q_UNUSED(projectPath)
}

void ProjectManager::clearRecentProjects()
{
    // Stub implementation - will be properly implemented when UI components are integrated
}
