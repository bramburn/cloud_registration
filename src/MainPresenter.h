#ifndef MAINPRESENTER_H
#define MAINPRESENTER_H

#include <QObject>
#include <QString>
#include <QList>
#include <memory>

// Forward declarations
class IMainView;
class IPointCloudViewer;
class IE57Parser;
class IE57Writer;
class ProjectManager;
class Project;
struct ScanInfo;
struct LasHeaderMetadata;
class LoadingSettings;

/**
 * @brief Presenter class for the main application window (MVP pattern)
 * 
 * This class contains all the business logic and application workflow that was
 * previously in MainWindow. It acts as an intermediary between the view (IMainView)
 * and the model (services like parsers, project manager, etc.).
 */
class MainPresenter : public QObject
{
    Q_OBJECT

public:
    explicit MainPresenter(IMainView* view, 
                          IE57Parser* e57Parser = nullptr,
                          IE57Writer* e57Writer = nullptr,
                          QObject* parent = nullptr);
    ~MainPresenter();

    // Initialization
    void initialize();
    void setProjectManager(ProjectManager* projectManager);

    // Project management operations
    void handleNewProject();
    void handleOpenProject();
    void handleCloseProject();
    void handleProjectOpened(const QString& projectPath);

    // Scan import operations (simplified for now)
    void handleImportScans();
    void handleScanActivated(const QString& scanId);

    // File operations
    void handleOpenFile();
    void handleLoadingSettings();

    // Point cloud loading operations
    void handleLoadingFinished(bool success, const QString& message);
    void handleParsingProgressUpdated(int percentage, const QString& stage);
    void handleParsingFinished(bool success, const QString& message, const std::vector<float>& points);
    void handleLasHeaderParsed(const LasHeaderMetadata& metadata);

    // E57-specific operations
    void handleScanMetadataReceived(int scanCount, const QStringList& scanNames);
    void handleIntensityDataReceived(const std::vector<float>& intensityValues);
    void handleColorDataReceived(const std::vector<uint8_t>& colorValues);

    // View control operations
    void handleTopViewClicked();
    void handleLeftViewClicked();
    void handleRightViewClicked();
    void handleBottomViewClicked();
    void handleFrontViewClicked();
    void handleBackViewClicked();
    void handleIsometricViewClicked();

    // Memory and performance monitoring
    void handleMemoryUsageChanged(size_t totalBytes);
    void handleStatsUpdated(float fps, int visiblePoints);

    // Progress management
    void handleProgressUpdated(const QString& operationId, int percentage, const QString& stage);
    void handleProgressCompleted(const QString& operationId, bool success, const QString& message);
    void handleProgressCancelled(const QString& operationId);

    // Application lifecycle
    void handleApplicationShutdown();

private slots:
    // Internal slots for service coordination
    void onParsingThreadFinished();
    void onProjectManagerError(const QString& error);

private:
    // Helper methods
    void setupConnections();
    void cleanupParsingThread(QObject* worker);
    void updateUIAfterParsing(bool success, const QString& message);
    void cleanupProgressDialog();
    void startE57Parsing(const QString& filePath, const LoadingSettings& settings);
    void startLasParsing(const QString& filePath, const LoadingSettings& settings);
    void validateProjectState();
    void updateWindowTitleForProject();

    // State management
    void setLoadingState(bool isLoading);
    void updateStatusForOperation(const QString& operation, bool success, const QString& details = QString());

    // Error handling
    void handleCriticalError(const QString& operation, const QString& error);
    void handleWarning(const QString& operation, const QString& warning);

private:
    // View and service dependencies
    IMainView* m_view;
    IE57Parser* m_e57Parser;
    IE57Writer* m_e57Writer;
    ProjectManager* m_projectManager;

    // Current state
    Project* m_currentProject;
    QString m_currentFilePath;
    QString m_currentFileName;
    bool m_isLoading;

    // E57-specific data
    int m_currentScanCount;
    QStringList m_currentScanNames;
    std::vector<float> m_currentIntensityData;
    std::vector<uint8_t> m_currentColorData;

    // Threading support
    QThread* m_parserThread;
    QObject* m_workerParser;

    // Progress tracking
    QString m_currentOperationId;
    int m_currentPointCount;
};

#endif // MAINPRESENTER_H
