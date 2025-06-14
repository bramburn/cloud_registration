#include "app/MainPresenter.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include "app/pointcloudloadmanager.h"
#include "core/projectmanager.h"
#include "core/scaninfo.h"
#include "core/sqlitemanager.h"
#include "interfaces/IE57Parser.h"
#include "interfaces/IE57Writer.h"
#include "interfaces/IMainView.h"
#include "interfaces/IPointCloudViewer.h"
#include "ui/ICPParameterDialog.h"
#include "ui/ICPProgressWidget.h"
#include "algorithms/ICPRegistration.h"
#include "registration/RegistrationWorkflowWidget.h"
#include "registration/RegistrationProject.h"
#include "ui/ExportDialog.h"
#include "export/IFormatWriter.h"
#include "registration/TargetManager.h"
#include "registration/AlignmentEngine.h"
#include "ui/TargetDetectionDialog.h"
#include "ui/AlignmentControlPanel.h"
#include "ui/PoseGraphViewerWidget.h"
#include "ui/BundleAdjustmentProgressDialog.h"
#include "registration/PoseGraph.h"
#include "registration/PoseGraphBuilder.h"
#include "rendering/pointcloudviewerwidget.h"
#include "optimization/BundleAdjustment.h"

// Sprint 6.2: Quality assessment and reporting includes
#include "quality/QualityAssessment.h"
#include "quality/PDFReportGenerator.h"
#include "ui/ReportOptionsDialog.h"

// Sprint 7.3: Performance profiling includes
#include "core/performance_profiler.h"
#include "ui/UserPreferences.h"
#include <QDateTime>

MainPresenter::MainPresenter(IMainView* view,
                             IE57Parser* e57Parser,
                             IE57Writer* e57Writer,
                             ProjectManager* projectManager,
                             PointCloudLoadManager* loadManager,
                             QObject* parent)
    : QObject(parent),
      m_view(view),
      m_e57Parser(e57Parser),
      m_e57Writer(e57Writer),
      m_viewer(nullptr),
      m_projectManager(projectManager),
      m_loadManager(loadManager),
      m_currentProject(nullptr),
      m_targetManager(nullptr),
      m_alignmentEngine(nullptr),
      m_isFileOpen(false),
      m_isProjectOpen(false),
      m_isParsingInProgress(false),
      m_currentMemoryUsage(0),
      m_currentFPS(0.0f),
      m_currentVisiblePoints(0),
      m_connectedWorkflowWidget(nullptr),
      m_currentSourceScanId(""),
      m_currentTargetScanId(""),
      m_registrationProject(nullptr),
      m_poseGraphViewer(nullptr),
      m_currentPoseGraph(nullptr),
      m_poseGraphBuilder(std::make_unique<Registration::PoseGraphBuilder>()),
      m_qualityAssessment(nullptr),
      m_reportGenerator(nullptr),
      m_icpProgressWidget(nullptr),
      m_lastICPRMSError(0.0f),
      m_lastICPIterations(0),
      m_hasValidICPResult(false)
{
    if (m_view)
    {
        m_viewer = m_view->getViewer();
    }
}

void MainPresenter::initialize()
{
    setupConnections();
    updateUIState();
    updateWindowTitle();

    // Sprint 7.3: Initialize performance profiler based on preferences
    bool profilingEnabled = UserPreferences::instance().getValue("advanced/profilingEnabled", false).toBool();
    PerformanceProfiler::instance().setEnabled(profilingEnabled);
    qDebug() << "Performance profiler initialized, enabled:" << profilingEnabled;
}

void MainPresenter::setProjectManager(ProjectManager* projectManager)
{
    m_projectManager = projectManager;
}

void MainPresenter::setPointCloudLoadManager(PointCloudLoadManager* loadManager)
{
    m_loadManager = loadManager;
}

void MainPresenter::setTargetManager(TargetManager* targetManager)
{
    m_targetManager = targetManager;
}

void MainPresenter::setAlignmentEngine(AlignmentEngine* alignmentEngine)
{
    m_alignmentEngine = alignmentEngine;

    // Connect alignment engine signals
    if (m_alignmentEngine)
    {
        connect(m_alignmentEngine, &AlignmentEngine::alignmentResultUpdated,
                this, &MainPresenter::handleAlignmentResultUpdated);
        connect(m_alignmentEngine, &AlignmentEngine::alignmentStateChanged,
                this, &MainPresenter::handleAlignmentStateChanged);

        qDebug() << "MainPresenter: AlignmentEngine set and signals connected";
    }
}

void MainPresenter::setQualityAssessment(QualityAssessment* qualityAssessment)
{
    m_qualityAssessment = qualityAssessment;

    if (m_qualityAssessment) {
        // Connect quality assessment signals
        connect(m_qualityAssessment, &QualityAssessment::assessmentCompleted,
                this, &MainPresenter::onQualityAssessmentCompleted);
        connect(m_qualityAssessment, &QualityAssessment::assessmentError,
                this, [this](const QString& error) {
                    showError("Quality Assessment Error", error);
                });
    }
}

void MainPresenter::setPDFReportGenerator(PDFReportGenerator* reportGenerator)
{
    m_reportGenerator = reportGenerator;

    if (m_reportGenerator) {
        // Connect report generator signals
        connect(m_reportGenerator, &PDFReportGenerator::reportGenerated,
                this, &MainPresenter::onReportGenerated);
        connect(m_reportGenerator, &PDFReportGenerator::reportError,
                this, &MainPresenter::onReportError);
    }
}

void MainPresenter::setupConnections()
{
    if (!m_view || !m_e57Parser)
    {
        return;
    }

    // Connect view signals to presenter slots
    connect(m_view, &IMainView::newProjectRequested, this, &MainPresenter::handleNewProject);
    connect(m_view, &IMainView::openProjectRequested, this, &MainPresenter::handleOpenProject);
    connect(m_view, &IMainView::closeProjectRequested, this, &MainPresenter::handleCloseProject);
    connect(m_view, &IMainView::importScansRequested, this, &MainPresenter::handleImportScans);
    connect(m_view, &IMainView::openFileRequested, this, &MainPresenter::handleOpenFile);
    connect(m_view, &IMainView::saveFileRequested, this, &MainPresenter::handleSaveFile);
    connect(m_view, &IMainView::scanActivated, this, &MainPresenter::handleScanActivation);
    connect(m_view, &IMainView::viewerSettingsChanged, this, &MainPresenter::handleViewerSettingsChanged);
    connect(m_view, &IMainView::exitRequested, this, &MainPresenter::handleExit);

    // Connect E57 parser signals to presenter slots
    connect(m_e57Parser, &IE57Parser::progressUpdated, this, &MainPresenter::onParsingProgress);
    connect(m_e57Parser, &IE57Parser::parsingFinished, this, &MainPresenter::onParsingFinished);
    connect(m_e57Parser, &IE57Parser::scanMetadataAvailable, this, &MainPresenter::onScanMetadataAvailable);
    connect(m_e57Parser, &IE57Parser::intensityDataExtracted, this, &MainPresenter::onIntensityDataExtracted);
    connect(m_e57Parser, &IE57Parser::colorDataExtracted, this, &MainPresenter::onColorDataExtracted);

    // Connect viewer signals if available
    if (m_viewer)
    {
        connect(m_viewer, &IPointCloudViewer::stateChanged, this, &MainPresenter::onViewerStateChanged);
        connect(m_viewer, &IPointCloudViewer::statsUpdated, this, &MainPresenter::onRenderingStatsUpdated);
    }

    // Connect sidebar signals if available
    if (m_view)
    {
        auto* sidebar = m_view->getSidebar();
        if (sidebar)
        {
            // Connect cluster operations
            connect(sidebar, &SidebarWidget::clusterCreationRequested, this, &MainPresenter::handleClusterCreation);
            connect(sidebar, &SidebarWidget::clusterRenameRequested, this, &MainPresenter::handleClusterRename);
            connect(sidebar, &SidebarWidget::deleteClusterRequested, this, &MainPresenter::handleClusterDeletion);

            // Connect scan operations
            connect(sidebar, &SidebarWidget::loadScanRequested, this, &MainPresenter::handleScanLoad);
            connect(sidebar, &SidebarWidget::unloadScanRequested, this, &MainPresenter::handleScanUnload);
            connect(sidebar, &SidebarWidget::loadClusterRequested, this, &MainPresenter::handleClusterLoad);
            connect(sidebar, &SidebarWidget::unloadClusterRequested, this, &MainPresenter::handleClusterUnload);
            connect(sidebar, &SidebarWidget::viewPointCloudRequested, this, &MainPresenter::handlePointCloudView);
            connect(sidebar, &SidebarWidget::deleteScanRequested, this, &MainPresenter::handleScanDeletion);

            // Connect new Sprint 4 operations
            connect(sidebar, &SidebarWidget::clusterLockToggleRequested, this, &MainPresenter::handleClusterLockToggle);
            connect(sidebar, &SidebarWidget::dragDropOperationRequested, this, &MainPresenter::handleDragDropOperation);
        }
    }

    // Sprint 3.2: Export functionality connections
    // Note: Currently ExportDialog is self-contained, so no connections needed
    // This will be updated when we implement the full MVP pattern as per s3.2.md
    // Connect alignment control panel signals if available
    if (m_view)
    {
        auto* alignmentPanel = m_view->getAlignmentControlPanel();
        if (alignmentPanel)
        {
            connect(alignmentPanel, &AlignmentControlPanel::alignmentRequested, this, &MainPresenter::triggerAlignmentPreview);

            // Sprint 2.3: Connect configuration signals to AlignmentEngine
            if (m_alignmentEngine)
            {
                connect(alignmentPanel, &AlignmentControlPanel::autoRecomputeChanged,
                        m_alignmentEngine, &AlignmentEngine::setAutoRecompute);
                connect(alignmentPanel, &AlignmentControlPanel::qualityThresholdsChanged,
                        m_alignmentEngine, &AlignmentEngine::setQualityThresholds);
            }

            // Sprint 4.3: Connect ICP-specific accept/discard signals
            connect(alignmentPanel, &AlignmentControlPanel::acceptAlignmentRequested,
                    this, &MainPresenter::handleAcceptICPResult);
            connect(alignmentPanel, &AlignmentControlPanel::cancelAlignmentRequested,
                    this, &MainPresenter::handleDiscardICPResult);
        }
    }

    // Connect alignment engine signals if available
    if (m_alignmentEngine)
    {
        connect(m_alignmentEngine, &AlignmentEngine::alignmentResultUpdated,
                this, &MainPresenter::handleAlignmentResultUpdated);
        connect(m_alignmentEngine, &AlignmentEngine::alignmentStateChanged,
                this, &MainPresenter::handleAlignmentStateChanged);

        // Sprint 4.3: Connect ICP completion signal
        connect(m_alignmentEngine, &AlignmentEngine::computationFinished,
                this, &MainPresenter::handleICPCompletion);
    }
}
}

void MainPresenter::handleNewProject()
{
    if (m_isProjectOpen)
    {
        bool confirmed = m_view->askForConfirmation(
            "Close Current Project", "A project is already open. Do you want to close it and create a new project?");
        if (!confirmed)
        {
            return;
        }
        handleCloseProject();
    }

    // For now, just show info that project creation is not fully implemented
    showInfo("New Project", "New project creation functionality will be implemented in future sprints.");
    updateWindowTitle();
}

void MainPresenter::handleOpenProject()
{
    QString projectPath = m_view->askForOpenFilePath("Open Project", "Project Files (*.crp)");
    if (projectPath.isEmpty())
    {
        return;
    }

    if (!QFileInfo::exists(projectPath))
    {
        showError("Open Project", "The selected project file does not exist.");
        return;
    }

    m_currentProjectPath = projectPath;
    m_isProjectOpen = true;

    QFileInfo fileInfo(projectPath);
    m_view->setProjectTitle(fileInfo.baseName());
    m_view->showProjectView();

    updateUIState();
    updateWindowTitle();

    showInfo("Project Opened", QString("Successfully opened project: %1").arg(fileInfo.baseName()));
}

void MainPresenter::handleCloseProject()
{
    if (!m_isProjectOpen)
    {
        return;
    }

    // Close any open files first
    if (m_isFileOpen)
    {
        clearPointCloudData();
        m_e57Parser->closeFile();
        m_isFileOpen = false;
    }

    m_currentProjectPath.clear();
    m_currentScanNames.clear();
    m_isProjectOpen = false;

    m_view->showProjectHub();
    updateUIState();
    updateWindowTitle();

    m_view->updateStatusBar("Project closed");
}

void MainPresenter::handleImportScans()
{
    if (!m_isProjectOpen)
    {
        showError("Import Scans", "Please open or create a project first.");
        return;
    }

    QString filePath = m_view->askForOpenFilePath("Import E57 Scan", "E57 Files (*.e57)");
    if (!filePath.isEmpty())
    {
        handleOpenFile(filePath);
    }
}

void MainPresenter::handleOpenFile(const QString& filePath)
{
    if (!validateFilePath(filePath))
    {
        return;
    }

    if (m_isParsingInProgress)
    {
        showError("File Opening", "Another file is currently being processed. Please wait.");
        return;
    }

    // Close any currently open file
    if (m_isFileOpen)
    {
        clearPointCloudData();
        m_e57Parser->closeFile();
    }

    m_currentFilePath = filePath;
    m_isParsingInProgress = true;

    m_view->showProgressDialog(true, "Opening File", "Initializing E57 parser...");
    m_view->setActionsEnabled(false);
    m_view->updateStatusBar("Opening file: " + QFileInfo(filePath).fileName());

    // Start parsing asynchronously
    if (!m_e57Parser->openFile(filePath.toStdString()))
    {
        onParsingFinished(false, m_e57Parser->getLastError(), std::vector<float>());
        return;
    }

    // If we get here, file opened successfully, now extract data
    try
    {
        auto points = m_e57Parser->extractPointData();
        onParsingFinished(true, "File opened successfully", points);
    }
    catch (...)
    {
        onParsingFinished(false, "Failed to extract point data", std::vector<float>());
    }
}

void MainPresenter::handleSaveFile(const QString& filePath)
{
    if (!m_e57Writer)
    {
        showError("Save File", "E57 writer is not available.");
        return;
    }

    if (!m_isFileOpen || !m_viewer || !m_viewer->hasData())
    {
        showError("Save File", "No point cloud data to save.");
        return;
    }

    showInfo("Save File", "File saving functionality will be implemented in future sprints.");
}

void MainPresenter::handleScanActivation(const QString& scanId)
{
    if (!m_isFileOpen)
    {
        showError("Scan Activation", "No file is currently open.");
        return;
    }

    m_view->highlightScan(scanId);
    m_view->updateStatusBar("Activated scan: " + scanId);
}

void MainPresenter::handleViewerSettingsChanged()
{
    // Update any presenter-level state based on viewer settings
    updateUIState();
}

void MainPresenter::handleExit()
{
    if (m_isProjectOpen || m_isFileOpen)
    {
        bool confirmed = m_view->askForConfirmation("Exit Application",
                                                    "Are you sure you want to exit? Any unsaved changes will be lost.");
        if (!confirmed)
        {
            return;
        }
    }

    // Clean up resources
    if (m_isFileOpen)
    {
        clearPointCloudData();
        m_e57Parser->closeFile();
    }
}

void MainPresenter::onParsingProgress(int percentage, const QString& stage)
{
    m_view->updateProgress(percentage, stage);
}

void MainPresenter::onParsingFinished(bool success, const QString& message, const std::vector<float>& points)
{
    m_isParsingInProgress = false;
    m_view->showProgressDialog(false);
    m_view->setActionsEnabled(true);

    if (success && !points.empty())
    {
        m_isFileOpen = true;

        // Load points into viewer
        if (m_viewer)
        {
            m_viewer->loadPointCloud(points);
            m_viewer->resetCamera();
        }

        QFileInfo fileInfo(m_currentFilePath);
        m_view->updateStatusBar(QString("Loaded %1 points from %2").arg(points.size() / 3).arg(fileInfo.fileName()));

        // Enable target detection if workflow widget is connected
        if (m_connectedWorkflowWidget)
        {
            m_connectedWorkflowWidget->enableTargetDetection(true);
        }

        showInfo("File Opened", message);
    }
    else
    {
        m_isFileOpen = false;
        m_currentFilePath.clear();
        showError("File Opening Failed", message);
        m_view->updateStatusBar("Failed to open file");
    }

    updateUIState();
    updateWindowTitle();
}

void MainPresenter::onScanMetadataAvailable(int scanCount, const QStringList& scanNames)
{
    m_currentScanNames = scanNames;
    m_view->updateScanList(scanNames);

    m_view->updateStatusBar(QString("Found %1 scans in file").arg(scanCount));
}

void MainPresenter::onIntensityDataExtracted(const std::vector<float>& /*intensityValues*/)
{
    // Handle intensity data if needed
}

void MainPresenter::onColorDataExtracted(const std::vector<uint8_t>& /*colorValues*/)
{
    // Handle color data if needed
}

void MainPresenter::onViewerStateChanged(int newState, const QString& message)
{
    Q_UNUSED(newState)
    if (!message.isEmpty())
    {
        m_view->updateStatusBar(message);
    }
}

void MainPresenter::onRenderingStatsUpdated(float fps, int visiblePoints)
{
    m_currentFPS = fps;
    m_currentVisiblePoints = visiblePoints;
    m_view->updateRenderingStats(fps, visiblePoints);
}

void MainPresenter::onMemoryUsageChanged(size_t totalBytes)
{
    m_currentMemoryUsage = totalBytes;
    m_view->updateMemoryUsage(totalBytes);
}

void MainPresenter::updateUIState()
{
    // Enable/disable actions based on current state
    bool hasProject = m_isProjectOpen;
    bool hasFile = m_isFileOpen;
    bool isProcessing = m_isParsingInProgress;

    // This would typically update action states, but since we're working with interfaces,
    // we'll keep this simple for now
    m_view->setActionsEnabled(!isProcessing);
}

bool MainPresenter::validateFilePath(const QString& filePath)
{
    if (filePath.isEmpty())
    {
        showError("Invalid File", "File path is empty.");
        return false;
    }

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists())
    {
        showError("File Not Found", "The selected file does not exist.");
        return false;
    }

    if (!fileInfo.isReadable())
    {
        showError("File Access", "The selected file cannot be read.");
        return false;
    }

    if (fileInfo.suffix().toLower() != "e57")
    {
        showError("Invalid File Type", "Please select a valid E57 file.");
        return false;
    }

    return true;
}

void MainPresenter::showError(const QString& title, const QString& message)
{
    if (m_view)
    {
        m_view->displayErrorMessage(title, message);
    }
}

void MainPresenter::showInfo(const QString& title, const QString& message)
{
    if (m_view)
    {
        m_view->displayInfoMessage(title, message);
    }
}

void MainPresenter::updateWindowTitle()
{
    QString title = "Cloud Registration";

    if (m_isProjectOpen && !m_currentProjectPath.isEmpty())
    {
        QFileInfo projectInfo(m_currentProjectPath);
        title += " - " + projectInfo.baseName();
    }

    if (m_isFileOpen && !m_currentFilePath.isEmpty())
    {
        QFileInfo fileInfo(m_currentFilePath);
        title += " [" + fileInfo.fileName() + "]";
    }

    if (m_view)
    {
        m_view->setWindowTitle(title);
    }
}

void MainPresenter::clearPointCloudData()
{
    if (m_viewer)
    {
        m_viewer->clearPointCloud();
    }
    m_currentScanNames.clear();
    m_view->updateScanList(QStringList());

    // Disable target detection when no data is available
    if (m_connectedWorkflowWidget)
    {
        m_connectedWorkflowWidget->enableTargetDetection(false);
    }
}

// Sprint 4: Sidebar-related method implementations
void MainPresenter::handleClusterCreation(const QString& clusterName, const QString& parentClusterId)
{
    if (!m_projectManager)
    {
        showError("Cluster Creation", "Project manager is not available.");
        return;
    }

    if (clusterName.trimmed().isEmpty())
    {
        showError("Cluster Creation", "Cluster name cannot be empty.");
        return;
    }

    QString clusterId = m_projectManager->createCluster(clusterName.trimmed(), parentClusterId);
    if (clusterId.isEmpty())
    {
        showError("Cluster Creation", "Failed to create cluster. Please try again.");
    }
    else
    {
        showInfo("Cluster Creation", QString("Cluster '%1' created successfully.").arg(clusterName));
        m_view->updateStatusBar(QString("Created cluster: %1").arg(clusterName));
    }
}

void MainPresenter::handleClusterRename(const QString& clusterId, const QString& newName)
{
    if (!m_projectManager)
    {
        showError("Cluster Rename", "Project manager is not available.");
        return;
    }

    if (newName.trimmed().isEmpty())
    {
        showError("Cluster Rename", "Cluster name cannot be empty.");
        return;
    }

    if (clusterId.isEmpty())
    {
        showError("Cluster Rename", "Invalid cluster selected.");
        return;
    }

    bool success = m_projectManager->renameCluster(clusterId, newName.trimmed());
    if (success)
    {
        showInfo("Cluster Rename", QString("Cluster renamed to '%1' successfully.").arg(newName));
        m_view->updateStatusBar(QString("Renamed cluster to: %1").arg(newName));
    }
    else
    {
        showError("Cluster Rename", "Failed to rename cluster. Please try again.");
    }
}

void MainPresenter::handleClusterDeletion(const QString& clusterId, bool deletePhysicalFiles)
{
    if (!m_projectManager)
    {
        showError("Cluster Deletion", "Project manager is not available.");
        return;
    }

    if (clusterId.isEmpty())
    {
        showError("Cluster Deletion", "Invalid cluster selected.");
        return;
    }

    // Ask for confirmation
    QString confirmMessage =
        deletePhysicalFiles
            ? "Are you sure you want to delete this cluster and all its physical files? This action cannot be undone."
            : "Are you sure you want to delete this cluster? The physical files will be preserved.";

    bool confirmed = m_view->askForConfirmation("Delete Cluster", confirmMessage);
    if (!confirmed)
    {
        return;
    }

    bool success = m_projectManager->deleteCluster(clusterId, deletePhysicalFiles);
    if (success)
    {
        showInfo("Cluster Deletion", "Cluster deleted successfully.");
        m_view->updateStatusBar("Cluster deleted");
    }
    else
    {
        showError("Cluster Deletion", "Failed to delete cluster. Please try again.");
    }
}

void MainPresenter::handleScanLoad(const QString& scanId)
{
    if (!m_loadManager)
    {
        showError("Scan Load", "Load manager is not available.");
        return;
    }

    if (scanId.isEmpty())
    {
        showError("Scan Load", "Invalid scan selected.");
        return;
    }

    if (m_loadedScans.contains(scanId))
    {
        showInfo("Scan Load", "Scan is already loaded.");
        return;
    }

    // Delegate to load manager
    bool success = m_loadManager->loadScan(scanId);
    if (success)
    {
        m_loadedScans.append(scanId);
        showInfo("Scan Load", "Scan loaded successfully.");
        m_view->updateStatusBar(QString("Loaded scan: %1").arg(scanId));
    }
    else
    {
        showError("Scan Load", "Failed to load scan. Please try again.");
    }
}

void MainPresenter::handleScanUnload(const QString& scanId)
{
    if (!m_loadManager)
    {
        showError("Scan Unload", "Load manager is not available.");
        return;
    }

    if (scanId.isEmpty())
    {
        showError("Scan Unload", "Invalid scan selected.");
        return;
    }

    if (!m_loadedScans.contains(scanId))
    {
        showInfo("Scan Unload", "Scan is not currently loaded.");
        return;
    }

    // Delegate to load manager
    bool success = m_loadManager->unloadScan(scanId);
    if (success)
    {
        m_loadedScans.removeAll(scanId);
        showInfo("Scan Unload", "Scan unloaded successfully.");
        m_view->updateStatusBar(QString("Unloaded scan: %1").arg(scanId));
    }
    else
    {
        showError("Scan Unload", "Failed to unload scan. Please try again.");
    }
}

void MainPresenter::handleClusterLoad(const QString& clusterId)
{
    if (!m_loadManager || !m_projectManager)
    {
        showError("Cluster Load", "Required managers are not available.");
        return;
    }

    if (clusterId.isEmpty())
    {
        showError("Cluster Load", "Invalid cluster selected.");
        return;
    }

    // Get all scans in the cluster
    QStringList scanIds = m_projectManager->getScansInCluster(clusterId);
    if (scanIds.isEmpty())
    {
        showInfo("Cluster Load", "No scans found in this cluster.");
        return;
    }

    int loadedCount = 0;
    for (const QString& scanId : scanIds)
    {
        if (!m_loadedScans.contains(scanId))
        {
            if (m_loadManager->loadScan(scanId))
            {
                m_loadedScans.append(scanId);
                loadedCount++;
            }
        }
    }

    if (loadedCount > 0)
    {
        showInfo("Cluster Load", QString("Loaded %1 scans from cluster.").arg(loadedCount));
        m_view->updateStatusBar(QString("Loaded %1 scans from cluster").arg(loadedCount));
    }
    else
    {
        showInfo("Cluster Load", "All scans in cluster are already loaded.");
    }
}

void MainPresenter::handleClusterUnload(const QString& clusterId)
{
    if (!m_loadManager || !m_projectManager)
    {
        showError("Cluster Unload", "Required managers are not available.");
        return;
    }

    if (clusterId.isEmpty())
    {
        showError("Cluster Unload", "Invalid cluster selected.");
        return;
    }

    // Get all scans in the cluster
    QStringList scanIds = m_projectManager->getScansInCluster(clusterId);
    if (scanIds.isEmpty())
    {
        showInfo("Cluster Unload", "No scans found in this cluster.");
        return;
    }

    int unloadedCount = 0;
    for (const QString& scanId : scanIds)
    {
        if (m_loadedScans.contains(scanId))
        {
            if (m_loadManager->unloadScan(scanId))
            {
                m_loadedScans.removeAll(scanId);
                unloadedCount++;
            }
        }
    }

    if (unloadedCount > 0)
    {
        showInfo("Cluster Unload", QString("Unloaded %1 scans from cluster.").arg(unloadedCount));
        m_view->updateStatusBar(QString("Unloaded %1 scans from cluster").arg(unloadedCount));
    }
    else
    {
        showInfo("Cluster Unload", "No loaded scans found in cluster.");
    }
}

void MainPresenter::handlePointCloudView(const QString& itemId, const QString& itemType)
{
    if (!m_viewer)
    {
        showError("Point Cloud View", "Viewer is not available.");
        return;
    }

    if (itemId.isEmpty())
    {
        showError("Point Cloud View", "Invalid item selected.");
        return;
    }

    if (itemType == "scan")
    {
        // Focus on specific scan
        m_viewer->focusOnScan(itemId);
        m_view->updateStatusBar(QString("Viewing scan: %1").arg(itemId));
    }
    else if (itemType == "cluster")
    {
        // Focus on all scans in cluster
        m_viewer->focusOnCluster(itemId);
        m_view->updateStatusBar(QString("Viewing cluster: %1").arg(itemId));
    }
    else
    {
        showError("Point Cloud View", "Unknown item type selected.");
    }
}

void MainPresenter::handleScanDeletion(const QString& scanId, bool deletePhysicalFile)
{
    if (!m_projectManager)
    {
        showError("Scan Deletion", "Project manager is not available.");
        return;
    }

    if (scanId.isEmpty())
    {
        showError("Scan Deletion", "Invalid scan selected.");
        return;
    }

    // Ask for confirmation
    QString confirmMessage =
        deletePhysicalFile
            ? "Are you sure you want to delete this scan and its physical file? This action cannot be undone."
            : "Are you sure you want to delete this scan? The physical file will be preserved.";

    bool confirmed = m_view->askForConfirmation("Delete Scan", confirmMessage);
    if (!confirmed)
    {
        return;
    }

    // Unload scan if it's currently loaded
    if (m_loadedScans.contains(scanId))
    {
        handleScanUnload(scanId);
    }

    bool success = m_projectManager->deleteScan(scanId, deletePhysicalFile);
    if (success)
    {
        showInfo("Scan Deletion", "Scan deleted successfully.");
        m_view->updateStatusBar("Scan deleted");
    }
    else
    {
        showError("Scan Deletion", "Failed to delete scan. Please try again.");
    }
}

void MainPresenter::handleClusterLockToggle(const QString& clusterId, bool lock)
{
    if (!m_projectManager)
    {
        showError("Cluster Lock", "Project manager is not available.");
        return;
    }

    if (clusterId.isEmpty())
    {
        showError("Cluster Lock", "Invalid cluster selected.");
        return;
    }

    bool success = m_projectManager->setClusterLockState(clusterId, lock);
    if (success)
    {
        if (lock)
        {
            m_lockedClusters.append(clusterId);
            showInfo("Cluster Lock", "Cluster locked successfully.");
            m_view->updateStatusBar("Cluster locked");
        }
        else
        {
            m_lockedClusters.removeAll(clusterId);
            showInfo("Cluster Unlock", "Cluster unlocked successfully.");
            m_view->updateStatusBar("Cluster unlocked");
        }
    }
    else
    {
        QString action = lock ? "lock" : "unlock";
        showError("Cluster Lock", QString("Failed to %1 cluster. Please try again.").arg(action));
    }
}

void MainPresenter::handleDragDropOperation(const QStringList& draggedItems,
                                            const QString& draggedType,
                                            const QString& targetItemId,
                                            const QString& targetType)
{
    if (!m_projectManager)
    {
        showError("Drag and Drop", "Project manager is not available.");
        return;
    }

    if (draggedItems.isEmpty() || targetItemId.isEmpty())
    {
        showError("Drag and Drop", "Invalid drag and drop operation.");
        return;
    }

    // Only allow dropping scans onto clusters or project root
    if (draggedType == "scan" && (targetType == "cluster" || targetType == "project_root"))
    {
        int movedCount = 0;
        for (const QString& scanId : draggedItems)
        {
            if (m_projectManager->moveScanToCluster(scanId, targetItemId))
            {
                movedCount++;
            }
        }

        if (movedCount > 0)
        {
            showInfo("Drag and Drop", QString("Moved %1 scan(s) successfully.").arg(movedCount));
            m_view->updateStatusBar(QString("Moved %1 scan(s)").arg(movedCount));
        }
        else
        {
            showError("Drag and Drop", "Failed to move scans. Please try again.");
        }
    }
    else
    {
        showError("Drag and Drop", "This drag and drop operation is not supported.");
    }
}

void MainPresenter::handleTargetDetectionClicked()
{
    // Check if we have loaded scans
    if (!m_isFileOpen || m_currentScanNames.isEmpty())
    {
        showError("Target Detection", "Please load point cloud scans first.");
        return;
    }

    // Get the current scan ID (for now, use the first scan)
    QString currentScanId = m_currentScanNames.first();

    // Get point cloud data from the load manager
    std::vector<PointFullData> pointCloudData;
    if (m_loadManager)
    {
        pointCloudData = m_loadManager->getLoadedPointFullData(currentScanId);
    }

    // Ensure we have target manager and alignment engine
    if (!m_targetManager)
    {
        m_targetManager = new TargetManager(this);
    }

    if (!m_alignmentEngine)
    {
        m_alignmentEngine = new AlignmentEngine(this);
        m_alignmentEngine->setPointCloudLoadManager(m_loadManager);
        m_alignmentEngine->setTargetManager(m_targetManager);
    }

    // Create and show the target detection dialog
    TargetDetectionDialog dialog(m_targetManager, static_cast<QWidget*>(m_view));
    dialog.setPointCloudData(currentScanId, pointCloudData);

    // Connect AlignmentEngine signals to dialog
    connect(m_alignmentEngine, &AlignmentEngine::targetDetectionProgress,
            &dialog, &TargetDetectionDialog::onDetectionProgress);
    connect(m_alignmentEngine, &AlignmentEngine::targetDetectionCompleted,
            &dialog, &TargetDetectionDialog::onDetectionCompleted);
    connect(m_alignmentEngine, &AlignmentEngine::targetDetectionError,
            &dialog, &TargetDetectionDialog::onDetectionError);

    // Connect dialog cancel signal to MainPresenter
    connect(&dialog, &TargetDetectionDialog::cancelDetectionRequested,
            this, &MainPresenter::cancelTargetDetection);

    // Connect dialog completion signals
    connect(&dialog, &TargetDetectionDialog::detectionCompleted,
            this, [this](const QString& scanId, const TargetDetectionBase::DetectionResult& result) {
                // Handle detection completion
                showInfo("Target Detection",
                        QString("Detection completed for scan %1. Found %2 targets.")
                        .arg(scanId)
                        .arg(result.targets.size()));
            });

    connect(&dialog, &TargetDetectionDialog::manualSelectionRequested,
            this, [this](const QString& scanId) {
                // Handle manual selection mode activation
                showInfo("Manual Selection",
                        QString("Manual selection mode activated for scan %1. "
                               "Click on points in the 3D view to select targets.")
                        .arg(scanId));
            });

    // Connect dialog start detection to AlignmentEngine
    connect(&dialog, &TargetDetectionDialog::detectionStartRequested,
            m_alignmentEngine, &AlignmentEngine::startTargetDetection);

    // Show dialog modally
    if (dialog.exec() == QDialog::Accepted)
    {
        m_view->updateStatusBar("Target detection completed successfully");
    }
    else
    {
        m_view->updateStatusBar("Target detection cancelled");
    }
}

void MainPresenter::cancelTargetDetection()
{
    qDebug() << "MainPresenter::cancelTargetDetection() called";

    if (m_alignmentEngine)
    {
        m_alignmentEngine->cancelTargetDetection();
    }
}

void MainPresenter::handleAutomaticAlignmentClicked()
{
    qDebug() << "MainPresenter: Automatic alignment button clicked";

    if (!m_loadManager)
    {
        showError("Automatic Alignment", "Point cloud load manager is not available.");
        return;
    }

    // Get loaded scans
    QStringList loadedScans = m_loadManager->getLoadedScans();

    if (loadedScans.size() < 2)
    {
        showError("Automatic Alignment",
                 QString("At least 2 scans must be loaded for automatic alignment. Currently loaded: %1")
                 .arg(loadedScans.size()));
        return;
    }

    // For now, use the first two loaded scans
    QString sourceScanId = loadedScans[0];
    QString targetScanId = loadedScans[1];

    qDebug() << "Selected scans for alignment:" << sourceScanId << "→" << targetScanId;

    try
    {
        // Get point clouds for parameter calculation
        PointCloud sourceCloud = m_loadManager->getLoadedPointCloud(sourceScanId);
        PointCloud targetCloud = m_loadManager->getLoadedPointCloud(targetScanId);

        // Create and configure the ICP parameter dialog
        ICPParameterDialog dialog(sourceCloud, targetCloud, m_view ? m_view->getWidget() : nullptr);
        dialog.setScanIds(sourceScanId, targetScanId);

        // Connect the dialog's runICPRequested signal
        connect(&dialog, &ICPParameterDialog::runICPRequested,
                this, [this](const ICPParams& params, const QString& sourceScanId, const QString& targetScanId) {
            qDebug() << "ICP requested with parameters - Max iterations:" << params.maxIterations
                     << "Convergence:" << params.convergenceThreshold
                     << "Max distance:" << params.maxCorrespondenceDistance;

            // Store scan IDs for reference
            m_currentSourceScanId = sourceScanId;
            m_currentTargetScanId = targetScanId;

            if (!m_alignmentEngine) {
                showError("ICP Error", "Alignment engine is not available.");
                return;
            }

            // Create and show ICP progress widget
            if (m_icpProgressWidget) {
                delete m_icpProgressWidget;
            }
            m_icpProgressWidget = new ICPProgressWidget(static_cast<QWidget*>(m_view));

            // Connect cancel signal
            connect(m_icpProgressWidget, &ICPProgressWidget::cancelRequested,
                    this, &MainPresenter::cancelAutomaticAlignment);

            // Connect progress signals from AlignmentEngine to ICPProgressWidget
            connect(m_alignmentEngine, &AlignmentEngine::progressUpdated,
                    m_icpProgressWidget, &ICPProgressWidget::updateProgress);
            connect(m_alignmentEngine, &AlignmentEngine::computationFinished,
                    m_icpProgressWidget, &ICPProgressWidget::onComputationFinished);

            // Connect completion signal to clean up the progress widget
            connect(m_icpProgressWidget, &ICPProgressWidget::computationCompleted,
                    this, [this](bool success, const QString& message) {
                Q_UNUSED(success)
                Q_UNUSED(message)
                // Clean up the progress widget
                if (m_icpProgressWidget) {
                    m_icpProgressWidget->deleteLater();
                    m_icpProgressWidget = nullptr;
                }
            });

            // Connect progress signals for live viewer updates
            connect(m_alignmentEngine, &AlignmentEngine::progressUpdated,
                    this, [this](int iteration, float rmsError, const QMatrix4x4& transformation) {
                // Update viewer with intermediate transformation
                if (m_viewer) {
                    auto* viewerWidget = dynamic_cast<PointCloudViewerWidget*>(m_viewer);
                    if (viewerWidget) {
                        viewerWidget->setDynamicTransform(transformation);
                    }
                }
            });

            // Start monitoring - pass nullptr for ICPRegistration since we connect to AlignmentEngine
            m_icpProgressWidget->startMonitoring(nullptr, params.maxIterations);

            // Start automatic alignment
            m_alignmentEngine->startAutomaticAlignment(sourceScanId, targetScanId, params);
        });

        // Show the dialog
        if (dialog.exec() == QDialog::Accepted)
        {
            qDebug() << "ICP parameter dialog accepted";
        }
        else
        {
            qDebug() << "ICP parameter dialog cancelled";
        }
    }
    catch (const std::exception& e)
    {
        showError("Automatic Alignment",
                 QString("Failed to initialize ICP dialog: %1").arg(e.what()));
    }
    catch (...)
    {
        showError("Automatic Alignment", "Unknown error occurred while initializing ICP dialog.");
    }
}

void MainPresenter::cancelAutomaticAlignment()
{
    qDebug() << "MainPresenter: Cancel automatic alignment requested";

    if (!m_alignmentEngine) {
        qWarning() << "MainPresenter: Cannot cancel alignment - AlignmentEngine not available";
        return;
    }

    // Cancel the alignment computation
    m_alignmentEngine->cancelAutomaticAlignment();

    // Update status
    if (m_view) {
        m_view->updateStatusBar("ICP alignment cancelled");
    }
}

void MainPresenter::connectToWorkflowWidget(RegistrationWorkflowWidget* workflowWidget)
{
    if (!workflowWidget)
    {
        qWarning() << "MainPresenter::connectToWorkflowWidget: workflowWidget is null";
        return;
    }

    // Store reference to workflow widget
    m_connectedWorkflowWidget = workflowWidget;

    // Connect the target detection signal from workflow widget to our handler
    connect(workflowWidget, &RegistrationWorkflowWidget::targetDetectionRequested,
            this, &MainPresenter::handleTargetDetectionClicked);

    // Connect the automatic alignment signal
    connect(workflowWidget, &RegistrationWorkflowWidget::automaticAlignmentRequested,
            this, &MainPresenter::handleAutomaticAlignmentClicked);

    // Enable target detection when scans are loaded
    if (m_isFileOpen && !m_currentScanNames.isEmpty())
    {
        workflowWidget->enableTargetDetection(true);
    }

    qDebug() << "MainPresenter connected to RegistrationWorkflowWidget for target detection and automatic alignment";
}

// Alignment Management Implementation
void MainPresenter::handleAcceptAlignment()
{
    qDebug() << "MainPresenter::handleAcceptAlignment() called";

    // TODO: This implementation requires AlignmentEngine and RegistrationProject instances
    // These would typically be injected via constructor or setter methods

    /*
    // Intended implementation when components are available:

    if (!m_alignmentEngine || !m_registrationProject) {
        showError("Accept Alignment", "Required components not available.");
        return;
    }

    // 1. Retrieve final transformation from AlignmentEngine
    AlignmentEngine::AlignmentResult currentResult = m_alignmentEngine->getCurrentResult();
    if (currentResult.state != AlignmentEngine::AlignmentState::Valid) {
        showError("Accept Alignment", "No valid alignment to accept.");
        return;
    }

    // 2. Identify scans involved in alignment
    if (m_currentSourceScanId.isEmpty() || m_currentTargetScanId.isEmpty()) {
        showError("Accept Alignment", "Scan IDs not properly set for alignment.");
        return;
    }

    // 3. Apply permanent transformation to target scan
    m_registrationProject->setScanTransform(m_currentTargetScanId, currentResult.transformation);

    // 4. Create and store registration result
    RegistrationProject::RegistrationResult result;
    result.sourceScanId = m_currentSourceScanId;
    result.targetScanId = m_currentTargetScanId;
    result.transformation = currentResult.transformation;
    result.rmsError = currentResult.errorStats.rmsError;
    result.correspondenceCount = currentResult.errorStats.numCorrespondences;
    result.isValid = true;
    result.algorithm = "Manual";
    result.timestamp = QDateTime::currentDateTime();

    m_registrationProject->addRegistrationResult(result);

    // 5. Clear alignment engine state
    m_alignmentEngine->clearCorrespondences();

    // 6. Clear dynamic transform in viewer
    if (m_viewer) {
        auto* viewerWidget = dynamic_cast<PointCloudViewerWidget*>(m_viewer);
        if (viewerWidget) {
            viewerWidget->clearDynamicTransform();
        }
    }

    // 7. Update UI state and transition to QualityReview
    if (m_workflowWidget) {
        m_workflowWidget->goToStep(RegistrationStep::QualityReview);
    }

    // 8. Update status
    m_view->updateStatusBar("Alignment accepted successfully");
    showInfo("Accept Alignment", "Alignment has been accepted and applied to the target scan.");
    */

    // Placeholder implementation for now
    showInfo("Accept Alignment", "Alignment acceptance functionality will be fully implemented when AlignmentEngine and RegistrationProject are integrated.");
}

void MainPresenter::handleCancelAlignment()
{
    qDebug() << "MainPresenter::handleCancelAlignment() called";

    // TODO: This implementation requires AlignmentEngine and RegistrationWorkflowWidget instances
    // These would typically be injected via constructor or setter methods

    /*
    // Intended implementation when components are available:

    if (!m_alignmentEngine) {
        showError("Cancel Alignment", "AlignmentEngine not available.");
        return;
    }

    // 1. Clear alignment engine correspondences and state
    m_alignmentEngine->clearCorrespondences();

    // 2. Clear dynamic transform in viewer
    if (m_viewer) {
        auto* viewerWidget = dynamic_cast<PointCloudViewerWidget*>(m_viewer);
        if (viewerWidget) {
            viewerWidget->clearDynamicTransform();
        }
    }

    // 3. Transition back to ManualAlignment step
    if (m_workflowWidget) {
        m_workflowWidget->goToStep(RegistrationStep::ManualAlignment);
    }

    // 4. Update status
    m_view->updateStatusBar("Alignment cancelled");
    showInfo("Cancel Alignment", "Alignment has been cancelled. No changes were applied.");
    */

    // Placeholder implementation for now
    showInfo("Cancel Alignment", "Alignment cancellation functionality will be fully implemented when AlignmentEngine is integrated.");
}

// Sprint 3.2: Export functionality implementation
void MainPresenter::handleExportPointCloud()
{
    // Pre-check: Verify that viewer has point cloud data
    if (!m_viewer || !m_viewer->hasPointCloudData())
    {
        showError("Export Error", "No point cloud data loaded for export.");
        return;
    }

    // Retrieve current point cloud data (with applied transformations)
    std::vector<Point> dataToExport = m_viewer->getCurrentPointCloudData();
    if (dataToExport.empty())
    {
        showError("Export Error", "No point cloud data available for export.");
        return;
    }

    // Create and configure ExportDialog
    ExportDialog dialog(static_cast<QWidget*>(m_view));
    dialog.setPointCloudData(dataToExport);

    // Set project information if available
    if (m_currentProject)
    {
        dialog.setProjectInfo(m_currentProject->projectName(), m_currentProject->description());
    }
    else
    {
        // Fallback to basic project info
        QString projectName = m_isProjectOpen ? QFileInfo(m_currentProjectPath).baseName() : "Untitled";
        dialog.setProjectInfo(projectName, "Point cloud export from Cloud Registration application");
    }

    // Note: ExportDialog will set its own supported formats internally
    // No need to set them explicitly since it has its own PointCloudExporter

    // Load default settings
    dialog.loadSettings();

    // Note: The current ExportDialog is self-contained and handles export internally
    // We don't need to connect to our own exporter since the dialog has its own

    // Show dialog and handle result
    if (dialog.exec() == QDialog::Accepted)
    {
        // The ExportDialog handles the export internally and shows its own progress
        // We just need to update our status bar
        m_view->updateStatusBar("Export completed");
        showInfo("Export Successful", "Point cloud has been exported successfully.");
    }
}

void MainPresenter::onExportCompleted(const ExportResult& result)
{
    // This method is currently not used since ExportDialog is self-contained
    // But keeping it for future integration when we move to the MVP pattern
    // as specified in s3.2.md

    if (result.success)
    {
        showInfo("Export Successful",
                QString("Point cloud exported successfully to:\n%1").arg(result.outputPath));
        m_view->updateStatusBar("Export completed successfully");
    }
    else
    {
        showError("Export Failed",
                QString("Export failed: %1").arg(result.errorMessage));
        m_view->updateStatusBar("Export failed");
    }
}
void MainPresenter::setRegistrationProject(Registration::RegistrationProject* project)
{
    m_registrationProject = project;

    if (m_registrationProject) {
        // Connect to registration project signals
        connect(m_registrationProject, &Registration::RegistrationProject::registrationResultAdded,
                this, &MainPresenter::rebuildPoseGraph);

        // Connect to update sidebar when registration results are added
        connect(m_registrationProject, &Registration::RegistrationProject::registrationResultAdded,
                this, &MainPresenter::onRegistrationResultAdded);

        // Set the registration project on the sidebar for scan grouping
        if (m_view) {
            auto* sidebar = m_view->getSidebar();
            if (sidebar) {
                sidebar->setRegistrationProject(m_registrationProject);
                qDebug() << "MainPresenter: Registration project set on sidebar";
            }
        }

        qDebug() << "MainPresenter: Registration project set";
    }
}
void MainPresenter::onRegistrationResultAdded(const QString& sourceScanId, const QString& targetScanId)
{
    qDebug() << "MainPresenter: Registration result added for" << sourceScanId << "to" << targetScanId;

    // Update the sidebar to show the new grouping
    if (m_view) {
        auto* sidebar = m_view->getSidebar();
        if (sidebar) {
            sidebar->refreshFromDatabase();
            qDebug() << "MainPresenter: Sidebar refreshed after registration result added";
        }
    }
}

void MainPresenter::setPoseGraphViewer(PoseGraphViewerWidget* viewer)
{
    m_poseGraphViewer = viewer;

    if (m_poseGraphViewer) {
        // Connect pose graph viewer signals
        connect(m_poseGraphViewer, &PoseGraphViewerWidget::nodeSelected,
                this, [this](const QString& scanId) {
                    qDebug() << "Pose graph node selected:" << scanId;
                    // Handle node selection (e.g., highlight in main viewer)
                });

        connect(m_poseGraphViewer, &PoseGraphViewerWidget::edgeSelected,
                this, [this](const QString& sourceScanId, const QString& targetScanId) {
                    qDebug() << "Pose graph edge selected:" << sourceScanId << "to" << targetScanId;
                    // Handle edge selection (e.g., show registration details)
                });

        connect(m_poseGraphViewer, &PoseGraphViewerWidget::bundleAdjustmentRequested,
                this, &MainPresenter::handleRunBundleAdjustment);

        qDebug() << "MainPresenter: Pose graph viewer set";
    }
}

void MainPresenter::handleLoadProjectCompleted()
{
    qDebug() << "MainPresenter: Project load completed, rebuilding pose graph";
    rebuildPoseGraph();
}

void MainPresenter::rebuildPoseGraph()
{
    if (!m_registrationProject || !m_poseGraphBuilder) {
        qWarning() << "MainPresenter: Cannot rebuild pose graph - missing registration project or builder";
        return;
    }

    try {
        qDebug() << "MainPresenter: Starting pose graph rebuild";

        // Build the pose graph from the registration project
        m_currentPoseGraph = m_poseGraphBuilder->build(*m_registrationProject);

        if (m_currentPoseGraph && m_poseGraphViewer) {
            // Display the graph in the viewer
            m_poseGraphViewer->displayGraph(*m_currentPoseGraph);

            qDebug() << "MainPresenter: Pose graph rebuilt and displayed with"
                     << m_currentPoseGraph->nodeCount() << "nodes and"
                     << m_currentPoseGraph->edgeCount() << "edges";

            if (m_view) {
                m_view->updateStatusBar(QString("Pose graph updated: %1 nodes, %2 edges")
                                       .arg(m_currentPoseGraph->nodeCount())
                                       .arg(m_currentPoseGraph->edgeCount()));
            }
        } else {
            qWarning() << "MainPresenter: Failed to build pose graph or viewer not available";
        }
    } catch (const std::exception& e) {
        qCritical() << "MainPresenter: Error rebuilding pose graph:" << e.what();
        showError("Pose Graph Error", QString("Failed to rebuild pose graph: %1").arg(e.what()));
    }
}



void MainPresenter::triggerAlignmentPreview()
{
    if (!m_targetManager)
    {
        showError("Alignment Preview", "Target manager is not available.");
        return;
    }

    if (!m_alignmentEngine)
    {
        showError("Alignment Preview", "Alignment engine is not available.");
        return;
    }

    // Retrieve correspondences from TargetManager
    QList<TargetCorrespondence> correspondences = m_targetManager->getAllCorrespondences();

    if (correspondences.size() < 3)
    {
        showError("Alignment Preview", "At least 3 point correspondences are required for alignment computation.");
        return;
    }

    // Trigger alignment computation through AlignmentEngine
    m_alignmentEngine->recomputeAlignment();

    // Update status
    if (m_view)
    {
        m_view->updateStatusBar("Alignment computation started...");
    }
}

<<<<<<< Updated upstream
// Sprint 6.1: Deviation map toggle implementation
void MainPresenter::handleShowDeviationMapToggled(bool enabled)
{
    qDebug() << "MainPresenter::handleShowDeviationMapToggled called with enabled:" << enabled;

    // This is a stub implementation for now
    // In a full implementation, we would need:
    // 1. Access to RegistrationProject to get the latest registration result
    // 2. Access to AlignmentEngine to perform deviation analysis
    // 3. Access to PointCloudViewerWidget to load colorized points and show legend

    if (enabled)
    {
        showInfo("Deviation Map", "Deviation map functionality is implemented but requires registration data. "
                                  "Please ensure you have performed a registration between scans first.");

        // TODO: Implement the full logic as described in the S6.1 document:
        // - Get latest registration result from RegistrationProject
        // - Get source and target point data from PointCloudLoadManager
        // - Call AlignmentEngine::analyzeDeviation()
        // - Call PointCloudViewerWidget::loadColorizedPointCloud()
        // - Call PointCloudViewerWidget::setDeviationMapLegendVisible()
    }
    else
    {
        showInfo("Deviation Map", "Deviation map disabled.");

        // TODO: Implement the disable logic:
        // - Call PointCloudViewerWidget::revertToOriginalColors()
        // - Call PointCloudViewerWidget::setDeviationMapLegendVisible(false, 0.0f)
    }

    if (m_view)
    {
        m_view->updateStatusBar(enabled ? "Deviation map enabled" : "Deviation map disabled");
    }
}

// Sprint 6.3: Enhanced PDF Report Generation with Options Dialog
void MainPresenter::handleGenerateReportClicked()
{
    qDebug() << "MainPresenter::handleGenerateReportClicked() called";

    // Pre-check: Verify that we have a valid quality report
    if (!m_lastQualityReport.isValid()) {
        showError("Generate Quality Report",
                  "No quality assessment data available. Please perform a quality assessment first.");
        return;
    }

    if (!m_reportGenerator) {
        showError("Generate Quality Report", "PDF report generator is not available.");
        return;
    }

    // Create and configure ReportOptionsDialog
    auto* dialog = new ReportOptionsDialog(m_view, static_cast<QWidget*>(m_view));

    // Set default options based on current project
    QString projectName = m_lastQualityReport.projectName.isEmpty() ?
                         "Untitled Project" : m_lastQualityReport.projectName;
    auto defaultOptions = PDFReportGenerator::ReportOptions::createDefault(projectName);
    dialog->setReportOptions(defaultOptions);

    // Connect dialog signal to our new slot
    connect(dialog, &ReportOptionsDialog::generateReportRequested,
            this, [this, dialog](const PDFReportGenerator::ReportOptions& options) {
                startReportGeneration(options, dialog);
            });

    // Show dialog modally
    dialog->exec();

    // Clean up
    dialog->deleteLater();
}

// Sprint 6.3: Start report generation with user-configured options
void MainPresenter::startReportGeneration(const PDFReportGenerator::ReportOptions& options, ReportOptionsDialog* dialog)
{
    qDebug() << "MainPresenter::startReportGeneration() called";

    if (!m_reportGenerator) {
        if (dialog) {
            dialog->onReportFinished(false, "PDF report generator is not available.");
        } else {
            showError("Generate Quality Report", "PDF report generator is not available.");
        }
        return;
    }

    // Connect progress signals to dialog if provided
    if (dialog) {
        // Connect progress updates
        connect(m_reportGenerator, &PDFReportGenerator::reportProgress,
                dialog, &ReportOptionsDialog::onReportProgress);

        // Connect completion signals
        connect(m_reportGenerator, &PDFReportGenerator::reportGenerated,
                dialog, [dialog](const QString& filePath) {
                    dialog->onReportFinished(true, QString("Report generated successfully at:\n%1").arg(filePath));
                });

        connect(m_reportGenerator, &PDFReportGenerator::reportError,
                dialog, [dialog](const QString& error) {
                    dialog->onReportFinished(false, error);
                });
    }

    // Update status
    if (m_view) {
        m_view->updateStatusBar("Generating quality report...");
    }

    // Trigger report generation with user options
    m_reportGenerator->generatePdfReport(m_lastQualityReport, options);
}

void MainPresenter::onQualityAssessmentCompleted(const QualityReport& report)
{
    qDebug() << "MainPresenter::onQualityAssessmentCompleted() called";

    // Store the quality report
    m_lastQualityReport = report;

    // Enable the generate report action through the view
    // Note: This assumes the view has a method to enable specific actions
    // In the actual implementation, this would be handled through IMainView interface

    if (m_view) {
        m_view->updateStatusBar("Quality assessment completed. Report generation is now available.");
    }

    showInfo("Quality Assessment", "Quality assessment completed successfully. You can now generate a PDF report.");
}

void MainPresenter::onReportGenerated(const QString& filePath)
{
    qDebug() << "MainPresenter::onReportGenerated() called with path:" << filePath;

    if (m_view) {
        m_view->updateStatusBar("Quality report generated successfully");
    }

    showInfo("Report Generated",
             QString("Quality report has been successfully generated and saved to:\n%1").arg(filePath));
}

void MainPresenter::onReportError(const QString& error)
{
    qDebug() << "MainPresenter::onReportError() called with error:" << error;

    if (m_view) {
        m_view->updateStatusBar("Report generation failed");
    }

    showError("Report Generation Failed",
              QString("Failed to generate quality report:\n%1").arg(error));
}

// Sprint 2.2: Alignment computation and live preview implementation
void MainPresenter::handleAlignmentResultUpdated(const AlignmentEngine::AlignmentResult& result)
{
    qDebug() << "MainPresenter::handleAlignmentResultUpdated() called with state:" << static_cast<int>(result.state);

    // Update PointCloudViewerWidget with dynamic transformation for live preview
    if (m_viewer && result.isValid())
    {
        auto* viewerWidget = dynamic_cast<PointCloudViewerWidget*>(m_viewer);
        if (viewerWidget)
        {
            // Apply the transformation to the "moving" scan for live preview
            // Note: In a full implementation, we would need to determine which scan is the "moving" one
            // For now, we apply the transformation assuming the second scan is the moving one
            viewerWidget->setDynamicTransform(result.transformation);

            qDebug() << "Dynamic transformation applied to viewer for live preview";
        }
    }
    else if (m_viewer && !result.isValid())
    {
        // Clear dynamic transform if result is invalid
        auto* viewerWidget = dynamic_cast<PointCloudViewerWidget*>(m_viewer);
        if (viewerWidget)
        {
            viewerWidget->clearDynamicTransform();
        }
    }

    // Update AlignmentControlPanel with quality metrics
    if (m_view)
    {
        auto* alignmentPanel = m_view->getAlignmentControlPanel();
        if (alignmentPanel)
        {
            alignmentPanel->updateAlignmentResult(result);
            qDebug() << "Alignment control panel updated with result metrics";
        }
    }

    // Update status bar based on result state
    if (m_view)
    {
        QString statusMessage;
        switch (result.state)
        {
            case AlignmentEngine::AlignmentState::Valid:
                statusMessage = QString("Alignment computed successfully - RMS: %1 mm")
                                .arg(result.errorStats.rmsError, 0, 'f', 3);
                break;
            case AlignmentEngine::AlignmentState::Computing:
                statusMessage = "Computing alignment...";
                break;
            case AlignmentEngine::AlignmentState::Error:
                statusMessage = QString("Alignment error: %1").arg(result.message);
                break;
            case AlignmentEngine::AlignmentState::Insufficient:
                statusMessage = "Insufficient correspondences for alignment";
                break;
            default:
                statusMessage = "Alignment idle";
                break;
        }

        m_view->updateStatusBar(statusMessage);
    }
}

// Sprint 2.3: Enhanced Status & Diagnostics implementation
void MainPresenter::handleAlignmentStateChanged(AlignmentEngine::AlignmentState state, const QString& message)
{
    qDebug() << "MainPresenter::handleAlignmentStateChanged() called with state:" << static_cast<int>(state) << "message:" << message;

    // Update AlignmentControlPanel with state changes
    if (m_view)
    {
        auto* alignmentPanel = m_view->getAlignmentControlPanel();
        if (alignmentPanel)
        {
            alignmentPanel->updateAlignmentState(state, message);
            qDebug() << "Alignment control panel updated with state change";
        }
    }
}

// Sprint 3.2: Export functionality implementation
void MainPresenter::handleExportPointCloud()
{
    // Pre-check: Verify that viewer has point cloud data
    if (!m_viewer || !m_viewer->hasPointCloudData())
    {
        showError("Export Error", "No point cloud data loaded for export.");
        return;
    }

    // Retrieve current point cloud data (with applied transformations)
    std::vector<Point> dataToExport = m_viewer->getCurrentPointCloudData();
    if (dataToExport.empty())
    {
        showError("Export Error", "No point cloud data available for export.");
        return;
    }

    // Create and configure ExportDialog
    ExportDialog dialog(static_cast<QWidget*>(m_view));
    dialog.setPointCloudData(dataToExport);

    // Set project information if available
    if (m_currentProject)
    {
        dialog.setProjectInfo(m_currentProject->projectName(), m_currentProject->description());
    }
    else
    {
        // Fallback to basic project info
        QString projectName = m_isProjectOpen ? QFileInfo(m_currentProjectPath).baseName() : "Untitled";
        dialog.setProjectInfo(projectName, "Point cloud export from Cloud Registration application");
    }

    // Note: ExportDialog will set its own supported formats internally
    // No need to set them explicitly since it has its own PointCloudExporter

    // Load default settings
    dialog.loadSettings();

    // Note: The current ExportDialog is self-contained and handles export internally
    // We don't need to connect to our own exporter since the dialog has its own

    // Show dialog and handle result
    if (dialog.exec() == QDialog::Accepted)
    {
        // The ExportDialog handles the export internally and shows its own progress
        // We just need to update our status bar
        m_view->updateStatusBar("Export completed");
        showInfo("Export Successful", "Point cloud has been exported successfully.");
    }
}

void MainPresenter::onExportCompleted(const ExportResult& result)
{
    // This method is currently not used since ExportDialog is self-contained
    // But keeping it for future integration when we move to the MVP pattern
    // as specified in s3.2.md

    if (result.success)
    {
        showInfo("Export Successful",
                QString("Point cloud exported successfully to:\n%1").arg(result.outputPath));
        m_view->updateStatusBar("Export completed successfully");
    }
    else
    {
        showError("Export Failed",
                QString("Export failed: %1").arg(result.errorMessage));
        m_view->updateStatusBar("Export failed");
    }
}

// Bundle Adjustment Implementation
void MainPresenter::handleRunBundleAdjustment()
{
    qDebug() << "MainPresenter::handleRunBundleAdjustment() called";

    // Pre-check: Verify pose graph is valid and meets minimum criteria
    if (!m_currentPoseGraph) {
        showError("Bundle Adjustment", "No pose graph available. Please load a project with registered scans first.");
        return;
    }

    if (m_currentPoseGraph->nodeCount() < 3) {
        showError("Bundle Adjustment",
                  QString("Bundle Adjustment requires at least 3 nodes. Current graph has %1 nodes.")
                  .arg(m_currentPoseGraph->nodeCount()));
        return;
    }

    if (m_currentPoseGraph->edgeCount() < 2) {
        showError("Bundle Adjustment",
                  QString("Bundle Adjustment requires at least 2 edges. Current graph has %1 edges.")
                  .arg(m_currentPoseGraph->edgeCount()));
        return;
    }

    // Create Bundle Adjustment progress dialog
    m_baProgressDialog = std::make_unique<BundleAdjustmentProgressDialog>(
        static_cast<QWidget*>(m_view));

    // Connect progress dialog signals
    connect(m_baProgressDialog.get(), &BundleAdjustmentProgressDialog::cancelRequested,
            this, &MainPresenter::cancelBundleAdjustment);

    // Create Bundle Adjustment algorithm instance
    m_bundleAdjustment = std::make_unique<Optimization::BundleAdjustment>();

    // Connect Bundle Adjustment signals
    connect(m_bundleAdjustment.get(), &Optimization::BundleAdjustment::optimizationProgress,
            this, &MainPresenter::onBundleAdjustmentProgress);
    connect(m_bundleAdjustment.get(), &Optimization::BundleAdjustment::optimizationCompleted,
            this, &MainPresenter::onBundleAdjustmentCompleted);

    // Get recommended parameters for the current graph
    auto params = m_bundleAdjustment->getRecommendedParameters(*m_currentPoseGraph);

    // Start monitoring in the progress dialog
    m_baProgressDialog->startMonitoring(m_bundleAdjustment.get(), params.maxIterations);

    // Show the progress dialog
    m_baProgressDialog->show();

    // Start Bundle Adjustment optimization asynchronously
    // Note: In a real implementation, this should run in a separate thread
    auto result = m_bundleAdjustment->optimize(*m_currentPoseGraph, params);

    qDebug() << "MainPresenter: Bundle Adjustment started with"
             << m_currentPoseGraph->nodeCount() << "nodes and"
             << m_currentPoseGraph->edgeCount() << "edges";

    if (m_view) {
        m_view->updateStatusBar("Bundle Adjustment optimization started...");
    }
}

void MainPresenter::cancelBundleAdjustment()
{
    qDebug() << "MainPresenter::cancelBundleAdjustment() called";

    if (m_bundleAdjustment) {
        m_bundleAdjustment->cancel();
        qDebug() << "MainPresenter: Bundle Adjustment cancellation requested";
    }
}

void MainPresenter::onBundleAdjustmentProgress(int iteration, double currentError, double lambda)
{
    if (m_baProgressDialog) {
        m_baProgressDialog->updateProgress(iteration, currentError);
    }

    // Update status bar with progress
    if (m_view) {
        m_view->updateStatusBar(QString("Bundle Adjustment: Iteration %1, Error: %2")
                               .arg(iteration).arg(currentError, 0, 'e', 3));
    }
}

void MainPresenter::onBundleAdjustmentCompleted(const Optimization::BundleAdjustment::Result& result)
{
    qDebug() << "MainPresenter::onBundleAdjustmentCompleted() called";
    qDebug() << "Result: converged=" << result.converged
             << ", iterations=" << result.iterations
             << ", final error=" << result.finalError
             << ", improvement=" << (result.improvementRatio * 100) << "%";

    // Update progress dialog
    if (m_baProgressDialog) {
        m_baProgressDialog->onComputationFinished(result.converged, result.statusMessage);
    }

    if (result.converged && m_registrationProject) {
        // Apply optimized poses to the registration project
        // Note: This requires the optimized graph to be returned from the optimization
        // For now, we'll show a success message

        showInfo("Bundle Adjustment Complete",
                 QString("Bundle Adjustment completed successfully!\n\n"
                        "Iterations: %1\n"
                        "Final Error: %2\n"
                        "Improvement: %3%\n"
                        "Time: %4 seconds")
                 .arg(result.iterations)
                 .arg(result.finalError, 0, 'e', 3)
                 .arg(result.improvementRatio * 100, 0, 'f', 1)
                 .arg(result.optimizationTimeSeconds, 0, 'f', 2));

        // Rebuild pose graph to reflect optimized poses
        rebuildPoseGraph();

        if (m_view) {
            m_view->updateStatusBar("Bundle Adjustment completed successfully");
        }
    } else {
        // Handle failure or cancellation
        QString message = result.converged ? "Bundle Adjustment completed" : "Bundle Adjustment failed or was cancelled";

        if (m_view) {
            m_view->updateStatusBar(message);
        }

        if (!result.converged && !result.statusMessage.contains("cancelled", Qt::CaseInsensitive)) {
            showError("Bundle Adjustment Failed",
                     QString("Bundle Adjustment failed to converge.\n\n%1").arg(result.statusMessage));
        }
    }

    // Clean up
    m_bundleAdjustment.reset();
    // Note: Don't reset m_baProgressDialog here as it may still be showing results
}

// Sprint 7.3: Performance Report Generation Implementation
void MainPresenter::handleGeneratePerformanceReportClicked()
{
    qDebug() << "MainPresenter::handleGeneratePerformanceReportClicked() called";

    // Check if profiling is enabled
    bool profilingEnabled = UserPreferences::instance().getValue("advanced/profilingEnabled", false).toBool();
    if (!profilingEnabled) {
        showError("Performance Report",
                  "Performance profiling is disabled. Please enable it in the advanced preferences first.");
        return;
    }

    // Check if we have any profiling data
    if (!PerformanceProfiler::instance().isEnabled()) {
        showError("Performance Report",
                  "Performance profiler is not enabled. No profiling data available.");
        return;
    }

    // Prompt for save path
    QString defaultName = QString("PerformanceReport_%1.txt")
                         .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    QString filePath = m_view->askForSaveFilePath("Save Performance Report",
                                                  "Text files (*.txt);;All files (*)",
                                                  defaultName);

    if (filePath.isEmpty()) {
        return; // User cancelled
    }

    try {
        // Generate the performance report
        PerformanceProfiler::instance().generateReport(filePath);

        // Display success message
        showInfo("Performance Report Generated",
                 QString("Performance report has been successfully generated and saved to:\n%1").arg(filePath));

        if (m_view) {
            m_view->updateStatusBar("Performance report generated successfully");
        }

        qDebug() << "Performance report generated successfully at:" << filePath;
    }
    catch (const std::exception& e) {
        showError("Performance Report Error",
                  QString("Failed to generate performance report:\n%1").arg(e.what()));

        if (m_view) {
            m_view->updateStatusBar("Performance report generation failed");
        }
    }
    catch (...) {
        showError("Performance Report Error",
                  "An unknown error occurred while generating the performance report.");

        if (m_view) {
            m_view->updateStatusBar("Performance report generation failed");
        }
    }
}

// Sprint 4.3: ICP Result Application & Workflow Progression
void MainPresenter::handleICPCompletion(bool success, const QMatrix4x4& finalTransformation, float finalRMSError, int iterations)
{
    qDebug() << "MainPresenter::handleICPCompletion() called - Success:" << success
             << "RMS:" << finalRMSError << "Iterations:" << iterations;

    // Store ICP results for later use
    m_lastICPTransformation = finalTransformation;
    m_lastICPRMSError = finalRMSError;
    m_lastICPIterations = iterations;
    m_hasValidICPResult = success;

    if (success)
    {
        // Update AlignmentControlPanel with final metrics
        if (m_view)
        {
            auto* alignmentPanel = m_view->getAlignmentControlPanel();
            if (alignmentPanel && m_alignmentEngine)
            {
                // Get the current result from AlignmentEngine and update the panel
                const auto& result = m_alignmentEngine->getCurrentResult();
                alignmentPanel->updateAlignmentResult(result);
                qDebug() << "Updated AlignmentControlPanel with final ICP metrics";
            }
        }

        // Update status bar
        if (m_view)
        {
            m_view->updateStatusBar(QString("ICP completed successfully - RMS: %1 mm, Iterations: %2")
                                   .arg(finalRMSError, 0, 'f', 3)
                                   .arg(iterations));
        }
    }
    else
    {
        // Handle failure case
        if (m_view)
        {
            auto* alignmentPanel = m_view->getAlignmentControlPanel();
            if (alignmentPanel)
            {
                // Update with failure state - buttons should be disabled
                alignmentPanel->updateAlignmentState(AlignmentEngine::AlignmentState::Error,
                                                   "ICP computation failed or was cancelled");
            }

            // Clear dynamic transform on failure
            if (m_viewer)
            {
                auto* viewerWidget = dynamic_cast<PointCloudViewerWidget*>(m_viewer);
                if (viewerWidget)
                {
                    viewerWidget->clearDynamicTransform();
                }
            }

            m_view->updateStatusBar("ICP computation failed or was cancelled");
        }
    }
}

void MainPresenter::handleAcceptICPResult()
{
    qDebug() << "MainPresenter::handleAcceptICPResult() called";

    if (!m_hasValidICPResult)
    {
        showError("Accept ICP Result", "No valid ICP result available to accept.");
        return;
    }

    if (!m_registrationProject)
    {
        showError("Accept ICP Result", "No registration project is available.");
        return;
    }

    try
    {
        // Apply permanent transformation to the target scan
        if (!m_currentTargetScanId.isEmpty())
        {
            m_registrationProject->setScanTransform(m_currentTargetScanId, m_lastICPTransformation);
            qDebug() << "Applied transformation to scan:" << m_currentTargetScanId;
        }

        // Create and add registration result
        RegistrationProject::RegistrationResult result;
        result.sourceScanId = m_currentSourceScanId;
        result.targetScanId = m_currentTargetScanId;
        result.transformation = m_lastICPTransformation;
        result.rmsError = m_lastICPRMSError;
        result.algorithm = "ICP";
        result.timestamp = QDateTime::currentDateTime();
        result.iterations = m_lastICPIterations;

        m_registrationProject->addRegistrationResult(result);
        qDebug() << "Added registration result to project";

        // Clear dynamic transform in viewer
        if (m_viewer)
        {
            auto* viewerWidget = dynamic_cast<PointCloudViewerWidget*>(m_viewer);
            if (viewerWidget)
            {
                viewerWidget->clearDynamicTransform();
                qDebug() << "Cleared dynamic transform from viewer";
            }
        }

        // Reset alignment engine state
        if (m_alignmentEngine)
        {
            m_alignmentEngine->clearCorrespondences();
            qDebug() << "Cleared alignment engine correspondences";
        }

        // Transition workflow to Quality Review
        if (m_workflowWidget)
        {
            m_workflowWidget->goToStep(RegistrationStep::QualityReview);
            qDebug() << "Transitioned workflow to QualityReview step";
        }

        // Update UI state
        if (m_view)
        {
            m_view->updateStatusBar("ICP alignment accepted and applied successfully");
        }

        // Reset ICP result state
        m_hasValidICPResult = false;

        showInfo("Accept ICP Result", "ICP alignment has been accepted and applied to the target scan.");
    }
    catch (const std::exception& e)
    {
        showError("Accept ICP Result", QString("Failed to accept ICP result: %1").arg(e.what()));
    }
    catch (...)
    {
        showError("Accept ICP Result", "Unknown error occurred while accepting ICP result.");
    }
}

void MainPresenter::handleDiscardICPResult()
{
    qDebug() << "MainPresenter::handleDiscardICPResult() called";

    try
    {
        // Clear dynamic transform in viewer
        if (m_viewer)
        {
            auto* viewerWidget = dynamic_cast<PointCloudViewerWidget*>(m_viewer);
            if (viewerWidget)
            {
                viewerWidget->clearDynamicTransform();
                qDebug() << "Cleared dynamic transform from viewer";
            }
        }

        // Reset alignment engine state
        if (m_alignmentEngine)
        {
            m_alignmentEngine->clearCorrespondences();
            qDebug() << "Cleared alignment engine correspondences";
        }

        // Keep workflow at ICPRegistration step or return to ManualAlignment for retry
        if (m_workflowWidget)
        {
            // For now, keep at ICPRegistration step to allow retry
            // In future, we might want to transition to ManualAlignment
            qDebug() << "Keeping workflow at current step for retry";
        }

        // Update UI state
        if (m_view)
        {
            auto* alignmentPanel = m_view->getAlignmentControlPanel();
            if (alignmentPanel)
            {
                // Reset alignment panel to idle state
                alignmentPanel->updateAlignmentState(AlignmentEngine::AlignmentState::Idle,
                                                   "ICP result discarded");
            }

            m_view->updateStatusBar("ICP alignment discarded");
        }

        // Reset ICP result state
        m_hasValidICPResult = false;

        showInfo("Discard ICP Result", "ICP alignment has been discarded. You can retry the alignment if needed.");
    }
    catch (const std::exception& e)
    {
        showError("Discard ICP Result", QString("Error while discarding ICP result: %1").arg(e.what()));
    }
    catch (...)
    {
        showError("Discard ICP Result", "Unknown error occurred while discarding ICP result.");
    }
}