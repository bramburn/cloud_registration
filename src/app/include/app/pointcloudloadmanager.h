#ifndef POINTCLOUDLOADMANAGER_H
#define POINTCLOUDLOADMANAGER_H

#include <QObject>
#include <QString>

#include <vector>

#include "core/octree.h"

// Forward declaration
class PointCloud;

/**
 * @brief PointCloudLoadManager - Manages point cloud loading operations
 *
 * This is a stub implementation for Sprint 1 to resolve compilation issues.
 * Full implementation will be added in future sprints.
 */
class PointCloudLoadManager : public QObject
{
    Q_OBJECT

public:
    explicit PointCloudLoadManager(QObject* parent = nullptr);
    virtual ~PointCloudLoadManager() = default;

    // Basic loading methods
    void loadPointCloud(const QString& filePath);
    void cancelLoading();
    bool isLoading() const
    {
        return m_isLoading;
    }

    // Sprint 4: Sidebar integration methods
    bool loadScan(const QString& scanId);
    bool unloadScan(const QString& scanId);
    bool isScanLoaded(const QString& scanId) const;

    // Sprint 4.1: ICP integration methods
    PointCloud getLoadedPointCloud(const QString& scanId) const;
    QStringList getLoadedScans() const { return m_loadedScans; }

    // Sprint 6.1: Get loaded point data for deviation analysis
    std::vector<PointFullData> getLoadedPointFullData(const QString& scanId) const;

private:
    // Helper method for generating test sphere data
    void generateSpherePoints(std::vector<PointFullData>& points,
                             const QVector3D& center,
                             float radius,
                             int numPoints) const;

signals:
    void loadingStarted(const QString& filePath);
    void loadingProgress(int percentage, const QString& stage);
    void loadingFinished(bool success, const QString& message, const std::vector<float>& points);
    void loadingCancelled();

private:
    bool m_isLoading = false;
    QStringList m_loadedScans;  // Track loaded scans
};

#endif  // POINTCLOUDLOADMANAGER_H
