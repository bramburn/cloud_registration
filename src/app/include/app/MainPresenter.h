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
class RegistrationProject;
struct ExportResult;
class TargetManager;
class AlignmentEngine;
class PoseGraphViewerWidget;
class BundleAdjustmentProgressDialog;
class QualityAssessment;
class PDFReportGenerator;
struct QualityReport;
class ICPProgressWidget;
class ReportOptionsDialog;

namespace Registration {
    class PoseGraph;
    class PoseGraphBuilder;
    class RegistrationProject;
}

namespace Optimization {
    class BundleAdjustment;
}

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
class MainPresenter : public QObject
{
        Q_OBJECT

        public :
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
    /**
     * @brief Set the target manager.
     * @param targetManager Pointer to target manager.
     */
    void setTargetManager(TargetManager* targetManager);

    /**
     * @brief Set the alignment engine.
     * @param alignmentEngine Pointer to alignment engine.
     */
    void setAlignmentEngine(AlignmentEngine* alignmentEngine);

    /**
     * @brief Set the quality assessment instance (Sprint 6.2)
     * @param qualityAssessment Pointer to the quality assessment
     */
    void setQualityAssessment(QualityAssessment* qualityAssessment);

    /**
     * @brief Set the PDF report generator instance (Sprint 6.2)
     * @param reportGenerator Pointer to the PDF report generator
     */
    void setPDFReportGenerator(PDFReportGenerator* reportGenerator);

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
        /**
         * @brief Handle alignment acceptance request.
         */
    void handleAcceptAlignment();

        /**
         * @brief Handle alignment cancellation request.
         */
    void handleCancelAlignment();

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
    /**
     * @brief Handle automatic alignment (ICP) button click
     *
     * This slot is triggered when the user clicks the "Automatic Alignment (ICP)"
     * button in the RegistrationWorkflowWidget. It launches the ICPParameterDialog
     * and initiates the ICP computation if parameters are accepted.
     */
    void handleAutomaticAlignmentClicked();

    /**
     * @brief Cancel currently running automatic alignment
     *
     * This slot is connected to ICPProgressWidget::cancelRequested signal
     * and cancels the ongoing ICP computation.
     */
    void cancelAutomaticAlignment();

    /**
     * @brief Connect to a RegistrationWorkflowWidget
     * @param workflowWidget Pointer to the workflow widget to connect
     */
    void connectToWorkflowWidget(class RegistrationWorkflowWidget* workflowWidget);

    // Sprint 3.2: Export functionality
    void handleExportPointCloud();

    /**
     * @brief Handle target detection request from workflow widget.
     */
    void handleTargetDetectionClicked();

    /**
     * @brief Cancel currently running target detection
     */
    void cancelTargetDetection();

    /**
     * @brief Connect to a RegistrationWorkflowWidget for target detection integration.
     * @param workflowWidget Pointer to the workflow widget to connect
     */
    void connectToWorkflowWidget(class RegistrationWorkflowWidget* workflowWidget);

    // Pose Graph Management
    /**
     * @brief Set the registration project for pose graph operations
     * @param project Pointer to the registration project
     */
    void setRegistrationProject(Registration::RegistrationProject* project);

    /**
     * @brief Set the pose graph viewer widget
     * @param viewer Pointer to the pose graph viewer widget
     */
    void setPoseGraphViewer(PoseGraphViewerWidget* viewer);

    /**
     * @brief Handle project load completion and rebuild pose graph
     */
    void handleLoadProjectCompleted();

    /**
     * @brief Rebuild the pose graph from current registration project
     */
    void rebuildPoseGraph();

    /**
     * @brief Handle Bundle Adjustment request
     */
    void handleRunBundleAdjustment();

    /**
     * @brief Handle Bundle Adjustment cancellation request
     */
    void cancelBundleAdjustment();
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

    // Sprint 3.2: Export functionality slots
    void onExportCompleted(const ExportResult& result);

    /**
     * @brief Handle Bundle Adjustment progress updates
     * @param iteration Current iteration number
     * @param currentError Current optimization error
     * @param lambda Current lambda value
     */
    void onBundleAdjustmentProgress(int iteration, double currentError, double lambda);

    /**
     * @brief Handle Bundle Adjustment completion
     * @param result Bundle adjustment result
     */
    void onBundleAdjustmentCompleted(const Optimization::BundleAdjustment::Result& result);

    /**
     * @brief Handle deviation map toggle (Sprint 6.1)
     * @param enabled Whether to show or hide the deviation map
     */
    void handleShowDeviationMapToggled(bool enabled);

    /**
     * @brief Handle registration result added signal (Sprint 3.3)
     * @param sourceScanId ID of the source scan
     * @param targetScanId ID of the target scan
     */
    void onRegistrationResultAdded(const QString& sourceScanId, const QString& targetScanId);

    /**
     * @brief Handle generate quality report request (Sprint 6.2)
     */
    void handleGenerateReportClicked();

    /**
     * @brief Start report generation with specified options (Sprint 6.3)
     * @param options Report generation options from ReportOptionsDialog
     * @param dialog Pointer to the dialog for progress updates
     */
    void startReportGeneration(const PDFReportGenerator::ReportOptions& options, ReportOptionsDialog* dialog);

    /**
     * @brief Handle generate performance report request (Sprint 7.3)
     */
    void handleGeneratePerformanceReportClicked();

private slots:
    /**
     * @brief Handle quality assessment completion (Sprint 6.2)
     * @param report The completed quality report
     */
    void onQualityAssessmentCompleted(const QualityReport& report);

    /**
     * @brief Handle successful report generation (Sprint 6.2)
     * @param filePath Path to the generated report
     */
    void onReportGenerated(const QString& filePath);

    /**
     * @brief Handle report generation error (Sprint 6.2)
     * @param error Error message
     */
    void onReportError(const QString& error);

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

    /**
     * @brief Trigger alignment computation preview
     *
     * This slot is connected to AlignmentControlPanel::alignmentRequested() signal.
     * It retrieves correspondences from TargetManager and initiates alignment computation
     * through AlignmentEngine.
     */
    void triggerAlignmentPreview();

    /**
     * @brief Handle alignment result updates from AlignmentEngine
     *
     * This slot receives the complete alignment result and updates both the
     * PointCloudViewerWidget with the dynamic transformation and the
     * AlignmentControlPanel with quality metrics.
     *
     * @param result Complete alignment result with transformation and error statistics
     */
    void handleAlignmentResultUpdated(const AlignmentEngine::AlignmentResult& result);

    /**
     * @brief Handle ICP computation completion
     * @param success True if ICP converged successfully
     * @param finalTransformation Final transformation matrix
     * @param finalRMSError Final RMS error
     * @param iterations Number of iterations performed
     */
    void handleICPCompletion(bool success, const QMatrix4x4& finalTransformation, float finalRMSError, int iterations);

    /**
     * @brief Handle user acceptance of ICP result
     */
    void handleAcceptICPResult();

    /**
     * @brief Handle user discard of ICP result
     */
    void handleDiscardICPResult();

    /**
     * @brief Handle alignment state changes from AlignmentEngine (Sprint 2.3)
     *
     * This slot receives alignment state changes and forwards them to the
     * AlignmentControlPanel for UI updates.
     *
     * @param state New alignment state
     * @param message Status message
     */
    void handleAlignmentStateChanged(AlignmentEngine::AlignmentState state, const QString& message);

private:
         // Interface pointers (not owned by this class)
    IMainView* m_view;
        IE57Parser* m_e57Parser;
        IE57Writer* m_e57Writer;
        IPointCloudViewer* m_viewer;

         // Manager pointers (not owned by this class)
    ProjectManager* m_projectManager;
        PointCloudLoadManager* m_loadManager;
    // Sprint 3.2: Export functionality
    RegistrationProject* m_currentProject;
    TargetManager* m_targetManager;
    AlignmentEngine* m_alignmentEngine;

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
        // Workflow widget integration
        class RegistrationWorkflowWidget* m_connectedWorkflowWidget;

    // Alignment state management
    QString m_currentSourceScanId;
        QString m_currentTargetScanId;

    // Pose Graph Management
    Registration::RegistrationProject* m_registrationProject;
    PoseGraphViewerWidget* m_poseGraphViewer;
    std::unique_ptr<Registration::PoseGraph> m_currentPoseGraph;
    std::unique_ptr<Registration::PoseGraphBuilder> m_poseGraphBuilder;

    // Bundle Adjustment Management
    std::unique_ptr<Optimization::BundleAdjustment> m_bundleAdjustment;
    std::unique_ptr<BundleAdjustmentProgressDialog> m_baProgressDialog;

    // Sprint 6.2: Quality assessment and reporting
    QualityAssessment* m_qualityAssessment;
    PDFReportGenerator* m_reportGenerator;
    QualityReport m_lastQualityReport;

    // Sprint 4.2: ICP Progress monitoring
    ICPProgressWidget* m_icpProgressWidget;

    // Sprint 4.3: ICP Result Management
    QMatrix4x4 m_lastICPTransformation;
    float m_lastICPRMSError;
    int m_lastICPIterations;
    bool m_hasValidICPResult;
};

#endif  // MAINPRESENTER_H
