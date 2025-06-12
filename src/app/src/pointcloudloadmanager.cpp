#include "app/pointcloudloadmanager.h"

PointCloudLoadManager::PointCloudLoadManager(QObject *parent)
    : QObject(parent)
{
}

void PointCloudLoadManager::loadPointCloud(const QString& filePath)
{
    m_isLoading = true;
    emit loadingStarted(filePath);
    
    // Stub implementation - actual loading logic will be added in future sprints
    emit loadingProgress(100, "Complete");
    emit loadingFinished(true, "Loaded successfully", std::vector<float>());
    m_isLoading = false;
}

void PointCloudLoadManager::cancelLoading()
{
    if (m_isLoading) {
        m_isLoading = false;
        emit loadingCancelled();
    }
}

// Sprint 4: Sidebar integration methods
bool PointCloudLoadManager::loadScan(const QString& scanId)
{
    if (scanId.isEmpty()) {
        return false;
    }

    if (m_loadedScans.contains(scanId)) {
        return true; // Already loaded
    }

    // Stub implementation - actual loading logic will be added in future sprints
    m_loadedScans.append(scanId);
    return true;
}

bool PointCloudLoadManager::unloadScan(const QString& scanId)
{
    if (scanId.isEmpty()) {
        return false;
    }

    if (!m_loadedScans.contains(scanId)) {
        return true; // Already unloaded
    }

    // Stub implementation - actual unloading logic will be added in future sprints
    m_loadedScans.removeAll(scanId);
    return true;
}

bool PointCloudLoadManager::isScanLoaded(const QString& scanId) const
{
    return m_loadedScans.contains(scanId);
}
