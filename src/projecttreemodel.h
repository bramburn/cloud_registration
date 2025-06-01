#ifndef PROJECTTREEMODEL_H
#define PROJECTTREEMODEL_H

#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QList>
#include <QIcon>
#include <QHash>

class SQLiteManager;
struct ScanInfo;
struct ClusterInfo;

// Enum for tracking loaded state of scans and clusters
enum class LoadedState {
    Unloaded,       // Not loaded in memory
    Loaded,         // Fully loaded in memory
    Partial,        // Partially loaded (for clusters with some loaded scans)
    Loading,        // Currently being loaded
    Error           // Error occurred during loading
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

    // New methods for Sprint 2.1 - Loaded state management
    void setScanLoadedState(const QString &scanId, LoadedState state);
    LoadedState getScanLoadedState(const QString &scanId) const;
    void updateClusterLoadedStates();
    LoadedState calculateClusterLoadedState(const QString &clusterId) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

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

    // New for Sprint 2.1 - Loaded state tracking
    QHash<QString, LoadedState> m_scanLoadedStates;
    QHash<QString, LoadedState> m_clusterLoadedStates;

    // Icons for different loaded states
    QIcon m_loadedIcon;
    QIcon m_unloadedIcon;
    QIcon m_partialIcon;
    QIcon m_loadingIcon;
    QIcon m_errorIcon;

    void initializeIcons();
    void setItemLoadedState(QStandardItem *item, LoadedState state);
};

#endif // PROJECTTREEMODEL_H
