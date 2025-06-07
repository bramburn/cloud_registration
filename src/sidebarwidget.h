#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include "pointcloudloadmanager.h"

class ProjectTreeModel;
class SQLiteManager;
class ProjectManager;
struct ScanInfo;
struct ClusterInfo;

class SidebarWidget : public QTreeView
{
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget *parent = nullptr);
    void setProject(const QString &projectName, const QString &projectPath);
    void clearProject();
    void setSQLiteManager(SQLiteManager *manager);

    // Sprint 4: Remove direct manager dependencies - use signals instead
    // void setProjectManager(ProjectManager *manager);
    // void setPointCloudLoadManager(PointCloudLoadManager *manager);
    void refreshFromDatabase();
    void addScan(const ScanInfo &scan);

    // Sprint 3.2: Model access for load manager
    ProjectTreeModel* getModel() const { return m_model; }

    // New methods for Sprint 1.3
    void addCluster(const ClusterInfo &cluster);
    void removeCluster(const QString &clusterId);
    void updateCluster(const ClusterInfo &cluster);

signals:
    void clusterCreated(const ClusterInfo &cluster);
    void clusterDeleted(const QString &clusterId);
    void clusterRenamed(const QString &clusterId, const QString &newName);
    void scanMovedToCluster(const QString &scanId, const QString &clusterId);

    // Sprint 4: New signals for business logic delegation to MainPresenter
    void clusterCreationRequested(const QString &clusterName, const QString &parentClusterId);
    void clusterRenameRequested(const QString &clusterId, const QString &newName);
    void clusterDeletionRequested(const QString &clusterId);
    void dragDropOperationRequested(const QStringList &draggedItems, const QString &draggedType,
                                   const QString &targetItemId, const QString &targetType);

    // New signals for Sprint 2.1
    void loadScanRequested(const QString &scanId);
    void unloadScanRequested(const QString &scanId);
    void loadClusterRequested(const QString &clusterId);
    void unloadClusterRequested(const QString &clusterId);
    void viewPointCloudRequested(const QString &itemId, const QString &itemType);

    // Sprint 2.1: Enhanced signals
    void preprocessScanRequested(const QString &scanId);
    void optimizeScanRequested(const QString &scanId);
    void batchOperationRequested(const QString &operation, const QStringList &scanIds);
    void memoryOptimizationRequested();
    void filterMovingObjectsRequested(const QString &scanId);
    void colorBalanceRequested(const QString &scanId);
    void registrationPreviewRequested(const QString &scanId);

    // Sprint 2.3 - New signals
    void lockClusterRequested(const QString &clusterId);
    void unlockClusterRequested(const QString &clusterId);
    void deleteScanRequested(const QString &scanId, bool deletePhysicalFile);
    void deleteClusterRequested(const QString &clusterId, bool deletePhysicalFiles);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void startDrag(Qt::DropActions supportedActions) override;

private slots:
    void onCreateCluster();
    void onCreateSubCluster();
    void onRenameCluster();
    void onDeleteCluster();

    // Enhanced slots for Sprint 2.1
    void onLoadScan();
    void onUnloadScan();
    void onLoadCluster();
    void onUnloadCluster();
    void onViewPointCloud();

    // Sprint 2.1: Advanced operation slots
    void onPreprocessScan();
    void onOptimizeScan();
    void onBatchLoad();
    void onBatchUnload();
    void onMemoryOptimize();
    void onFilterMovingObjects();
    void onColorBalance();
    void onRegistrationPreview();

    // Sprint 2.3 - New slots
    void onLockCluster();
    void onUnlockCluster();
    void onDeleteScan();
    void onDeleteClusterRecursive();

private:
    void setupUI();
    void setupDragDrop();
    void createContextMenu();
    QStandardItem* getItemAt(const QPoint &position);
    QString promptForClusterName(const QString &title = "Create Cluster", const QString &defaultName = "");
    bool canDropOn(QStandardItem *item, const QString &draggedType);

    // Sprint 2.1: Helper methods for batch operations
    QStringList getSelectedScanIds() const;
    QString getItemIdFromIndex(const QModelIndex &index) const;
    QString getItemTypeFromIndex(const QModelIndex &index) const;

    ProjectTreeModel *m_model;
    QString m_currentProjectPath;

    // Sprint 4: Remove direct manager dependencies
    // ProjectManager *m_projectManager;
    // PointCloudLoadManager *m_loadManager;

    // Context menu and actions
    QMenu *m_contextMenu;
    QAction *m_createClusterAction;
    QAction *m_createSubClusterAction;
    QAction *m_renameClusterAction;
    QAction *m_deleteClusterAction;

    // Enhanced actions for Sprint 2.1
    QAction *m_loadScanAction;
    QAction *m_unloadScanAction;
    QAction *m_loadClusterAction;
    QAction *m_unloadClusterAction;
    QAction *m_viewPointCloudAction;

    // Sprint 2.1: Advanced operations
    QAction *m_preprocessScanAction;
    QAction *m_optimizeScanAction;
    QAction *m_batchLoadAction;
    QAction *m_batchUnloadAction;
    QAction *m_memoryOptimizeAction;

    // Sprint 2.1: Advanced submenu
    QMenu *m_advancedMenu;
    QAction *m_filterMovingObjectsAction;
    QAction *m_colorBalanceAction;
    QAction *m_registrationPreviewAction;

    // Sprint 2.3 - New actions
    QAction *m_lockClusterAction;
    QAction *m_unlockClusterAction;
    QAction *m_deleteScanAction;
    QAction *m_deleteClusterRecursiveAction;

    // Current context item for menu actions
    QStandardItem *m_contextItem;
};

#endif // SIDEBARWIDGET_H
