#include "mainwindow.h"
#include "projecthubwidget.h"
#include "sidebarwidget.h"
#include "createprojectdialog.h"
#include "projectmanager.h"
#include "project.h"
#include "pointcloudviewerwidget.h"
#include "e57parser.h"
#include "lasparser.h"
#include "loadingsettingsdialog.h"
#include "lasheadermetadata.h"
#include "loadingsettings.h"
#include "performance_profiler.h"
// Sprint 1.2: Scan Import functionality
#include "scanimportdialog.h"
#include "scanimportmanager.h"
#include "sqlitemanager.h"
#include <QApplication>
#include <QThread>
#include <QTimer>
#include <QFileInfo>
#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralStack(nullptr)
    , m_projectHub(nullptr)
    , m_projectView(nullptr)
    , m_projectSplitter(nullptr)
    , m_sidebar(nullptr)
    , m_mainContentArea(nullptr)
    , m_viewer(nullptr)
    , m_progressDialog(nullptr)
    , m_projectManager(new ProjectManager(this))
    , m_currentProject(nullptr)
    , m_newProjectAction(nullptr)
    , m_openProjectAction(nullptr)
    , m_closeProjectAction(nullptr)
    , m_importScansAction(nullptr)
    , m_loadingSettingsAction(nullptr)
    , m_topViewAction(nullptr)
    , m_leftViewAction(nullptr)
    , m_rightViewAction(nullptr)
    , m_bottomViewAction(nullptr)
    , m_importGuidanceWidget(nullptr)
    , m_importGuidanceButton(nullptr)
    , m_e57Parser(nullptr)
    , m_lasParser(nullptr)
    , m_parserThread(nullptr)
    , m_isLoading(false)
    , m_statusLabel(nullptr)
    , m_permanentStatusLabel(nullptr)
    , m_currentPointCount(0)
{
    qDebug() << "MainWindow constructor started";

    try {
        qDebug() << "Setting up UI...";
        setupUI();
        qDebug() << "UI setup completed";

        qDebug() << "Setting up menu bar...";
        setupMenuBar();
        qDebug() << "Menu bar setup completed";

        qDebug() << "Setting up status bar...";
        setupStatusBar();
        qDebug() << "Status bar setup completed";

        // Initialize legacy parsers for point cloud loading
        qDebug() << "Initializing parsers...";
        m_e57Parser = new E57Parser(this);
        m_lasParser = new LasParser(this);
        qDebug() << "Parsers initialized";

        // Sprint 1.2: Connect project manager signals
        connect(m_projectManager, &ProjectManager::scansImported,
                this, &MainWindow::onScansImported);
        connect(m_projectManager, &ProjectManager::projectScansChanged,
                this, [this]() {
                    if (m_sidebar) {
                        m_sidebar->refreshFromDatabase();
                    }
                });

        // Set window properties
        qDebug() << "Setting window properties...";
        updateWindowTitle();
        setMinimumSize(1000, 700);
        resize(1200, 800);
        qDebug() << "Window properties set";

        // Start with Project Hub
        m_centralStack->setCurrentWidget(m_projectHub);
        setStatusReady();

        qDebug() << "MainWindow constructor completed successfully";
    } catch (const std::exception& e) {
        qCritical() << "Exception in MainWindow constructor:" << e.what();
        throw;
    } catch (...) {
        qCritical() << "Unknown exception in MainWindow constructor";
        throw;
    }
}

MainWindow::~MainWindow()
{
    delete m_currentProject;
}

void MainWindow::setupUI()
{
    m_centralStack = new QStackedWidget(this);
    setCentralWidget(m_centralStack);

    // Create Project Hub
    m_projectHub = new ProjectHubWidget(this);
    connect(m_projectHub, &ProjectHubWidget::projectOpened,
            this, &MainWindow::onProjectOpened);

    // Create Project View
    m_projectView = new QWidget();
    m_projectSplitter = new QSplitter(Qt::Horizontal, m_projectView);

    // Create Sidebar
    m_sidebar = new SidebarWidget(this);
    m_sidebar->setMinimumWidth(250);
    m_sidebar->setMaximumWidth(400);

    // Create main content area with point cloud viewer
    m_mainContentArea = new QWidget();
    auto *contentLayout = new QVBoxLayout(m_mainContentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    // Create legacy point cloud viewer
    m_viewer = new PointCloudViewerWidget(this);
    contentLayout->addWidget(m_viewer);

    // Setup splitter
    m_projectSplitter->addWidget(m_sidebar);
    m_projectSplitter->addWidget(m_mainContentArea);
    m_projectSplitter->setStretchFactor(0, 0); // Sidebar doesn't stretch
    m_projectSplitter->setStretchFactor(1, 1); // Main content stretches

    auto *projectLayout = new QHBoxLayout(m_projectView);
    projectLayout->setContentsMargins(0, 0, 0, 0);
    projectLayout->addWidget(m_projectSplitter);

    // Add both views to stack
    m_centralStack->addWidget(m_projectHub);
    m_centralStack->addWidget(m_projectView);
}

void MainWindow::setupMenuBar()
{
    // Create File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");

    m_newProjectAction = fileMenu->addAction("&New Project...");
    m_newProjectAction->setShortcut(QKeySequence::New);
    m_newProjectAction->setStatusTip("Create a new project");
    connect(m_newProjectAction, &QAction::triggered, this, &MainWindow::onFileNewProject);

    m_openProjectAction = fileMenu->addAction("&Open Project...");
    m_openProjectAction->setShortcut(QKeySequence::Open);
    m_openProjectAction->setStatusTip("Open an existing project");
    connect(m_openProjectAction, &QAction::triggered, this, &MainWindow::onFileOpenProject);

    fileMenu->addSeparator();

    m_closeProjectAction = fileMenu->addAction("&Close Project");
    m_closeProjectAction->setEnabled(false);
    m_closeProjectAction->setStatusTip("Close the current project");
    connect(m_closeProjectAction, &QAction::triggered, this, &MainWindow::closeCurrentProject);

    fileMenu->addSeparator();

    // Sprint 1.2: Import Scans action
    m_importScansAction = fileMenu->addAction("&Import Scans...");
    m_importScansAction->setShortcut(QKeySequence("Ctrl+I"));
    m_importScansAction->setEnabled(false);
    m_importScansAction->setStatusTip("Import scan files into the current project");
    connect(m_importScansAction, &QAction::triggered, this, &MainWindow::onImportScans);

    fileMenu->addSeparator();

    QAction *openFileAction = new QAction("Open Point Cloud &File...", this);
    openFileAction->setShortcut(QKeySequence("Ctrl+Shift+O"));
    openFileAction->setStatusTip("Open a point cloud file (E57 or LAS)");
    connect(openFileAction, &QAction::triggered, this, &MainWindow::onOpenFileClicked);
    fileMenu->addAction(openFileAction);

    // Add Loading Settings action
    m_loadingSettingsAction = new QAction("Loading &Settings...", this);
    m_loadingSettingsAction->setStatusTip("Configure point cloud loading options");
    connect(m_loadingSettingsAction, &QAction::triggered, this, &MainWindow::onLoadingSettingsTriggered);
    fileMenu->addAction(m_loadingSettingsAction);

    fileMenu->addSeparator();

    QAction *exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setStatusTip("Exit the application");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);

    // Create View menu
    QMenu *viewMenu = menuBar()->addMenu("&View");

    m_topViewAction = new QAction("&Top View", this);
    m_topViewAction->setShortcut(QKeySequence("Ctrl+1"));
    m_topViewAction->setStatusTip("Switch to top view");
    connect(m_topViewAction, &QAction::triggered, this, &MainWindow::onTopViewClicked);
    viewMenu->addAction(m_topViewAction);

    m_leftViewAction = new QAction("&Left View", this);
    m_leftViewAction->setShortcut(QKeySequence("Ctrl+2"));
    m_leftViewAction->setStatusTip("Switch to left view");
    connect(m_leftViewAction, &QAction::triggered, this, &MainWindow::onLeftViewClicked);
    viewMenu->addAction(m_leftViewAction);

    m_rightViewAction = new QAction("&Right View", this);
    m_rightViewAction->setShortcut(QKeySequence("Ctrl+3"));
    m_rightViewAction->setStatusTip("Switch to right view");
    connect(m_rightViewAction, &QAction::triggered, this, &MainWindow::onRightViewClicked);
    viewMenu->addAction(m_rightViewAction);

    m_bottomViewAction = new QAction("&Bottom View", this);
    m_bottomViewAction->setShortcut(QKeySequence("Ctrl+4"));
    m_bottomViewAction->setStatusTip("Switch to bottom view");
    connect(m_bottomViewAction, &QAction::triggered, this, &MainWindow::onBottomViewClicked);
    viewMenu->addAction(m_bottomViewAction);

    // Create Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");

    QAction *aboutAction = new QAction("&About", this);
    aboutAction->setStatusTip("Show information about this application");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "About Cloud Registration",
            "Cloud Registration v1.0\n\n"
            "An open-source point cloud registration application\n"
            "Built with Qt6 and OpenGL");
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupStatusBar()
{
    // Sprint 2.3: Create status bar widgets
    m_statusLabel = new QLabel(this);
    m_statusLabel->setMinimumWidth(300);

    m_permanentStatusLabel = new QLabel(this);
    m_permanentStatusLabel->setAlignment(Qt::AlignRight);

    // Add to status bar
    statusBar()->addWidget(m_statusLabel, 1); // Stretch factor 1
    statusBar()->addPermanentWidget(m_permanentStatusLabel);

    // Setup status bar style
    statusBar()->setStyleSheet(
        "QStatusBar { border-top: 1px solid #cccccc; }"
        "QStatusBar::item { border: none; }"
    );
}

void MainWindow::onOpenFileClicked()
{
    if (m_isLoading) {
        QMessageBox::information(this, "Loading", "Please wait for the current file to finish loading.");
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Open Point Cloud File",
        QString(),
        "Point Cloud Files (*.e57 *.las);;E57 Files (*.e57);;LAS Files (*.las);;All Files (*)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    // Task 1.4.3.1-1.4.3.5: Show settings dialog with file type configuration
    LoadingSettingsDialog settingsDialog(this);
    QFileInfo fileInfo(fileName);
    settingsDialog.configureForFileType(fileInfo.suffix());

    if (settingsDialog.exec() != QDialog::Accepted) {
        return;
    }

    LoadingSettings loadingSettings = settingsDialog.getSettings();

    m_currentFilePath = fileName;
    m_currentFileName = fileInfo.baseName();
    m_isLoading = true;

    // Sprint 2.3: Update status and viewer state
    setStatusLoading(m_currentFileName);
    m_viewer->onLoadingStarted();

    // Update UI - disable relevant actions during loading
    // Note: No longer have m_openFileButton in project-based UI

    // Determine file type
    QString extension = fileInfo.suffix().toLower();

    // Sprint 2.3: Setup progress dialog
    if (!m_progressDialog) {
        m_progressDialog = new QProgressDialog(this);
        m_progressDialog->setWindowModality(Qt::WindowModal);
        m_progressDialog->setMinimumDuration(500); // Show after 500ms
        m_progressDialog->setAutoClose(true);
        m_progressDialog->setAutoReset(false);
    }

    m_progressDialog->setLabelText(QString("Loading %1...").arg(m_currentFileName));
    m_progressDialog->setCancelButtonText("Cancel");
    m_progressDialog->setRange(0, 100);
    m_progressDialog->setValue(0);
    m_progressDialog->show();

    // Create worker thread
    m_parserThread = new QThread(this);

    // Create parser instance for the worker thread
    QObject* workerParser = nullptr;
    if (extension == "e57") {
        E57Parser* e57Worker = new E57Parser();
        e57Worker->moveToThread(m_parserThread);
        workerParser = e57Worker;

        // Connect signals
        connect(m_parserThread, &QThread::started, e57Worker, [=]() {
            e57Worker->startParsing(m_currentFilePath);
        });
        connect(e57Worker, &E57Parser::progressUpdated, this, &MainWindow::onParsingProgressUpdated, Qt::QueuedConnection);
        connect(e57Worker, &E57Parser::parsingFinished, this, &MainWindow::onParsingFinished, Qt::QueuedConnection);

        // Sprint 2.3: Connect to viewer for visual feedback
        connect(e57Worker, &E57Parser::progressUpdated, m_viewer, &PointCloudViewerWidget::onLoadingProgress, Qt::QueuedConnection);
        connect(e57Worker, &E57Parser::parsingFinished, m_viewer, &PointCloudViewerWidget::onLoadingFinished, Qt::QueuedConnection);

    } else if (extension == "las") {
        LasParser* lasWorker = new LasParser();
        lasWorker->moveToThread(m_parserThread);
        workerParser = lasWorker;

        // Connect signals
        connect(m_parserThread, &QThread::started, lasWorker, [=]() {
            lasWorker->startParsing(m_currentFilePath, loadingSettings);
        });
        connect(lasWorker, &LasParser::progressUpdated, this, &MainWindow::onParsingProgressUpdated, Qt::QueuedConnection);
        connect(lasWorker, &LasParser::parsingFinished, this, &MainWindow::onParsingFinished, Qt::QueuedConnection);
        connect(lasWorker, &LasParser::headerParsed, this, &MainWindow::onLasHeaderParsed, Qt::QueuedConnection);

        // Sprint 2.3: Connect to viewer for visual feedback
        connect(lasWorker, &LasParser::progressUpdated, m_viewer, &PointCloudViewerWidget::onLoadingProgress, Qt::QueuedConnection);
        connect(lasWorker, &LasParser::parsingFinished, m_viewer, &PointCloudViewerWidget::onLoadingFinished, Qt::QueuedConnection);

    } else {
        // Cleanup and show error
        m_isLoading = false;
        m_openFileButton->setEnabled(true);
        if (m_progressDialog) {
            m_progressDialog->close();
            m_progressDialog->deleteLater();
            m_progressDialog = nullptr;
        }
        QMessageBox::warning(this, "Error", "Unsupported file format");
        return;
    }

    // Setup cleanup when thread finishes
    connect(m_parserThread, &QThread::finished, workerParser, &QObject::deleteLater);

    // Start the worker thread
    m_parserThread->start();
}

void MainWindow::onLoadingFinished(bool success, const QString& message)
{
    cleanupProgressDialog();
    updateUIAfterParsing(success, message);
}

void MainWindow::onParsingProgressUpdated(int percentage, const QString &stage)
{
    if (m_progressDialog) {
        m_progressDialog->setValue(percentage);
        m_progressDialog->setLabelText(
            QString("Loading %1... (%2%)").arg(m_currentFileName).arg(percentage)
        );

        // Optional: Update status bar with current stage
        if (!stage.isEmpty()) {
            setStatusLoading(QString("%1 - %2").arg(m_currentFileName, stage));
        }
    }
}

void MainWindow::onParsingFinished(bool success, const QString& message, const std::vector<float>& points)
{
    // Debug logging for data flow verification (User Story 1)
    qDebug() << "=== MainWindow::onParsingFinished ===";
    qDebug() << "Success:" << success;
    qDebug() << "Message:" << message;
    qDebug() << "Points vector size:" << points.size();
    qDebug() << "Number of points:" << (points.size() / 3);

    // Log sample coordinates if we have data
    if (!points.empty() && points.size() >= 9) {
        qDebug() << "First point coordinates:" << points[0] << points[1] << points[2];
        if (points.size() >= 6) {
            size_t midIndex = (points.size() / 6) * 3; // Middle point
            if (midIndex + 2 < points.size()) {
                qDebug() << "Middle point coordinates:" << points[midIndex] << points[midIndex + 1] << points[midIndex + 2];
            }
        }
        size_t lastIndex = points.size() - 3;
        qDebug() << "Last point coordinates:" << points[lastIndex] << points[lastIndex + 1] << points[lastIndex + 2];
    }

    // Clean up resources
    cleanupParsingThread();
    cleanupProgressDialog();

    // Sprint 2.3: Enhanced handling with standardized status bar methods
    if (success && !points.empty()) {
        qDebug() << "Calling m_viewer->loadPointCloud with" << (points.size() / 3) << "points";
        m_currentPointCount = static_cast<int>(points.size() / 3);
        setStatusLoadSuccess(m_currentFileName, m_currentPointCount);

        {
            PROFILE_SECTION("MainWindow::DataTransferToViewer");
            m_viewer->loadPointCloud(points);
        }

        // Connect to viewer's onLoadingFinished slot (handled by viewer's state management)
        // m_viewer->onLoadingFinished(success, message, points); // This is now handled by the viewer's slot

    } else if (success && points.empty()) {
        qDebug() << "Points vector is empty - this might be due to 'Header-Only' mode or a parsing error";
        setStatusLoadSuccess(m_currentFileName, 0);
    } else {
        // Task 1.3.3.2: Clear any stale data from previous successful loads
        qDebug() << "Parsing failed - clearing viewer to prevent stale data display";
        setStatusLoadFailed(m_currentFileName, message);
        m_viewer->clearPointCloud();
    }

    // Update UI (simplified since status bar is now handled above)
    updateUIAfterParsing(success, message);
}

// View control slot implementations
void MainWindow::onTopViewClicked()
{
    if (m_viewer) {
        m_viewer->setTopView();
        setStatusViewChanged("Top");
    }
}

void MainWindow::onLeftViewClicked()
{
    if (m_viewer) {
        m_viewer->setLeftView();
        setStatusViewChanged("Left");
    }
}

void MainWindow::onRightViewClicked()
{
    if (m_viewer) {
        m_viewer->setRightView();
        setStatusViewChanged("Right");
    }
}

void MainWindow::onBottomViewClicked()
{
    if (m_viewer) {
        m_viewer->setBottomView();
        setStatusViewChanged("Bottom");
    }
}

// Helper methods for cleanup and UI updates
void MainWindow::cleanupParsingThread()
{
    if (m_parserThread) {
        m_parserThread->quit();
        m_parserThread->wait();
        m_parserThread->deleteLater();
        m_parserThread = nullptr;
    }
}

void MainWindow::cleanupProgressDialog()
{
    if (m_progressDialog) {
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }
}

void MainWindow::updateUIAfterParsing(bool success, const QString& message)
{
    m_isLoading = false;
    // Note: No longer have m_openFileButton in project-based UI

    // Sprint 2.3: Status bar is now handled by standardized methods in onParsingFinished
    // Only show error dialog for failures
    if (!success) {
        // Task 1.3.3.2: Enhanced error display for LAS parsing failures (Sprint 1.3)
        // Create detailed error dialog with LAS-specific guidance
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle("LAS Parsing Error");
        msgBox.setText("Failed to parse LAS file");

        // Enhanced detailed text with troubleshooting guidance
        QString detailedMessage = QString("%1\n\n"
                                        "Please verify:\n"
                                        "• File is a valid LAS format (versions 1.2-1.4)\n"
                                        "• Point Data Record Format is 0-3\n"
                                        "• File is not corrupted or truncated\n"
                                        "• File has proper read permissions")
                                 .arg(message);

        msgBox.setDetailedText(detailedMessage);
        msgBox.setStandardButtons(QMessageBox::Ok);

        // Make dialog larger to show details better
        msgBox.setStyleSheet("QLabel{min-width: 400px;}");
        msgBox.exec();
    }
}

void MainWindow::onLoadingSettingsTriggered()
{
    LoadingSettingsDialog dialog(this);
    dialog.exec();
}

void MainWindow::onLasHeaderParsed(const LasHeaderMetadata& metadata)
{
    // Sprint 2.3: Update status bar with file metadata using standardized method
    setStatusFileInfo(m_currentFileName,
                     metadata.numberOfPointRecords,
                     metadata.minBounds.x, metadata.minBounds.y, metadata.minBounds.z,
                     metadata.maxBounds.x, metadata.maxBounds.y, metadata.maxBounds.z);

    // Debug logging for enhanced metadata (Sprint 1.3)
    qDebug() << "=== LAS Header Parsed (Sprint 1.3) ===";
    qDebug() << "File:" << QFileInfo(metadata.filePath).fileName();
    qDebug() << "Version:" << metadata.versionMajor << "." << metadata.versionMinor;
    qDebug() << "PDRF:" << metadata.pointDataFormat;
    qDebug() << "Points:" << metadata.numberOfPointRecords;
    qDebug() << "System ID:" << metadata.systemIdentifier;
    qDebug() << "Software:" << metadata.generatingSoftware;
    qDebug() << "BBox: Min(" << metadata.minBounds.x << "," << metadata.minBounds.y << "," << metadata.minBounds.z
             << ") Max(" << metadata.maxBounds.x << "," << metadata.maxBounds.y << "," << metadata.maxBounds.z << ")";
}

// Sprint 2.3: Standardized status bar message methods
void MainWindow::setStatusReady()
{
    if (m_statusLabel) {
        m_statusLabel->setText("Ready to load point cloud files");
    }
    if (m_permanentStatusLabel) {
        m_permanentStatusLabel->clear();
    }
}

void MainWindow::setStatusLoading(const QString &filename)
{
    if (m_statusLabel) {
        m_statusLabel->setText(QString("Loading %1...").arg(filename));
    }
    if (m_permanentStatusLabel) {
        m_permanentStatusLabel->setText("Processing");
    }
}

void MainWindow::setStatusLoadSuccess(const QString &filename, int pointCount)
{
    if (m_statusLabel) {
        m_statusLabel->setText(
            QString("Successfully loaded %1: %2 points")
            .arg(filename)
            .arg(pointCount)
        );
    }
    if (m_permanentStatusLabel) {
        m_permanentStatusLabel->setText("Ready");
    }
}

void MainWindow::setStatusLoadFailed(const QString &filename, const QString &error)
{
    // Extract brief error summary (first sentence or first 50 characters)
    QString briefError = error;
    int dotIndex = error.indexOf('.');
    if (dotIndex != -1 && dotIndex < 50) {
        briefError = error.left(dotIndex);
    } else if (error.length() > 50) {
        briefError = error.left(47) + "...";
    }

    if (m_statusLabel) {
        m_statusLabel->setText(
            QString("Failed to load %1: %2").arg(filename, briefError)
        );
    }
    if (m_permanentStatusLabel) {
        m_permanentStatusLabel->setText("Error");
    }
}

void MainWindow::setStatusFileInfo(const QString &filename, int pointCount,
                                  double minX, double minY, double minZ,
                                  double maxX, double maxY, double maxZ)
{
    if (m_statusLabel) {
        m_statusLabel->setText(
            QString("File: %1, Points: %2, BBox: (%.1f,%.1f,%.1f)-(%.1f,%.1f,%.1f)")
            .arg(filename)
            .arg(pointCount)
            .arg(minX).arg(minY).arg(minZ)
            .arg(maxX).arg(maxY).arg(maxZ)
        );
    }
    if (m_permanentStatusLabel) {
        m_permanentStatusLabel->setText("Header parsed");
    }
}

void MainWindow::setStatusViewChanged(const QString &viewName)
{
    // Temporary message with timeout
    QString tempMessage = QString("Switched to %1 view").arg(viewName);
    statusBar()->showMessage(tempMessage, 3000); // Show for 3 seconds

    // After timeout, message will revert to permanent widget content
}

// Project management methods
void MainWindow::onProjectOpened(const QString &projectPath)
{
    try {
        // Load project using ProjectManager
        auto projectInfo = m_projectManager->loadProject(projectPath);

        // Create Project object
        delete m_currentProject;
        m_currentProject = new Project(projectInfo, this);

        // Sprint 1.2: Check if project has scans and show/hide guidance accordingly
        bool hasScans = m_projectManager->hasScans(projectPath);
        showImportGuidance(!hasScans);

        transitionToProjectView(projectPath);

    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Project Load Error",
                             QString("Failed to load project: %1").arg(e.what()));
    }
}

void MainWindow::transitionToProjectView(const QString &projectPath)
{
    if (m_currentProject) {
        // Sprint 1.2: Set up sidebar with SQLite manager and load scans
        m_sidebar->setSQLiteManager(m_projectManager->getSQLiteManager());
        m_sidebar->setProject(m_currentProject->projectName(), projectPath);

        // Update window title
        updateWindowTitle(m_currentProject->projectName());

        // Enable project-specific menu items
        m_closeProjectAction->setEnabled(true);
        m_importScansAction->setEnabled(true);

        // Switch to project view
        m_centralStack->setCurrentWidget(m_projectView);

        // Update status bar
        statusBar()->showMessage(QString("Project loaded: %1").arg(m_currentProject->projectName()));
    }
}

void MainWindow::updateWindowTitle(const QString &projectName)
{
    QString title = "Cloud Registration";
    if (!projectName.isEmpty()) {
        title += QString(" - %1").arg(projectName);
    }
    setWindowTitle(title);
}

void MainWindow::showProjectHub()
{
    m_centralStack->setCurrentWidget(m_projectHub);
    m_projectHub->refreshRecentProjects();
}

void MainWindow::onFileNewProject()
{
    CreateProjectDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        QString projectName = dialog.projectName().trimmed();
        QString basePath = dialog.projectPath();

        try {
            QString projectPath = m_projectManager->createProject(projectName, basePath);
            if (!projectPath.isEmpty()) {
                onProjectOpened(projectPath);
            }
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Project Creation Failed", e.what());
        }
    }
}

void MainWindow::onFileOpenProject()
{
    QString projectPath = QFileDialog::getExistingDirectory(this, "Select Project Folder");
    if (!projectPath.isEmpty()) {
        if (m_projectManager->isValidProject(projectPath)) {
            onProjectOpened(projectPath);
        } else {
            QMessageBox::warning(this, "Invalid Project", "Selected folder is not a valid project.");
        }
    }
}

void MainWindow::closeCurrentProject()
{
    delete m_currentProject;
    m_currentProject = nullptr;

    // Clear sidebar
    m_sidebar->clearProject();

    // Update window title
    updateWindowTitle();

    // Disable project-specific menu items
    m_closeProjectAction->setEnabled(false);
    m_importScansAction->setEnabled(false);

    // Switch back to Project Hub
    showProjectHub();

    // Update status bar
    statusBar()->showMessage("Project closed", 2000);
}

// Sprint 1.2: Scan Import functionality
void MainWindow::onImportScans()
{
    if (!m_currentProject) {
        return;
    }

    ScanImportDialog dialog(this);
    dialog.setProjectPath(m_currentProject->projectPath());

    if (dialog.exec() == QDialog::Accepted) {
        QStringList files = dialog.selectedFiles();
        ImportMode mode = dialog.importMode();

        if (!files.isEmpty()) {
            auto result = m_projectManager->getScanImportManager()->importScans(
                files, m_currentProject->projectPath(), m_currentProject->projectId(), mode, this);

            if (result.success) {
                // Hide guidance and refresh sidebar
                showImportGuidance(false);
                m_sidebar->refreshFromDatabase();

                statusBar()->showMessage(
                    QString("Successfully imported %1 scan(s)").arg(result.successfulFiles.size()), 3000);
            } else {
                QMessageBox::warning(this, "Import Failed", result.errorMessage);
            }
        }
    }
}

void MainWindow::onScansImported(const QList<ScanInfo> &scans)
{
    // Update sidebar with new scans
    for (const ScanInfo &scan : scans) {
        m_sidebar->addScan(scan);
    }

    // Hide import guidance since we now have scans
    showImportGuidance(false);

    qDebug() << "Imported" << scans.size() << "scans";
}

void MainWindow::showImportGuidance(bool show)
{
    if (!m_importGuidanceWidget) {
        createImportGuidanceWidget();
    }

    m_importGuidanceWidget->setVisible(show);
}

void MainWindow::createImportGuidanceWidget()
{
    m_importGuidanceWidget = new QWidget(m_mainContentArea);
    auto *layout = new QVBoxLayout(m_importGuidanceWidget);
    layout->setAlignment(Qt::AlignCenter);

    auto *iconLabel = new QLabel(this);
    iconLabel->setPixmap(style()->standardIcon(QStyle::SP_FileDialogDetailedView).pixmap(64, 64));
    iconLabel->setAlignment(Qt::AlignCenter);

    auto *titleLabel = new QLabel("Get Started with Your Project", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin: 10px 0;");
    titleLabel->setAlignment(Qt::AlignCenter);

    auto *descLabel = new QLabel("Your project is ready! Start by importing scan files to populate your project.", this);
    descLabel->setStyleSheet("color: #666; margin-bottom: 20px;");
    descLabel->setAlignment(Qt::AlignCenter);
    descLabel->setWordWrap(true);

    m_importGuidanceButton = new QPushButton("Import Scan Files", this);
    m_importGuidanceButton->setStyleSheet(R"(
        QPushButton {
            background-color: #0078d4;
            color: white;
            border: none;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: bold;
            border-radius: 6px;
        }
        QPushButton:hover {
            background-color: #106ebe;
        }
        QPushButton:pressed {
            background-color: #005a9e;
        }
    )");

    connect(m_importGuidanceButton, &QPushButton::clicked, this, &MainWindow::onImportScans);

    layout->addWidget(iconLabel);
    layout->addWidget(titleLabel);
    layout->addWidget(descLabel);
    layout->addWidget(m_importGuidanceButton);
    layout->addStretch();

    // Add to main content area
    auto *mainLayout = qobject_cast<QVBoxLayout*>(m_mainContentArea->layout());
    if (mainLayout) {
        mainLayout->addWidget(m_importGuidanceWidget);
    }
}
