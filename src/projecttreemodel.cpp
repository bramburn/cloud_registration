#include "projecttreemodel.h"
#include "sqlitemanager.h"
#include "projectmanager.h"
#include "iconmanager.h"
#include <QApplication>
#include <QStyle>
#include <QFileInfo>
#include <QDebug>

ProjectTreeModel::ProjectTreeModel(QObject *parent)
    : QStandardItemModel(parent)
    , m_sqliteManager(nullptr)
    , m_rootItem(nullptr)
    , m_scansFolder(nullptr)
    , m_totalMemoryUsage(0)
    , m_memoryWarningThreshold(1536 * 1024 * 1024) // 1.5GB default
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

    // Sprint 2.1: Clear memory tracking
    m_scanMemoryUsage.clear();
    m_scanPointCounts.clear();
    m_totalMemoryUsage = 0;

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

    // Sprint 3.1 - Missing file icon
    m_missingFileIcon = QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton);
}

void ProjectTreeModel::setScanLoadedState(const QString &scanId, LoadedState state)
{
    LoadedState oldState = m_scanLoadedStates.value(scanId, LoadedState::Unloaded);
    m_scanLoadedStates[scanId] = state;

    // Update the visual representation of the scan item
    QStandardItem *scanItem = findScanItem(scanId);
    if (scanItem) {
        setItemLoadedState(scanItem, state);
    }

    // Update cluster states that might be affected
    updateClusterLoadedStates();

    // Emit state change signal
    if (oldState != state) {
        emit scanStateChanged(scanId, oldState, state);
    }

    qDebug() << "Set scan loaded state:" << scanId << "from" << static_cast<int>(oldState)
             << "to" << static_cast<int>(state);
}

LoadedState ProjectTreeModel::getScanLoadedState(const QString &scanId) const
{
    return m_scanLoadedStates.value(scanId, LoadedState::Unloaded);
}

// Sprint 2.1: Enhanced memory monitoring methods
void ProjectTreeModel::setMemoryWarningThreshold(size_t thresholdMB)
{
    m_memoryWarningThreshold = thresholdMB * 1024 * 1024; // Convert to bytes
    qDebug() << "Memory warning threshold set to:" << thresholdMB << "MB";
}

void ProjectTreeModel::updateMemoryInfo(const QString &scanId, size_t memoryUsage, size_t pointCount)
{
    size_t oldMemoryUsage = m_scanMemoryUsage.value(scanId, 0);
    m_scanMemoryUsage[scanId] = memoryUsage;
    m_scanPointCounts[scanId] = pointCount;

    // Update total memory usage
    m_totalMemoryUsage = m_totalMemoryUsage - oldMemoryUsage + memoryUsage;

    // Check for memory warning
    if (m_totalMemoryUsage > m_memoryWarningThreshold) {
        setScanLoadedState(scanId, LoadedState::MemoryWarning);
        emit memoryWarningTriggered(m_totalMemoryUsage, m_memoryWarningThreshold);
    }

    emit memoryUsageChanged(m_totalMemoryUsage);

    qDebug() << "Updated memory info for scan" << scanId << ":" << memoryUsage << "bytes,"
             << pointCount << "points. Total usage:" << m_totalMemoryUsage << "bytes";
}

size_t ProjectTreeModel::getTotalMemoryUsage() const
{
    return m_totalMemoryUsage;
}

void ProjectTreeModel::setClusterState(const QString &clusterId, LoadedState state)
{
    m_clusterLoadedStates[clusterId] = state;

    // Update visual representation
    QStandardItem *clusterItem = findClusterItem(clusterId);
    if (clusterItem) {
        setItemLoadedState(clusterItem, state);
    }

    qDebug() << "Set cluster loaded state:" << clusterId << "to" << static_cast<int>(state);
}

QStringList ProjectTreeModel::getScansInState(LoadedState state) const
{
    QStringList scansInState;
    for (auto it = m_scanLoadedStates.begin(); it != m_scanLoadedStates.end(); ++it) {
        if (it.value() == state) {
            scansInState.append(it.key());
        }
    }
    return scansInState;
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
            /*LoadedState subClusterState =*/ calculateClusterLoadedState(cluster.clusterId);
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

    QString itemType = getItemType(item);
    QString itemId = getItemId(item);

    switch (role) {
        case Qt::DecorationRole: {
            // Use IconManager for enhanced icons
            if (itemType == "scan") {
                ItemState state = convertLoadedStateToItemState(getScanLoadedState(itemId));
                ImportType importType = getItemImportType(item);

                // Check if scan is missing
                if (isScanMissing(itemId)) {
                    state = ItemState::Missing;
                }

                return IconManager::instance().getCompositeIcon(ItemType::Scan, state, importType);

            } else if (itemType == "cluster") {
                LoadedState loadedState = m_clusterLoadedStates.value(itemId, LoadedState::Unloaded);
                ItemState state = convertLoadedStateToItemState(loadedState);

                // Check if cluster is locked
                if (getClusterLockState(itemId)) {
                    state = ItemState::Locked;
                }

                return IconManager::instance().getIcon(ItemType::Cluster, state);

            } else if (itemType == "project_root") {
                return IconManager::instance().getIcon(ItemType::Project, ItemState::Unloaded);
            }
            break;
        }

        case Qt::ToolTipRole: {
            if (itemType == "scan" && m_sqliteManager) {
                ScanInfo scan = m_sqliteManager->getScanById(itemId);
                return generateScanTooltip(scan);

            } else if (itemType == "cluster" && m_sqliteManager) {
                ClusterInfo cluster = m_sqliteManager->getClusterById(itemId);
                return generateClusterTooltip(cluster);
            }
            break;
        }

        // Enhanced custom data roles for Sprint 3.3
        case ItemTypeRole: {
            if (itemType == "scan") return static_cast<int>(ItemType::Scan);
            if (itemType == "cluster") return static_cast<int>(ItemType::Cluster);
            if (itemType == "project_root") return static_cast<int>(ItemType::Project);
            break;
        }

        case ItemStateRole: {
            if (itemType == "scan") {
                return static_cast<int>(convertLoadedStateToItemState(getScanLoadedState(itemId)));
            } else if (itemType == "cluster") {
                LoadedState loadedState = m_clusterLoadedStates.value(itemId, LoadedState::Unloaded);
                return static_cast<int>(convertLoadedStateToItemState(loadedState));
            }
            break;
        }

        case PointCountRole: {
            if (itemType == "scan" && m_sqliteManager) {
                ScanInfo scan = m_sqliteManager->getScanById(itemId);
                return scan.pointCountEstimate;
            }
            break;
        }

        case FileSizeRole: {
            if (itemType == "scan" && m_sqliteManager) {
                ScanInfo scan = m_sqliteManager->getScanById(itemId);
                // fileSize field not available in current ScanInfo structure
                return QVariant(); // Return invalid QVariant for missing data
            }
            break;
        }

        case DateAddedRole: {
            if (itemType == "scan" && m_sqliteManager) {
                ScanInfo scan = m_sqliteManager->getScanById(itemId);
                return scan.dateAdded;
            } else if (itemType == "cluster" && m_sqliteManager) {
                ClusterInfo cluster = m_sqliteManager->getClusterById(itemId);
                return cluster.creationDate;
            }
            break;
        }

        case FullPathRole: {
            if (itemType == "scan" && m_sqliteManager) {
                ScanInfo scan = m_sqliteManager->getScanById(itemId);
                // filePathAbsolute field not available, use filePathRelative instead
                return scan.filePathRelative;
            }
            break;
        }

        case IsLoadedRole: {
            if (itemType == "scan") {
                return getScanLoadedState(itemId) == LoadedState::Loaded;
            } else if (itemType == "cluster") {
                LoadedState state = m_clusterLoadedStates.value(itemId, LoadedState::Unloaded);
                return state == LoadedState::Loaded;
            }
            break;
        }

        case IsLockedRole: {
            if (itemType == "cluster") {
                return getClusterLockState(itemId);
            }
            break;
        }

        case IsMissingRole: {
            if (itemType == "scan") {
                return isScanMissing(itemId);
            }
            break;
        }

        case ImportTypeRole: {
            if (itemType == "scan" && m_sqliteManager) {
                ScanInfo scan = m_sqliteManager->getScanById(itemId);
                return scan.importType;
            }
            break;
        }
    }

    return QStandardItemModel::data(index, role);
}

// Sprint 3.1 - Missing file support and data export methods
void ProjectTreeModel::markScanAsMissing(const QString &scanId) {
    m_missingScanIds.insert(scanId);

    QStandardItem *scanItem = findScanItem(scanId);
    if (scanItem) {
        // Update icon to show missing file
        scanItem->setIcon(m_missingFileIcon);

        // Update tooltip
        QString tooltip = scanItem->toolTip();
        if (!tooltip.contains("FILE MISSING")) {
            tooltip += "\n⚠ FILE MISSING";
            scanItem->setToolTip(tooltip);
        }

        // Set custom data role
        scanItem->setData(true, IsMissingRole);
    }
}

void ProjectTreeModel::clearScanMissingFlag(const QString &scanId) {
    m_missingScanIds.remove(scanId);

    QStandardItem *scanItem = findScanItem(scanId);
    if (scanItem) {
        // Restore original icon based on file type
        ScanInfo scan = m_sqliteManager->getScanById(scanId);
        QString extension = QFileInfo(scan.filePathRelative).suffix().toLower();
        scanItem->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileIcon));

        // Update tooltip to remove missing file warning
        QString tooltip = scanItem->toolTip();
        tooltip.remove("\n⚠ FILE MISSING");
        scanItem->setToolTip(tooltip);

        // Clear custom data role
        scanItem->setData(false, IsMissingRole);
    }
}

bool ProjectTreeModel::isScanMissing(const QString &scanId) const {
    return m_missingScanIds.contains(scanId);
}

void ProjectTreeModel::updateScanFilePath(const QString &scanId, const QString &newPath) {
    QStandardItem *scanItem = findScanItem(scanId);
    if (scanItem) {
        // Update tooltip with new path
        ScanInfo scan = m_sqliteManager->getScanById(scanId);
        QString tooltip = QString("Scan: %1\nFile: %2\nImported: %3\nMethod: %4")
                         .arg(scan.scanName)
                         .arg(newPath)
                         .arg(scan.dateAdded)
                         .arg(scan.importType);
        scanItem->setToolTip(tooltip);

        // Set custom data role
        scanItem->setData(newPath, FilePathRole);
    }
}

void ProjectTreeModel::removeScan(const QString &scanId) {
    QStandardItem *scanItem = m_scanItems.value(scanId);
    if (scanItem) {
        QStandardItem *parentItem = scanItem->parent();
        if (parentItem) {
            parentItem->removeRow(scanItem->row());
        }
        m_scanItems.remove(scanId);
        m_scanLoadedStates.remove(scanId);
        m_missingScanIds.remove(scanId);
        qDebug() << "Removed scan from tree model:" << scanId;
    }
}

QList<ClusterInfo> ProjectTreeModel::getAllClusters() const {
    if (!m_sqliteManager) {
        return QList<ClusterInfo>();
    }
    return m_sqliteManager->getAllClusters();
}

QList<ScanInfo> ProjectTreeModel::getAllScans() const {
    if (!m_sqliteManager) {
        return QList<ScanInfo>();
    }
    return m_sqliteManager->getAllScans();
}

void ProjectTreeModel::populateFromData(const QList<ClusterInfo> &clusters, const QList<ScanInfo> &scans) {
    // Clear existing structure except root
    if (m_rootItem) {
        m_rootItem->removeRows(0, m_rootItem->rowCount());
    }

    // Clear caches
    m_clusterItems.clear();
    m_scanItems.clear();
    m_scansFolder = nullptr;
    m_missingScanIds.clear();

    // Create cluster items
    for (const ClusterInfo &cluster : clusters) {
        QStandardItem *clusterItem = createClusterItem(cluster);
        m_clusterItems[cluster.clusterId] = clusterItem;
    }

    // Create scan items
    for (const ScanInfo &scan : scans) {
        QStandardItem *scanItem = createScanItem(scan);
        m_scanItems[scan.scanId] = scanItem;
    }

    // Build hierarchical structure
    buildHierarchicalStructure();
}

// Sprint 3.3 - Enhanced tooltip and utility methods
QString ProjectTreeModel::generateScanTooltip(const ScanInfo& scan) const
{
    QString tooltip = QString(
        "<b>%1</b><br/>"
        "<b>Path:</b> %2<br/>"
        "<b>Import Type:</b> %3<br/>"
        "<b>Points:</b> %4<br/>"
        "<b>File Size:</b> %5<br/>"
        "<b>Date Added:</b> %6<br/>"
        "<b>Status:</b> %7"
    ).arg(scan.scanName)
     .arg(scan.filePathRelative)
     .arg(scan.importType)
     .arg(formatPointCount(scan.pointCountEstimate))
     .arg("N/A") // fileSize field not available in current ScanInfo structure
     .arg(scan.dateAdded) // dateAdded is already a QString in ISO format
     .arg(getScanLoadedState(scan.scanId) == LoadedState::Loaded ? "Loaded" : "Unloaded");

    if (isScanMissing(scan.scanId)) {
        tooltip += "<br/><font color='red'><b>⚠ WARNING: Source file not found</b></font>";
    }

    return tooltip;
}

QString ProjectTreeModel::generateClusterTooltip(const ClusterInfo& cluster) const
{
    // Count scans and sub-clusters
    int scanCount = 0;
    int subClusterCount = 0;

    if (m_sqliteManager) {
        QList<ScanInfo> allScans = m_sqliteManager->getAllScans();
        QList<ClusterInfo> allClusters = m_sqliteManager->getAllClusters();

        for (const ScanInfo& scan : allScans) {
            if (scan.parentClusterId == cluster.clusterId) {
                scanCount++;
            }
        }

        for (const ClusterInfo& subCluster : allClusters) {
            if (subCluster.parentClusterId == cluster.clusterId) {
                subClusterCount++;
            }
        }
    }

    QString tooltip = QString(
        "<b>%1</b><br/>"
        "<b>Scans:</b> %2<br/>"
        "<b>Sub-clusters:</b> %3<br/>"
        "<b>Created:</b> %4<br/>"
        "<b>Status:</b> %5<br/>"
        "<b>Lock Status:</b> %6"
    ).arg(cluster.clusterName)
     .arg(scanCount)
     .arg(subClusterCount)
     .arg(cluster.creationDate) // creationDate is already a QString in ISO format
     .arg(m_clusterLoadedStates.value(cluster.clusterId, LoadedState::Unloaded) == LoadedState::Loaded ? "Loaded" : "Unloaded")
     .arg(getClusterLockState(cluster.clusterId) ? "Locked" : "Unlocked");

    if (getClusterLockState(cluster.clusterId)) {
        tooltip += "<br/><font color='orange'><b>🔒 Locked clusters cannot be modified during registration</b></font>";
    }

    return tooltip;
}

QString ProjectTreeModel::formatFileSize(qint64 bytes) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;

    if (bytes >= GB) {
        return QString("%1 GB").arg(QString::number(bytes / double(GB), 'f', 2));
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(QString::number(bytes / double(MB), 'f', 1));
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(QString::number(bytes / double(KB), 'f', 0));
    }
    return QString("%1 bytes").arg(bytes);
}

QString ProjectTreeModel::formatPointCount(qint64 points) const
{
    const qint64 K = 1000;
    const qint64 M = K * 1000;
    const qint64 B = M * 1000;

    if (points >= B) {
        return QString("%1B points").arg(QString::number(points / double(B), 'f', 2));
    } else if (points >= M) {
        return QString("%1M points").arg(QString::number(points / double(M), 'f', 1));
    } else if (points >= K) {
        return QString("%1K points").arg(QString::number(points / double(K), 'f', 0));
    }
    return QString("%1 points").arg(points);
}

QString ProjectTreeModel::getImportTypeString(ImportType type) const
{
    switch (type) {
        case ImportType::Copy: return "Copy";
        case ImportType::Move: return "Move";
        case ImportType::Link: return "Link to Source";
        default: return "Unknown";
    }
}

ItemState ProjectTreeModel::convertLoadedStateToItemState(LoadedState state) const
{
    switch (state) {
        case LoadedState::Loaded: return ItemState::Loaded;
        case LoadedState::Loading: return ItemState::Loading;
        case LoadedState::Error: return ItemState::Error;
        // Sprint 2.1: New state mappings
        case LoadedState::Processing: return ItemState::Processing;
        case LoadedState::Cached: return ItemState::Cached;
        case LoadedState::MemoryWarning: return ItemState::MemoryWarning;
        case LoadedState::Optimized: return ItemState::Optimized;
        default: return ItemState::Unloaded;
    }
}

ImportType ProjectTreeModel::getItemImportType(QStandardItem* item) const
{
    if (!item || !m_sqliteManager) {
        return ImportType::None;
    }

    QString itemType = getItemType(item);
    if (itemType == "scan") {
        QString scanId = getItemId(item);
        ScanInfo scan = m_sqliteManager->getScanById(scanId);

        // Convert QString importType to ImportType enum
        if (scan.importType == "COPIED") {
            return ImportType::Copy;
        } else if (scan.importType == "MOVED") {
            return ImportType::Move;
        } else if (scan.importType == "LINKED") {
            return ImportType::Link;
        }
    }

    return ImportType::None;
}
