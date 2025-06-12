#ifndef MAINPRESENTER_H
#define MAINPRESENTER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <vector>

class IMainView;
class IE57Parser;
class IE57Writer;
class IPointCloudViewer;
struct LasHeaderMetadata;

/**
 * @brief MainPresenter - Presentation layer for the main application window
 * 
 * This class implements the MVP (Model-View-Presenter) pattern by acting as the
 * intermediary between the view (IMainView) and the model (services like IE57Parser).
 * It contains all the application logic and coordinates between different components
 * without being coupled to specific UI or service implementations.
 * 
 * Sprint 4 Decoupling Requirements:
 * - Separates presentation logic from UI implementation
 * - Coordinates between view and model components through interfaces
 * - Enables unit testing of application logic without UI dependencies
 * - Promotes loose coupling and high cohesion
 */
class MainPresenter : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructor with dependency injection
     * @param view Pointer to the main view interface
     * @param e57Parser Pointer to the E57 parser interface
     * @param e57Writer Pointer to the E57 writer interface
     * @param parent Parent QObject
     */
    explicit MainPresenter(IMainView* view, 
                          IE57Parser* e57Parser, 
                          IE57Writer* e57Writer = nullptr,
                          QObject* parent = nullptr);

    virtual ~MainPresenter() = default;

    /**
     * @brief Initialize the presenter and set up connections
     */
    void initialize();

public slots:
    /**
     * @brief Handle new project creation request
     */
    void handleNewProject();

    /**
     * @brief Handle project opening request
     */
    void handleOpenProject();

    /**
     * @brief Handle project closing request
     */
    void handleCloseProject();

    /**
     * @brief Handle scan import request
     */
    void handleImportScans();

    /**
     * @brief Handle file opening request
     * @param filePath Path of the file to open
     */
    void handleOpenFile(const QString& filePath);

    /**
     * @brief Handle file opening request (overload without parameters)
     */
    void handleOpenFile();

    /**
     * @brief Handle file saving request
     * @param filePath Path where to save the file
     */
    void handleSaveFile(const QString& filePath);

    /**
     * @brief Handle scan activation
     * @param scanId ID of the scan to activate
     */
    void handleScanActivation(const QString& scanId);

    /**
     * @brief Handle scan activation (alias for compatibility)
     * @param scanId ID of the scan to activate
     */
    void handleScanActivated(const QString& scanId);

    /**
     * @brief Handle project opened event
     * @param projectPath Path of the opened project
     */
    void handleProjectOpened(const QString& projectPath);

    /**
     * @brief Handle loading finished event
     * @param success Whether loading was successful
     * @param message Status message
     */
    void handleLoadingFinished(bool success, const QString& message);

    /**
     * @brief Handle parsing progress update
     * @param percentage Progress percentage
     * @param stage Current parsing stage
     */
    void handleParsingProgressUpdated(int percentage, const QString& stage);

    /**
     * @brief Handle parsing finished event
     * @param success Whether parsing was successful
     * @param message Status message
     * @param points Extracted point data
     */
    void handleParsingFinished(bool success, const QString& message, const std::vector<float>& points);

    /**
     * @brief Handle loading settings request
     */
    void handleLoadingSettings();

    /**
     * @brief Handle LAS header parsed event
     * @param metadata LAS header metadata
     */
    void handleLasHeaderParsed(const LasHeaderMetadata& metadata);

    /**
     * @brief Handle scan metadata received event
     * @param scanCount Number of scans
     * @param scanNames List of scan names
     */
    void handleScanMetadataReceived(int scanCount, const QStringList& scanNames);

    /**
     * @brief Handle intensity data received event
     * @param intensityValues Intensity data
     */
    void handleIntensityDataReceived(const std::vector<float>& intensityValues);

    /**
     * @brief Handle color data received event
     * @param colorValues Color data
     */
    void handleColorDataReceived(const std::vector<uint8_t>& colorValues);

    /**
     * @brief Handle view button clicks
     */
    void handleTopViewClicked();
    void handleLeftViewClicked();
    void handleRightViewClicked();
    void handleBottomViewClicked();

    /**
     * @brief Handle viewer settings changes
     */
    void handleViewerSettingsChanged();

    /**
     * @brief Handle application exit request
     */
    void handleExit();

private slots:
    /**
     * @brief Handle E57 parsing progress updates
     * @param percentage Progress percentage (0-100)
     * @param stage Current parsing stage
     */
    void onParsingProgress(int percentage, const QString& stage);

    /**
     * @brief Handle E57 parsing completion
     * @param success true if parsing succeeded, false otherwise
     * @param message Success or error message
     * @param points Extracted point data
     */
    void onParsingFinished(bool success, const QString& message, const std::vector<float>& points);

    /**
     * @brief Handle scan metadata availability
     * @param scanCount Number of scans found
     * @param scanNames List of scan names
     */
    void onScanMetadataAvailable(int scanCount, const QStringList& scanNames);

    /**
     * @brief Handle intensity data extraction
     * @param intensityValues Extracted intensity values
     */
    void onIntensityDataExtracted(const std::vector<float>& intensityValues);

    /**
     * @brief Handle color data extraction
     * @param colorValues Extracted color values (RGB interleaved)
     */
    void onColorDataExtracted(const std::vector<uint8_t>& colorValues);

    /**
     * @brief Handle point cloud viewer state changes
     * @param newState New viewer state
     * @param message Optional message
     */
    void onViewerStateChanged(int newState, const QString& message);

    /**
     * @brief Handle rendering statistics updates
     * @param fps Frames per second
     * @param visiblePoints Number of visible points
     */
    void onRenderingStatsUpdated(float fps, int visiblePoints);

    /**
     * @brief Handle memory usage updates
     * @param totalBytes Total memory usage in bytes
     */
    void onMemoryUsageChanged(size_t totalBytes);

private:
    /**
     * @brief Set up signal-slot connections between components
     */
    void setupConnections();

    /**
     * @brief Update UI state based on current application state
     */
    void updateUIState();

    /**
     * @brief Validate file path for opening
     * @param filePath Path to validate
     * @return true if valid, false otherwise
     */
    bool validateFilePath(const QString& filePath);

    /**
     * @brief Show error message to user
     * @param title Error title
     * @param message Error message
     */
    void showError(const QString& title, const QString& message);

    /**
     * @brief Show information message to user
     * @param title Info title
     * @param message Info message
     */
    void showInfo(const QString& title, const QString& message);

    /**
     * @brief Update window title based on current state
     */
    void updateWindowTitle();

    /**
     * @brief Clear current point cloud data
     */
    void clearPointCloudData();

private:
    // Interface pointers (not owned by this class)
    IMainView* m_view;
    IE57Parser* m_e57Parser;
    IE57Writer* m_e57Writer;
    IPointCloudViewer* m_viewer;

    // Application state
    QString m_currentProjectPath;
    QString m_currentFilePath;
    QStringList m_currentScanNames;
    bool m_isFileOpen;
    bool m_isProjectOpen;
    bool m_isParsingInProgress;

    // Statistics
    size_t m_currentMemoryUsage;
    float m_currentFPS;
    int m_currentVisiblePoints;
};

#endif // MAINPRESENTER_H
