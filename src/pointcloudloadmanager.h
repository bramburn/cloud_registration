#ifndef POINTCLOUDLOADMANAGER_H
#define POINTCLOUDLOADMANAGER_H

#include <QObject>
#include <QString>
#include <QHash>
#include <QMap>
#include <QUuid>
#include <QTimer>
#include <QMutex>
#include <QRandomGenerator>
#include <QDateTime>
#include <QFuture>
#include <QtConcurrent>
#include <memory>
#include <vector>

// Forward declarations
class SQLiteManager;
class ProjectTreeModel;
struct ScanInfo;

// Sprint 1.3: Include E57 data structures
struct PointData;
struct ScanMetadata;

// Include the LoadedState enum from ProjectTreeModel
#include "projecttreemodel.h"

// Structure to hold loaded point cloud data
struct PointCloudData {
    std::vector<float> points;
    size_t pointCount = 0;
    QString filePath;
    QDateTime loadTime;
    size_t memoryUsage = 0; // in bytes

    // Sprint 3.4: LOD support
    bool lodActive = false;
    std::vector<float> lodPoints;
    size_t lodPointCount = 0;
    float lodSubsampleRate = 0.5f;

    bool isValid() const {
        return !points.empty() && pointCount > 0;
    }

    void clear() {
        points.clear();
        pointCount = 0;
        memoryUsage = 0;
        // Clear LOD data
        lodPoints.clear();
        lodPointCount = 0;
        lodActive = false;
    }

    // Sprint 3.4: Calculate total memory usage including LOD
    size_t getTotalMemoryUsage() const {
        return memoryUsage + (lodPoints.size() * sizeof(float));
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
    
    // Sprint 3.2: Enhanced view operations for point cloud rendering
    bool viewPointCloud(const QString &itemId, const QString &itemType);
    bool viewScan(const QString &scanId);
    bool viewCluster(const QString &clusterId);

    // Sprint 1.3: E57-specific loading methods
    void loadE57Scan(const QString& filePath, const QString& scanGuid);

    // Get aggregated point cloud data for rendering
    std::vector<float> getAggregatedPointCloudData(const QStringList &scanIds);
    std::vector<float> getScanPointCloudData(const QString &scanId);

    // Sprint 3.4: LOD functionality
    QFuture<bool> loadScanWithLOD(const QString &scanId, float subsampleRate = 0.5f);
    std::vector<float> subsamplePointCloud(const std::vector<float> &points, float rate);
    void generateLODForScan(const QString &scanId, float subsampleRate = 0.5f);
    bool isLODActive(const QString &scanId) const;
    void setLODActive(const QString &scanId, bool active);
    std::vector<float> getLODPointCloudData(const QString &scanId);

    // Memory management
    size_t getTotalMemoryUsage() const;
    void enforceMemoryLimit();
    void setMemoryLimit(size_t limitMB);

    // Sprint 3.4: Enhanced memory tracking
    size_t getScanMemoryUsage(const QString &scanId) const;
    size_t getClusterMemoryUsage(const QString &clusterId) const;
    
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

    // Sprint 2.1: Enhanced operation slots
    void onPreprocessScanRequested(const QString &scanId);
    void onOptimizeScanRequested(const QString &scanId);
    void onBatchOperationRequested(const QString &operation, const QStringList &scanIds);
    void onMemoryOptimizationRequested();
    void onFilterMovingObjectsRequested(const QString &scanId);
    void onColorBalanceRequested(const QString &scanId);
    void onRegistrationPreviewRequested(const QString &scanId);

signals:
    void scanLoaded(const QString &scanId);
    void scanUnloaded(const QString &scanId);
    void scanLoadFailed(const QString &scanId, const QString &error);
    void clusterLoaded(const QString &clusterId);
    void clusterUnloaded(const QString &clusterId);
    void memoryLimitExceeded(size_t currentUsage, size_t limit);
    void loadingProgress(const QString &scanId, int percentage);

    // Sprint 3.2: Point cloud viewing signals
    void pointCloudDataReady(const std::vector<float> &points, const QString &sourceInfo);
    void pointCloudViewFailed(const QString &error);

    // Sprint 1.3: E57-specific loading signals
    void loadingStarted(const QString& message);
    void loadingCompleted();
    void statusUpdate(const QString& status);

    // Sprint 2.1: Enhanced signals
    void batchOperationProgress(const QString &operation, int completed, int total);
    void preprocessingStarted(const QString &scanId);
    void preprocessingFinished(const QString &scanId, bool success);
    void optimizationStarted(const QString &scanId);
    void optimizationFinished(const QString &scanId, bool success);

    // Sprint 3.4: Memory usage and LOD signals
    void memoryUsageChanged(size_t totalBytes);
    void lodGenerationStarted(const QString &scanId);
    void lodGenerationFinished(const QString &scanId, bool success);
    void lodStateChanged(const QString &scanId, bool active);

private slots:
    void onMemoryCheckTimer();

private:
    // Core loading functionality
    bool loadScanData(const QString &scanId);
    bool unloadScanData(const QString &scanId);
    std::unique_ptr<PointCloudData> parsePointCloudFile(const QString &filePath);

    // Sprint 1.3: E57-specific helper methods
    void onE57ScanLoaded(const std::vector<float>& points, const QString& sourceInfo);
    void onLoadError(const QString& error);
    
    // Memory management
    void updateMemoryUsage();
    void evictLeastRecentlyUsed();

    // Sprint 2.1: Enhanced memory management
    void predictiveLoadCandidates();
    void optimizeMemoryUsage();
    size_t estimateScanMemoryUsage(const QString &scanId) const;

    // Sprint 2.1: Processing operations
    bool preprocessScan(const QString &scanId);
    bool optimizeScanForRegistration(const QString &scanId);
    void batchProcessScans(const QString &operation, const QStringList &scanIds);

    // Helper methods
    QString getScanFilePath(const QString &scanId) const;
    void updateScanState(const QString &scanId, LoadedState state, const QString &error = QString());
    void logMemoryUsage() const;

    // Member variables
    SQLiteManager *m_sqliteManager;
    ProjectTreeModel *m_treeModel;
    
    // State tracking - Use QMap instead of QHash for move-only types
    QMap<QString, std::unique_ptr<ScanLoadState>> m_scanStates;
    mutable QMutex m_stateMutex;
    
    // Memory management
    size_t m_memoryLimitMB;
    size_t m_currentMemoryUsage;
    QTimer *m_memoryCheckTimer;

    // Sprint 2.1: Enhanced memory management
    size_t m_predictiveLoadThreshold;
    QTimer *m_predictiveLoadTimer;
    QHash<QString, QStringList> m_clusterRelationships;

    // Error handling
    QString m_lastError;

    // Constants
    static const size_t DEFAULT_MEMORY_LIMIT_MB = 2048; // 2GB default
    static const int MEMORY_CHECK_INTERVAL_MS = 30000;  // 30 seconds
    static const size_t DEFAULT_PREDICTIVE_THRESHOLD_MB = 512; // 512MB for predictive loading
};

#endif // POINTCLOUDLOADMANAGER_H
