#include "ui/projecttreemodel.h"

#include <QApplication>
#include <QStyle>
#include <QDebug>
#include <QDateTime>

#include "core/sqlitemanager.h"
#include "registration/RegistrationProject.h"
#include "core/scaninfo.h"
#include "core/clusterinfo.h"

ProjectTreeModel::ProjectTreeModel(QObject* parent)
    : QStandardItemModel(parent)
    , m_sqliteManager(nullptr)
    , m_registrationProject(nullptr)
{
    initializeIcons();
    setHorizontalHeaderLabels({"Project Structure"});
}

void ProjectTreeModel::setSQLiteManager(SQLiteManager* manager)
{
    m_sqliteManager = manager;
}

void ProjectTreeModel::setRegistrationProject(Registration::RegistrationProject* project)
{
    m_registrationProject = project;
}

void ProjectTreeModel::initializeIcons()
{
    QStyle* style = QApplication::style();
    m_projectIcon = style->standardIcon(QStyle::SP_DirIcon);
    m_scanIcon = style->standardIcon(QStyle::SP_FileIcon);
    m_clusterIcon = style->standardIcon(QStyle::SP_DirOpenIcon);
    m_alignedGroupIcon = style->standardIcon(QStyle::SP_DialogApplyButton);
    m_referenceScanIcon = style->standardIcon(QStyle::SP_MediaPlay);
    m_targetScanIcon = style->standardIcon(QStyle::SP_MediaStop);
}

void ProjectTreeModel::refreshFromDatabase()
{
    qDebug() << "ProjectTreeModel: Refreshing from database";

    clear();
    setHorizontalHeaderLabels({"Project Structure"});

    createProjectStructure();
    loadScansFromDatabase();
    loadClustersFromDatabase();
    createAlignedGroups();

    qDebug() << "ProjectTreeModel: Refresh complete";
}

void ProjectTreeModel::refreshScans()
{
    // Alias for refreshFromDatabase for backward compatibility
    refreshFromDatabase();
}

void ProjectTreeModel::createProjectStructure()
{
    // Create root project item
    QStandardItem* rootItem = new QStandardItem("Project");
    rootItem->setIcon(m_projectIcon);
    rootItem->setEditable(false);
    rootItem->setData(ProjectRootItem, ItemTypeRole);
    rootItem->setData("project_root", ItemIdRole);

    appendRow(rootItem);
}

void ProjectTreeModel::loadScansFromDatabase()
{
    if (!m_sqliteManager) {
        qWarning() << "ProjectTreeModel: No SQLiteManager set";
        return;
    }

    // Get all scans from database
    QList<ScanInfo> scans = m_sqliteManager->getAllScans();

    for (const ScanInfo& scan : scans) {
        QStandardItem* scanItem = createScanItem(scan);
        if (scanItem) {
            item(0)->appendRow(scanItem); // Add to root
        }
    }

    qDebug() << "ProjectTreeModel: Loaded" << scans.size() << "scans from database";
}

void ProjectTreeModel::loadClustersFromDatabase()
{
    if (!m_sqliteManager) {
        qWarning() << "ProjectTreeModel: No SQLiteManager set";
        return;
    }

    // Get all clusters from database
    QList<ClusterInfo> clusters = m_sqliteManager->getAllClusters();

    for (const ClusterInfo& cluster : clusters) {
        QStandardItem* clusterItem = createClusterItem(cluster);
        if (clusterItem) {
            item(0)->appendRow(clusterItem); // Add to root
        }
    }

    qDebug() << "ProjectTreeModel: Loaded" << clusters.size() << "clusters from database";
}

void ProjectTreeModel::createAlignedGroups()
{
    if (!m_registrationProject) {
        qDebug() << "ProjectTreeModel: No RegistrationProject set, skipping aligned groups";
        return;
    }

    // Get all registration results
    QList<Registration::RegistrationProject::RegistrationResult> results =
        m_registrationProject->getRegistrationResults();

    qDebug() << "ProjectTreeModel: Creating" << results.size() << "aligned groups";

    // Track which scans are already grouped to avoid duplicates
    QSet<QString> groupedScans;

    for (const auto& result : results) {
        if (!result.isValid) {
            continue; // Skip invalid results
        }

        // Create aligned group item
        QStandardItem* groupItem = createAlignedGroupItem(
            result.sourceScanId, result.targetScanId, result.rmsError);

        if (groupItem) {
            // Find and move scan items to this group
            QStandardItem* sourceScanItem = findScanItem(result.sourceScanId);
            QStandardItem* targetScanItem = findScanItem(result.targetScanId);

            if (sourceScanItem && targetScanItem) {
                // Remove scans from their current parent if not already grouped
                if (!groupedScans.contains(result.sourceScanId)) {
                    QStandardItem* sourceParent = sourceScanItem->parent();
                    if (sourceParent) {
                        sourceParent->removeRow(sourceScanItem->row());
                    }

                    // Update visual cues for reference scan
                    updateScanItemVisualCues(sourceScanItem, result.sourceScanId);
                    sourceScanItem->setIcon(m_referenceScanIcon);

                    groupItem->appendRow(sourceScanItem);
                    groupedScans.insert(result.sourceScanId);
                }

                if (!groupedScans.contains(result.targetScanId)) {
                    QStandardItem* targetParent = targetScanItem->parent();
                    if (targetParent) {
                        targetParent->removeRow(targetScanItem->row());
                    }

                    // Update visual cues for target scan
                    updateScanItemVisualCues(targetScanItem, result.targetScanId);
                    targetScanItem->setIcon(m_targetScanIcon);

                    groupItem->appendRow(targetScanItem);
                    groupedScans.insert(result.targetScanId);
                }

                // Add group to root
                item(0)->appendRow(groupItem);
            }
        }
    }
}

QStandardItem* ProjectTreeModel::createScanItem(const ScanInfo& scan)
{
    QStandardItem* scanItem = new QStandardItem(scan.name.isEmpty() ? scan.scanId : scan.name);
    scanItem->setIcon(m_scanIcon);
    scanItem->setEditable(false);
    scanItem->setData(ScanItem, ItemTypeRole);
    scanItem->setData(scan.scanId, ItemIdRole);
    scanItem->setData(QVariant::fromValue(scan), ScanInfoRole);
    scanItem->setData(scan.pointCount, PointCountRole);

    // Add tooltip with scan information
    QString tooltip = QString("Scan ID: %1\nFile: %2\nPoints: %3")
                     .arg(scan.scanId)
                     .arg(scan.filePath)
                     .arg(scan.pointCount);
    scanItem->setToolTip(tooltip);

    return scanItem;
}

QStandardItem* ProjectTreeModel::createClusterItem(const ClusterInfo& cluster)
{
    QStandardItem* clusterItem = new QStandardItem(cluster.name.isEmpty() ? cluster.clusterId : cluster.name);
    clusterItem->setIcon(m_clusterIcon);
    clusterItem->setEditable(false);
    clusterItem->setData(ClusterItem, ItemTypeRole);
    clusterItem->setData(cluster.clusterId, ItemIdRole);
    clusterItem->setData(QVariant::fromValue(cluster), ClusterInfoRole);

    // Add tooltip with cluster information
    QString tooltip = QString("Cluster ID: %1\nDescription: %2")
                     .arg(cluster.clusterId)
                     .arg(cluster.description);
    clusterItem->setToolTip(tooltip);

    return clusterItem;
}

QStandardItem* ProjectTreeModel::createAlignedGroupItem(const QString& sourceScanId,
                                                       const QString& targetScanId,
                                                       float rmsError)
{
    QString groupName = QString("Aligned: %1 - %2 (%3)")
                       .arg(sourceScanId)
                       .arg(targetScanId)
                       .arg(formatRMSError(rmsError));

    QStandardItem* groupItem = new QStandardItem(groupName);
    groupItem->setIcon(m_alignedGroupIcon);
    groupItem->setEditable(false);
    groupItem->setData(AlignedGroupItem, ItemTypeRole);
    groupItem->setData(QString("%1-%2").arg(sourceScanId, targetScanId), ItemIdRole);

    // Set tooltip with detailed registration information
    QString tooltip = createTooltipText(sourceScanId, targetScanId, rmsError);
    groupItem->setToolTip(tooltip);

    return groupItem;
}

QStandardItem* ProjectTreeModel::findScanItem(const QString& scanId)
{
    // Search through all items to find the scan
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem* rootItem = item(i);
        if (!rootItem) continue;

        // Search in root level
        for (int j = 0; j < rootItem->rowCount(); ++j) {
            QStandardItem* childItem = rootItem->child(j);
            if (childItem &&
                childItem->data(ItemTypeRole).toInt() == ScanItem &&
                childItem->data(ItemIdRole).toString() == scanId) {
                return childItem;
            }

            // Search in nested items (clusters, groups)
            for (int k = 0; k < childItem->rowCount(); ++k) {
                QStandardItem* nestedItem = childItem->child(k);
                if (nestedItem &&
                    nestedItem->data(ItemTypeRole).toInt() == ScanItem &&
                    nestedItem->data(ItemIdRole).toString() == scanId) {
                    return nestedItem;
                }
            }
        }
    }

    return nullptr;
}

QStandardItem* ProjectTreeModel::findClusterItem(const QString& clusterId)
{
    // Search through all items to find the cluster
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem* rootItem = item(i);
        if (!rootItem) continue;

        for (int j = 0; j < rootItem->rowCount(); ++j) {
            QStandardItem* childItem = rootItem->child(j);
            if (childItem &&
                childItem->data(ItemTypeRole).toInt() == ClusterItem &&
                childItem->data(ItemIdRole).toString() == clusterId) {
                return childItem;
            }
        }
    }

    return nullptr;
}

QStandardItem* ProjectTreeModel::findAlignedGroupItem(const QString& sourceScanId, const QString& targetScanId)
{
    QString groupId1 = QString("%1-%2").arg(sourceScanId, targetScanId);
    QString groupId2 = QString("%1-%2").arg(targetScanId, sourceScanId);

    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem* rootItem = item(i);
        if (!rootItem) continue;

        for (int j = 0; j < rootItem->rowCount(); ++j) {
            QStandardItem* childItem = rootItem->child(j);
            if (childItem &&
                childItem->data(ItemTypeRole).toInt() == AlignedGroupItem) {
                QString itemId = childItem->data(ItemIdRole).toString();
                if (itemId == groupId1 || itemId == groupId2) {
                    return childItem;
                }
            }
        }
    }

    return nullptr;
}

void ProjectTreeModel::updateScanItemVisualCues(QStandardItem* scanItem, const QString& scanId)
{
    if (!scanItem || !m_registrationProject) return;

    // Check if this scan is a reference or target in any registration
    QList<Registration::RegistrationProject::RegistrationResult> results =
        m_registrationProject->getRegistrationResults();

    for (const auto& result : results) {
        if (result.sourceScanId == scanId) {
            // This is a reference scan
            QString currentText = scanItem->text();
            if (!currentText.contains("(Ref)")) {
                scanItem->setText(currentText + " (Ref)");
            }
            break;
        } else if (result.targetScanId == scanId) {
            // This is a target scan
            QString currentText = scanItem->text();
            if (!currentText.contains("(Tgt)")) {
                scanItem->setText(currentText + " (Tgt)");
            }
            break;
        }
    }
}

QString ProjectTreeModel::formatRMSError(float rmsError) const
{
    return QString("RMS: %1mm").arg(QString::number(rmsError, 'f', 2));
}

QString ProjectTreeModel::createTooltipText(const QString& sourceScanId, const QString& targetScanId, float rmsError) const
{
    QString tooltip = QString("Registration Result\n"
                             "Source Scan: %1\n"
                             "Target Scan: %2\n"
                             "RMS Error: %3mm\n"
                             "Status: Valid")
                     .arg(sourceScanId)
                     .arg(targetScanId)
                     .arg(QString::number(rmsError, 'f', 3));

    return tooltip;
}
void ProjectTreeModel::addScan(const ScanInfo& scan)
{
    QStandardItem* scanItem = createScanItem(scan);
    if (scanItem && rowCount() > 0) {
        item(0)->appendRow(scanItem); // Add to root
        qDebug() << "ProjectTreeModel: Added scan" << scan.scanId;
    }
}

void ProjectTreeModel::removeScan(const QString& scanId)
{
    QStandardItem* scanItem = findScanItem(scanId);
    if (scanItem) {
        QStandardItem* parent = scanItem->parent();
        if (parent) {
            parent->removeRow(scanItem->row());
            qDebug() << "ProjectTreeModel: Removed scan" << scanId;
        }
    }
}

void ProjectTreeModel::updateScan(const ScanInfo& scan)
{
    QStandardItem* scanItem = findScanItem(scan.scanId);
    if (scanItem) {
        // Update the scan item with new data
        scanItem->setText(scan.name.isEmpty() ? scan.scanId : scan.name);
        scanItem->setData(QVariant::fromValue(scan), ScanInfoRole);
        scanItem->setData(scan.pointCount, PointCountRole);

        // Update tooltip
        QString tooltip = QString("Scan ID: %1\nFile: %2\nPoints: %3")
                         .arg(scan.scanId)
                         .arg(scan.filePath)
                         .arg(scan.pointCount);
        scanItem->setToolTip(tooltip);

        qDebug() << "ProjectTreeModel: Updated scan" << scan.scanId;
    }
}

void ProjectTreeModel::addCluster(const ClusterInfo& cluster)
{
    QStandardItem* clusterItem = createClusterItem(cluster);
    if (clusterItem && rowCount() > 0) {
        item(0)->appendRow(clusterItem); // Add to root
        qDebug() << "ProjectTreeModel: Added cluster" << cluster.clusterId;
    }
}

void ProjectTreeModel::removeCluster(const QString& clusterId)
{
    QStandardItem* clusterItem = findClusterItem(clusterId);
    if (clusterItem) {
        QStandardItem* parent = clusterItem->parent();
        if (parent) {
            parent->removeRow(clusterItem->row());
            qDebug() << "ProjectTreeModel: Removed cluster" << clusterId;
        }
    }
}

void ProjectTreeModel::updateCluster(const ClusterInfo& cluster)
{
    QStandardItem* clusterItem = findClusterItem(cluster.clusterId);
    if (clusterItem) {
        // Update the cluster item with new data
        clusterItem->setText(cluster.name.isEmpty() ? cluster.clusterId : cluster.name);
        clusterItem->setData(QVariant::fromValue(cluster), ClusterInfoRole);

        // Update tooltip
        QString tooltip = QString("Cluster ID: %1\nDescription: %2")
                         .arg(cluster.clusterId)
                         .arg(cluster.description);
        clusterItem->setToolTip(tooltip);

        qDebug() << "ProjectTreeModel: Updated cluster" << cluster.clusterId;
    }
}

QString ProjectTreeModel::getItemType(QStandardItem* item) const
{
    if (!item) return QString();

    int itemType = item->data(ItemTypeRole).toInt();
    switch (itemType) {
        case ProjectRootItem: return "project_root";
        case ScanItem: return "scan";
        case ClusterItem: return "cluster";
        case AlignedGroupItem: return "aligned_group";
        default: return "unknown";
    }
}

QString ProjectTreeModel::getItemId(QStandardItem* item) const
{
    if (!item) return QString();
    return item->data(ItemIdRole).toString();
}

QVariant ProjectTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    QStandardItem* item = itemFromIndex(index);
    if (!item) {
        return QStandardItemModel::data(index, role);
    }

    // Handle custom tooltip role for aligned groups
    if (role == Qt::ToolTipRole && item->data(ItemTypeRole).toInt() == AlignedGroupItem) {
        return item->toolTip();
    }

    // Handle decoration role for reference/target scans
    if (role == Qt::DecorationRole) {
        int itemType = item->data(ItemTypeRole).toInt();
        if (itemType == ScanItem && m_registrationProject) {
            QString scanId = item->data(ItemIdRole).toString();
            QList<Registration::RegistrationProject::RegistrationResult> results =
                m_registrationProject->getRegistrationResults();

            for (const auto& result : results) {
                if (result.sourceScanId == scanId) {
                    return m_referenceScanIcon;
                } else if (result.targetScanId == scanId) {
                    return m_targetScanIcon;
                }
            }
        }
    }

    // Default to base class implementation
    return QStandardItemModel::data(index, role);
}
