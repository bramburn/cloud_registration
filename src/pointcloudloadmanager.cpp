#include "pointcloudloadmanager.h"

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
