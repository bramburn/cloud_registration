#include "mainwindow.h"
#include "pointcloudviewerwidget.h"
#include "e57parser.h"
#include "lasparser.h"
#include <QApplication>
#include <QThread>
#include <QTimer>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_openFileButton(nullptr)
    , m_viewer(nullptr)
    , m_statusLabel(nullptr)
    , m_progressDialog(nullptr)
    , m_e57Parser(nullptr)
    , m_lasParser(nullptr)
    , m_parserThread(nullptr)
    , m_isLoading(false)
{
    setupUI();
    setupMenuBar();
    setupStatusBar();

    // Initialize parsers
    m_e57Parser = new E57Parser(this);
    m_lasParser = new LasParser(this);

    // Set window properties
    setWindowTitle("Cloud Registration - Point Cloud Viewer");
    setMinimumSize(800, 600);
    resize(1200, 800);
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

    // Add button to layout
    m_buttonLayout->addWidget(m_openFileButton);
    m_buttonLayout->addStretch(); // Push button to the left

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

    QAction *exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setStatusTip("Exit the application");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);

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
    m_statusLabel = new QLabel("Ready", this);
    statusBar()->addWidget(m_statusLabel);
    statusBar()->showMessage("Ready to load point cloud files");
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

    m_currentFilePath = fileName;
    m_isLoading = true;

    // Update UI
    m_openFileButton->setEnabled(false);
    statusBar()->showMessage("Loading point cloud file...");

    // Determine file type
    QFileInfo fileInfo(fileName);
    QString extension = fileInfo.suffix().toLower();

    // Create progress dialog with determinate progress
    QString progressText = QString("Loading %1 file...").arg(extension.toUpper());
    m_progressDialog = new QProgressDialog(progressText, "Cancel", 0, 100, this);
    m_progressDialog->setWindowModality(Qt::WindowModal);
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

    } else if (extension == "las") {
        LasParser* lasWorker = new LasParser();
        lasWorker->moveToThread(m_parserThread);
        workerParser = lasWorker;

        // Connect signals
        connect(m_parserThread, &QThread::started, lasWorker, [=]() {
            lasWorker->startParsing(m_currentFilePath);
        });
        connect(lasWorker, &LasParser::progressUpdated, this, &MainWindow::onParsingProgressUpdated, Qt::QueuedConnection);
        connect(lasWorker, &LasParser::parsingFinished, this, &MainWindow::onParsingFinished, Qt::QueuedConnection);

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
    m_isLoading = false;
    m_openFileButton->setEnabled(true);

    if (m_progressDialog) {
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }

    if (success) {
        statusBar()->showMessage(message);
        m_statusLabel->setText(message);
    } else {
        statusBar()->showMessage("Failed to load file");
        QMessageBox::warning(this, "Loading Error", message);
    }
}

void MainWindow::onParsingProgressUpdated(int percentage)
{
    if (m_progressDialog) {
        m_progressDialog->setValue(percentage);
    }
}

void MainWindow::onParsingFinished(bool success, const QString& message, const std::vector<float>& points)
{
    // Clean up thread
    if (m_parserThread) {
        m_parserThread->quit();
        m_parserThread->wait();
        m_parserThread->deleteLater();
        m_parserThread = nullptr;
    }

    // Update UI state
    m_isLoading = false;
    m_openFileButton->setEnabled(true);

    if (m_progressDialog) {
        m_progressDialog->close();
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }

    if (success && !points.empty()) {
        m_viewer->loadPointCloud(points);
        statusBar()->showMessage(message);
        m_statusLabel->setText(message);
    } else {
        statusBar()->showMessage("Failed to load file");
        QMessageBox::warning(this, "Loading Error", message);
    }
}
