#include "ui/sidebarwidget.h"

#include <QApplication>
#include <QContextMenuEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QStandardItem>

#include "core/projectmanager.h"
#include "core/sqlitemanager.h"
#include "ui/confirmationdialog.h"
#include "ui/projecttreemodel.h"

SidebarWidget::SidebarWidget(QWidget* parent)
    : QTreeView(parent), m_model(nullptr), m_contextMenu(nullptr), m_contextItem(nullptr)
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
    setSelectionMode(QAbstractItemView::ExtendedSelection);  // Allow multiple selection for drag-drop
}

void SidebarWidget::setProject(const QString& projectName, const QString& projectPath)
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

void SidebarWidget::setSQLiteManager(SQLiteManager* manager)
{
    m_model->setSQLiteManager(manager);
}

// Sprint 4: Removed setter methods - SidebarWidget now only emits signals
// void setProjectManager(ProjectManager *manager);
// void setPointCloudLoadManager(PointCloudLoadManager *manager);

void SidebarWidget::refreshFromDatabase()
{
    if (m_model)
    {
        m_model->refreshScans();
        expandAll();
    }
}

void SidebarWidget::addScan(const ScanInfo& scan)
{
    if (m_model)
    {
        m_model->addScan(scan);
        expandAll();
    }
}

// New methods for Sprint 1.3
void SidebarWidget::addCluster(const ClusterInfo& cluster)
{
    if (m_model)
    {
        m_model->addCluster(cluster);
        expandAll();
    }
}

void SidebarWidget::removeCluster(const QString& clusterId)
{
    if (m_model)
    {
        m_model->removeCluster(clusterId);
    }
}

void SidebarWidget::updateCluster(const ClusterInfo& cluster)
{
    if (m_model)
    {
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

    // Enhanced Sprint 2.1 actions
    m_loadScanAction = new QAction("Load Scan", this);
    m_unloadScanAction = new QAction("Unload Scan", this);
    m_loadClusterAction = new QAction("Load All Scans in Cluster", this);
    m_unloadClusterAction = new QAction("Unload All Scans in Cluster", this);
    m_viewPointCloudAction = new QAction("View Point Cloud", this);

    // Sprint 2.1: Advanced operations
    m_preprocessScanAction = new QAction("Preprocess Scan", this);
    m_optimizeScanAction = new QAction("Optimize for Registration", this);
    m_batchLoadAction = new QAction("Batch Load Selected", this);
    m_batchUnloadAction = new QAction("Batch Unload Selected", this);
    m_memoryOptimizeAction = new QAction("Optimize Memory Usage", this);

    // Sprint 2.1: Advanced submenu
    m_advancedMenu = new QMenu("Advanced Operations", this);
    m_filterMovingObjectsAction = new QAction("Filter Moving Objects", this);
    m_colorBalanceAction = new QAction("Color Balance", this);
    m_registrationPreviewAction = new QAction("Registration Preview", this);

    m_advancedMenu->addAction(m_filterMovingObjectsAction);
    m_advancedMenu->addAction(m_colorBalanceAction);
    m_advancedMenu->addAction(m_registrationPreviewAction);

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

    // Connect enhanced Sprint 2.1 actions
    connect(m_loadScanAction, &QAction::triggered, this, &SidebarWidget::onLoadScan);
    connect(m_unloadScanAction, &QAction::triggered, this, &SidebarWidget::onUnloadScan);
    connect(m_loadClusterAction, &QAction::triggered, this, &SidebarWidget::onLoadCluster);
    connect(m_unloadClusterAction, &QAction::triggered, this, &SidebarWidget::onUnloadCluster);
    connect(m_viewPointCloudAction, &QAction::triggered, this, &SidebarWidget::onViewPointCloud);

    // Sprint 2.1: Connect advanced operation actions
    connect(m_preprocessScanAction, &QAction::triggered, this, &SidebarWidget::onPreprocessScan);
    connect(m_optimizeScanAction, &QAction::triggered, this, &SidebarWidget::onOptimizeScan);
    connect(m_batchLoadAction, &QAction::triggered, this, &SidebarWidget::onBatchLoad);
    connect(m_batchUnloadAction, &QAction::triggered, this, &SidebarWidget::onBatchUnload);
    connect(m_memoryOptimizeAction, &QAction::triggered, this, &SidebarWidget::onMemoryOptimize);
    connect(m_filterMovingObjectsAction, &QAction::triggered, this, &SidebarWidget::onFilterMovingObjects);
    connect(m_colorBalanceAction, &QAction::triggered, this, &SidebarWidget::onColorBalance);
    connect(m_registrationPreviewAction, &QAction::triggered, this, &SidebarWidget::onRegistrationPreview);

    // Connect Sprint 2.3 actions
    connect(m_lockClusterAction, &QAction::triggered, this, &SidebarWidget::onLockCluster);
    connect(m_unlockClusterAction, &QAction::triggered, this, &SidebarWidget::onUnlockCluster);
    connect(m_deleteScanAction, &QAction::triggered, this, &SidebarWidget::onDeleteScan);
    connect(m_deleteClusterRecursiveAction, &QAction::triggered, this, &SidebarWidget::onDeleteClusterRecursive);
}

void SidebarWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (!m_contextMenu)
    {
        return;
    }

    m_contextItem = getItemAt(event->pos());
    m_contextMenu->clear();

    if (!m_contextItem)
    {
        // Right-clicked on empty space
        m_contextMenu->addAction(m_createClusterAction);
    }
    else
    {
        QString itemType = m_model->getItemType(m_contextItem);
        QString itemId = m_model->getItemId(m_contextItem);

        if (itemType == "scan")
        {
            // Sprint 4: Simplified scan context menu - no manager dependencies
            m_contextMenu->addAction(m_loadScanAction);
            m_contextMenu->addAction(m_unloadScanAction);
            m_contextMenu->addAction(m_preprocessScanAction);
            m_contextMenu->addAction(m_optimizeScanAction);
            m_contextMenu->addMenu(m_advancedMenu);
            m_contextMenu->addSeparator();
            m_contextMenu->addAction(m_viewPointCloudAction);
            m_contextMenu->addSeparator();
            m_contextMenu->addAction(m_deleteScanAction);
        }
        else if (itemType == "project_root" || itemType == "cluster")
        {
            // Right-clicked on project root or cluster
            m_contextMenu->addAction(m_createClusterAction);

            if (itemType == "cluster")
            {
                m_contextMenu->addAction(m_createSubClusterAction);
                m_contextMenu->addSeparator();

                // Sprint 4: Simplified cluster context menu - no manager dependencies
                m_contextMenu->addAction(m_loadClusterAction);
                m_contextMenu->addAction(m_unloadClusterAction);
                m_contextMenu->addSeparator();

                // Sprint 2.1: Batch operations
                m_contextMenu->addAction(m_batchLoadAction);
                m_contextMenu->addAction(m_batchUnloadAction);
                m_contextMenu->addSeparator();

                m_contextMenu->addAction(m_viewPointCloudAction);
                m_contextMenu->addSeparator();

                // Sprint 4: Always show lock/unlock actions - MainPresenter will handle state
                m_contextMenu->addAction(m_lockClusterAction);
                m_contextMenu->addAction(m_unlockClusterAction);
                m_contextMenu->addSeparator();

                m_contextMenu->addAction(m_renameClusterAction);
                m_contextMenu->addAction(m_deleteClusterRecursiveAction);
            }
        }
    }

    // Sprint 2.1: Always available memory optimization
    if (!m_contextMenu->isEmpty())
    {
        m_contextMenu->addSeparator();
        m_contextMenu->addAction(m_memoryOptimizeAction);
    }

    if (!m_contextMenu->isEmpty())
    {
        m_contextMenu->exec(event->globalPos());
    }
}

void SidebarWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-scan-ids") ||
        event->mimeData()->hasFormat("application/x-cluster-ids"))
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void SidebarWidget::dragMoveEvent(QDragMoveEvent* event)
{
    QStandardItem* item = getItemAt(event->position().toPoint());
    if (item)
    {
        QString itemType = m_model->getItemType(item);
        QString draggedType = event->mimeData()->hasFormat("application/x-scan-ids") ? "scan" : "cluster";

        if (canDropOn(item, draggedType))
        {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void SidebarWidget::dropEvent(QDropEvent* event)
{
    QStandardItem* targetItem = getItemAt(event->position().toPoint());
    if (!targetItem)
    {
        event->ignore();
        return;
    }

    QString targetType = m_model->getItemType(targetItem);
    QString targetId = m_model->getItemId(targetItem);

    // Only allow dropping on project root or clusters
    if (targetType != "project_root" && targetType != "cluster")
    {
        event->ignore();
        return;
    }

    if (event->mimeData()->hasFormat("application/x-scan-ids"))
    {
        // Handle scan drop
        QByteArray mimeData = event->mimeData()->data("application/x-scan-ids");
        QStringList scanIds = QString::fromUtf8(mimeData).split(',', Qt::SkipEmptyParts);

        // Sprint 4: Emit signal instead of executing business logic
        emit dragDropOperationRequested(scanIds, "scan", targetId, targetType);
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void SidebarWidget::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectedIndexes();
    if (indexes.isEmpty())
    {
        return;
    }

    QStringList scanIds;
    QStringList clusterIds;

    for (const QModelIndex& index : indexes)
    {
        QStandardItem* item = m_model->itemFromIndex(index);
        if (item)
        {
            QString itemType = m_model->getItemType(item);
            QString itemId = m_model->getItemId(item);

            if (itemType == "scan")
            {
                scanIds << itemId;
            }
            else if (itemType == "cluster")
            {
                clusterIds << itemId;
            }
        }
    }

    if (scanIds.isEmpty() && clusterIds.isEmpty())
    {
        return;
    }

    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;

    if (!scanIds.isEmpty())
    {
        mimeData->setData("application/x-scan-ids", scanIds.join(',').toUtf8());
    }
    if (!clusterIds.isEmpty())
    {
        mimeData->setData("application/x-cluster-ids", clusterIds.join(',').toUtf8());
    }

    drag->setMimeData(mimeData);
    drag->exec(supportedActions, Qt::MoveAction);
}

// Context menu action slots
void SidebarWidget::onCreateCluster()
{
    QString clusterName = promptForClusterName("Create New Cluster");
    if (clusterName.isEmpty())
    {
        return;
    }

    QString parentClusterId;
    if (m_contextItem)
    {
        QString itemType = m_model->getItemType(m_contextItem);
        if (itemType == "cluster")
        {
            parentClusterId = m_model->getItemId(m_contextItem);
        }
    }

    // Sprint 4: Emit signal instead of executing business logic
    emit clusterCreationRequested(clusterName, parentClusterId);
}

void SidebarWidget::onCreateSubCluster()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster")
    {
        return;
    }

    QString parentClusterId = m_model->getItemId(m_contextItem);
    QString clusterName = promptForClusterName("Create New Sub-Cluster");
    if (clusterName.isEmpty())
    {
        return;
    }

    // Sprint 4: Emit signal instead of executing business logic
    emit clusterCreationRequested(clusterName, parentClusterId);
}

void SidebarWidget::onRenameCluster()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster")
    {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);
    QString currentName = m_contextItem->text();

    QString newName = promptForClusterName("Rename Cluster", currentName);
    if (newName.isEmpty() || newName == currentName)
    {
        return;
    }

    // Sprint 4: Emit signal instead of executing business logic
    emit clusterRenameRequested(clusterId, newName);
}

void SidebarWidget::onDeleteCluster()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster")
    {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);

    // Sprint 4: Emit signal instead of executing business logic
    // The confirmation dialog will be handled by MainPresenter
    emit clusterDeletionRequested(clusterId);
}

// Helper methods
QStandardItem* SidebarWidget::getItemAt(const QPoint& position)
{
    QModelIndex index = indexAt(position);
    if (index.isValid())
    {
        return m_model->itemFromIndex(index);
    }
    return nullptr;
}

QString SidebarWidget::promptForClusterName(const QString& title, const QString& defaultName)
{
    bool ok;
    QString name = QInputDialog::getText(this, title, "Cluster name:", QLineEdit::Normal, defaultName, &ok);

    if (ok && !name.trimmed().isEmpty())
    {
        return name.trimmed();
    }

    return QString();
}

bool SidebarWidget::canDropOn(QStandardItem* item, const QString& draggedType)
{
    if (!item)
    {
        return false;
    }

    QString itemType = m_model->getItemType(item);

    // Can drop scans on project root or clusters
    if (draggedType == "scan")
    {
        return (itemType == "project_root" || itemType == "cluster");
    }

    // For now, don't allow cluster drag-drop (could be implemented later)
    return false;
}

// New slot implementations for Sprint 2.1
void SidebarWidget::onLoadScan()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "scan")
    {
        return;
    }

    QString scanId = m_model->getItemId(m_contextItem);
    emit loadScanRequested(scanId);
}

void SidebarWidget::onUnloadScan()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "scan")
    {
        return;
    }

    QString scanId = m_model->getItemId(m_contextItem);
    emit unloadScanRequested(scanId);
}

void SidebarWidget::onLoadCluster()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster")
    {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);
    emit loadClusterRequested(clusterId);
}

void SidebarWidget::onUnloadCluster()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster")
    {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);
    emit unloadClusterRequested(clusterId);
}

void SidebarWidget::onViewPointCloud()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    QString itemId = m_model->getItemId(m_contextItem);

    if (itemType == "scan" || itemType == "cluster")
    {
        emit viewPointCloudRequested(itemId, itemType);
    }
}

// Sprint 2.3 - New slot implementations
void SidebarWidget::onLockCluster()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster")
    {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);
    emit lockClusterRequested(clusterId);
}

void SidebarWidget::onUnlockCluster()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster")
    {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);
    emit unlockClusterRequested(clusterId);
}

void SidebarWidget::onDeleteScan()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "scan")
    {
        return;
    }

    QString scanId = m_model->getItemId(m_contextItem);

    // Sprint 4: Emit signal instead of executing business logic
    // The confirmation dialog and scan info retrieval will be handled by MainPresenter
    emit deleteScanRequested(scanId, false);  // Default to not deleting physical file
}

void SidebarWidget::onDeleteClusterRecursive()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "cluster")
    {
        return;
    }

    QString clusterId = m_model->getItemId(m_contextItem);

    // Sprint 4: Emit signal instead of executing business logic
    // The confirmation dialog and scan path checking will be handled by MainPresenter
    emit deleteClusterRequested(clusterId, false);  // Default to not deleting physical files
}

// Sprint 2.1: Enhanced operation slot implementations
void SidebarWidget::onPreprocessScan()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "scan")
    {
        return;
    }

    QString scanId = m_model->getItemId(m_contextItem);
    emit preprocessScanRequested(scanId);
}

void SidebarWidget::onOptimizeScan()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "scan")
    {
        return;
    }

    QString scanId = m_model->getItemId(m_contextItem);
    emit optimizeScanRequested(scanId);
}

void SidebarWidget::onBatchLoad()
{
    QStringList selectedScans = getSelectedScanIds();
    if (!selectedScans.isEmpty())
    {
        emit batchOperationRequested("load", selectedScans);
    }
}

void SidebarWidget::onBatchUnload()
{
    QStringList selectedScans = getSelectedScanIds();
    if (!selectedScans.isEmpty())
    {
        emit batchOperationRequested("unload", selectedScans);
    }
}

void SidebarWidget::onMemoryOptimize()
{
    emit memoryOptimizationRequested();
}

void SidebarWidget::onFilterMovingObjects()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "scan")
    {
        return;
    }

    QString scanId = m_model->getItemId(m_contextItem);
    emit filterMovingObjectsRequested(scanId);
}

void SidebarWidget::onColorBalance()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "scan")
    {
        return;
    }

    QString scanId = m_model->getItemId(m_contextItem);
    emit colorBalanceRequested(scanId);
}

void SidebarWidget::onRegistrationPreview()
{
    if (!m_contextItem)
    {
        return;
    }

    QString itemType = m_model->getItemType(m_contextItem);
    if (itemType != "scan")
    {
        return;
    }

    QString scanId = m_model->getItemId(m_contextItem);
    emit registrationPreviewRequested(scanId);
}

// Sprint 2.1: Helper method implementations
QStringList SidebarWidget::getSelectedScanIds() const
{
    QStringList scanIds;
    QModelIndexList indexes = selectedIndexes();

    for (const QModelIndex& index : indexes)
    {
        QStandardItem* item = m_model->itemFromIndex(index);
        if (item)
        {
            QString itemType = m_model->getItemType(item);
            if (itemType == "scan")
            {
                QString scanId = m_model->getItemId(item);
                scanIds.append(scanId);
            }
        }
    }

    return scanIds;
}

QString SidebarWidget::getItemIdFromIndex(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QString();
    }

    QStandardItem* item = m_model->itemFromIndex(index);
    return item ? m_model->getItemId(item) : QString();
}

QString SidebarWidget::getItemTypeFromIndex(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QString();
    }

    QStandardItem* item = m_model->itemFromIndex(index);
    return item ? m_model->getItemType(item) : QString();
}
