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
class E57ParserLib;
class LasParser;
class ScanImportDialog;
class SQLiteManager;
struct LasHeaderMetadata;
struct ScanInfo;

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

    // Sprint 1.2: Scan Import functionality
    void onImportScans();
    void onScansImported(const QList<ScanInfo> &scans);

    // Legacy point cloud loading slots (for existing functionality)
    void onOpenFileClicked();
    void onLoadingFinished(bool success, const QString& message);
    void onParsingProgressUpdated(int percentage, const QString &stage);
    void onParsingFinished(bool success, const QString& message, const std::vector<float>& points);
    void onLoadingSettingsTriggered();
    void onLasHeaderParsed(const LasHeaderMetadata& metadata);

    // E57-specific slots
    void onScanMetadataReceived(int scanCount, const QStringList& scanNames);
    void onIntensityDataReceived(const std::vector<float>& intensityValues);
    void onColorDataReceived(const std::vector<uint8_t>& colorValues);
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

    // Sprint 1.2: Import guidance methods
    void showImportGuidance(bool show);
    void createImportGuidanceWidget();

    // Legacy helper methods for point cloud loading
    void cleanupParsingThread();
    void cleanupParsingThread(QObject* parser);
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
    QAction *m_importScansAction;
    QAction *m_loadingSettingsAction;
    QAction *m_topViewAction;
    QAction *m_leftViewAction;
    QAction *m_rightViewAction;
    QAction *m_bottomViewAction;

    // Sprint 1.2: Import guidance widgets
    QWidget *m_importGuidanceWidget;
    QPushButton *m_importGuidanceButton;

    // Legacy data processing
    LasParser *m_lasParser;
    QThread *m_parserThread;
    QObject *m_workerParser = nullptr;  // Generic pointer for any parser type
    QString m_currentFilePath;
    bool m_isLoading;

    // E57-specific data storage
    int m_currentScanCount = 0;
    QStringList m_currentScanNames;
    std::vector<float> m_currentIntensityData;
    std::vector<uint8_t> m_currentColorData;

    // Status bar widgets
    QLabel *m_statusLabel;
    QLabel *m_permanentStatusLabel;
    QString m_currentFileName;
    int m_currentPointCount;
};

#endif // MAINWINDOW_H
