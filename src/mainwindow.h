#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QSplitter>
#include <QLabel>
#include <QAction>
#include <vector>

class ProjectHubWidget;
class SidebarWidget;
class PointCloudViewerWidget;
class ProjectManager;
class Project;
class E57Parser;
class LasParser;
struct LasHeaderMetadata;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onProjectOpened(const QString &projectPath);
    void showProjectHub();
    void onFileNewProject();
    void onFileOpenProject();
    void closeCurrentProject();

    // Legacy point cloud loading slots (for existing functionality)
    void onOpenFileClicked();
    void onLoadingFinished(bool success, const QString& message);
    void onParsingProgressUpdated(int percentage, const QString &stage);
    void onParsingFinished(bool success, const QString& message, const std::vector<float>& points);
    void onLoadingSettingsTriggered();
    void onLasHeaderParsed(const LasHeaderMetadata& metadata);
    void onTopViewClicked();
    void onLeftViewClicked();
    void onRightViewClicked();
    void onBottomViewClicked();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void transitionToProjectView(const QString &projectPath);
    void updateWindowTitle(const QString &projectName = QString());

    // Legacy helper methods for point cloud loading
    void cleanupParsingThread();
    void cleanupProgressDialog();
    void updateUIAfterParsing(bool success, const QString& message);
    void setStatusReady();
    void setStatusLoading(const QString &filename);
    void setStatusLoadSuccess(const QString &filename, int pointCount);
    void setStatusLoadFailed(const QString &filename, const QString &error);
    void setStatusFileInfo(const QString &filename, int pointCount,
                          double minX, double minY, double minZ,
                          double maxX, double maxY, double maxZ);
    void setStatusViewChanged(const QString &viewName);

    // Main UI Components
    QStackedWidget *m_centralStack;
    ProjectHubWidget *m_projectHub;
    QWidget *m_projectView;
    QSplitter *m_projectSplitter;
    SidebarWidget *m_sidebar;
    QWidget *m_mainContentArea;

    // Legacy point cloud viewer
    PointCloudViewerWidget *m_viewer;
    QProgressDialog *m_progressDialog;

    // Project management
    ProjectManager *m_projectManager;
    Project *m_currentProject;

    // Menu actions
    QAction *m_newProjectAction;
    QAction *m_openProjectAction;
    QAction *m_closeProjectAction;
    QAction *m_loadingSettingsAction;
    QAction *m_topViewAction;
    QAction *m_leftViewAction;
    QAction *m_rightViewAction;
    QAction *m_bottomViewAction;

    // Legacy data processing
    E57Parser *m_e57Parser;
    LasParser *m_lasParser;
    QThread *m_parserThread;
    QString m_currentFilePath;
    bool m_isLoading;

    // Status bar widgets
    QLabel *m_statusLabel;
    QLabel *m_permanentStatusLabel;
    QString m_currentFileName;
    int m_currentPointCount;
};

#endif // MAINWINDOW_H
