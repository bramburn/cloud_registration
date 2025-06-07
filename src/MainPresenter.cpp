#include "MainPresenter.h"
#include "IMainView.h"
#include "IPointCloudViewer.h"
#include "IE57Parser.h"
#include "IE57Writer.h"
#include "projectmanager.h"
#include "project.h"
#include "loadingsettings.h"
#include "loadingsettingsdialog.h"
#include "lasheadermetadata.h"
#include "lasparser.h"

#include <QThread>
#include <QFileInfo>
#include <QDebug>
#include <QTimer>

MainPresenter::MainPresenter(IMainView* view, 
                           IE57Parser* e57Parser,
                           IE57Writer* e57Writer,
                           QObject* parent)
    : QObject(parent)
    , m_view(view)
    , m_e57Parser(e57Parser)
    , m_e57Writer(e57Writer)
    , m_projectManager(nullptr)
    , m_currentProject(nullptr)
    , m_isLoading(false)
    , m_currentScanCount(0)
    , m_parserThread(nullptr)
    , m_workerParser(nullptr)
    , m_currentPointCount(0)
{
    qDebug() << "MainPresenter constructor started";
    
    if (!m_view) {
        qCritical() << "MainPresenter: View cannot be null";
        throw std::invalid_argument("View cannot be null");
    }
    
    qDebug() << "MainPresenter constructor completed";
}

MainPresenter::~MainPresenter()
{
    qDebug() << "MainPresenter destructor started";
    
    // Cleanup any active parsing threads
    if (m_parserThread && m_parserThread->isRunning()) {
        m_parserThread->quit();
        m_parserThread->wait(3000);
    }
    
    qDebug() << "MainPresenter destructor completed";
}

void MainPresenter::initialize()
{
    qDebug() << "MainPresenter::initialize() started";
    
    setupConnections();
    
    // Initialize view state
    m_view->setStatusReady();
    m_view->updateWindowTitle();
    m_view->enableProjectActions(false);
    m_view->showProjectHub();
    
    qDebug() << "MainPresenter::initialize() completed";
}

void MainPresenter::setProjectManager(ProjectManager* projectManager)
{
    m_projectManager = projectManager;
    if (m_projectManager) {
        connect(m_projectManager, &ProjectManager::errorOccurred,
                this, &MainPresenter::onProjectManagerError);
    }
}



void MainPresenter::setupConnections()
{
    qDebug() << "MainPresenter::setupConnections() started";
    
    // E57 Parser connections
    if (m_e57Parser) {
        connect(m_e57Parser, &IE57Parser::progressUpdated,
                this, &MainPresenter::handleParsingProgressUpdated);
        connect(m_e57Parser, &IE57Parser::parsingFinished,
                this, &MainPresenter::handleParsingFinished);
        connect(m_e57Parser, &IE57Parser::scanMetadataAvailable,
                this, &MainPresenter::handleScanMetadataReceived);
        connect(m_e57Parser, &IE57Parser::intensityDataExtracted,
                this, &MainPresenter::handleIntensityDataReceived);
        connect(m_e57Parser, &IE57Parser::colorDataExtracted,
                this, &MainPresenter::handleColorDataReceived);
    }
    
    qDebug() << "MainPresenter::setupConnections() completed";
}

void MainPresenter::handleNewProject()
{
    qDebug() << "MainPresenter::handleNewProject() called";
    
    QString projectName, projectPath;
    if (m_view->showCreateProjectDialog(projectName, projectPath)) {
        try {
            if (m_projectManager) {
                QString fullProjectPath = m_projectManager->createProject(projectName, projectPath);
                if (!fullProjectPath.isEmpty()) {
                    handleProjectOpened(fullProjectPath);
                }
            }
        } catch (const std::exception& e) {
            m_view->displayErrorMessage("Project Creation Failed", e.what());
        }
    }
}

void MainPresenter::handleOpenProject()
{
    qDebug() << "MainPresenter::handleOpenProject() called";
    
    QString projectPath = m_view->showOpenProjectDialog();
    if (!projectPath.isEmpty()) {
        if (m_projectManager && m_projectManager->isValidProject(projectPath)) {
            handleProjectOpened(projectPath);
        } else {
            m_view->displayWarningMessage("Invalid Project", "Selected folder is not a valid project.");
        }
    }
}

void MainPresenter::handleCloseProject()
{
    qDebug() << "MainPresenter::handleCloseProject() called";
    
    delete m_currentProject;
    m_currentProject = nullptr;
    
    // Update view state
    m_view->updateWindowTitle();
    m_view->enableProjectActions(false);
    m_view->showProjectHub();
    m_view->refreshScanList();
    m_view->updateStatusBar("Project closed");
}

void MainPresenter::handleProjectOpened(const QString& projectPath)
{
    qDebug() << "MainPresenter::handleProjectOpened() called with path:" << projectPath;
    
    try {
        if (!m_projectManager) {
            throw std::runtime_error("Project manager not available");
        }
        
        // Load project info
        auto projectInfo = m_projectManager->loadProjectLegacy(projectPath);
        
        // Create Project object
        delete m_currentProject;
        m_currentProject = new Project(projectInfo, this);
        
        // Check if project has scans and show/hide guidance accordingly
        bool hasScans = m_projectManager->hasScans(projectPath);
        m_view->showImportGuidance(!hasScans);
        
        // Update view state
        m_view->transitionToProjectView(projectPath);
        m_view->enableProjectActions(true);
        m_view->updateWindowTitle();
        
    } catch (const std::exception& e) {
        m_view->displayErrorMessage("Project Load Error",
                                   QString("Failed to load project: %1").arg(e.what()));
    }
}

void MainPresenter::handleImportScans()
{
    qDebug() << "MainPresenter::handleImportScans() called";

    if (!m_currentProject) {
        qWarning() << "No current project for scan import";
        return;
    }

    // Simplified implementation - just show a placeholder message
    m_view->displayInfoMessage("Import Scans", "Scan import functionality will be implemented in a future sprint.");
}

void MainPresenter::handleScanActivated(const QString& scanId)
{
    qDebug() << "MainPresenter::handleScanActivated() called with scanId:" << scanId;

    // Simplified implementation - just show a placeholder message
    m_view->displayInfoMessage("Scan Activation",
        QString("Scan activation for ID %1 will be implemented in a future sprint.").arg(scanId));
}

void MainPresenter::handleOpenFile()
{
    qDebug() << "MainPresenter::handleOpenFile() called";
    
    if (m_isLoading) {
        m_view->displayInfoMessage("Loading", "Please wait for the current file to finish loading.");
        return;
    }
    
    QString fileName = m_view->showOpenFileDialog(
        "Open Point Cloud File",
        "Point Cloud Files (*.e57 *.las);;E57 Files (*.e57);;LAS Files (*.las);;All Files (*)"
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Show settings dialog
    if (!m_view->showLoadingSettingsDialog()) {
        return;
    }
    
    // Start loading process
    QFileInfo fileInfo(fileName);
    m_currentFilePath = fileName;
    m_currentFileName = fileInfo.baseName();
    setLoadingState(true);
    
    // Update status and viewer state
    m_view->setStatusLoading(m_currentFileName);
    if (auto viewer = m_view->getViewer()) {
        viewer->onLoadingStarted();
    }
    
    // Show progress dialog
    m_view->showProgressDialog("Loading Point Cloud", "Preparing to load file...");
    
    // Determine file type and start appropriate parsing
    QString extension = fileInfo.suffix().toLower();
    LoadingSettings settings; // Get from dialog in real implementation
    
    if (extension == "e57") {
        startE57Parsing(fileName, settings);
    } else if (extension == "las") {
        startLasParsing(fileName, settings);
    } else {
        setLoadingState(false);
        m_view->hideProgressDialog();
        m_view->displayWarningMessage("Error", "Unsupported file format");
    }
}

void MainPresenter::handleLoadingSettings()
{
    qDebug() << "MainPresenter::handleLoadingSettings() called";
    m_view->showLoadingSettingsDialog();
}

void MainPresenter::handleLoadingFinished(bool success, const QString& message)
{
    qDebug() << "MainPresenter::handleLoadingFinished() called with success:" << success;

    cleanupProgressDialog();
    updateUIAfterParsing(success, message);
}

void MainPresenter::handleParsingProgressUpdated(int percentage, const QString& stage)
{
    qDebug() << "MainPresenter::handleParsingProgressUpdated() called:" << percentage << "%" << stage;

    m_view->updateProgressDialog(percentage, stage);
    m_view->updateLoadingProgress(percentage, stage);

    if (auto viewer = m_view->getViewer()) {
        viewer->onLoadingProgress(percentage, stage);
    }
}

void MainPresenter::handleParsingFinished(bool success, const QString& message, const std::vector<float>& points)
{
    qDebug() << "MainPresenter::handleParsingFinished() called with success:" << success << "points:" << points.size();

    setLoadingState(false);
    cleanupProgressDialog();

    if (success && !points.empty()) {
        m_currentPointCount = static_cast<int>(points.size() / 6); // Assuming XYZ RGB format

        // Load points into viewer
        if (auto viewer = m_view->getViewer()) {
            viewer->loadPointCloud(points);
            viewer->setState(ViewerState::DisplayingData, "Point cloud loaded successfully");
        }

        m_view->setStatusLoadSuccess(m_currentFileName, m_currentPointCount);

    } else if (success && points.empty()) {
        qDebug() << "Points vector is empty - this might be due to 'Header-Only' mode or a parsing error";
        m_view->setStatusLoadSuccess(m_currentFileName, 0);

    } else {
        qDebug() << "Parsing failed - clearing viewer to prevent stale data display";
        m_view->setStatusLoadFailed(m_currentFileName, message);

        if (auto viewer = m_view->getViewer()) {
            viewer->clearPointCloud();
            viewer->setState(ViewerState::LoadFailed, message);
        }
    }

    updateUIAfterParsing(success, message);
}

void MainPresenter::handleLasHeaderParsed(const LasHeaderMetadata& metadata)
{
    qDebug() << "MainPresenter::handleLasHeaderParsed() called";

    // Update progress with header information
    QString headerInfo = QString("LAS Header: %1 points, version %2.%3")
                        .arg(metadata.numberOfPointRecords)
                        .arg(metadata.versionMajor)
                        .arg(metadata.versionMinor);

    m_view->updateProgressDialog(10, headerInfo);
}

void MainPresenter::handleScanMetadataReceived(int scanCount, const QStringList& scanNames)
{
    qDebug() << "MainPresenter::handleScanMetadataReceived() called with" << scanCount << "scans";

    m_currentScanCount = scanCount;
    m_currentScanNames = scanNames;

    for (int i = 0; i < scanNames.size(); ++i) {
        qDebug() << "  Scan" << i << ":" << scanNames[i];
    }

    if (scanCount > 1) {
        QString statusMsg = QString("Multi-scan E57 file detected (%1 scans), loading first scan...").arg(scanCount);
        m_view->updateProgressDialog(15, statusMsg);
    }
}

void MainPresenter::handleIntensityDataReceived(const std::vector<float>& intensityValues)
{
    qDebug() << "MainPresenter::handleIntensityDataReceived() called with" << intensityValues.size() << "values";

    m_currentIntensityData = intensityValues;

    // Update viewer with intensity data if available
    if (auto viewer = m_view->getViewer()) {
        viewer->setRenderWithIntensity(true);
    }
}

void MainPresenter::handleColorDataReceived(const std::vector<uint8_t>& colorValues)
{
    qDebug() << "MainPresenter::handleColorDataReceived() called with" << colorValues.size() << "values";

    m_currentColorData = colorValues;

    // Update viewer with color data if available
    if (auto viewer = m_view->getViewer()) {
        viewer->setRenderWithColor(true);
    }
}

// View control operations
void MainPresenter::handleTopViewClicked()
{
    qDebug() << "MainPresenter::handleTopViewClicked() called";

    if (auto viewer = m_view->getViewer()) {
        viewer->setTopView();
        m_view->setStatusViewChanged("Top");
    }
}

void MainPresenter::handleLeftViewClicked()
{
    qDebug() << "MainPresenter::handleLeftViewClicked() called";

    if (auto viewer = m_view->getViewer()) {
        viewer->setLeftView();
        m_view->setStatusViewChanged("Left");
    }
}

void MainPresenter::handleRightViewClicked()
{
    qDebug() << "MainPresenter::handleRightViewClicked() called";

    if (auto viewer = m_view->getViewer()) {
        viewer->setRightView();
        m_view->setStatusViewChanged("Right");
    }
}

void MainPresenter::handleBottomViewClicked()
{
    qDebug() << "MainPresenter::handleBottomViewClicked() called";

    if (auto viewer = m_view->getViewer()) {
        viewer->setBottomView();
        m_view->setStatusViewChanged("Bottom");
    }
}

void MainPresenter::handleFrontViewClicked()
{
    qDebug() << "MainPresenter::handleFrontViewClicked() called";

    if (auto viewer = m_view->getViewer()) {
        viewer->setFrontView();
        m_view->setStatusViewChanged("Front");
    }
}

void MainPresenter::handleBackViewClicked()
{
    qDebug() << "MainPresenter::handleBackViewClicked() called";

    if (auto viewer = m_view->getViewer()) {
        viewer->setBackView();
        m_view->setStatusViewChanged("Back");
    }
}

void MainPresenter::handleIsometricViewClicked()
{
    qDebug() << "MainPresenter::handleIsometricViewClicked() called";

    if (auto viewer = m_view->getViewer()) {
        viewer->setIsometricView();
        m_view->setStatusViewChanged("Isometric");
    }
}

void MainPresenter::handleMemoryUsageChanged(size_t totalBytes)
{
    qDebug() << "MainPresenter::handleMemoryUsageChanged() called with" << totalBytes << "bytes";
    m_view->updateMemoryDisplay(totalBytes);
}

void MainPresenter::handleStatsUpdated(float fps, int visiblePoints)
{
    qDebug() << "MainPresenter::handleStatsUpdated() called with FPS:" << fps << "visible points:" << visiblePoints;
    m_view->updatePerformanceStats(fps, visiblePoints);
}

void MainPresenter::handleProgressUpdated(const QString& operationId, int percentage, const QString& stage)
{
    qDebug() << "MainPresenter::handleProgressUpdated() called for operation:" << operationId;

    if (operationId == m_currentOperationId) {
        m_view->updateProgressDialog(percentage, stage);
        m_view->updateLoadingProgress(percentage, stage);
    }
}

void MainPresenter::handleProgressCompleted(const QString& operationId, bool success, const QString& message)
{
    qDebug() << "MainPresenter::handleProgressCompleted() called for operation:" << operationId << "success:" << success;

    if (operationId == m_currentOperationId) {
        m_view->hideProgressDialog();
        updateStatusForOperation(operationId, success, message);
        m_currentOperationId.clear();
    }
}

void MainPresenter::handleProgressCancelled(const QString& operationId)
{
    qDebug() << "MainPresenter::handleProgressCancelled() called for operation:" << operationId;

    if (operationId == m_currentOperationId) {
        // Cancel any active parsing
        if (m_e57Parser) {
            m_e57Parser->cancelParsing();
        }

        setLoadingState(false);
        m_view->hideProgressDialog();
        m_view->updateStatusBar("Operation cancelled");
        m_currentOperationId.clear();
    }
}

void MainPresenter::handleApplicationShutdown()
{
    qDebug() << "MainPresenter::handleApplicationShutdown() called";

    // Cancel any active operations
    if (!m_currentOperationId.isEmpty()) {
        handleProgressCancelled(m_currentOperationId);
    }

    // Close current project
    if (m_currentProject) {
        handleCloseProject();
    }

    // Cleanup resources
    m_view->cleanupResources();
}

// Private slots
void MainPresenter::onParsingThreadFinished()
{
    qDebug() << "MainPresenter::onParsingThreadFinished() called";

    if (m_parserThread) {
        m_parserThread->deleteLater();
        m_parserThread = nullptr;
    }

    if (m_workerParser) {
        m_workerParser->deleteLater();
        m_workerParser = nullptr;
    }
}

void MainPresenter::onProjectManagerError(const QString& error)
{
    qCritical() << "MainPresenter::onProjectManagerError() called with error:" << error;
    handleCriticalError("Project Management", error);
}



// Private helper methods
void MainPresenter::cleanupParsingThread(QObject* worker)
{
    qDebug() << "MainPresenter::cleanupParsingThread() called";

    if (worker) {
        worker->deleteLater();
    }

    if (m_parserThread) {
        m_parserThread->quit();
        m_parserThread->wait(3000);
        m_parserThread->deleteLater();
        m_parserThread = nullptr;
    }

    m_workerParser = nullptr;
}

void MainPresenter::updateUIAfterParsing(bool success, const QString& message)
{
    qDebug() << "MainPresenter::updateUIAfterParsing() called with success:" << success;

    setLoadingState(false);

    if (success) {
        m_view->enableViewControls(true);
        m_view->updateViewControlsState();
    } else {
        m_view->enableViewControls(false);
    }
}

void MainPresenter::cleanupProgressDialog()
{
    qDebug() << "MainPresenter::cleanupProgressDialog() called";
    m_view->hideProgressDialog();
}

void MainPresenter::startE57Parsing(const QString& filePath, const LoadingSettings& settings)
{
    qDebug() << "MainPresenter::startE57Parsing() called with file:" << filePath;

    if (!m_e57Parser) {
        handleCriticalError("E57 Parsing", "E57 parser not available");
        return;
    }

    // Create worker thread
    m_parserThread = new QThread(this);

    // Move parser to thread and start parsing
    m_e57Parser->moveToThread(m_parserThread);

    connect(m_parserThread, &QThread::started, [this, filePath, settings]() {
        m_e57Parser->startParsing(filePath, settings);
    });

    connect(m_parserThread, &QThread::finished, this, &MainPresenter::onParsingThreadFinished);

    m_parserThread->start();
}

void MainPresenter::startLasParsing(const QString& filePath, const LoadingSettings& settings)
{
    qDebug() << "MainPresenter::startLasParsing() called with file:" << filePath;

    // Create LAS parser worker
    auto lasParser = new LasParser();
    m_workerParser = lasParser;
    m_parserThread = new QThread(this);

    lasParser->moveToThread(m_parserThread);

    // Connect signals
    connect(lasParser, &LasParser::progressUpdated,
            this, &MainPresenter::handleParsingProgressUpdated);
    connect(lasParser, &LasParser::parsingFinished,
            this, &MainPresenter::handleParsingFinished);
    connect(lasParser, &LasParser::headerParsed,
            this, &MainPresenter::handleLasHeaderParsed);

    connect(m_parserThread, &QThread::started, [lasParser, filePath, settings]() {
        lasParser->parseFile(filePath, settings);
    });

    connect(m_parserThread, &QThread::finished, this, &MainPresenter::onParsingThreadFinished);

    m_parserThread->start();
}

void MainPresenter::validateProjectState()
{
    qDebug() << "MainPresenter::validateProjectState() called";

    if (m_currentProject) {
        // Validate project is still accessible
        QString projectPath = m_currentProject->projectPath();
        if (m_projectManager && !m_projectManager->isValidProject(projectPath)) {
            handleWarning("Project Validation", "Current project appears to be invalid or inaccessible");
        }
    }
}

void MainPresenter::updateWindowTitleForProject()
{
    qDebug() << "MainPresenter::updateWindowTitleForProject() called";

    if (m_currentProject) {
        QString title = QString("Point Cloud Viewer - %1").arg(m_currentProject->projectName());
        m_view->setWindowTitle(title);
    } else {
        m_view->setWindowTitle("Point Cloud Viewer");
    }
}

void MainPresenter::setLoadingState(bool isLoading)
{
    qDebug() << "MainPresenter::setLoadingState() called with isLoading:" << isLoading;

    m_isLoading = isLoading;
    m_view->setLoadingState(isLoading);

    // Update UI controls based on loading state
    m_view->enableViewControls(!isLoading);
}

void MainPresenter::updateStatusForOperation(const QString& operation, bool success, const QString& details)
{
    qDebug() << "MainPresenter::updateStatusForOperation() called for operation:" << operation << "success:" << success;

    QString statusMessage;
    if (success) {
        statusMessage = QString("%1 completed successfully").arg(operation);
        if (!details.isEmpty()) {
            statusMessage += QString(" - %1").arg(details);
        }
    } else {
        statusMessage = QString("%1 failed").arg(operation);
        if (!details.isEmpty()) {
            statusMessage += QString(": %1").arg(details);
        }
    }

    m_view->updateStatusBar(statusMessage);
}

void MainPresenter::handleCriticalError(const QString& operation, const QString& error)
{
    qCritical() << "MainPresenter::handleCriticalError() called for operation:" << operation << "error:" << error;

    setLoadingState(false);
    m_view->hideProgressDialog();
    m_view->displayErrorMessage(QString("%1 Error").arg(operation), error);
    m_view->updateStatusBar(QString("%1 failed: %2").arg(operation, error));
}

void MainPresenter::handleWarning(const QString& operation, const QString& warning)
{
    qWarning() << "MainPresenter::handleWarning() called for operation:" << operation << "warning:" << warning;

    m_view->displayWarningMessage(QString("%1 Warning").arg(operation), warning);
    m_view->updateStatusBar(QString("%1 warning: %2").arg(operation, warning));
}
