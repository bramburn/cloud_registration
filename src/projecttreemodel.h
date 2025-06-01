#ifndef PROJECTTREEMODEL_H
#define PROJECTTREEMODEL_H

#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QList>

class SQLiteManager;
struct ScanInfo;

class ProjectTreeModel : public QStandardItemModel
{
    Q_OBJECT

public:
    explicit ProjectTreeModel(QObject *parent = nullptr);
    
    void setProject(const QString &projectName, const QString &projectPath);
    void refreshScans();
    void addScan(const ScanInfo &scan);
    void setSQLiteManager(SQLiteManager *manager);

private:
    void createProjectStructure();
    void loadScansFromDatabase();
    QStandardItem* createScanItem(const ScanInfo &scan);
    QStandardItem* getOrCreateScansFolder();
    
    QString m_projectName;
    QString m_projectPath;
    SQLiteManager *m_sqliteManager;
    QStandardItem *m_rootItem;
    QStandardItem *m_scansFolder;
};

#endif // PROJECTTREEMODEL_H
