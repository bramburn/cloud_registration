#ifndef POINTCLOUDLOADMANAGER_H
#define POINTCLOUDLOADMANAGER_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QUuid>
#include <QTimer>
#include <QMutex>
#include <memory>
#include <vector>

// Forward declarations
class SQLiteManager;
class ProjectTreeModel;
struct ScanInfo;

// Include the LoadedState enum from ProjectTreeModel
#include "projecttreemodel.h"

// Structure to hold loaded point cloud data
struct PointCloudData {
    std::vector<float> points;
    size_t pointCount = 0;
    QString filePath;
    QDateTime loadTime;
    size_t memoryUsage = 0; // in bytes
    
    bool isValid() const {
        return !points.empty() && pointCount > 0;
    }
    
    void clear() {
        points.clear();
        pointCount = 0;
        memoryUsage = 0;
    }
};

// Structure to track scan loading state
struct ScanLoadState {
    QString scanId;
    LoadedState state = LoadedState::Unloaded;
    std::unique_ptr<PointCloudData> data;
    QString errorMessage;
    QDateTime lastAccessed;
    
    ScanLoadState() = default;
    ScanLoadState(const QString &id) : scanId(id) {}
    
    // Move constructor and assignment
    ScanLoadState(ScanLoadState &&other) noexcept
        : scanId(std::move(other.scanId))
        , state(other.state)
        , data(std::move(other.data))
        , errorMessage(std::move(other.errorMessage))
        , lastAccessed(other.lastAccessed) {}
    
    ScanLoadState& operator=(ScanLoadState &&other) noexcept {
        if (this != &other) {
            scanId = std::move(other.scanId);
            state = other.state;
            data = std::move(other.data);
            errorMessage = std::move(other.errorMessage);
            lastAccessed = other.lastAccessed;
        }
        return *this;
    }
    
    // Delete copy constructor and assignment
    ScanLoadState(const ScanLoadState&) = delete;
    ScanLoadState& operator=(const ScanLoadState&) = delete;
};

class PointCloudLoadManager : public QObject
{
    Q_OBJECT

public:
    explicit PointCloudLoadManager(QObject *parent = nullptr);
    ~PointCloudLoadManager();

    // Initialization
    void setSQLiteManager(SQLiteManager *manager);
    void setProjectTreeModel(ProjectTreeModel *model);

    // Individual scan operations
    bool loadScan(const QString &scanId);
    bool unloadScan(const QString &scanId);
    LoadedState getScanLoadedState(const QString &scanId) const;
    
    // Cluster operations
    bool loadCluster(const QString &clusterId);
    bool unloadCluster(const QString &clusterId);
    QStringList getClusterScanIds(const QString &clusterId) const;
    
    // View operations (placeholder for Sprint 2.1)
    bool viewPointCloud(const QString &itemId, const QString &itemType);
    
    // Memory management
    size_t getTotalMemoryUsage() const;
    void enforceMemoryLimit();
    void setMemoryLimit(size_t limitMB);
    
    // State queries
    bool isScanLoaded(const QString &scanId) const;
    QStringList getLoadedScans() const;
    
    // Error handling
    QString getLastError() const;

public slots:
    void onLoadScanRequested(const QString &scanId);
    void onUnloadScanRequested(const QString &scanId);
    void onLoadClusterRequested(const QString &clusterId);
    void onUnloadClusterRequested(const QString &clusterId);
    void onViewPointCloudRequested(const QString &itemId, const QString &itemType);

signals:
    void scanLoaded(const QString &scanId);
    void scanUnloaded(const QString &scanId);
    void scanLoadFailed(const QString &scanId, const QString &error);
    void clusterLoaded(const QString &clusterId);
    void clusterUnloaded(const QString &clusterId);
    void memoryLimitExceeded(size_t currentUsage, size_t limit);
    void loadingProgress(const QString &scanId, int percentage);

private slots:
    void onMemoryCheckTimer();

private:
    // Core loading functionality
    bool loadScanData(const QString &scanId);
    bool unloadScanData(const QString &scanId);
    std::unique_ptr<PointCloudData> parsePointCloudFile(const QString &filePath);
    
    // Memory management
    void updateMemoryUsage();
    void evictLeastRecentlyUsed();
    
    // Helper methods
    QString getScanFilePath(const QString &scanId) const;
    void updateScanState(const QString &scanId, LoadedState state, const QString &error = QString());
    void logMemoryUsage() const;

    // Member variables
    SQLiteManager *m_sqliteManager;
    ProjectTreeModel *m_treeModel;
    
    // State tracking
    QHash<QString, std::unique_ptr<ScanLoadState>> m_scanStates;
    mutable QMutex m_stateMutex;
    
    // Memory management
    size_t m_memoryLimitMB;
    size_t m_currentMemoryUsage;
    QTimer *m_memoryCheckTimer;
    
    // Error handling
    QString m_lastError;
    
    // Constants
    static const size_t DEFAULT_MEMORY_LIMIT_MB = 2048; // 2GB default
    static const int MEMORY_CHECK_INTERVAL_MS = 30000;  // 30 seconds
};

#endif // POINTCLOUDLOADMANAGER_H
