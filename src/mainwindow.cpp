#include "mainwindow.h"
#include "pointcloudviewerwidget.h"
#include "e57parser.h"
#include "lasparser.h"
#include "loadingsettingsdialog.h"
#include "lasheadermetadata.h"
#include "loadingsettings.h"
#include "performance_profiler.h"
#include <QApplication>
#include <QThread>
#include <QTimer>
#include <QFileInfo>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_openFileButton(nullptr)
    , m_topViewButton(nullptr)
    , m_leftViewButton(nullptr)
    , m_rightViewButton(nullptr)
    , m_bottomViewButton(nullptr)
    , m_viewer(nullptr)
    , m_progressDialog(nullptr)
    , m_loadingSettingsAction(nullptr)
    , m_topViewAction(nullptr)
    , m_leftViewAction(nullptr)
    , m_rightViewAction(nullptr)
    , m_bottomViewAction(nullptr)
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

        // Initialize parsers
        qDebug() << "Initializing parsers...";
        m_e57Parser = new E57Parser(this);
        m_lasParser = new LasParser(this);
        qDebug() << "Parsers initialized";

        // Sprint 2.3: Initialize in ready state
        setStatusReady();

        // Set window properties
        qDebug() << "Setting window properties...";
        setWindowTitle("Cloud Registration - Point Cloud Viewer");
        setMinimumSize(800, 600);
        resize(1200, 800);
        qDebug() << "Window properties set";

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
    // Cleanup is handled by Qt's parent-child relationship
}

void MainWindow::setupUI()
{
    // Create central widget and main layout
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QVBoxLayout(m_centralWidget);

    // Create button layout
    m_buttonLayout = new QHBoxLayout();

    // Create Open File button
    m_openFileButton = new QPushButton("Open Point Cloud File", this);
    m_openFileButton->setMinimumHeight(40);
    m_openFileButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    border: none;"
        "    padding: 8px 16px;"
        "    font-size: 14px;"
        "    border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3d8b40;"
        "}"
    );

    connect(m_openFileButton, &QPushButton::clicked, this, &MainWindow::onOpenFileClicked);

    // Create view control buttons
    m_topViewButton = new QPushButton("Top View", this);
    m_leftViewButton = new QPushButton("Left View", this);
    m_rightViewButton = new QPushButton("Right View", this);
    m_bottomViewButton = new QPushButton("Bottom View", this);

    // Style view buttons
    QString viewButtonStyle =
        "QPushButton {"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    padding: 6px 12px;"
        "    font-size: 12px;"
        "    border-radius: 3px;"
        "    min-width: 80px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1565C0;"
        "}";

    m_topViewButton->setStyleSheet(viewButtonStyle);
    m_leftViewButton->setStyleSheet(viewButtonStyle);
    m_rightViewButton->setStyleSheet(viewButtonStyle);
    m_bottomViewButton->setStyleSheet(viewButtonStyle);

    // Connect view buttons
    connect(m_topViewButton, &QPushButton::clicked, this, &MainWindow::onTopViewClicked);
    connect(m_leftViewButton, &QPushButton::clicked, this, &MainWindow::onLeftViewClicked);
    connect(m_rightViewButton, &QPushButton::clicked, this, &MainWindow::onRightViewClicked);
    connect(m_bottomViewButton, &QPushButton::clicked, this, &MainWindow::onBottomViewClicked);

    // Add buttons to layout
    m_buttonLayout->addWidget(m_openFileButton);
    m_buttonLayout->addSpacing(20); // Add some space
    m_buttonLayout->addWidget(m_topViewButton);
    m_buttonLayout->addWidget(m_leftViewButton);
    m_buttonLayout->addWidget(m_rightViewButton);
    m_buttonLayout->addWidget(m_bottomViewButton);
    m_buttonLayout->addStretch(); // Push buttons to the left

    // Create 3D viewer widget
    m_viewer = new PointCloudViewerWidget(this);

    // Add components to main layout
    m_mainLayout->addLayout(m_buttonLayout);
    m_mainLayout->addWidget(m_viewer, 1); // Give viewer most of the space
}

void MainWindow::setupMenuBar()
{
    // Create File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");

    QAction *openAction = new QAction("&Open Point Cloud File...", this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("Open a point cloud file (E57 or LAS)");
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFileClicked);
    fileMenu->addAction(openAction);

    fileMenu->addSeparator();

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

    // Update UI
    m_openFileButton->setEnabled(false);

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
    m_openFileButton->setEnabled(true);

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
