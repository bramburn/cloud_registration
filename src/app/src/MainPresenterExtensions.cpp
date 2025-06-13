#include <QFileInfo>

#include "interfaces/IMainView.h"
#include "interfaces/IPointCloudViewer.h"
#include "app/MainPresenter.h"
#include "core/lasheadermetadata.h"

// Additional methods for MainWindow compatibility

void MainPresenter::handleOpenFile()
{
    QString filePath = m_view->askForOpenFilePath("Open File", "E57 Files (*.e57)");
    if (!filePath.isEmpty())
    {
        handleOpenFile(filePath);
    }
}

void MainPresenter::handleScanActivated(const QString& scanId)
{
    handleScanActivation(scanId);
}

void MainPresenter::handleProjectOpened(const QString& projectPath)
{
    m_currentProjectPath = projectPath;
    m_isProjectOpen = true;

    QFileInfo fileInfo(projectPath);
    m_view->setProjectTitle(fileInfo.baseName());
    m_view->showProjectView();

    updateUIState();
    updateWindowTitle();

    showInfo("Project Opened", QString("Successfully opened project: %1").arg(fileInfo.baseName()));
}

void MainPresenter::handleLoadingFinished(bool success, const QString& message)
{
    m_view->showProgressDialog(false);
    m_view->setActionsEnabled(true);

    if (success)
    {
        showInfo("Loading Complete", message);
    }
    else
    {
        showError("Loading Failed", message);
    }

    updateUIState();
}

void MainPresenter::handleParsingProgressUpdated(int percentage, const QString& stage)
{
    m_view->updateProgress(percentage, stage);
}

void MainPresenter::handleParsingFinished(bool success, const QString& message, const std::vector<float>& points)
{
    onParsingFinished(success, message, points);
}

void MainPresenter::handleLoadingSettings()
{
    showInfo("Loading Settings", "Loading settings dialog will be implemented in future sprints.");
}

void MainPresenter::handleLasHeaderParsed(const LasHeaderMetadata& metadata)
{
    // Handle LAS header metadata
    m_view->updateStatusBar("LAS header parsed successfully");
}

void MainPresenter::handleScanMetadataReceived(int scanCount, const QStringList& scanNames)
{
    m_currentScanNames = scanNames;
    m_view->updateScanList(scanNames);
    m_view->updateStatusBar(QString("Received metadata for %1 scans").arg(scanCount));
}

void MainPresenter::handleIntensityDataReceived(const std::vector<float>& intensityValues)
{
    // Handle intensity data
    m_view->updateStatusBar(QString("Received intensity data for %1 points").arg(intensityValues.size()));
}

void MainPresenter::handleColorDataReceived(const std::vector<uint8_t>& colorValues)
{
    // Handle color data
    m_view->updateStatusBar(QString("Received color data for %1 values").arg(colorValues.size()));
}

void MainPresenter::handleTopViewClicked()
{
    if (m_viewer)
    {
        m_viewer->setTopView();
        m_view->updateStatusBar("Switched to top view");
    }
}

void MainPresenter::handleLeftViewClicked()
{
    if (m_viewer)
    {
        m_viewer->setLeftView();
        m_view->updateStatusBar("Switched to left view");
    }
}

void MainPresenter::handleRightViewClicked()
{
    if (m_viewer)
    {
        m_viewer->setRightView();
        m_view->updateStatusBar("Switched to right view");
    }
}

void MainPresenter::handleBottomViewClicked()
{
    if (m_viewer)
    {
        m_viewer->setBottomView();
        m_view->updateStatusBar("Switched to bottom view");
    }
}
