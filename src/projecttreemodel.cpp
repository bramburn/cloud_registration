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
}

void ProjectTreeModel::setProject(const QString &projectName, const QString &projectPath)
{
    m_projectName = projectName;
    m_projectPath = projectPath;
    
    clear();
    setHorizontalHeaderLabels({"Project Structure"});
    
    createProjectStructure();
    loadScansFromDatabase();
}

void ProjectTreeModel::createProjectStructure()
{
    m_rootItem = new QStandardItem(m_projectName);
    m_rootItem->setIcon(QApplication::style()->standardIcon(QStyle::SP_DirIcon));
    m_rootItem->setEditable(false);
    m_rootItem->setData(m_projectPath, Qt::UserRole);
    m_rootItem->setData("project_root", Qt::UserRole + 1);
    
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
    
    if (!scans.isEmpty()) {
        QStandardItem *scansFolder = getOrCreateScansFolder();
        
        for (const ScanInfo &scan : scans) {
            QStandardItem *scanItem = createScanItem(scan);
            scansFolder->appendRow(scanItem);
        }
        
        qDebug() << "Loaded" << scans.size() << "scans from database";
    } else {
        qDebug() << "No scans found in database";
    }
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
    item->setData(scan.scanId, Qt::UserRole);
    item->setData("scan_item", Qt::UserRole + 1);
    
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
    QStandardItem *scansFolder = getOrCreateScansFolder();
    QStandardItem *scanItem = createScanItem(scan);
    scansFolder->appendRow(scanItem);
    
    qDebug() << "Added scan to tree model:" << scan.scanName;
}

void ProjectTreeModel::refreshScans()
{
    if (m_scansFolder) {
        m_scansFolder->removeRows(0, m_scansFolder->rowCount());
    }
    
    loadScansFromDatabase();
}

void ProjectTreeModel::setSQLiteManager(SQLiteManager *manager)
{
    m_sqliteManager = manager;
}
