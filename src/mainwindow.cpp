#include "mainwindow.h"
#include "projecthubwidget.h"
#include "sidebarwidget.h"
#include "createprojectdialog.h"
#include "projectmanager.h"
#include "project.h"
#include "pointcloudviewerwidget.h"
#include "pointcloudloadmanager.h"
#include "e57parserlib.h"
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
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QSlider>

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
    , m_loadManager(new PointCloudLoadManager(this))
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
    , m_lasParser(nullptr)
    , m_parserThread(nullptr)
    , m_workerParser(nullptr)
    , m_isLoading(false)
    , m_currentScanCount(0)
    , m_statusLabel(nullptr)
    , m_permanentStatusLabel(nullptr)
    , m_currentPointCount(0)
    , m_colorRenderCheckbox(nullptr)
    , m_intensityRenderCheckbox(nullptr)
    , m_attenuationCheckbox(nullptr)
    , m_minSizeSlider(nullptr)
    , m_maxSizeSlider(nullptr)
    , m_attenuationFactorSlider(nullptr)
    , m_minSizeLabel(nullptr)
    , m_maxSizeLabel(nullptr)
    , m_attenuationFactorLabel(nullptr)
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

        // Sprint 3.2: Connect point cloud load manager signals
        connect(m_loadManager, &PointCloudLoadManager::pointCloudDataReady,
                this, &MainWindow::onPointCloudDataReady);
        connect(m_loadManager, &PointCloudLoadManager::pointCloudViewFailed,
                this, &MainWindow::onPointCloudViewFailed);

        // Sprint 1.3: Connect E57 loading signals
        connect(m_loadManager, &PointCloudLoadManager::loadingStarted,
                this, [this](const QString& message) {
                    statusBar()->showMessage(message);
                    setCursor(Qt::WaitCursor);
                });
        connect(m_loadManager, &PointCloudLoadManager::loadingCompleted,
                this, [this]() {
                    setCursor(Qt::ArrowCursor);
                });
        connect(m_loadManager, &PointCloudLoadManager::statusUpdate,
                this, [this](const QString& status) {
                    statusBar()->showMessage(status);
                });

        // Sprint 2.1: Connect enhanced state management signals
        connect(m_loadManager, &PointCloudLoadManager::batchOperationProgress,
                this, [this](const QString& operation, int completed, int total) {
                    QString message = QString("Batch %1: %2/%3 completed").arg(operation).arg(completed).arg(total);
                    statusBar()->showMessage(message);
                });

        connect(m_loadManager, &PointCloudLoadManager::preprocessingStarted,
                this, [this](const QString& scanId) {
                    statusBar()->showMessage(QString("Preprocessing scan: %1").arg(scanId));
                });

        connect(m_loadManager, &PointCloudLoadManager::preprocessingFinished,
                this, [this](const QString& scanId, bool success) {
                    QString message = success ?
                        QString("Preprocessing completed: %1").arg(scanId) :
                        QString("Preprocessing failed: %1").arg(scanId);
                    statusBar()->showMessage(message, 3000);
                });

        connect(m_loadManager, &PointCloudLoadManager::optimizationStarted,
                this, [this](const QString& scanId) {
                    statusBar()->showMessage(QString("Optimizing scan: %1").arg(scanId));
                });

        connect(m_loadManager, &PointCloudLoadManager::optimizationFinished,
                this, [this](const QString& scanId, bool success) {
                    QString message = success ?
                        QString("Optimization completed: %1").arg(scanId) :
                        QString("Optimization failed: %1").arg(scanId);
                    statusBar()->showMessage(message, 3000);
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

    // Sprint R3: Setup attribute rendering controls
    setupSprintR3Controls(contentLayout);

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

    // Sprint 3.3: Create progress display widgets
    m_progressLabel = new QLabel();
    m_progressLabel->setVisible(false);
    m_progressLabel->setMinimumWidth(200);

    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    m_progressBar->setMaximumWidth(200);
    m_progressBar->setTextVisible(true);

    m_timeLabel = new QLabel();
    m_timeLabel->setVisible(false);
    m_timeLabel->setStyleSheet("QLabel { color: #666; }");

    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setVisible(false);
    m_cancelButton->setMaximumWidth(60);

    // Add to status bar
    statusBar()->addWidget(m_statusLabel, 1); // Stretch factor 1
    statusBar()->addWidget(new QLabel()); // Spacer
    statusBar()->addPermanentWidget(m_progressLabel);
    statusBar()->addPermanentWidget(m_progressBar);
    statusBar()->addPermanentWidget(m_timeLabel);
    statusBar()->addPermanentWidget(m_cancelButton);
    statusBar()->addPermanentWidget(m_permanentStatusLabel);

    // Sprint 3.4: Setup memory display
    setupMemoryDisplay();

    // Sprint 2.2: Setup performance statistics display
    m_fpsLabel = new QLabel(this);
    m_fpsLabel->setText("FPS: 0.0");
    m_fpsLabel->setMinimumWidth(80);
    m_fpsLabel->setAlignment(Qt::AlignCenter);
    m_fpsLabel->setStyleSheet("QLabel { color: #666; margin: 0 5px; }");

    m_pointsLabel = new QLabel(this);
    m_pointsLabel->setText("Points: 0");
    m_pointsLabel->setMinimumWidth(100);
    m_pointsLabel->setAlignment(Qt::AlignCenter);
    m_pointsLabel->setStyleSheet("QLabel { color: #666; margin: 0 5px; }");

    // Add performance labels to status bar
    statusBar()->addPermanentWidget(m_fpsLabel);
    statusBar()->addPermanentWidget(m_pointsLabel);

    // Connect to viewer performance statistics
    if (m_viewer) {
        connect(m_viewer, &PointCloudViewerWidget::statsUpdated,
                this, &MainWindow::onStatsUpdated);
    }

    // Setup status bar style
    statusBar()->setStyleSheet(
        "QStatusBar { border-top: 1px solid #cccccc; }"
        "QStatusBar::item { border: none; }"
    );

    // Sprint 3.3: Connect to ProgressManager
    auto& progressManager = ProgressManager::instance();
    connect(&progressManager, &ProgressManager::operationStarted,
            this, &MainWindow::onOperationStarted);
    connect(&progressManager, &ProgressManager::progressUpdated,
            this, &MainWindow::onProgressUpdated);
    connect(&progressManager, &ProgressManager::operationFinished,
            this, &MainWindow::onOperationFinished);
    connect(&progressManager, &ProgressManager::operationCancelled,
            this, &MainWindow::onOperationCancelled);
    connect(&progressManager, &ProgressManager::estimatedTimeChanged,
            this, &MainWindow::onEstimatedTimeChanged);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &MainWindow::onCancelCurrentOperation);
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
    if (extension == "e57") {
        E57ParserLib* e57Worker = new E57ParserLib();
        e57Worker->moveToThread(m_parserThread);
        m_workerParser = e57Worker;

        // Convert dialog settings to E57ParserLib::LoadingSettings
        E57ParserLib::LoadingSettings e57Settings;
        e57Settings.loadIntensity = loadingSettings.parameters.value("loadIntensity", true).toBool();
        e57Settings.loadColor = loadingSettings.parameters.value("loadColor", true).toBool();
        e57Settings.maxPointsPerScan = loadingSettings.parameters.value("maxPoints", -1).toInt();
        e57Settings.subsamplingRatio = loadingSettings.parameters.value("subsamplingRatio", 1.0).toDouble();

        // Connect signals
        connect(m_parserThread, &QThread::started, [=]() {
            e57Worker->startParsing(m_currentFilePath, e57Settings);
        });

        // Connect progress updates
        connect(e57Worker, &E57ParserLib::progressUpdated, this, &MainWindow::onParsingProgressUpdated, Qt::QueuedConnection);

        // Connect parsing finished
        connect(e57Worker, &E57ParserLib::parsingFinished, this, &MainWindow::onParsingFinished, Qt::QueuedConnection);

        // Connect additional E57-specific signals
        connect(e57Worker, &E57ParserLib::scanMetadataAvailable, this, &MainWindow::onScanMetadataReceived, Qt::QueuedConnection);
        connect(e57Worker, &E57ParserLib::intensityDataExtracted, this, &MainWindow::onIntensityDataReceived, Qt::QueuedConnection);
        connect(e57Worker, &E57ParserLib::colorDataExtracted, this, &MainWindow::onColorDataReceived, Qt::QueuedConnection);

        // Sprint 2.3: Connect to viewer for visual feedback
        connect(e57Worker, &E57ParserLib::progressUpdated, m_viewer, &PointCloudViewerWidget::onLoadingProgress, Qt::QueuedConnection);
        connect(e57Worker, &E57ParserLib::parsingFinished, m_viewer, &PointCloudViewerWidget::onLoadingFinished, Qt::QueuedConnection);

        // Connect thread cleanup
        connect(e57Worker, &E57ParserLib::parsingFinished, [this, e57Worker]() {
            cleanupParsingThread(e57Worker);
        });

        // Connect cancel button
        connect(m_progressDialog, &QProgressDialog::canceled, [this, e57Worker]() {
            e57Worker->cancelParsing();
        });

    } else if (extension == "las") {
        LasParser* lasWorker = new LasParser();
        lasWorker->moveToThread(m_parserThread);
        m_workerParser = lasWorker;

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
        if (m_progressDialog) {
            m_progressDialog->close();
            m_progressDialog->deleteLater();
            m_progressDialog = nullptr;
        }
        QMessageBox::warning(this, "Error", "Unsupported file format");
        return;
    }

    // Setup cleanup when thread finishes
    connect(m_parserThread, &QThread::finished, m_workerParser, &QObject::deleteLater);

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

void MainWindow::cleanupParsingThread(QObject* parser)
{
    if (m_parserThread) {
        m_parserThread->quit();
        m_parserThread->wait(5000); // Wait up to 5 seconds

        if (m_parserThread->isRunning()) {
            qWarning() << "Parser thread did not quit gracefully, terminating";
            m_parserThread->terminate();
            m_parserThread->wait(1000);
        }

        m_parserThread->deleteLater();
        m_parserThread = nullptr;
    }

    if (parser) {
        parser->deleteLater();
        m_workerParser = nullptr;
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

// E57-specific slot implementations
void MainWindow::onScanMetadataReceived(int scanCount, const QStringList& scanNames)
{
    m_currentScanCount = scanCount;
    m_currentScanNames = scanNames;

    qDebug() << "E57 scan metadata received:" << scanCount << "scans";
    for (int i = 0; i < scanNames.size(); ++i) {
        qDebug() << "  Scan" << i << ":" << scanNames[i];
    }

    if (scanCount > 1) {
        QString statusMsg = QString("Multi-scan E57 file detected (%1 scans), loading first scan...").arg(scanCount);
        if (m_progressDialog) {
            m_progressDialog->setLabelText(statusMsg);
        }
    }
}

void MainWindow::onIntensityDataReceived(const std::vector<float>& intensityValues)
{
    m_currentIntensityData = intensityValues;
    qDebug() << "E57 intensity data received:" << intensityValues.size() << "values";

    // Future enhancement: Pass intensity data to viewer
    // m_viewer->setIntensityData(intensityValues);
}

void MainWindow::onColorDataReceived(const std::vector<uint8_t>& colorValues)
{
    m_currentColorData = colorValues;
    qDebug() << "E57 color data received:" << colorValues.size() << "values (RGB interleaved)";

    // Future enhancement: Pass color data to viewer
    // m_viewer->setColorData(colorValues);
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
        auto loadResult = m_projectManager->loadProject(projectPath);

        if (loadResult != ProjectLoadResult::Success) {
            QString errorMsg = "Failed to load project";
            switch (loadResult) {
                case ProjectLoadResult::MetadataCorrupted:
                    errorMsg = "Project metadata is corrupted";
                    break;
                case ProjectLoadResult::DatabaseCorrupted:
                    errorMsg = "Project database is corrupted";
                    break;
                case ProjectLoadResult::DatabaseMissing:
                    errorMsg = "Project database is missing";
                    break;
                case ProjectLoadResult::MetadataMissing:
                    errorMsg = "Project metadata is missing";
                    break;
                default:
                    errorMsg = "Unknown error loading project";
                    break;
            }
            QMessageBox::critical(this, "Project Load Error", errorMsg);
            return;
        }

        // Get project info using legacy method for Project object creation
        auto projectInfo = m_projectManager->loadProjectLegacy(projectPath);

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

        // Sprint 3.2: Set up point cloud load manager
        m_loadManager->setSQLiteManager(m_projectManager->getSQLiteManager());
        m_loadManager->setProjectTreeModel(m_sidebar->getModel());
        m_sidebar->setPointCloudLoadManager(m_loadManager);

        // Sprint 2.1: Connect ProjectTreeModel enhanced signals
        connect(m_sidebar->getModel(), &ProjectTreeModel::memoryWarningTriggered,
                this, [this](size_t currentUsage, size_t threshold) {
                    QString message = QString("Memory warning: %1 MB used (threshold: %2 MB)")
                                     .arg(currentUsage / (1024 * 1024))
                                     .arg(threshold / (1024 * 1024));
                    statusBar()->showMessage(message, 5000);

                    // Show warning dialog for critical memory usage
                    if (currentUsage > threshold * 1.2) { // 20% over threshold
                        QMessageBox::warning(this, "Memory Warning",
                            "Memory usage is critically high. Consider unloading some scans to free memory.");
                    }
                });

        connect(m_sidebar->getModel(), &ProjectTreeModel::scanStateChanged,
                this, [this](const QString& scanId, LoadedState oldState, LoadedState newState) {
                    Q_UNUSED(oldState)
                    QString stateStr;
                    switch (newState) {
                        case LoadedState::Loaded: stateStr = "loaded"; break;
                        case LoadedState::Unloaded: stateStr = "unloaded"; break;
                        case LoadedState::Loading: stateStr = "loading"; break;
                        case LoadedState::Processing: stateStr = "processing"; break;
                        case LoadedState::Error: stateStr = "error"; break;
                        case LoadedState::Cached: stateStr = "cached"; break;
                        case LoadedState::MemoryWarning: stateStr = "memory warning"; break;
                        case LoadedState::Optimized: stateStr = "optimized"; break;
                        default: stateStr = "unknown"; break;
                    }
                    qDebug() << "Scan state changed:" << scanId << "to" << stateStr;
                });

        // Sprint 1.3: Connect scan activation for E57 handling
        connect(m_sidebar, &SidebarWidget::viewPointCloudRequested,
                this, [this](const QString& itemId, const QString& itemType) {
                    if (itemType == "scan") {
                        onScanActivated(itemId);
                    } else {
                        // For clusters, use the existing mechanism
                        m_loadManager->viewPointCloud(itemId, itemType);
                    }
                });

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

    // Sprint 1.3: Connect E57-specific import signals
    auto* scanImportManager = m_projectManager->getScanImportManager();
    scanImportManager->setProjectTreeModel(m_sidebar->getModel());

    connect(&dialog, &ScanImportDialog::importE57FileRequested,
            scanImportManager, &ScanImportManager::handleE57Import);
    connect(&dialog, &ScanImportDialog::importLasFileRequested,
            this, [this, scanImportManager](const QString& filePath) {
                // Handle LAS import using existing mechanism
                QStringList files = {filePath};
                auto result = scanImportManager->importScans(
                    files, m_currentProject->projectPath(), m_currentProject->projectId(), ImportMode::Copy, this);

                if (result.success) {
                    showImportGuidance(false);
                    m_sidebar->refreshFromDatabase();
                    statusBar()->showMessage("Successfully imported LAS file", 3000);
                } else {
                    QMessageBox::warning(this, "Import Failed", result.errorMessage);
                }
            });

    connect(scanImportManager, &ScanImportManager::importCompleted,
            this, [this](const QString& filePath, int scanCount) {
                Q_UNUSED(filePath)
                showImportGuidance(false);
                m_sidebar->refreshFromDatabase();
                statusBar()->showMessage(
                    QString("Successfully imported %1 scan(s) from E57 file").arg(scanCount), 3000);
            });

    connect(scanImportManager, &ScanImportManager::importFailed,
            this, [this](const QString& filePath, const QString& error) {
                QMessageBox::critical(this, "E57 Import Failed",
                    QString("Failed to import %1:\n%2").arg(QFileInfo(filePath).fileName(), error));
            });

    if (dialog.exec() == QDialog::Accepted) {
        // The dialog will emit the appropriate signals for E57 or LAS files
        // No additional processing needed here for Sprint 1.3
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

// Sprint 1.3: E57 scan activation implementation
void MainWindow::onScanActivated(const QString& scanId)
{
    try {
        if (!m_projectManager || !m_projectManager->getSQLiteManager()) {
            qDebug() << "MainWindow: No project manager or database available";
            return;
        }

        // Get scan info from database
        ScanInfo scanInfo = m_projectManager->getSQLiteManager()->getScanById(scanId);

        if (scanInfo.scanId.isEmpty()) {
            QMessageBox::warning(this, "Scan Not Found",
                QString("Scan with ID %1 was not found in the database.").arg(scanId));
            return;
        }

        qDebug() << "MainWindow: Activating scan" << scanInfo.scanName << "of type" << scanInfo.importType;

        if (scanInfo.importType == "E57") {
            // Load E57 scan using the stored GUID (stored in originalSourcePath field)
            QString e57Guid = scanInfo.originalSourcePath;
            QString filePath = scanInfo.filePathRelative;

            if (e57Guid.isEmpty() || filePath.isEmpty()) {
                QMessageBox::warning(this, "Invalid E57 Data",
                    "E57 scan data is incomplete. Please re-import the file.");
                return;
            }

            m_loadManager->loadE57Scan(filePath, e57Guid);
        } else {
            // Handle other file types using existing mechanism
            emit m_sidebar->viewPointCloudRequested(scanId, "scan");
        }

    } catch (const std::exception& ex) {
        QMessageBox::critical(this, "Load Error",
            QString("Failed to load scan: %1").arg(ex.what()));
    }
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

// Sprint 3.2: Point cloud viewing slot implementations
void MainWindow::onPointCloudDataReady(const std::vector<float> &points, const QString &sourceInfo)
{
    qDebug() << "MainWindow::onPointCloudDataReady - Loading point cloud data:" << sourceInfo;
    qDebug() << "Point count:" << (points.size() / 3);

    if (!points.empty()) {
        // Hide import guidance if visible
        showImportGuidance(false);

        // Load data into viewer
        m_viewer->loadPointCloud(points);

        // Update status bar
        setStatusLoadSuccess(sourceInfo, static_cast<int>(points.size() / 3));

        qDebug() << "Successfully loaded point cloud data into viewer";
    } else {
        qDebug() << "Warning: Empty point cloud data received";
        setStatusLoadFailed(sourceInfo, "No point data available");
    }
}

void MainWindow::onPointCloudViewFailed(const QString &error)
{
    qDebug() << "MainWindow::onPointCloudViewFailed - Error:" << error;

    // Show error message to user
    QMessageBox::warning(this, "Point Cloud View Failed",
                        QString("Failed to view point cloud:\n%1").arg(error));

    // Update status bar
    setStatusLoadFailed("Point Cloud", error);

    // Clear viewer to prevent stale data
    m_viewer->clearPointCloud();
}

// Sprint 3.3: Progress management slot implementations
void MainWindow::onOperationStarted(const QString& operationId, const QString& name, OperationType type)
{
    m_currentOperationId = operationId;

    // Set operation-specific styling
    QString color;
    switch (type) {
        case OperationType::ScanImport: color = "#2196F3"; break;
        case OperationType::ClusterLoad: color = "#4CAF50"; break;
        case OperationType::ProjectSave: color = "#FF9800"; break;
        case OperationType::DataExport: color = "#9C27B0"; break;
        default: color = "#607D8B"; break;
    }

    m_progressBar->setStyleSheet(QString("QProgressBar::chunk { background-color: %1; }").arg(color));

    m_progressLabel->setText(name);
    m_progressLabel->setVisible(true);
    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);

    // Show cancel button if operation is cancellable
    ProgressInfo info = ProgressManager::instance().getProgressInfo(operationId);
    m_cancelButton->setVisible(info.isCancellable);

    qDebug() << "Progress operation started:" << name << "ID:" << operationId;
}

void MainWindow::onProgressUpdated(const QString& operationId, int value, int max, const QString& step, const QString& details)
{
    if (operationId == m_currentOperationId) {
        m_progressBar->setMaximum(max);
        m_progressBar->setValue(value);

        QString labelText = ProgressManager::instance().getProgressInfo(operationId).operationName;
        if (!step.isEmpty()) {
            labelText += QString(" - %1").arg(step);
        }
        m_progressLabel->setText(labelText);

        // Update tooltip with detailed information
        if (!details.isEmpty()) {
            m_progressBar->setToolTip(details);
        }

        // Update percentage display
        if (max > 0) {
            int percentage = (value * 100) / max;
            m_progressBar->setFormat(QString("%1%").arg(percentage));
        }
    }
}

void MainWindow::onEstimatedTimeChanged(const QString& operationId, const QDateTime& estimatedEnd)
{
    Q_UNUSED(estimatedEnd)
    if (operationId == m_currentOperationId) {
        QString timeText = ProgressManager::instance().formatTimeRemaining(operationId);
        m_timeLabel->setText(timeText);
        m_timeLabel->setVisible(!timeText.isEmpty());
    }
}

void MainWindow::onOperationFinished(const QString& operationId, const QString& result)
{
    if (operationId == m_currentOperationId) {
        m_progressBar->setVisible(false);
        m_progressLabel->setVisible(false);
        m_timeLabel->setVisible(false);
        m_cancelButton->setVisible(false);
        m_currentOperationId.clear();

        // Show brief success message if result provided
        if (!result.isEmpty()) {
            statusBar()->showMessage(result, 3000);
        }

        qDebug() << "Progress operation finished:" << operationId << "Result:" << result;
    }
}

void MainWindow::onOperationCancelled(const QString& operationId)
{
    if (operationId == m_currentOperationId) {
        m_progressBar->setVisible(false);
        m_progressLabel->setVisible(false);
        m_timeLabel->setVisible(false);
        m_cancelButton->setVisible(false);
        m_currentOperationId.clear();

        statusBar()->showMessage("Operation cancelled", 3000);
        qDebug() << "Progress operation cancelled:" << operationId;
    }
}

void MainWindow::onCancelCurrentOperation()
{
    if (!m_currentOperationId.isEmpty()) {
        ProgressManager::instance().cancelOperation(m_currentOperationId);
    }
}

// Sprint 3.4: Memory statistics display implementation
void MainWindow::setupMemoryDisplay()
{
    m_memoryLabel = new QLabel(this);
    m_memoryLabel->setText("Memory: 0 MB");
    m_memoryLabel->setMinimumWidth(100);
    m_memoryLabel->setAlignment(Qt::AlignCenter);
    m_memoryLabel->setStyleSheet("QLabel { color: #666; margin: 0 5px; }");

    // Add to status bar before the permanent status label
    statusBar()->addPermanentWidget(m_memoryLabel);

    // Connect to load manager memory usage signal
    if (m_loadManager) {
        connect(m_loadManager, &PointCloudLoadManager::memoryUsageChanged,
                this, &MainWindow::onMemoryUsageChanged);
    }

    qDebug() << "Memory display setup completed";
}

void MainWindow::onMemoryUsageChanged(size_t totalBytes)
{
    if (m_memoryLabel) {
        double megabytes = totalBytes / (1024.0 * 1024.0);
        QString text;

        if (megabytes >= 1024.0) {
            // Display in GB if >= 1GB
            double gigabytes = megabytes / 1024.0;
            text = QString("Memory: %1 GB").arg(gigabytes, 0, 'f', 1);
        } else {
            // Display in MB
            text = QString("Memory: %1 MB").arg(megabytes, 0, 'f', 1);
        }

        m_memoryLabel->setText(text);

        // Change color based on usage level (assuming 2GB default limit)
        if (megabytes > 1536) { // > 1.5GB (75% of 2GB)
            m_memoryLabel->setStyleSheet("QLabel { color: #d32f2f; margin: 0 5px; font-weight: bold; }");
        } else if (megabytes > 1024) { // > 1GB (50% of 2GB)
            m_memoryLabel->setStyleSheet("QLabel { color: #f57c00; margin: 0 5px; }");
        } else {
            m_memoryLabel->setStyleSheet("QLabel { color: #666; margin: 0 5px; }");
        }

        qDebug() << "Memory usage updated:" << text;
    }
}

// Sprint R3: Setup attribute rendering controls (as per backlog Tasks R3.1.6, R3.2.5, R3.3.3)
void MainWindow::setupSprintR3Controls(QVBoxLayout* parentLayout)
{
    // Create controls widget
    QWidget* controlsWidget = new QWidget();
    controlsWidget->setMaximumHeight(120);
    controlsWidget->setStyleSheet("QWidget { background-color: #f5f5f5; border: 1px solid #ddd; }");

    QHBoxLayout* controlsLayout = new QHBoxLayout(controlsWidget);
    controlsLayout->setContentsMargins(10, 5, 10, 5);

    // Attribute rendering checkboxes
    QGroupBox* attributeGroup = new QGroupBox("Attribute Rendering");
    QHBoxLayout* attributeLayout = new QHBoxLayout(attributeGroup);

    m_colorRenderCheckbox = new QCheckBox("Color");
    m_intensityRenderCheckbox = new QCheckBox("Intensity");

    attributeLayout->addWidget(m_colorRenderCheckbox);
    attributeLayout->addWidget(m_intensityRenderCheckbox);

    // Point size attenuation controls
    QGroupBox* attenuationGroup = new QGroupBox("Point Size Attenuation");
    QVBoxLayout* attenuationLayout = new QVBoxLayout(attenuationGroup);

    m_attenuationCheckbox = new QCheckBox("Enable Attenuation");
    attenuationLayout->addWidget(m_attenuationCheckbox);

    // Attenuation parameter sliders
    QHBoxLayout* slidersLayout = new QHBoxLayout();

    // Min size slider
    QVBoxLayout* minSizeLayout = new QVBoxLayout();
    m_minSizeLabel = new QLabel("Min Size: 1.0");
    m_minSizeSlider = new QSlider(Qt::Horizontal);
    m_minSizeSlider->setRange(1, 20);
    m_minSizeSlider->setValue(10);
    minSizeLayout->addWidget(m_minSizeLabel);
    minSizeLayout->addWidget(m_minSizeSlider);

    // Max size slider
    QVBoxLayout* maxSizeLayout = new QVBoxLayout();
    m_maxSizeLabel = new QLabel("Max Size: 10.0");
    m_maxSizeSlider = new QSlider(Qt::Horizontal);
    m_maxSizeSlider->setRange(10, 100);
    m_maxSizeSlider->setValue(100);
    maxSizeLayout->addWidget(m_maxSizeLabel);
    maxSizeLayout->addWidget(m_maxSizeSlider);

    // Attenuation factor slider
    QVBoxLayout* factorLayout = new QVBoxLayout();
    m_attenuationFactorLabel = new QLabel("Factor: 0.1");
    m_attenuationFactorSlider = new QSlider(Qt::Horizontal);
    m_attenuationFactorSlider->setRange(1, 100);
    m_attenuationFactorSlider->setValue(10);
    factorLayout->addWidget(m_attenuationFactorLabel);
    factorLayout->addWidget(m_attenuationFactorSlider);

    slidersLayout->addLayout(minSizeLayout);
    slidersLayout->addLayout(maxSizeLayout);
    slidersLayout->addLayout(factorLayout);
    attenuationLayout->addLayout(slidersLayout);

    // Add groups to main layout
    controlsLayout->addWidget(attributeGroup);
    controlsLayout->addWidget(attenuationGroup);
    controlsLayout->addStretch();

    // Add controls widget to parent layout
    parentLayout->addWidget(controlsWidget);

    // Connect signals
    connect(m_colorRenderCheckbox, &QCheckBox::toggled, this, &MainWindow::onColorRenderToggled);
    connect(m_intensityRenderCheckbox, &QCheckBox::toggled, this, &MainWindow::onIntensityRenderToggled);
    connect(m_attenuationCheckbox, &QCheckBox::toggled, this, &MainWindow::onAttenuationToggled);
    connect(m_minSizeSlider, &QSlider::valueChanged, this, &MainWindow::onAttenuationParamsChanged);
    connect(m_maxSizeSlider, &QSlider::valueChanged, this, &MainWindow::onAttenuationParamsChanged);
    connect(m_attenuationFactorSlider, &QSlider::valueChanged, this, &MainWindow::onAttenuationParamsChanged);
}

// Sprint R3: Attribute rendering and point size attenuation slot implementations
void MainWindow::onColorRenderToggled(bool enabled)
{
    if (m_viewer) {
        m_viewer->setRenderWithColor(enabled);
    }
    qDebug() << "Color rendering toggled:" << enabled;
}

void MainWindow::onIntensityRenderToggled(bool enabled)
{
    if (m_viewer) {
        m_viewer->setRenderWithIntensity(enabled);
    }
    qDebug() << "Intensity rendering toggled:" << enabled;
}

void MainWindow::onAttenuationToggled(bool enabled)
{
    if (m_viewer) {
        m_viewer->setPointSizeAttenuationEnabled(enabled);
    }

    // Enable/disable sliders based on checkbox state
    m_minSizeSlider->setEnabled(enabled);
    m_maxSizeSlider->setEnabled(enabled);
    m_attenuationFactorSlider->setEnabled(enabled);

    qDebug() << "Point size attenuation toggled:" << enabled;
}

void MainWindow::onAttenuationParamsChanged()
{
    if (m_viewer) {
        float minSize = m_minSizeSlider->value() / 10.0f;
        float maxSize = m_maxSizeSlider->value() / 10.0f;
        float factor = m_attenuationFactorSlider->value() / 100.0f;

        m_viewer->setPointSizeAttenuationParams(minSize, maxSize, factor);

        // Update labels
        m_minSizeLabel->setText(QString("Min Size: %1").arg(minSize, 0, 'f', 1));
        m_maxSizeLabel->setText(QString("Max Size: %1").arg(maxSize, 0, 'f', 1));
        m_attenuationFactorLabel->setText(QString("Factor: %1").arg(factor, 0, 'f', 2));
    }
}

// Sprint 2.2: Performance statistics display implementation
void MainWindow::onStatsUpdated(float fps, int visiblePoints)
{
    if (m_fpsLabel) {
        m_fpsLabel->setText(QString("FPS: %1").arg(fps, 0, 'f', 1));

        // Color code FPS based on performance
        if (fps >= 30.0f) {
            m_fpsLabel->setStyleSheet("QLabel { color: #4caf50; margin: 0 5px; }"); // Green
        } else if (fps >= 15.0f) {
            m_fpsLabel->setStyleSheet("QLabel { color: #ff9800; margin: 0 5px; }"); // Orange
        } else {
            m_fpsLabel->setStyleSheet("QLabel { color: #f44336; margin: 0 5px; }"); // Red
        }
    }

    if (m_pointsLabel) {
        QString pointsText;
        if (visiblePoints >= 1000000) {
            // Display in millions
            double millions = visiblePoints / 1000000.0;
            pointsText = QString("Points: %1M").arg(millions, 0, 'f', 1);
        } else if (visiblePoints >= 1000) {
            // Display in thousands
            double thousands = visiblePoints / 1000.0;
            pointsText = QString("Points: %1K").arg(thousands, 0, 'f', 1);
        } else {
            pointsText = QString("Points: %1").arg(visiblePoints);
        }

        m_pointsLabel->setText(pointsText);
    }
}
