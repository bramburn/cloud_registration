#include "sidebarwidget.h"
#include "projecttreemodel.h"
#include "sqlitemanager.h"
#include "projectmanager.h"
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

    m_createClusterAction = new QAction("New Cluster", this);
    m_createSubClusterAction = new QAction("New Sub-Cluster", this);
    m_renameClusterAction = new QAction("Rename", this);
    m_deleteClusterAction = new QAction("Delete", this);

    connect(m_createClusterAction, &QAction::triggered, this, &SidebarWidget::onCreateCluster);
    connect(m_createSubClusterAction, &QAction::triggered, this, &SidebarWidget::onCreateSubCluster);
    connect(m_renameClusterAction, &QAction::triggered, this, &SidebarWidget::onRenameCluster);
    connect(m_deleteClusterAction, &QAction::triggered, this, &SidebarWidget::onDeleteCluster);
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

        if (itemType == "project_root" || itemType == "cluster") {
            // Right-clicked on project root or cluster
            m_contextMenu->addAction(m_createClusterAction);

            if (itemType == "cluster") {
                m_contextMenu->addAction(m_createSubClusterAction);
                m_contextMenu->addSeparator();
                m_contextMenu->addAction(m_renameClusterAction);
                m_contextMenu->addAction(m_deleteClusterAction);
            }
        }
        // For scans, we don't show context menu for now
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
