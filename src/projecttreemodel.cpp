#include "projecttreemodel.h"
#include "sqlitemanager.h"
#include "projectmanager.h"
#include <QApplication>
#include <QStyle>
#include <QFileInfo>
#include <QDebug>

ProjectTreeModel::ProjectTreeModel(QObject *parent)
    : QStandardItemModel(parent)
    , m_sqliteManager(nullptr)
    , m_rootItem(nullptr)
    , m_scansFolder(nullptr)
{
    setHorizontalHeaderLabels({"Project Structure"});
    initializeIcons();
}

void ProjectTreeModel::setProject(const QString &projectName, const QString &projectPath)
{
    m_projectName = projectName;
    m_projectPath = projectPath;

    clear();
    setHorizontalHeaderLabels({"Project Structure"});

    // Clear caches
    m_clusterItems.clear();
    m_scanItems.clear();
    m_scanLoadedStates.clear();
    m_clusterLoadedStates.clear();

    createProjectStructure();
    refreshHierarchy();
}

void ProjectTreeModel::createProjectStructure()
{
    m_rootItem = new QStandardItem(m_projectName);
    m_rootItem->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon));
    m_rootItem->setEditable(false);
    setItemData(m_rootItem, QString(), "project_root");

    appendRow(m_rootItem);
}

QStandardItem* ProjectTreeModel::getOrCreateScansFolder()
{
    if (!m_scansFolder) {
        m_scansFolder = new QStandardItem("Scans");
        m_scansFolder->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon));
        m_scansFolder->setEditable(false);
        m_scansFolder->setData("scans_folder", Qt::UserRole);
        m_scansFolder->setData("scans_folder", Qt::UserRole + 1);
        
        if (m_rootItem) {
            m_rootItem->appendRow(m_scansFolder);
        }
    }
    
    return m_scansFolder;
}

void ProjectTreeModel::loadScansFromDatabase()
{
    if (!m_sqliteManager) {
        qDebug() << "No SQLite manager available for loading scans";
        return;
    }

    QList<ScanInfo> scans = m_sqliteManager->getAllScans();

    for (const ScanInfo &scan : scans) {
        QStandardItem *scanItem = createScanItem(scan);
        m_scanItems[scan.scanId] = scanItem;
    }

    qDebug() << "Loaded" << scans.size() << "scans from database";
}

QStandardItem* ProjectTreeModel::createScanItem(const ScanInfo &scan)
{
    auto *item = new QStandardItem(scan.scanName);
    
    // Set appropriate icon based on file type
    QString extension = QFileInfo(scan.filePathRelative).suffix().toLower();
    QStyle::StandardPixmap iconType = QStyle::SP_FileIcon;
    
    if (extension == "las") {
        iconType = QStyle::SP_FileIcon;
    } else if (extension == "e57") {
        iconType = QStyle::SP_FileIcon;
    }
    
    item->setIcon(QApplication::style()->standardIcon(iconType));
    item->setEditable(false);
    setItemData(item, scan.scanId, "scan");
    
    // Set tooltip with scan information
    QString tooltip = QString("Scan: %1\nFile: %2\nImported: %3\nMethod: %4")
                     .arg(scan.scanName)
                     .arg(scan.filePathRelative)
                     .arg(scan.dateAdded)
                     .arg(scan.importType);
    item->setToolTip(tooltip);
    
    return item;
}

void ProjectTreeModel::addScan(const ScanInfo &scan)
{
    QStandardItem *scanItem = createScanItem(scan);
    m_scanItems[scan.scanId] = scanItem;

    QStandardItem *parentItem = getParentItem(scan.parentClusterId);
    if (parentItem) {
        parentItem->appendRow(scanItem);
    }

    qDebug() << "Added scan to tree model:" << scan.scanName;
}

void ProjectTreeModel::refreshScans()
{
    refreshHierarchy();
}

void ProjectTreeModel::setSQLiteManager(SQLiteManager *manager)
{
    m_sqliteManager = manager;
}

// New methods for Sprint 1.3 - Cluster support
void ProjectTreeModel::refreshHierarchy()
{
    if (!m_sqliteManager) {
        qDebug() << "No SQLite manager available for loading hierarchy";
        return;
    }

    // Clear existing structure except root
    if (m_rootItem) {
        m_rootItem->removeRows(0, m_rootItem->rowCount());
    }

    // Clear caches
    m_clusterItems.clear();
    m_scanItems.clear();
    m_scansFolder = nullptr;

    // Load clusters and scans from database
    loadClustersFromDatabase();
    loadScansFromDatabase();
    buildHierarchicalStructure();
}

void ProjectTreeModel::loadClustersFromDatabase()
{
    if (!m_sqliteManager) {
        return;
    }

    QList<ClusterInfo> clusters = m_sqliteManager->getAllClusters();

    for (const ClusterInfo &cluster : clusters) {
        QStandardItem *clusterItem = createClusterItem(cluster);
        m_clusterItems[cluster.clusterId] = clusterItem;
    }

    qDebug() << "Loaded" << clusters.size() << "clusters from database";
}

void ProjectTreeModel::buildHierarchicalStructure()
{
    // First, add all top-level clusters to root
    for (auto it = m_clusterItems.begin(); it != m_clusterItems.end(); ++it) {
        QString clusterId = it.key();
        QStandardItem *clusterItem = it.value();

        ClusterInfo cluster = m_sqliteManager->getClusterById(clusterId);
        if (cluster.parentClusterId.isEmpty()) {
            // Top-level cluster
            m_rootItem->appendRow(clusterItem);
        }
    }

    // Then, build parent-child relationships for clusters
    for (auto it = m_clusterItems.begin(); it != m_clusterItems.end(); ++it) {
        QString clusterId = it.key();
        QStandardItem *clusterItem = it.value();

        ClusterInfo cluster = m_sqliteManager->getClusterById(clusterId);
        if (!cluster.parentClusterId.isEmpty()) {
            // Child cluster
            QStandardItem *parentItem = m_clusterItems.value(cluster.parentClusterId);
            if (parentItem) {
                parentItem->appendRow(clusterItem);
            }
        }
    }

    // Finally, add scans to their appropriate parents
    for (auto it = m_scanItems.begin(); it != m_scanItems.end(); ++it) {
        QString scanId = it.key();
        QStandardItem *scanItem = it.value();

        ScanInfo scan = m_sqliteManager->getScanById(scanId);
        QStandardItem *parentItem = getParentItem(scan.parentClusterId);
        if (parentItem) {
            parentItem->appendRow(scanItem);
        }
    }
}

QStandardItem* ProjectTreeModel::createClusterItem(const ClusterInfo &cluster)
{
    auto *item = new QStandardItem(cluster.clusterName);
    item->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon));
    item->setEditable(false);
    setItemData(item, cluster.clusterId, "cluster");

    // Set tooltip with cluster information
    QString tooltip = QString("Cluster: %1\nCreated: %2")
                     .arg(cluster.clusterName)
                     .arg(cluster.creationDate);
    item->setToolTip(tooltip);

    return item;
}

QStandardItem* ProjectTreeModel::getParentItem(const QString &parentClusterId)
{
    if (parentClusterId.isEmpty()) {
        // Return project root for scans/clusters at top level
        return m_rootItem;
    }

    // Return the cluster item
    return m_clusterItems.value(parentClusterId, m_rootItem);
}

void ProjectTreeModel::setItemData(QStandardItem *item, const QString &id, const QString &type)
{
    item->setData(id, Qt::UserRole);        // ID
    item->setData(type, Qt::UserRole + 1);  // Type
}

QString ProjectTreeModel::getItemId(QStandardItem *item) const
{
    if (!item) return QString();
    return item->data(Qt::UserRole).toString();
}

QString ProjectTreeModel::getItemType(QStandardItem *item) const
{
    if (!item) return QString();
    return item->data(Qt::UserRole + 1).toString();
}

QStandardItem* ProjectTreeModel::findClusterItem(const QString &clusterId)
{
    return m_clusterItems.value(clusterId, nullptr);
}

QStandardItem* ProjectTreeModel::findScanItem(const QString &scanId)
{
    return m_scanItems.value(scanId, nullptr);
}

void ProjectTreeModel::addCluster(const ClusterInfo &cluster)
{
    QStandardItem *clusterItem = createClusterItem(cluster);
    m_clusterItems[cluster.clusterId] = clusterItem;

    QStandardItem *parentItem = getParentItem(cluster.parentClusterId);
    if (parentItem) {
        parentItem->appendRow(clusterItem);
    }

    qDebug() << "Added cluster to tree model:" << cluster.clusterName;
}

void ProjectTreeModel::removeCluster(const QString &clusterId)
{
    QStandardItem *clusterItem = m_clusterItems.value(clusterId);
    if (clusterItem) {
        QStandardItem *parentItem = clusterItem->parent();
        if (parentItem) {
            parentItem->removeRow(clusterItem->row());
        }
        m_clusterItems.remove(clusterId);
        qDebug() << "Removed cluster from tree model:" << clusterId;
    }
}

void ProjectTreeModel::updateCluster(const ClusterInfo &cluster)
{
    QStandardItem *clusterItem = m_clusterItems.value(cluster.clusterId);
    if (clusterItem) {
        clusterItem->setText(cluster.clusterName);

        // Update tooltip
        QString tooltip = QString("Cluster: %1\nCreated: %2")
                         .arg(cluster.clusterName)
                         .arg(cluster.creationDate);
        clusterItem->setToolTip(tooltip);

        qDebug() << "Updated cluster in tree model:" << cluster.clusterName;
    }
}

void ProjectTreeModel::moveScanToCluster(const QString &scanId, const QString &clusterId)
{
    QStandardItem *scanItem = m_scanItems.value(scanId);
    if (scanItem) {
        // Remove from current parent
        QStandardItem *currentParent = scanItem->parent();
        if (currentParent) {
            currentParent->removeRow(scanItem->row());
        }

        // Add to new parent
        QStandardItem *newParent = getParentItem(clusterId);
        if (newParent) {
            newParent->appendRow(scanItem);
        }

        qDebug() << "Moved scan in tree model:" << scanId << "to cluster:" << clusterId;
    }
}

// New methods for Sprint 2.1 - Loaded state management
void ProjectTreeModel::initializeIcons()
{
    // Initialize icons for different loaded states
    // Using standard Qt icons for now - can be replaced with custom icons later
    m_loadedIcon = QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton);
    m_unloadedIcon = QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton);
    m_partialIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
    m_loadingIcon = QApplication::style()->standardIcon(QStyle::SP_BrowserReload);
    m_errorIcon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
}

void ProjectTreeModel::setScanLoadedState(const QString &scanId, LoadedState state)
{
    m_scanLoadedStates[scanId] = state;

    // Update the visual representation of the scan item
    QStandardItem *scanItem = findScanItem(scanId);
    if (scanItem) {
        setItemLoadedState(scanItem, state);
    }

    // Update cluster states that might be affected
    updateClusterLoadedStates();

    qDebug() << "Set scan loaded state:" << scanId << "to" << static_cast<int>(state);
}

LoadedState ProjectTreeModel::getScanLoadedState(const QString &scanId) const
{
    return m_scanLoadedStates.value(scanId, LoadedState::Unloaded);
}

void ProjectTreeModel::updateClusterLoadedStates()
{
    // Update all cluster states based on their child scans
    for (auto it = m_clusterItems.begin(); it != m_clusterItems.end(); ++it) {
        QString clusterId = it.key();
        LoadedState clusterState = calculateClusterLoadedState(clusterId);
        m_clusterLoadedStates[clusterId] = clusterState;

        QStandardItem *clusterItem = it.value();
        if (clusterItem) {
            setItemLoadedState(clusterItem, clusterState);
        }
    }
}

LoadedState ProjectTreeModel::calculateClusterLoadedState(const QString &clusterId) const
{
    if (!m_sqliteManager) {
        return LoadedState::Unloaded;
    }

    // Get all scans in this cluster (including sub-clusters)
    QList<ScanInfo> scans = m_sqliteManager->getAllScans();
    QList<QString> clusterScanIds;

    // Find scans that belong to this cluster or its sub-clusters
    for (const ScanInfo &scan : scans) {
        if (scan.parentClusterId == clusterId) {
            clusterScanIds.append(scan.scanId);
        }
    }

    // Also check sub-clusters recursively
    QList<ClusterInfo> clusters = m_sqliteManager->getAllClusters();
    QList<QString> subClusterIds;
    for (const ClusterInfo &cluster : clusters) {
        if (cluster.parentClusterId == clusterId) {
            subClusterIds.append(cluster.clusterId);
            // Recursively get scans from sub-clusters
            LoadedState subClusterState = calculateClusterLoadedState(cluster.clusterId);
            // For now, we'll consider the cluster state based on direct child scans
        }
    }

    if (clusterScanIds.isEmpty()) {
        return LoadedState::Unloaded;
    }

    int loadedCount = 0;
    int totalCount = clusterScanIds.size();
    bool hasError = false;
    bool hasLoading = false;

    for (const QString &scanId : clusterScanIds) {
        LoadedState scanState = getScanLoadedState(scanId);
        switch (scanState) {
            case LoadedState::Loaded:
                loadedCount++;
                break;
            case LoadedState::Error:
                hasError = true;
                break;
            case LoadedState::Loading:
                hasLoading = true;
                break;
            default:
                break;
        }
    }

    if (hasError) {
        return LoadedState::Error;
    }
    if (hasLoading) {
        return LoadedState::Loading;
    }
    if (loadedCount == 0) {
        return LoadedState::Unloaded;
    }
    if (loadedCount == totalCount) {
        return LoadedState::Loaded;
    }
    return LoadedState::Partial;
}

void ProjectTreeModel::setItemLoadedState(QStandardItem *item, LoadedState state)
{
    if (!item) return;

    QIcon icon;
    QString tooltip;

    switch (state) {
        case LoadedState::Loaded:
            icon = m_loadedIcon;
            tooltip = "Loaded in memory";
            break;
        case LoadedState::Unloaded:
            icon = m_unloadedIcon;
            tooltip = "Not loaded";
            break;
        case LoadedState::Partial:
            icon = m_partialIcon;
            tooltip = "Partially loaded";
            break;
        case LoadedState::Loading:
            icon = m_loadingIcon;
            tooltip = "Loading...";
            break;
        case LoadedState::Error:
            icon = m_errorIcon;
            tooltip = "Error loading";
            break;
    }

    // Store the loaded state in the item's data
    item->setData(static_cast<int>(state), Qt::UserRole + 2);

    // Update tooltip to include loaded state
    QString existingTooltip = item->toolTip();
    if (!existingTooltip.isEmpty()) {
        item->setToolTip(existingTooltip + "\nState: " + tooltip);
    } else {
        item->setToolTip("State: " + tooltip);
    }
}

QVariant ProjectTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QStandardItem *item = itemFromIndex(index);
    if (!item) {
        return QStandardItemModel::data(index, role);
    }

    if (role == Qt::DecorationRole) {
        QString itemType = getItemType(item);
        QString itemId = getItemId(item);

        if (itemType == "scan") {
            LoadedState state = getScanLoadedState(itemId);
            switch (state) {
                case LoadedState::Loaded:
                    return m_loadedIcon;
                case LoadedState::Loading:
                    return m_loadingIcon;
                case LoadedState::Error:
                    return m_errorIcon;
                default:
                    return m_unloadedIcon;
            }
        } else if (itemType == "cluster") {
            LoadedState state = m_clusterLoadedStates.value(itemId, LoadedState::Unloaded);
            switch (state) {
                case LoadedState::Loaded:
                    return m_loadedIcon;
                case LoadedState::Partial:
                    return m_partialIcon;
                case LoadedState::Loading:
                    return m_loadingIcon;
                case LoadedState::Error:
                    return m_errorIcon;
                default:
                    return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
            }
        }
    }

    return QStandardItemModel::data(index, role);
}
