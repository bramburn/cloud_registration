#include "sidebarwidget.h"
#include "projecttreemodel.h"
#include "sqlitemanager.h"
#include "projectmanager.h"
#include "confirmationdialog.h"
#include <QStandardItem>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDrag>
#include <QMimeData>
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>

SidebarWidget::SidebarWidget(QWidget *parent)
    : QTreeView(parent)
    , m_model(nullptr)
    , m_projectManager(nullptr)
    , m_loadManager(nullptr)
    , m_contextMenu(nullptr)
    , m_contextItem(nullptr)
{
    setupUI();
    setupDragDrop();
    createContextMenu();
}

void SidebarWidget::setupUI()
{
    m_model = new ProjectTreeModel(this);
    setModel(m_model);
    setHeaderHidden(true);
    setMinimumWidth(200);
    setMaximumWidth(400);
    
    // Styling similar to modern IDEs
    setStyleSheet(R"(
        QTreeView {
            background-color: #2b2b2b;
            color: #ffffff;
            font-size: 14px;
            border: none;
            outline: none;
        }
        QTreeView::item {
            height: 30px;
            border: none;
            padding-left: 4px;
        }
        QTreeView::item:selected {
            background-color: #3d4348;
            color: #ffffff;
        }
        QTreeView::item:hover {
            background-color: #404040;
        }
        QTreeView::branch {
            background: transparent;
        }
        QTreeView::branch:has-children:!has-siblings:closed,
        QTreeView::branch:closed:has-children:has-siblings {
            border-image: none;
            image: url(:/icons/branch-closed.png);
        }
        QTreeView::branch:open:has-children:!has-siblings,
        QTreeView::branch:open:has-children:has-siblings {
            border-image: none;
            image: url(:/icons/branch-open.png);
        }
    )");
    
    // Set selection behavior
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection); // Allow multiple selection for drag-drop
}

void SidebarWidget::setProject(const QString &projectName, const QString &projectPath)
{
    m_currentProjectPath = projectPath;
    m_model->setProject(projectName, projectPath);
    expandAll();
}

void SidebarWidget::clearProject()
{
    m_currentProjectPath.clear();
    m_model->clear();
}

void SidebarWidget::setSQLiteManager(SQLiteManager *manager)
{
    m_model->setSQLiteManager(manager);
}

void SidebarWidget::setProjectManager(ProjectManager *manager)
{
    m_projectManager = manager;
}

void SidebarWidget::setPointCloudLoadManager(PointCloudLoadManager *manager)
{
    m_loadManager = manager;

    // Connect signals to load manager
    if (m_loadManager) {
        connect(this, &SidebarWidget::loadScanRequested,
                m_loadManager, &PointCloudLoadManager::onLoadScanRequested);
        connect(this, &SidebarWidget::unloadScanRequested,
                m_loadManager, &PointCloudLoadManager::onUnloadScanRequested);
        connect(this, &SidebarWidget::loadClusterRequested,
                m_loadManager, &PointCloudLoadManager::onLoadClusterRequested);
        connect(this, &SidebarWidget::unloadClusterRequested,
                m_loadManager, &PointCloudLoadManager::onUnloadClusterRequested);
        connect(this, &SidebarWidget::viewPointCloudRequested,
                m_loadManager, &PointCloudLoadManager::onViewPointCloudRequested);
    }
}

void SidebarWidget::refreshFromDatabase()
{
    if (m_model) {
        m_model->refreshScans();
        expandAll();
    }
}

void SidebarWidget::addScan(const ScanInfo &scan)
{
    if (m_model) {
        m_model->addScan(scan);
        expandAll();
    }
}

// New methods for Sprint 1.3
void SidebarWidget::addCluster(const ClusterInfo &cluster)
{
    if (m_model) {
        m_model->addCluster(cluster);
        expandAll();
    }
}

void SidebarWidget::removeCluster(const QString &clusterId)
{
    if (m_model) {
        m_model->removeCluster(clusterId);
    }
}

void SidebarWidget::updateCluster(const ClusterInfo &cluster)
{
    if (m_model) {
        m_model->updateCluster(cluster);
    }
}

void SidebarWidget::setupDragDrop()
{
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);
}

void SidebarWidget::createContextMenu()
{
    m_contextMenu = new QMenu(this);

    // Existing cluster actions
    m_createClusterAction = new QAction("New Cluster", this);
    m_createSubClusterAction = new QAction("New Sub-Cluster", this);
    m_renameClusterAction = new QAction("Rename", this);
    m_deleteClusterAction = new QAction("Delete", this);

    // New Sprint 2.1 actions
    m_loadScanAction = new QAction("Load Scan", this);
    m_unloadScanAction = new QAction("Unload Scan", this);
    m_loadClusterAction = new QAction("Load All Scans in Cluster", this);
    m_unloadClusterAction = new QAction("Unload All Scans in Cluster", this);
    m_viewPointCloudAction = new QAction("View Point Cloud", this);

    // Sprint 2.3 actions
    m_lockClusterAction = new QAction("Lock Cluster", this);
    m_unlockClusterAction = new QAction("Unlock Cluster", this);
    m_deleteScanAction = new QAction("Delete Scan", this);
    m_deleteClusterRecursiveAction = new QAction("Delete Cluster", this);

    // Connect existing actions
    connect(m_createClusterAction, &QAction::triggered, this, &SidebarWidget::onCreateCluster);
    connect(m_createSubClusterAction, &QAction::triggered, this, &SidebarWidget::onCreateSubCluster);
    connect(m_renameClusterAction, &QAction::triggered, this, &SidebarWidget::onRenameCluster);
    connect(m_deleteClusterAction, &QAction::triggered, this, &SidebarWidget::onDeleteCluster);

    // Connect new Sprint 2.1 actions
    connect(m_loadScanAction, &QAction::triggered, this, &SidebarWidget::onLoadScan);
    connect(m_unloadScanAction, &QAction::triggered, this, &SidebarWidget::onUnloadScan);
    connect(m_loadClusterAction, &QAction::triggered, this, &SidebarWidget::onLoadCluster);
    connect(m_unloadClusterAction, &QAction::triggered, this, &SidebarWidget::onUnloadCluster);
    connect(m_viewPointCloudAction, &QAction::triggered, this, &SidebarWidget::onViewPointCloud);

    // Connect Sprint 2.3 actions
    connect(m_lockClusterAction, &QAction::triggered, this, &SidebarWidget::onLockCluster);
    connect(m_unlockClusterAction, &QAction::triggered, this, &SidebarWidget::onUnlockCluster);
    connect(m_deleteScanAction, &QAction::triggered, this, &SidebarWidget::onDeleteScan);
    connect(m_deleteClusterRecursiveAction, &QAction::triggered, this, &SidebarWidget::onDeleteClusterRecursive);
}

void SidebarWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_contextMenu || !m_projectManager) {
        return;
    }

    m_contextItem = getItemAt(event->pos());
    m_contextMenu->clear();

    if (!m_contextItem) {
        // Right-clicked on empty space
        m_contextMenu->addAction(m_createClusterAction);
    } else {
        QString itemType = m_model->getItemType(m_contextItem);
        QString itemId = m_model->getItemId(m_contextItem);

        if (itemType == "scan") {
            // Right-clicked on scan
            if (m_loadManager) {
                bool isLoaded = m_loadManager->isScanLoaded(itemId);
                if (isLoaded) {
                    m_contextMenu->addAction(m_unloadScanAction);
                } else {
                    m_contextMenu->addAction(m_loadScanAction);
                }
                m_contextMenu->addSeparator();
                m_contextMenu->addAction(m_viewPointCloudAction);
                m_contextMenu->addSeparator();
                // Sprint 2.3 - Add delete scan action
                m_contextMenu->addAction(m_deleteScanAction);
            }
        } else if (itemType == "project_root" || itemType == "cluster") {
            // Right-clicked on project root or cluster
            m_contextMenu->addAction(m_createClusterAction);

            if (itemType == "cluster") {
                m_contextMenu->addAction(m_createSubClusterAction);
                m_contextMenu->addSeparator();

                // Add load/unload actions for clusters
                if (m_loadManager) {
                    m_contextMenu->addAction(m_loadClusterAction);
                    m_contextMenu->addAction(m_unloadClusterAction);
                    m_contextMenu->addSeparator();
                    m_contextMenu->addAction(m_viewPointCloudAction);
                    m_contextMenu->addSeparator();
                }

                // Sprint 2.3 - Add lock/unlock actions based on current state
                if (m_projectManager) {
                    bool isLocked = m_projectManager->getClusterLockState(itemId);
                    if (isLocked) {
                        m_contextMenu->addAction(m_unlockClusterAction);
                    } else {
                        m_contextMenu->addAction(m_lockClusterAction);
                    }
                    m_contextMenu->addSeparator();
                }

                m_contextMenu->addAction(m_renameClusterAction);
                // Sprint 2.3 - Replace old delete with recursive delete
                m_contextMenu->addAction(m_deleteClusterRecursiveAction);
            }
        }
    }

    if (!m_contextMenu->isEmpty()) {
        m_contextMenu->exec(event->globalPos());
    }
}

void SidebarWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-scan-ids") ||
        event->mimeData()->hasFormat("application/x-cluster-ids")) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void SidebarWidget::dragMoveEvent(QDragMoveEvent *event)
{
    QStandardItem *item = getItemAt(event->pos());
    if (item) {
        QString itemType = m_model->getItemType(item);
        QString draggedType = event->mimeData()->hasFormat("application/x-scan-ids") ? "scan" : "cluster";

        if (canDropOn(item, draggedType)) {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void SidebarWidget::dropEvent(QDropEvent *event)
{
    QStandardItem *targetItem = getItemAt(event->pos());
    if (!targetItem || !m_projectManager) {
        event->ignore();
        return;
    }

    QString targetType = m_model->getItemType(targetItem);
    QString targetId = m_model->getItemId(targetItem);

    // Only allow dropping on project root or clusters
    if (targetType != "project_root" && targetType != "cluster") {
        event->ignore();
        return;
    }

    // Get target cluster ID (empty for project root)
    QString targetClusterId = (targetType == "cluster") ? targetId : QString();

    if (event->mimeData()->hasFormat("application/x-scan-ids")) {
        // Handle scan drop
        QByteArray data = event->mimeData()->data("application/x-scan-ids");
        QStringList scanIds = QString::fromUtf8(data).split(',', Qt::SkipEmptyParts);

        for (const QString &scanId : scanIds) {
            if (m_projectManager->moveScanToCluster(scanId, targetClusterId)) {
                m_model->moveScanToCluster(scanId, targetClusterId);
                emit scanMovedToCluster(scanId, targetClusterId);
            }
        }

        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void SidebarWidget::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectedIndexes();
    if (indexes.isEmpty()) {
        return;
    }

    QStringList scanIds;
    QStringList clusterIds;

    for (const QModelIndex &index : indexes) {
        QStandardItem *item = m_model->itemFromIndex(index);
        if (item) {
            QString itemType = m_model->getItemType(item);
            QString itemId = m_model->getItemId(item);

            if (itemType == "scan") {
                scanIds << itemId;
            } else if (itemType == "cluster") {
                clusterIds << itemId;
            }
        }
    }

    if (scanIds.isEmpty() && clusterIds.isEmpty()) {
        return;
    }

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    if (!scanIds.isEmpty()) {
        mimeData->setData("application/x-scan-ids", scanIds.join(',').toUtf8());
    }
    if (!clusterIds.isEmpty()) {
        mimeData->setData("application/x-cluster-ids", clusterIds.join(',').toUtf8());
    }

    drag->setMimeData(mimeData);
    drag->exec(supportedActions, Qt::MoveAction);
}

// Context menu action slots
void SidebarWidget::onCreateCluster()
{
    if (!m_projectManager) {
        return;
    }

    QString clusterName = promptForClusterName("Create New Cluster");
    if (clusterName.isEmpty()) {
        return;
    }

    QString parentClusterId;
    if (m_contextItem) {
        QString itemType = m_model->getItemType(m_contextItem);
        if (itemType == "cluster") {
            parentClusterId = m_model->getItemId(m_contextItem);
        }
    }

    QString clusterId = m_projectManager->createCluster(clusterName, parentClusterId);
    if (!clusterId.isEmpty()) {
        // The model will be updated via signals from ProjectManager
        qDebug() << "Cluster created successfully:" << clusterName;
    }
}

void SidebarWidget::onCreateSubCluster()
{
    if (!m_projectManager || !m_contextItem) {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster") {
        return;
    }

    QString clusterName = promptForClusterName("Create New Sub-Cluster");
    if (clusterName.isEmpty()) {
        return;
    }

    QString parentClusterId = m_model->getItemId(m_contextItem);
    QString clusterId = m_projectManager->createCluster(clusterName, parentClusterId);
    if (!clusterId.isEmpty()) {
        qDebug() << "Sub-cluster created successfully:" << clusterName;
    }
}

void SidebarWidget::onRenameCluster()
{
    if (!m_projectManager || !m_contextItem) {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster") {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);
    QString currentName = m_contextItem->text();

    QString newName = promptForClusterName("Rename Cluster", currentName);
    if (newName.isEmpty() || newName == currentName) {
        return;
    }

    if (m_projectManager->renameCluster(clusterId, newName)) {
        qDebug() << "Cluster renamed successfully:" << currentName << "to" << newName;
    }
}

void SidebarWidget::onDeleteCluster()
{
    if (!m_projectManager || !m_contextItem) {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster") {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);
    QString clusterName = m_contextItem->text();

    int ret = QMessageBox::question(this, "Delete Cluster",
                                   QString("Are you sure you want to delete the cluster '%1'?\n\n"
                                          "All scans in this cluster will be moved to the project root.\n"
                                          "All sub-clusters will also be deleted.").arg(clusterName),
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        if (m_projectManager->deleteCluster(clusterId)) {
            qDebug() << "Cluster deleted successfully:" << clusterName;
        }
    }
}

// Helper methods
QStandardItem* SidebarWidget::getItemAt(const QPoint &position)
{
    QModelIndex index = indexAt(position);
    if (index.isValid()) {
        return m_model->itemFromIndex(index);
    }
    return nullptr;
}

QString SidebarWidget::promptForClusterName(const QString &title, const QString &defaultName)
{
    bool ok;
    QString name = QInputDialog::getText(this, title, "Cluster name:", QLineEdit::Normal, defaultName, &ok);

    if (ok && !name.trimmed().isEmpty()) {
        return name.trimmed();
    }

    return QString();
}

bool SidebarWidget::canDropOn(QStandardItem *item, const QString &draggedType)
{
    if (!item) {
        return false;
    }

    QString itemType = m_model->getItemType(item);

    // Can drop scans on project root or clusters
    if (draggedType == "scan") {
        return (itemType == "project_root" || itemType == "cluster");
    }

    // For now, don't allow cluster drag-drop (could be implemented later)
    return false;
}

// New slot implementations for Sprint 2.1
void SidebarWidget::onLoadScan()
{
    if (!m_contextItem || !m_loadManager) {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "scan") {
        return;
    }

    QString scanId = m_model->getItemId(m_contextItem);
    emit loadScanRequested(scanId);
}

void SidebarWidget::onUnloadScan()
{
    if (!m_contextItem || !m_loadManager) {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "scan") {
        return;
    }

    QString scanId = m_model->getItemId(m_contextItem);
    emit unloadScanRequested(scanId);
}

void SidebarWidget::onLoadCluster()
{
    if (!m_contextItem || !m_loadManager) {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster") {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);
    emit loadClusterRequested(clusterId);
}

void SidebarWidget::onUnloadCluster()
{
    if (!m_contextItem || !m_loadManager) {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster") {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);
    emit unloadClusterRequested(clusterId);
}

void SidebarWidget::onViewPointCloud()
{
    if (!m_contextItem || !m_loadManager) {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    QString itemId = m_model->getItemId(m_contextItem);

    if (itemType == "scan" || itemType == "cluster") {
        emit viewPointCloudRequested(itemId, itemType);
    }
}

// Sprint 2.3 - New slot implementations
void SidebarWidget::onLockCluster()
{
    if (!m_contextItem || !m_projectManager) {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster") {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);
    emit lockClusterRequested(clusterId);
}

void SidebarWidget::onUnlockCluster()
{
    if (!m_contextItem || !m_projectManager) {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster") {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);
    emit unlockClusterRequested(clusterId);
}

void SidebarWidget::onDeleteScan()
{
    if (!m_contextItem || !m_projectManager) {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "scan") {
        return;
    }

    QString scanId = m_model->getItemId(m_contextItem);
    QString scanName = m_contextItem->text();

    // Get scan info to check import type
    ScanInfo scan = m_projectManager->getSQLiteManager()->getScanById(scanId);
    if (!scan.isValid()) {
        QMessageBox::warning(this, "Error", "Could not retrieve scan information.");
        return;
    }

    QString message = QString("Are you sure you want to delete scan '%1'?\nThis action cannot be undone.")
                     .arg(scanName);

    ConfirmationDialog dialog("Delete Scan", message, this);

    // Add option to delete physical file for copied/moved scans
    if (scan.importType == "COPIED" || scan.importType == "MOVED") {
        dialog.addPhysicalFileOption("Also delete the physical scan file from the project folder?");
    }

    if (dialog.exec() == QDialog::Accepted) {
        bool deletePhysicalFile = dialog.deletePhysicalFiles();
        emit deleteScanRequested(scanId, deletePhysicalFile);
    }
}

void SidebarWidget::onDeleteClusterRecursive()
{
    if (!m_contextItem || !m_projectManager) {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster") {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);
    QString clusterName = m_contextItem->text();

    QString message = QString("Are you sure you want to delete cluster '%1' and all its contents?\n"
                             "This will delete all sub-clusters and scans within this cluster.\n"
                             "This action cannot be undone.")
                     .arg(clusterName);

    ConfirmationDialog dialog("Delete Cluster", message, this);

    // Check if cluster contains copied/moved scans
    QStringList scanPaths = m_projectManager->getSQLiteManager()->getClusterScanPaths(clusterId, m_currentProjectPath);
    bool hasCopiedMovedScans = !scanPaths.isEmpty();

    if (hasCopiedMovedScans) {
        dialog.addPhysicalFileOption("Also delete physical scan files for copied/moved scans?");
    }

    if (dialog.exec() == QDialog::Accepted) {
        bool deletePhysicalFiles = dialog.deletePhysicalFiles();
        emit deleteClusterRequested(clusterId, deletePhysicalFiles);
    }
}
