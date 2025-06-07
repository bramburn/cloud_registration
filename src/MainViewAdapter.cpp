#include "MainViewAdapter.h"
#include "mainwindow.h"

MainViewAdapter::MainViewAdapter(MainWindow* mainWindow, QObject* parent)
    : IMainView(parent)
    , m_mainWindow(mainWindow)
{
    Q_ASSERT(m_mainWindow != nullptr);
}

void MainViewAdapter::displayErrorMessage(const QString& title, const QString& message)
{
    m_mainWindow->displayErrorMessage(title, message);
}

void MainViewAdapter::displayInfoMessage(const QString& title, const QString& message)
{
    m_mainWindow->displayInfoMessage(title, message);
}

void MainViewAdapter::displayWarningMessage(const QString& title, const QString& message)
{
    m_mainWindow->displayWarningMessage(title, message);
}

void MainViewAdapter::updateStatusBar(const QString& text)
{
    m_mainWindow->updateStatusBar(text);
}

void MainViewAdapter::setWindowTitle(const QString& title)
{
    m_mainWindow->setWindowTitle(title);
}

IPointCloudViewer* MainViewAdapter::getViewer()
{
    return m_mainWindow->getViewer();
}

void MainViewAdapter::showProgressDialog(bool show, const QString& title, const QString& message)
{
    m_mainWindow->showProgressDialog(show, title, message);
}

void MainViewAdapter::updateProgress(int percentage, const QString& message)
{
    m_mainWindow->updateProgress(percentage, message);
}

void MainViewAdapter::setActionsEnabled(bool enabled)
{
    m_mainWindow->setActionsEnabled(enabled);
}

void MainViewAdapter::setProjectTitle(const QString& projectName)
{
    m_mainWindow->setProjectTitle(projectName);
}

void MainViewAdapter::updateScanList(const QStringList& scanNames)
{
    m_mainWindow->updateScanList(scanNames);
}

void MainViewAdapter::highlightScan(const QString& scanName)
{
    m_mainWindow->highlightScan(scanName);
}

void MainViewAdapter::showProjectHub()
{
    m_mainWindow->showProjectHub();
}

void MainViewAdapter::showProjectView()
{
    m_mainWindow->showProjectView();
}

void MainViewAdapter::updateMemoryUsage(size_t totalBytes)
{
    m_mainWindow->updateMemoryUsage(totalBytes);
}

void MainViewAdapter::updateRenderingStats(float fps, int visiblePoints)
{
    m_mainWindow->updateRenderingStats(fps, visiblePoints);
}

QString MainViewAdapter::askForOpenFilePath(const QString& title, const QString& filter)
{
    return m_mainWindow->askForOpenFilePath(title, filter);
}

QString MainViewAdapter::askForSaveFilePath(const QString& title, const QString& filter, const QString& defaultName)
{
    return m_mainWindow->askForSaveFilePath(title, filter, defaultName);
}

bool MainViewAdapter::askForConfirmation(const QString& title, const QString& message)
{
    return m_mainWindow->askForConfirmation(title, message);
}
