#ifndef PROJECTHUBWIDGET_H
#define PROJECTHUBWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QTimer>

class RecentProjectsManager;
class ProjectManager;
class QVBoxLayout;
class QHBoxLayout;

class ProjectHubWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectHubWidget(QWidget *parent = nullptr);
    void refreshRecentProjects();

signals:
    void projectOpened(const QString &projectPath);

private slots:
    void onCreateNewProject();
    void onOpenProject();
    void onRecentProjectClicked(QListWidgetItem *item);
    void onRecentProjectDoubleClicked(QListWidgetItem *item);
    void validateRecentProjects();

private:
    void setupUI();
    void setupStyles();
    void openProjectFromPath(const QString &projectPath);
    void showErrorMessage(const QString &title, const QString &message);
    void showSuccessMessage(const QString &message);
    
    QPushButton *m_createNewBtn;
    QPushButton *m_openProjectBtn;
    QListWidget *m_recentProjectsList;
    QLabel *m_titleLabel;
    QLabel *m_recentLabel;
    QLabel *m_statusLabel;
    
    RecentProjectsManager *m_recentManager;
    ProjectManager *m_projectManager;
    QTimer *m_validationTimer;
};

#endif // PROJECTHUBWIDGET_H
