#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QTreeView>
#include <QStandardItemModel>

class SidebarWidget : public QTreeView
{
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget *parent = nullptr);
    void setProject(const QString &projectName, const QString &projectPath);
    void clearProject();

private:
    void setupUI();
    
    QStandardItemModel *m_model;
};

#endif // SIDEBARWIDGET_H
