#ifndef PROJECTTREEMODEL_H
#define PROJECTTREEMODEL_H

#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include <QVariant>
#include <QIcon>

// Forward declarations
class SQLiteManager;
namespace Registration {
    class RegistrationProject;
}

struct ScanInfo;
struct ClusterInfo;

/**
 * @brief ProjectTreeModel - Model for project tree view with scan grouping support
 *
 * Sprint 3.3 Implementation: Enhanced to support grouping of registered scans
 * and display of registration quality metrics.
 */
class ProjectTreeModel : public QStandardItemModel
{
    Q_OBJECT

public:
    // Custom data roles
    enum CustomRoles {
        ItemTypeRole = Qt::UserRole + 1,
        ItemIdRole,
        ScanInfoRole,
        ClusterInfoRole,
        RegistrationResultRole,
        PointCountRole,
        FileSizeRole
    };

    // Item types
    enum ItemType {
        ProjectRootItem,
        ScanItem,
        ClusterItem,
        AlignedGroupItem
    };

    explicit ProjectTreeModel(QObject* parent = nullptr);
    virtual ~ProjectTreeModel() = default;

    // Configuration
    void setSQLiteManager(SQLiteManager* manager);
    void setRegistrationProject(Registration::RegistrationProject* project);

    // Data refresh
    void refreshFromDatabase();
    void refreshScans();

    // Scan management
    void addScan(const ScanInfo& scan);
    void removeScan(const QString& scanId);
    void updateScan(const ScanInfo& scan);

    // Cluster management
    void addCluster(const ClusterInfo& cluster);
    void removeCluster(const QString& clusterId);
    void updateCluster(const ClusterInfo& cluster);

    // Item queries
    QString getItemType(QStandardItem* item) const;
    QString getItemId(QStandardItem* item) const;

    // QStandardItemModel interface
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    SQLiteManager* m_sqliteManager;
    Registration::RegistrationProject* m_registrationProject;

    // Icons for different item types
    QIcon m_projectIcon;
    QIcon m_scanIcon;
    QIcon m_clusterIcon;
    QIcon m_alignedGroupIcon;
    QIcon m_referenceScanIcon;
    QIcon m_targetScanIcon;

    // Helper methods
    void initializeIcons();
    void createProjectStructure();
    void loadScansFromDatabase();
    void loadClustersFromDatabase();
    void createAlignedGroups();

    QStandardItem* createScanItem(const ScanInfo& scan);
    QStandardItem* createClusterItem(const ClusterInfo& cluster);
    QStandardItem* createAlignedGroupItem(const QString& sourceScanId,
                                         const QString& targetScanId,
                                         float rmsError);

    QStandardItem* findScanItem(const QString& scanId);
    QStandardItem* findClusterItem(const QString& clusterId);
    QStandardItem* findAlignedGroupItem(const QString& sourceScanId, const QString& targetScanId);

    void updateScanItemVisualCues(QStandardItem* scanItem, const QString& scanId);
    QString formatRMSError(float rmsError) const;
    QString createTooltipText(const QString& sourceScanId, const QString& targetScanId, float rmsError) const;
};

#endif  // PROJECTTREEMODEL_H
