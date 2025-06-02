#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>

class ProjectTreeModel;
class SQLiteManager;
class ProjectManager;
class PointCloudLoadManager;
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
    void setProjectManager(ProjectManager *manager);
    void setPointCloudLoadManager(PointCloudLoadManager *manager);
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

    // New signals for Sprint 2.1
    void loadScanRequested(const QString &scanId);
    void unloadScanRequested(const QString &scanId);
    void loadClusterRequested(const QString &clusterId);
    void unloadClusterRequested(const QString &clusterId);
    void viewPointCloudRequested(const QString &itemId, const QString &itemType);

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

    // New slots for Sprint 2.1
    void onLoadScan();
    void onUnloadScan();
    void onLoadCluster();
    void onUnloadCluster();
    void onViewPointCloud();

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

    ProjectTreeModel *m_model;
    ProjectManager *m_projectManager;
    PointCloudLoadManager *m_loadManager;
    QString m_currentProjectPath;

    // Context menu and actions
    QMenu *m_contextMenu;
    QAction *m_createClusterAction;
    QAction *m_createSubClusterAction;
    QAction *m_renameClusterAction;
    QAction *m_deleteClusterAction;

    // New actions for Sprint 2.1
    QAction *m_loadScanAction;
    QAction *m_unloadScanAction;
    QAction *m_loadClusterAction;
    QAction *m_unloadClusterAction;
    QAction *m_viewPointCloudAction;

    // Sprint 2.3 - New actions
    QAction *m_lockClusterAction;
    QAction *m_unlockClusterAction;
    QAction *m_deleteScanAction;
    QAction *m_deleteClusterRecursiveAction;

    // Current context item for menu actions
    QStandardItem *m_contextItem;
};

#endif // SIDEBARWIDGET_H
