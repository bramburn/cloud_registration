#include "MainWindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QAction>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Set window properties
    setWindowTitle("Point Cloud Application");
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    // Setup UI components
    setupMenuBar();
    setupStatusBar();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupMenuBar()
{
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    QAction *exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setStatusTip("Exit the application");
    connect(exitAction, &QAction::triggered, this, &MainWindow::exitApplication);
    fileMenu->addAction(exitAction);
    
    // Help menu
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    
    QAction *aboutAction = new QAction("&About", this);
    aboutAction->setStatusTip("Show information about the application");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready", 2000);
}

void MainWindow::exitApplication()
{
    QApplication::quit();
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "About Point Cloud Application",
                       "Point Cloud Application v1.0.0\n\n"
                       "A high-performance desktop application for point cloud "
                       "visualization, manipulation, and registration.\n\n"
                       "Built with Qt6 and C++17.");
}
