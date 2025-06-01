#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QTreeView>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>

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
    void setProjectManager(ProjectManager *manager);
    void refreshFromDatabase();
    void addScan(const ScanInfo &scan);

    // New methods for Sprint 1.3
    void addCluster(const ClusterInfo &cluster);
    void removeCluster(const QString &clusterId);
    void updateCluster(const ClusterInfo &cluster);

signals:
    void clusterCreated(const ClusterInfo &cluster);
    void clusterDeleted(const QString &clusterId);
    void clusterRenamed(const QString &clusterId, const QString &newName);
    void scanMovedToCluster(const QString &scanId, const QString &clusterId);

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

private:
    void setupUI();
    void setupDragDrop();
    void createContextMenu();
    QStandardItem* getItemAt(const QPoint &position);
    QString promptForClusterName(const QString &title = "Create Cluster", const QString &defaultName = "");
    bool canDropOn(QStandardItem *item, const QString &draggedType);

    ProjectTreeModel *m_model;
    ProjectManager *m_projectManager;
    QString m_currentProjectPath;

    // Context menu and actions
    QMenu *m_contextMenu;
    QAction *m_createClusterAction;
    QAction *m_createSubClusterAction;
    QAction *m_renameClusterAction;
    QAction *m_deleteClusterAction;

    // Current context item for menu actions
    QStandardItem *m_contextItem;
};

#endif // SIDEBARWIDGET_H
