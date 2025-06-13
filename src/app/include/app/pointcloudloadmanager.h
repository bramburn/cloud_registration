#ifndef POINTCLOUDLOADMANAGER_H
#define POINTCLOUDLOADMANAGER_H

#include <QObject>
#include <QString>

#include <vector>

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
