#ifndef MAINPRESENTER_H
#define MAINPRESENTER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>

// Forward declarations
class IMainView;
class IE57Parser;
class IE57Writer;
class IPointCloudViewer;
class ProjectManager;
class PointCloudLoadManager;

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
     * @brief Constructor with dependency injection.
     * @param view Pointer to the main view interface.
     * @param e57Parser Pointer to the E57 parser interface.
     * @param e57Writer Pointer to the E57 writer interface.
     * @param projectManager Pointer to the project manager.
     * @param loadManager Pointer to the point cloud load manager.
     * @param parent Parent QObject.
     */
    explicit MainPresenter(IMainView* view,
                          IE57Parser* e57Parser,
                          IE57Writer* e57Writer = nullptr,
                          ProjectManager* projectManager = nullptr,
                          PointCloudLoadManager* loadManager = nullptr,
                          QObject* parent = nullptr);

    virtual ~MainPresenter() = default;

    /**
     * @brief Initialize the presenter and set up connections.
     */
    void initialize();

    /**
     * @brief Set the project manager.
     * @param projectManager Pointer to project manager.
     */
    void setProjectManager(ProjectManager* projectManager);

    /**
     * @brief Set the point cloud load manager.
     * @param loadManager Pointer to load manager.
     */
    void setPointCloudLoadManager(PointCloudLoadManager* loadManager);

public slots:
    /**
     * @brief Handle new project creation request.
     */
    void handleNewProject();

    /**
     * @brief Handle project opening request.
     */
    void handleOpenProject();

    /**
     * @brief Handle project closing request.
     */
    void handleCloseProject();

    /**
     * @brief Handle scan import request.
     */
    void handleImportScans();

    /**
     * @brief Handle file opening request.
     * @param filePath Path of the file to open.
     */
    void handleOpenFile(const QString& filePath);

    /**
     * @brief Handle file saving request.
     * @param filePath Path where to save the file.
     */
    void handleSaveFile(const QString& filePath);

    /**
     * @brief Handle scan activation.
     * @param scanId ID of the scan to activate.
     */
    void handleScanActivation(const QString& scanId);

    /**
     * @brief Handle viewer settings changes.
     */
    void handleViewerSettingsChanged();

    /**
     * @brief Handle application exit request.
     */
    void handleExit();

    // Sidebar-related handlers
    /**
     * @brief Handle cluster creation request from sidebar.
     * @param clusterName Name of the new cluster.
     * @param parentClusterId ID of parent cluster (empty for root level).
     */
    void handleClusterCreation(const QString& clusterName, const QString& parentClusterId = QString());

    /**
     * @brief Handle cluster renaming request from sidebar.
     * @param clusterId ID of cluster to rename.
     * @param newName New name for the cluster.
     */
    void handleClusterRename(const QString& clusterId, const QString& newName);

    /**
     * @brief Handle cluster deletion request from sidebar.
     * @param clusterId ID of cluster to delete.
     * @param deletePhysicalFiles Whether to delete physical files.
     */
    void handleClusterDeletion(const QString& clusterId, bool deletePhysicalFiles = false);

    /**
     * @brief Handle scan loading request from sidebar.
     * @param scanId ID of scan to load.
     */
    void handleScanLoad(const QString& scanId);

    /**
     * @brief Handle scan unloading request from sidebar.
     * @param scanId ID of scan to unload.
     */
    void handleScanUnload(const QString& scanId);

    /**
     * @brief Handle cluster loading request from sidebar.
     * @param clusterId ID of cluster to load (all scans).
     */
    void handleClusterLoad(const QString& clusterId);

    /**
     * @brief Handle cluster unloading request from sidebar.
     * @param clusterId ID of cluster to unload (all scans).
     */
    void handleClusterUnload(const QString& clusterId);

    /**
     * @brief Handle point cloud viewing request from sidebar.
     * @param itemId ID of item to view.
     * @param itemType Type of item ("scan" or "cluster").
     */
    void handlePointCloudView(const QString& itemId, const QString& itemType);

    /**
     * @brief Handle scan deletion request from sidebar.
     * @param scanId ID of scan to delete.
     * @param deletePhysicalFile Whether to delete physical file.
     */
    void handleScanDeletion(const QString& scanId, bool deletePhysicalFile = false);

    /**
     * @brief Handle cluster lock/unlock request from sidebar.
     * @param clusterId ID of cluster to lock/unlock.
     * @param lock true to lock, false to unlock.
     */
    void handleClusterLockToggle(const QString& clusterId, bool lock);

    /**
     * @brief Handle drag and drop operation from sidebar.
     * @param draggedItems List of dragged item IDs.
     * @param draggedType Type of dragged items ("scan" or "cluster").
     * @param targetItemId ID of target item.
     * @param targetType Type of target item.
     */
    void handleDragDropOperation(const QStringList& draggedItems, const QString& draggedType,
                               const QString& targetItemId, const QString& targetType);

private slots:
    /**
     * @brief Handle E57 parsing progress updates.
     * @param percentage Progress percentage (0-100).
     * @param stage Current parsing stage.
     */
    void onParsingProgress(int percentage, const QString& stage);

    /**
     * @brief Handle E57 parsing completion.
     * @param success true if parsing succeeded, false otherwise.
     * @param message Success or error message.
     * @param points Extracted point data.
     */
    void onParsingFinished(bool success, const QString& message, const std::vector<float>& points);

    /**
     * @brief Handle scan metadata availability.
     * @param scanCount Number of scans found.
     * @param scanNames List of scan names.
     */
    void onScanMetadataAvailable(int scanCount, const QStringList& scanNames);

    /**
     * @brief Handle intensity data extraction.
     * @param intensityValues Extracted intensity values.
     */
    void onIntensityDataExtracted(const std::vector<float>& intensityValues);

    /**
     * @brief Handle color data extraction.
     * @param colorValues Extracted color values (RGB interleaved).
     */
    void onColorDataExtracted(const std::vector<uint8_t>& colorValues);

    /**
     * @brief Handle point cloud viewer state changes.
     * @param newState New viewer state.
     * @param message Optional message.
     */
    void onViewerStateChanged(int newState, const QString& message);

    /**
     * @brief Handle rendering statistics updates.
     * @param fps Frames per second.
     * @param visiblePoints Number of visible points.
     */
    void onRenderingStatsUpdated(float fps, int visiblePoints);

    /**
     * @brief Handle memory usage updates.
     * @param totalBytes Total memory usage in bytes.
     */
    void onMemoryUsageChanged(size_t totalBytes);

private:
    /**
     * @brief Set up signal-slot connections between components.
     */
    void setupConnections();

    /**
     * @brief Update UI state based on current application state.
     */
    void updateUIState();

    /**
     * @brief Validate file path for opening.
     * @param filePath Path to validate.
     * @return true if valid, false otherwise.
     */
    bool validateFilePath(const QString& filePath);

    /**
     * @brief Show error message to user.
     * @param title Error title.
     * @param message Error message.
     */
    void showError(const QString& title, const QString& message);

    /**
     * @brief Show information message to user.
     * @param title Info title.
     * @param message Info message.
     */
    void showInfo(const QString& title, const QString& message);

    /**
     * @brief Update window title based on current state.
     */
    void updateWindowTitle();

    /**
     * @brief Clear current point cloud data.
     */
    void clearPointCloudData();

private:
    // Interface pointers (not owned by this class)
    IMainView* m_view;
    IE57Parser* m_e57Parser;
    IE57Writer* m_e57Writer;
    IPointCloudViewer* m_viewer;

    // Manager pointers (not owned by this class)
    ProjectManager* m_projectManager;
    PointCloudLoadManager* m_loadManager;

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

    // Sidebar state management
    QStringList m_loadedScans;
    QStringList m_lockedClusters;
};

#endif // MAINPRESENTER_H
