#include "MainPresenter.h"
#include "IMainView.h"
#include "IE57Parser.h"
#include "IE57Writer.h"
#include "IPointCloudViewer.h"
#include "projectmanager.h"
#include "scaninfo.h"
#include "sqlitemanager.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>

MainPresenter::MainPresenter(IMainView* view,
                           IE57Parser* e57Parser,
                           IE57Writer* e57Writer,
                           QObject* parent)
    : QObject(parent)
    , m_view(view)
    , m_e57Parser(e57Parser)
    , m_e57Writer(e57Writer)
    , m_viewer(nullptr)
    , m_projectManager(nullptr)
    , m_isFileOpen(false)
    , m_isProjectOpen(false)
    , m_isParsingInProgress(false)
    , m_currentMemoryUsage(0)
    , m_currentFPS(0.0f)
    , m_currentVisiblePoints(0)
{
    if (m_view) {
        m_viewer = m_view->getViewer();
    }
}

MainPresenter::MainPresenter(IMainView* view,
                           IE57Parser* e57Parser,
                           IE57Writer* e57Writer,
                           ProjectManager* projectManager,
                           QObject* parent)
    : QObject(parent)
    , m_view(view)
    , m_e57Parser(e57Parser)
    , m_e57Writer(e57Writer)
    , m_viewer(nullptr)
    , m_projectManager(projectManager)
    , m_isFileOpen(false)
    , m_isProjectOpen(false)
    , m_isParsingInProgress(false)
    , m_currentMemoryUsage(0)
    , m_currentFPS(0.0f)
    , m_currentVisiblePoints(0)
{
    if (m_view) {
        m_viewer = m_view->getViewer();
    }
}

void MainPresenter::initialize() {
    setupConnections();
    updateUIState();
    updateWindowTitle();
}

void MainPresenter::setProjectManager(ProjectManager* projectManager) {
    m_projectManager = projectManager;
}

void MainPresenter::setupConnections() {
    if (!m_view || !m_e57Parser) {
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
    if (m_viewer) {
        connect(m_viewer, &IPointCloudViewer::stateChanged, this, &MainPresenter::onViewerStateChanged);
        connect(m_viewer, &IPointCloudViewer::statsUpdated, this, &MainPresenter::onRenderingStatsUpdated);
    }

    // Sprint 3: Connect sidebar signals if available
    if (m_view) {
        auto* sidebar = m_view->getSidebar();
        if (sidebar) {
            // Connect cluster operations
            connect(sidebar, &SidebarWidget::clusterCreationRequested,
                    this, &MainPresenter::handleCreateCluster);
            connect(sidebar, &SidebarWidget::clusterRenameRequested,
                    this, &MainPresenter::handleRenameCluster);
            connect(sidebar, &SidebarWidget::deleteClusterRequested,
                    this, &MainPresenter::handleDeleteCluster);

            // Connect scan operations
            connect(sidebar, &SidebarWidget::loadScanRequested,
                    this, &MainPresenter::handleLoadScan);
            connect(sidebar, &SidebarWidget::unloadScanRequested,
                    this, &MainPresenter::handleUnloadScan);
            connect(sidebar, &SidebarWidget::loadClusterRequested,
                    this, &MainPresenter::handleLoadCluster);
            connect(sidebar, &SidebarWidget::unloadClusterRequested,
                    this, &MainPresenter::handleUnloadCluster);
            connect(sidebar, &SidebarWidget::viewPointCloudRequested,
                    this, &MainPresenter::handleViewPointCloud);
            connect(sidebar, &SidebarWidget::deleteScanRequested,
                    this, &MainPresenter::handleDeleteScan);
            connect(sidebar, &SidebarWidget::batchOperationRequested,
                    this, &MainPresenter::handleBatchOperation);
        }
    }
}

void MainPresenter::handleNewProject() {
    if (m_isProjectOpen) {
        bool confirmed = m_view->askForConfirmation("Close Current Project", 
            "A project is already open. Do you want to close it and create a new project?");
        if (!confirmed) {
            return;
        }
        handleCloseProject();
    }

    // For now, just show info that project creation is not fully implemented
    showInfo("New Project", "New project creation functionality will be implemented in future sprints.");
    updateWindowTitle();
}

void MainPresenter::handleOpenProject() {
    QString projectPath = m_view->askForOpenFilePath("Open Project", "Project Files (*.crp)");
    if (projectPath.isEmpty()) {
        return;
    }

    if (!QFileInfo::exists(projectPath)) {
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

void MainPresenter::handleCloseProject() {
    if (!m_isProjectOpen) {
        return;
    }

    // Close any open files first
    if (m_isFileOpen) {
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

void MainPresenter::handleImportScans() {
    if (!m_isProjectOpen) {
        showError("Import Scans", "Please open or create a project first.");
        return;
    }

    QString filePath = m_view->askForOpenFilePath("Import E57 Scan", "E57 Files (*.e57)");
    if (!filePath.isEmpty()) {
        handleOpenFile(filePath);
    }
}

void MainPresenter::handleOpenFile(const QString& filePath) {
    if (!validateFilePath(filePath)) {
        return;
    }

    if (m_isParsingInProgress) {
        showError("File Opening", "Another file is currently being processed. Please wait.");
        return;
    }

    // Close any currently open file
    if (m_isFileOpen) {
        clearPointCloudData();
        m_e57Parser->closeFile();
    }

    m_currentFilePath = filePath;
    m_isParsingInProgress = true;
    
    m_view->showProgressDialog(true, "Opening File", "Initializing E57 parser...");
    m_view->setActionsEnabled(false);
    m_view->updateStatusBar("Opening file: " + QFileInfo(filePath).fileName());

    // Start parsing asynchronously
    if (!m_e57Parser->openFile(filePath.toStdString())) {
        onParsingFinished(false, m_e57Parser->getLastError(), std::vector<float>());
        return;
    }

    // If we get here, file opened successfully, now extract data
    try {
        auto points = m_e57Parser->extractPointData();
        onParsingFinished(true, "File opened successfully", points);
    } catch (...) {
        onParsingFinished(false, "Failed to extract point data", std::vector<float>());
    }
}

void MainPresenter::handleSaveFile(const QString& filePath) {
    if (!m_e57Writer) {
        showError("Save File", "E57 writer is not available.");
        return;
    }

    if (!m_isFileOpen || !m_viewer || !m_viewer->hasData()) {
        showError("Save File", "No point cloud data to save.");
        return;
    }

    showInfo("Save File", "File saving functionality will be implemented in future sprints.");
}

void MainPresenter::handleScanActivation(const QString& scanId) {
    if (!m_isFileOpen) {
        showError("Scan Activation", "No file is currently open.");
        return;
    }

    m_view->highlightScan(scanId);
    m_view->updateStatusBar("Activated scan: " + scanId);
}

void MainPresenter::handleViewerSettingsChanged() {
    // Update any presenter-level state based on viewer settings
    updateUIState();
}

void MainPresenter::handleExit() {
    if (m_isProjectOpen || m_isFileOpen) {
        bool confirmed = m_view->askForConfirmation("Exit Application", 
            "Are you sure you want to exit? Any unsaved changes will be lost.");
        if (!confirmed) {
            return;
        }
    }

    // Clean up resources
    if (m_isFileOpen) {
        clearPointCloudData();
        m_e57Parser->closeFile();
    }
}

void MainPresenter::onParsingProgress(int percentage, const QString& stage) {
    m_view->updateProgress(percentage, stage);
}

void MainPresenter::onParsingFinished(bool success, const QString& message, const std::vector<float>& points) {
    m_isParsingInProgress = false;
    m_view->showProgressDialog(false);
    m_view->setActionsEnabled(true);

    if (success && !points.empty()) {
        m_isFileOpen = true;
        
        // Load points into viewer
        if (m_viewer) {
            m_viewer->loadPointCloud(points);
            m_viewer->resetCamera();
        }
        
        QFileInfo fileInfo(m_currentFilePath);
        m_view->updateStatusBar(QString("Loaded %1 points from %2")
                               .arg(points.size() / 3)
                               .arg(fileInfo.fileName()));
        
        showInfo("File Opened", message);
    } else {
        m_isFileOpen = false;
        m_currentFilePath.clear();
        showError("File Opening Failed", message);
        m_view->updateStatusBar("Failed to open file");
    }

    updateUIState();
    updateWindowTitle();
}

void MainPresenter::onScanMetadataAvailable(int scanCount, const QStringList& scanNames) {
    m_currentScanNames = scanNames;
    m_view->updateScanList(scanNames);
    
    m_view->updateStatusBar(QString("Found %1 scans in file").arg(scanCount));
}

void MainPresenter::onIntensityDataExtracted(const std::vector<float>& /*intensityValues*/) {
    // Handle intensity data if needed
}

void MainPresenter::onColorDataExtracted(const std::vector<uint8_t>& /*colorValues*/) {
    // Handle color data if needed
}

void MainPresenter::onViewerStateChanged(int newState, const QString& message) {
    Q_UNUSED(newState)
    if (!message.isEmpty()) {
        m_view->updateStatusBar(message);
    }
}

void MainPresenter::onRenderingStatsUpdated(float fps, int visiblePoints) {
    m_currentFPS = fps;
    m_currentVisiblePoints = visiblePoints;
    m_view->updateRenderingStats(fps, visiblePoints);
}

void MainPresenter::onMemoryUsageChanged(size_t totalBytes) {
    m_currentMemoryUsage = totalBytes;
    m_view->updateMemoryUsage(totalBytes);
}

void MainPresenter::updateUIState() {
    // Enable/disable actions based on current state
    bool hasProject = m_isProjectOpen;
    bool hasFile = m_isFileOpen;
    bool isProcessing = m_isParsingInProgress;
    
    // This would typically update action states, but since we're working with interfaces,
    // we'll keep this simple for now
    m_view->setActionsEnabled(!isProcessing);
}

bool MainPresenter::validateFilePath(const QString& filePath) {
    if (filePath.isEmpty()) {
        showError("Invalid File", "File path is empty.");
        return false;
    }

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        showError("File Not Found", "The selected file does not exist.");
        return false;
    }

    if (!fileInfo.isReadable()) {
        showError("File Access", "The selected file cannot be read.");
        return false;
    }

    if (fileInfo.suffix().toLower() != "e57") {
        showError("Invalid File Type", "Please select a valid E57 file.");
        return false;
    }

    return true;
}

void MainPresenter::showError(const QString& title, const QString& message) {
    if (m_view) {
        m_view->displayErrorMessage(title, message);
    }
}

void MainPresenter::showInfo(const QString& title, const QString& message) {
    if (m_view) {
        m_view->displayInfoMessage(title, message);
    }
}

void MainPresenter::updateWindowTitle() {
    QString title = "Cloud Registration";
    
    if (m_isProjectOpen && !m_currentProjectPath.isEmpty()) {
        QFileInfo projectInfo(m_currentProjectPath);
        title += " - " + projectInfo.baseName();
    }
    
    if (m_isFileOpen && !m_currentFilePath.isEmpty()) {
        QFileInfo fileInfo(m_currentFilePath);
        title += " [" + fileInfo.fileName() + "]";
    }
    
    if (m_view) {
        m_view->setWindowTitle(title);
    }
}

void MainPresenter::clearPointCloudData() {
    if (m_viewer) {
        m_viewer->clearPointCloud();
    }
    m_currentScanNames.clear();
    m_view->updateScanList(QStringList());
}

// Sprint 3: Sidebar operation handler implementations
void MainPresenter::handleCreateCluster(const QString& parentClusterId) {
    if (!m_projectManager) {
        showError("Create Cluster", "Project manager is not available.");
        return;
    }

    QString clusterName = m_view->promptForClusterName("Create New Cluster");
    if (clusterName.isEmpty()) {
        return;
    }

    QString clusterId = m_projectManager->createCluster(clusterName, parentClusterId);
    if (!clusterId.isEmpty()) {
        qDebug() << "MainPresenter: Cluster created successfully:" << clusterName;
        m_view->updateStatusBar(QString("Created cluster: %1").arg(clusterName));
    } else {
        showError("Create Cluster", "Failed to create cluster.");
    }
}

void MainPresenter::handleRenameCluster(const QString& clusterId, const QString& newName) {
    if (!m_projectManager) {
        showError("Rename Cluster", "Project manager is not available.");
        return;
    }

    if (m_projectManager->renameCluster(clusterId, newName)) {
        qDebug() << "MainPresenter: Cluster renamed successfully to:" << newName;
        m_view->updateStatusBar(QString("Renamed cluster to: %1").arg(newName));
    } else {
        showError("Rename Cluster", "Failed to rename cluster.");
    }
}

void MainPresenter::handleDeleteCluster(const QString& clusterId, bool deletePhysicalFiles) {
    if (!m_projectManager) {
        showError("Delete Cluster", "Project manager is not available.");
        return;
    }

    // Show confirmation dialog
    QString message = QString("Are you sure you want to delete this cluster?\n\n"
                             "All scans in this cluster will be moved to the project root.\n"
                             "All sub-clusters will also be deleted.");

    bool confirmed = m_view->askForConfirmation("Delete Cluster", message);
    if (!confirmed) {
        return;
    }

    if (m_projectManager->deleteCluster(clusterId)) {
        qDebug() << "MainPresenter: Cluster deleted successfully:" << clusterId;
        m_view->updateStatusBar("Cluster deleted successfully");
    } else {
        showError("Delete Cluster", "Failed to delete cluster.");
    }
}

void MainPresenter::handleLoadScan(const QString& scanId) {
    if (!m_view) {
        return;
    }

    // Delegate to view's load manager through interface
    m_view->loadScan(scanId);
    m_view->updateStatusBar(QString("Loading scan: %1").arg(scanId));
}

void MainPresenter::handleUnloadScan(const QString& scanId) {
    if (!m_view) {
        return;
    }

    // Delegate to view's load manager through interface
    m_view->unloadScan(scanId);
    m_view->updateStatusBar(QString("Unloading scan: %1").arg(scanId));
}

void MainPresenter::handleLoadCluster(const QString& clusterId) {
    if (!m_view) {
        return;
    }

    // Delegate to view's load manager through interface
    m_view->loadCluster(clusterId);
    m_view->updateStatusBar(QString("Loading cluster: %1").arg(clusterId));
}

void MainPresenter::handleUnloadCluster(const QString& clusterId) {
    if (!m_view) {
        return;
    }

    // Delegate to view's load manager through interface
    m_view->unloadCluster(clusterId);
    m_view->updateStatusBar(QString("Unloading cluster: %1").arg(clusterId));
}

void MainPresenter::handleViewPointCloud(const QString& itemId, const QString& itemType) {
    if (!m_view) {
        return;
    }

    // Delegate to view's point cloud viewing mechanism
    m_view->viewPointCloud(itemId, itemType);
    m_view->updateStatusBar(QString("Viewing %1: %2").arg(itemType, itemId));
}

void MainPresenter::handleDeleteScan(const QString& scanId, bool deletePhysicalFile) {
    if (!m_projectManager) {
        showError("Delete Scan", "Project manager is not available.");
        return;
    }

    // Get scan info to check import type and show appropriate confirmation
    auto* sqliteManager = m_projectManager->getSQLiteManager();
    if (!sqliteManager) {
        showError("Delete Scan", "Database manager is not available.");
        return;
    }

    ScanInfo scan = sqliteManager->getScanById(scanId);
    if (!scan.isValid()) {
        showError("Delete Scan", "Could not retrieve scan information.");
        return;
    }

    QString message = QString("Are you sure you want to delete scan '%1'?\nThis action cannot be undone.")
                     .arg(scan.scanName);

    // For copied/moved scans, ask about deleting physical files
    bool shouldDeletePhysicalFile = deletePhysicalFile;
    if (scan.importType == "COPIED" || scan.importType == "MOVED") {
        QString extendedMessage = message + "\n\nAlso delete the physical scan file from the project folder?";
        shouldDeletePhysicalFile = m_view->askForConfirmation("Delete Scan", extendedMessage);
        if (!shouldDeletePhysicalFile) {
            // Still ask for basic confirmation
            bool basicConfirm = m_view->askForConfirmation("Delete Scan", message);
            if (!basicConfirm) {
                return;
            }
        }
    } else {
        bool confirmed = m_view->askForConfirmation("Delete Scan", message);
        if (!confirmed) {
            return;
        }
    }

    // Delegate to view's scan deletion mechanism
    m_view->deleteScan(scanId, shouldDeletePhysicalFile);
    m_view->updateStatusBar(QString("Deleting scan: %1").arg(scan.scanName));
}

void MainPresenter::handleBatchOperation(const QString& operation, const QStringList& scanIds) {
    if (!m_view || scanIds.isEmpty()) {
        return;
    }

    // Delegate to view's batch operation mechanism
    m_view->performBatchOperation(operation, scanIds);
    m_view->updateStatusBar(QString("Performing %1 operation on %2 scans").arg(operation).arg(scanIds.size()));
}
