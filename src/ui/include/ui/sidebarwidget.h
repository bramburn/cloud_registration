#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QTreeView>

// Forward declarations
class QStandardItemModel;
class QMenu;
class QAction;
class QContextMenuEvent;
class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QStandardItem;
class ProjectTreeModel;
class SQLiteManager;
struct ScanInfo;
struct ClusterInfo;

class SidebarWidget : public QTreeView
{
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget* parent = nullptr);
    void setProject(const QString& projectName, const QString& projectPath);
    void clearProject();
    void setSQLiteManager(SQLiteManager* manager);

    // Sprint 4: Remove direct manager dependencies - use signals instead
    // void setProjectManager(ProjectManager *manager);
    // void setPointCloudLoadManager(PointCloudLoadManager *manager);

    void refreshFromDatabase();
    void addScan(const ScanInfo& scan);
    void addCluster(const ClusterInfo& cluster);
    void removeCluster(const QString& clusterId);
    void updateCluster(const ClusterInfo& cluster);
    ProjectTreeModel* getModel() const
    {
        return m_model;
    }

signals:
    // Sprint 4: Signals for business logic delegation to MainPresenter
    void clusterCreationRequested(const QString& clusterName, const QString& parentClusterId);
    void clusterRenameRequested(const QString& clusterId, const QString& newName);
    void clusterDeletionRequested(const QString& clusterId, bool deletePhysicalFiles = false);
    void dragDropOperationRequested(const QStringList& draggedItems,
                                    const QString& draggedType,
                                    const QString& targetItemId,
                                    const QString& targetType);
    void lockClusterRequested(const QString& clusterId);
    void unlockClusterRequested(const QString& clusterId);
    void deleteScanRequested(const QString& scanId, bool deletePhysicalFile);

    // Signals for loading/viewing operations
    void loadScanRequested(const QString& scanId);
    void unloadScanRequested(const QString& scanId);
    void loadClusterRequested(const QString& clusterId);
    void unloadClusterRequested(const QString& clusterId);
    void viewPointCloudRequested(const QString& itemId, const QString& itemType);

    // Signals for advanced/batch operations
    void preprocessScanRequested(const QString& scanId);
    void optimizeScanRequested(const QString& scanId);
    void batchOperationRequested(const QString& operation, const QStringList& scanIds);
    void memoryOptimizationRequested();
    void filterMovingObjectsRequested(const QString& scanId);
    void colorBalanceRequested(const QString& scanId);
    void registrationPreviewRequested(const QString& scanId);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void startDrag(Qt::DropActions supportedActions) override;

private slots:
    void onCreateCluster();
    void onCreateSubCluster();
    void onRenameCluster();
    void onDeleteCluster();
    void onLoadScan();
    void onUnloadScan();
    void onLoadCluster();
    void onUnloadCluster();
    void onViewPointCloud();
    void onPreprocessScan();
    void onOptimizeScan();
    void onBatchLoad();
    void onBatchUnload();
    void onMemoryOptimize();
    void onFilterMovingObjects();
    void onColorBalance();
    void onRegistrationPreview();
    void onLockCluster();
    void onUnlockCluster();
    void onDeleteScan();
    void onDeleteClusterRecursive();

private:
    void setupUI();
    void setupDragDrop();
    void createContextMenu();
    QStandardItem* getItemAt(const QPoint& position);
    QString promptForClusterName(const QString& title = "Create Cluster", const QString& defaultName = "");
    bool canDropOn(QStandardItem* item, const QString& draggedType);
    QStringList getSelectedScanIds() const;
    QString getItemIdFromIndex(const QModelIndex& index) const;
    QString getItemTypeFromIndex(const QModelIndex& index) const;

    ProjectTreeModel* m_model;
    QString m_currentProjectPath;

    // Context menu and actions
    QMenu* m_contextMenu;
    QAction* m_createClusterAction;
    QAction* m_createSubClusterAction;
    QAction* m_renameClusterAction;
    QAction* m_deleteClusterAction;
    QAction* m_loadScanAction;
    QAction* m_unloadScanAction;
    QAction* m_loadClusterAction;
    QAction* m_unloadClusterAction;
    QAction* m_viewPointCloudAction;
    QAction* m_preprocessScanAction;
    QAction* m_optimizeScanAction;
    QAction* m_batchLoadAction;
    QAction* m_batchUnloadAction;
    QAction* m_memoryOptimizeAction;
    QMenu* m_advancedMenu;
    QAction* m_filterMovingObjectsAction;
    QAction* m_colorBalanceAction;
    QAction* m_registrationPreviewAction;
    QAction* m_lockClusterAction;
    QAction* m_unlockClusterAction;
    QAction* m_deleteScanAction;
    QAction* m_deleteClusterRecursiveAction;

    // Current context item for menu actions
    QStandardItem* m_contextItem;
};

#endif  // SIDEBARWIDGET_H
