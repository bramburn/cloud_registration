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
#include <memory>
#include "progressmanager.h"
#include "projectmanager.h"
#include "IMainView.h"

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
class QGroupBox;
class MainPresenter;
class IPointCloudViewer;
struct LasHeaderMetadata;

class MainWindow : public QMainWindow, public IMainView
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    explicit MainWindow(IE57Parser* e57Parser, QWidget *parent = nullptr);
    ~MainWindow();

    // IMainView interface implementation
    void setWindowTitle(const QString& title) override;
    void updateWindowTitle() override;
    void updateStatusBar(const QString& text) override;
    void setStatusReady() override;
    void setStatusLoading(const QString& fileName) override;
    void setStatusLoadSuccess(const QString& fileName, int pointCount) override;
    void setStatusLoadFailed(const QString& fileName, const QString& message) override;
    void setStatusViewChanged(const QString& viewName) override;
    void displayErrorMessage(const QString& title, const QString& message) override;
    void displayWarningMessage(const QString& title, const QString& message) override;
    void displayInfoMessage(const QString& title, const QString& message) override;
    void showProjectHub() override;
    void transitionToProjectView(const QString& projectPath) override;
    void enableProjectActions(bool enabled) override;
    void showImportGuidance(bool show) override;
    IPointCloudViewer* getViewer() override;
    void showProgressDialog(const QString& title, const QString& message) override;
    void updateProgressDialog(int percentage, const QString& stage) override;
    void hideProgressDialog() override;
    void updateMemoryDisplay(size_t totalBytes) override;
    void updatePerformanceStats(float fps, int visiblePoints) override;
    void setLoadingState(bool isLoading) override;
    void updateLoadingProgress(int percentage, const QString& stage) override;
    QString showOpenFileDialog(const QString& title, const QString& filter) override;
    QString showOpenProjectDialog() override;
    QString showSaveFileDialog(const QString& title, const QString& filter) override;
    bool showLoadingSettingsDialog() override;
    bool showCreateProjectDialog(QString& projectName, QString& projectPath) override;
    bool showScanImportDialog() override;
    void refreshScanList() override;
    void enableViewControls(bool enabled) override;
    void updateViewControlsState() override;
    bool isProjectOpen() const override;
    QString getCurrentProjectPath() const override;
    Project* getCurrentProject() const override;
    void prepareForShutdown() override;
    void cleanupResources() override;

private slots:
    // Simplified slots that delegate to presenter
    void onProjectOpened(const QString &projectPath) { if (m_presenter) m_presenter->handleProjectOpened(projectPath); }
    void onFileNewProject() { if (m_presenter) m_presenter->handleNewProject(); }
    void onFileOpenProject() { if (m_presenter) m_presenter->handleOpenProject(); }
    void closeCurrentProject() { if (m_presenter) m_presenter->handleCloseProject(); }
    void onImportScans() { if (m_presenter) m_presenter->handleImportScans(); }

    void onScanActivated(const QString& scanId) { if (m_presenter) m_presenter->handleScanActivated(scanId); }
    void onOpenFileClicked() { if (m_presenter) m_presenter->handleOpenFile(); }
    void onLoadingFinished(bool success, const QString& message) { if (m_presenter) m_presenter->handleLoadingFinished(success, message); }
    void onParsingProgressUpdated(int percentage, const QString &stage) { if (m_presenter) m_presenter->handleParsingProgressUpdated(percentage, stage); }
    void onParsingFinished(bool success, const QString& message, const std::vector<float>& points) { if (m_presenter) m_presenter->handleParsingFinished(success, message, points); }
    void onLoadingSettingsTriggered() { if (m_presenter) m_presenter->handleLoadingSettings(); }
    void onLasHeaderParsed(const LasHeaderMetadata& metadata) { if (m_presenter) m_presenter->handleLasHeaderParsed(metadata); }
    void onScanMetadataReceived(int scanCount, const QStringList& scanNames) { if (m_presenter) m_presenter->handleScanMetadataReceived(scanCount, scanNames); }
    void onIntensityDataReceived(const std::vector<float>& intensityValues) { if (m_presenter) m_presenter->handleIntensityDataReceived(intensityValues); }
    void onColorDataReceived(const std::vector<uint8_t>& colorValues) { if (m_presenter) m_presenter->handleColorDataReceived(colorValues); }
    void onTopViewClicked() { if (m_presenter) m_presenter->handleTopViewClicked(); }
    void onLeftViewClicked() { if (m_presenter) m_presenter->handleLeftViewClicked(); }
    void onRightViewClicked() { if (m_presenter) m_presenter->handleRightViewClicked(); }
    void onBottomViewClicked() { if (m_presenter) m_presenter->handleBottomViewClicked(); }

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

    // Sprint R4: Splatting and lighting slots (Task R4.3.1)
    void onSplattingToggled(bool enabled);
    void onLightingToggled(bool enabled);
    void onLightDirectionChanged();
    void onLightColorClicked();
    void onAmbientIntensityChanged(int value);

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupSprintR3Controls(QVBoxLayout* parentLayout);
    void setupSprintR4Controls(QVBoxLayout* parentLayout);
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
    PointCloudViewerWidget* getPointCloudViewer() const { return m_viewer; }
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

    // Legacy point cloud viewer
    PointCloudViewerWidget *m_viewer;
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

    // Sprint R4: Splatting and lighting controls (Task R4.3.1)
    QGroupBox* m_splattingGroupBox;
    QCheckBox* m_splattingCheckbox;

    QGroupBox* m_lightingGroupBox;
    QCheckBox* m_lightingCheckbox;
    QSlider* m_lightDirXSlider;
    QSlider* m_lightDirYSlider;
    QSlider* m_lightDirZSlider;
    QLabel* m_lightDirXLabel;
    QLabel* m_lightDirYLabel;
    QLabel* m_lightDirZLabel;
    QPushButton* m_lightColorButton;
    QLabel* m_lightColorLabel;
    QSlider* m_ambientIntensitySlider;
    QLabel* m_ambientIntensityLabel;

    // Current state
    QColor m_currentLightColor;

    // Sprint 4: MVP Pattern - Presenter
    std::unique_ptr<MainPresenter> m_presenter;

    // Interface pointer for viewer (Sprint 3 decoupling)
    IPointCloudViewer* m_viewerInterface;
};

#endif // MAINWINDOW_H
