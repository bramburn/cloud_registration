#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QTreeView>
#include <QStandardItemModel>

class ProjectTreeModel;
class SQLiteManager;
struct ScanInfo;

class SidebarWidget : public QTreeView
{
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget *parent = nullptr);
    void setProject(const QString &projectName, const QString &projectPath);
    void clearProject();
    void setSQLiteManager(SQLiteManager *manager);
    void refreshFromDatabase();
    void addScan(const ScanInfo &scan);

private:
    void setupUI();

    ProjectTreeModel *m_model;
    QString m_currentProjectPath;
};

#endif // SIDEBARWIDGET_H
