#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QSplitter>
#include <QLabel>
#include <QAction>
#include <QProgressBar>
#include <QVBoxLayout>
#include <vector>
#include "progressmanager.h"
#include "projectmanager.h"
#include "IPointCloudViewer.h"

class ProjectHubWidget;
class SidebarWidget;
class PointCloudViewerWidget;
class PointCloudLoadManager;
class Project;
class IE57Parser;
class LasParser;
class ScanImportDialog;
class SQLiteManager;
class QProgressDialog;
class QPushButton;
class QCheckBox;
class QSlider;
struct LasHeaderMetadata;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    explicit MainWindow(IE57Parser* e57Parser, QWidget *parent = nullptr);
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

    // Sprint 1.3: E57 scan activation
    void onScanActivated(const QString& scanId);

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

    // Sprint 3.2: Point cloud viewing slots
    void onPointCloudDataReady(const std::vector<float> &points, const QString &sourceInfo);
    void onPointCloudViewFailed(const QString &error);

    // Sprint 3.3: Progress management slots
    void onOperationStarted(const QString& operationId, const QString& name, OperationType type);
    void onProgressUpdated(const QString& operationId, int value, int max, const QString& step, const QString& details);
    void onOperationFinished(const QString& operationId, const QString& result);
    void onOperationCancelled(const QString& operationId);
    void onEstimatedTimeChanged(const QString& operationId, const QDateTime& estimatedEnd);
    void onCancelCurrentOperation();

    // Sprint R3: Attribute rendering and point size attenuation slots (as per backlog Tasks R3.1.6, R3.2.5, R3.3.3)
    void onColorRenderToggled(bool enabled);
    void onIntensityRenderToggled(bool enabled);
    void onAttenuationToggled(bool enabled);
    void onAttenuationParamsChanged();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupSprintR3Controls(QVBoxLayout* parentLayout);
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

    // Sprint 3.2: Test helper methods
    IPointCloudViewer* getPointCloudViewer() const { return m_viewer; }
    PointCloudLoadManager* getPointCloudLoadManager() const { return m_loadManager; }

    // Sprint 3.4: Memory statistics display
    void setupMemoryDisplay();
    void onMemoryUsageChanged(size_t totalBytes);

    // Sprint 2.2: Performance statistics display
    void onStatsUpdated(float fps, int visiblePoints);

    // Main UI Components
    QStackedWidget *m_centralStack;
    ProjectHubWidget *m_projectHub;
    QWidget *m_projectView;
    QSplitter *m_projectSplitter;
    SidebarWidget *m_sidebar;
    QWidget *m_mainContentArea;

    // Point cloud viewer (interface-based for decoupling)
    IPointCloudViewer *m_viewer;
    PointCloudViewerWidget *m_viewerWidget; // Concrete widget for layout management
    QProgressDialog *m_progressDialog;

    // Project management
    ProjectManager *m_projectManager;
    PointCloudLoadManager *m_loadManager;
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

    // Sprint 1 Decoupling: Injected E57 parser interface
    IE57Parser *m_e57Parser = nullptr;

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

    // Sprint 3.4: Memory usage display
    QLabel *m_memoryLabel;

    // Sprint 2.2: Performance monitoring display
    QLabel *m_fpsLabel;
    QLabel *m_pointsLabel;

    // Sprint 3.3: Progress display widgets
    QProgressBar* m_progressBar;
    QLabel* m_progressLabel;
    QLabel* m_timeLabel;
    QPushButton* m_cancelButton;
    QString m_currentOperationId;

    // Sprint R3: Attribute rendering and point size controls (as per backlog member variables)
    QCheckBox* m_colorRenderCheckbox;
    QCheckBox* m_intensityRenderCheckbox;
    QCheckBox* m_attenuationCheckbox;
    QSlider* m_minSizeSlider;
    QSlider* m_maxSizeSlider;
    QSlider* m_attenuationFactorSlider;
    QLabel* m_minSizeLabel;
    QLabel* m_maxSizeLabel;
    QLabel* m_attenuationFactorLabel;
};

#endif // MAINWINDOW_H
