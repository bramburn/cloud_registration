#ifndef PROJECTTREEMODEL_H
#define PROJECTTREEMODEL_H

#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QList>
#include <QIcon>
#include <QHash>
#include <QDateTime>
#include <QSet>
#include "iconmanager.h"

class SQLiteManager;
struct ScanInfo;
struct ClusterInfo;

// Enhanced enum for tracking loaded state of scans and clusters (Sprint 2.1)
enum class LoadedState {
    Unloaded,       // Not loaded in memory
    Loaded,         // Fully loaded in memory
    Partial,        // Partially loaded (for clusters with some loaded scans)
    Loading,        // Currently being loaded
    Processing,     // Being processed (filtering, registration)
    Error,          // Error occurred during loading
    Cached,         // In LRU cache but not actively displayed
    MemoryWarning,  // Approaching memory limits
    Optimized       // Processed and ready for registration
};

class ProjectTreeModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit ProjectTreeModel(QObject *parent = nullptr);
    
    void setProject(const QString &projectName, const QString &projectPath);
    void refreshScans();
    void addScan(const ScanInfo &scan);
    void setSQLiteManager(SQLiteManager *manager);

    // New methods for Sprint 1.3 - Cluster support
    void refreshHierarchy();
    void addCluster(const ClusterInfo &cluster);
    void removeCluster(const QString &clusterId);
    void updateCluster(const ClusterInfo &cluster);
    void moveScanToCluster(const QString &scanId, const QString &clusterId);

    // Helper methods for tree navigation
    QStandardItem* findClusterItem(const QString &clusterId);
    QStandardItem* findScanItem(const QString &scanId);
    QString getItemId(QStandardItem *item) const;
    QString getItemType(QStandardItem *item) const;

    // Enhanced methods for Sprint 2.1 - Loaded state management
    void setScanLoadedState(const QString &scanId, LoadedState state);
    LoadedState getScanLoadedState(const QString &scanId) const;
    void updateClusterLoadedStates();
    LoadedState calculateClusterLoadedState(const QString &clusterId) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Sprint 2.1: Memory monitoring and warnings
    void setMemoryWarningThreshold(size_t thresholdMB);
    void updateMemoryInfo(const QString &scanId, size_t memoryUsage, size_t pointCount);
    size_t getTotalMemoryUsage() const;

    // Sprint 2.1: Batch operations support
    void setClusterState(const QString &clusterId, LoadedState state);
    QStringList getScansInState(LoadedState state) const;

    // Sprint 2.3 - Lock state management
    void setClusterLockState(const QString &clusterId, bool isLocked);
    bool getClusterLockState(const QString &clusterId) const;
    void refreshClusterLockStates();

    // Sprint 3.1 - Missing file support and data export
    void markScanAsMissing(const QString &scanId);
    void clearScanMissingFlag(const QString &scanId);
    bool isScanMissing(const QString &scanId) const;
    void updateScanFilePath(const QString &scanId, const QString &newPath);
    void removeScan(const QString &scanId);

    // Data export for persistence
    QList<ClusterInfo> getAllClusters() const;
    QList<ScanInfo> getAllScans() const;
    void populateFromData(const QList<ClusterInfo> &clusters, const QList<ScanInfo> &scans);

signals:
    // Sprint 2.1: Enhanced state management signals
    void scanStateChanged(const QString &scanId, LoadedState oldState, LoadedState newState);
    void memoryWarningTriggered(size_t currentUsage, size_t threshold);
    void memoryUsageChanged(size_t totalUsage);

private:
    void createProjectStructure();
    void loadScansFromDatabase();
    void loadClustersFromDatabase();
    void buildHierarchicalStructure();

    QStandardItem* createScanItem(const ScanInfo &scan);
    QStandardItem* createClusterItem(const ClusterInfo &cluster);
    QStandardItem* getOrCreateScansFolder();
    QStandardItem* getParentItem(const QString &parentClusterId);

    void setItemData(QStandardItem *item, const QString &id, const QString &type);
    
    QString m_projectName;
    QString m_projectPath;
    SQLiteManager *m_sqliteManager;
    QStandardItem *m_rootItem;
    QStandardItem *m_scansFolder;

    // Cache for quick lookups
    QHash<QString, QStandardItem*> m_clusterItems;
    QHash<QString, QStandardItem*> m_scanItems;

    // Enhanced for Sprint 2.1 - Loaded state tracking
    QHash<QString, LoadedState> m_scanLoadedStates;
    QHash<QString, LoadedState> m_clusterLoadedStates;

    // Sprint 2.1: Memory tracking
    QHash<QString, size_t> m_scanMemoryUsage;
    QHash<QString, size_t> m_scanPointCounts;
    size_t m_totalMemoryUsage;
    size_t m_memoryWarningThreshold;

    // Sprint 2.3 - Lock state tracking
    QHash<QString, bool> m_clusterLockStates;

    // Sprint 3.1 - Missing file tracking
    QSet<QString> m_missingScanIds;

    // Icons for different loaded states
    QIcon m_loadedIcon;
    QIcon m_unloadedIcon;
    QIcon m_partialIcon;
    QIcon m_loadingIcon;
    QIcon m_errorIcon;

    // Sprint 2.3 - Lock state icons
    QIcon m_lockedClusterIcon;
    QIcon m_unlockedClusterIcon;

    // Sprint 3.1 - Missing file icons
    QIcon m_missingFileIcon;

    // Enhanced custom data roles for Sprint 3.3
    enum CustomRoles {
        ScanIdRole = Qt::UserRole + 1,
        ClusterIdRole,
        IsMissingRole,
        ImportTypeRole,
        FilePathRole,
        ItemTypeRole,
        ItemStateRole,
        PointCountRole,
        FileSizeRole,
        DateAddedRole,
        ScanCountRole,
        SubClusterCountRole,
        IsLoadedRole,
        IsLockedRole,
        FullPathRole,
        DetailedStatusRole
    };

    void initializeIcons();
    void setItemLoadedState(QStandardItem *item, LoadedState state);

    // Sprint 3.3 - Enhanced tooltip and icon support
    QString generateScanTooltip(const ScanInfo& scan) const;
    QString generateClusterTooltip(const ClusterInfo& cluster) const;
    QString formatFileSize(qint64 bytes) const;
    QString formatPointCount(qint64 points) const;
    QString getImportTypeString(ImportType type) const;
    ItemState convertLoadedStateToItemState(LoadedState state) const;
    ImportType getItemImportType(QStandardItem* item) const;
};

#endif // PROJECTTREEMODEL_H
