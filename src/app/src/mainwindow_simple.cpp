#include "mainwindow_simple.h"

#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_centralLabel(nullptr)
{
    setupUI();

    // Set window properties
    setWindowTitle("Cloud Registration - Sprint 2");
    setMinimumSize(800, 600);
    resize(1000, 700);

    // Show status message
    statusBar()->showMessage("Sprint 2: Qt Integration & Main Window - Ready");
}

MainWindow::~MainWindow()
{
    // Qt handles cleanup automatically
}

void MainWindow::setupUI()
{
    // Create central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Create layout
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    // Create central label
    m_centralLabel = new QLabel("Sprint 2: Qt Integration Complete!\n\n"
                                "✓ vcpkg integration working\n"
                                "✓ Qt6 components linked\n"
                                "✓ MainWindow created\n"
                                "✓ CMAKE_AUTOMOC enabled\n\n"
                                "Ready for Sprint 3!",
                                this);
    m_centralLabel->setAlignment(Qt::AlignCenter);
    m_centralLabel->setStyleSheet("QLabel { font-size: 16px; padding: 20px; }");

    layout->addWidget(m_centralLabel);

    // Create simple menu bar
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Exit", this, &QWidget::close);

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About Qt", QApplication::instance(), &QApplication::aboutQt);
}
