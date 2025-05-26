#include "mainwindow.h"
#include "pointcloudviewerwidget.h"
#include "e57parser.h"
#include <QApplication>
#include <QThread>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_openFileButton(nullptr)
    , m_viewer(nullptr)
    , m_statusLabel(nullptr)
    , m_progressDialog(nullptr)
    , m_parser(nullptr)
    , m_isLoading(false)
{
    setupUI();
    setupMenuBar();
    setupStatusBar();
    
    // Initialize parser
    m_parser = new E57Parser(this);
    
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
    m_openFileButton = new QPushButton("Open E57 File", this);
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
    
    QAction *openAction = new QAction("&Open E57 File...", this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("Open an E57 point cloud file");
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
        "Open E57 Point Cloud File",
        QString(),
        "E57 Files (*.e57);;All Files (*)"
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    m_currentFilePath = fileName;
    m_isLoading = true;
    
    // Update UI
    m_openFileButton->setEnabled(false);
    statusBar()->showMessage("Loading point cloud file...");
    
    // Create progress dialog
    m_progressDialog = new QProgressDialog("Loading E57 file...", "Cancel", 0, 0, this);
    m_progressDialog->setWindowModality(Qt::WindowModal);
    m_progressDialog->show();
    
    // Parse file in a separate thread (simplified for now - will run in main thread)
    QTimer::singleShot(100, [this]() {
        try {
            std::vector<float> points = m_parser->parse(m_currentFilePath);
            
            if (!points.empty()) {
                m_viewer->loadPointCloud(points);
                onLoadingFinished(true, QString("Loaded %1 points successfully").arg(points.size() / 3));
            } else {
                onLoadingFinished(false, "No point data found in file");
            }
        } catch (const std::exception& e) {
            onLoadingFinished(false, QString("Error loading file: %1").arg(e.what()));
        }
    });
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
