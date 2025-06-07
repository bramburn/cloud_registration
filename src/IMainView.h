#ifndef IMAINVIEW_H
#define IMAINVIEW_H

#include <QString>
#include <QList>

// Forward declarations
class IPointCloudViewer;
class Project;

/**
 * @brief Abstract interface for the main application window
 * 
 * This interface defines the contract for the main view component in the MVP pattern.
 * It allows the presenter to update the UI without being coupled to QMainWindow
 * or other Qt-specific implementations.
 */
class IMainView
{
public:
    virtual ~IMainView() = default;

    // Window management
    virtual void setWindowTitle(const QString& title) = 0;
    virtual void updateWindowTitle() = 0;

    // Status bar operations
    virtual void updateStatusBar(const QString& text) = 0;
    virtual void setStatusReady() = 0;
    virtual void setStatusLoading(const QString& fileName) = 0;
    virtual void setStatusLoadSuccess(const QString& fileName, int pointCount) = 0;
    virtual void setStatusLoadFailed(const QString& fileName, const QString& message) = 0;
    virtual void setStatusViewChanged(const QString& viewName) = 0;

    // Error and message display
    virtual void displayErrorMessage(const QString& title, const QString& message) = 0;
    virtual void displayWarningMessage(const QString& title, const QString& message) = 0;
    virtual void displayInfoMessage(const QString& title, const QString& message) = 0;

    // Project management UI
    virtual void showProjectHub() = 0;
    virtual void transitionToProjectView(const QString& projectPath) = 0;
    virtual void enableProjectActions(bool enabled) = 0;
    virtual void showImportGuidance(bool show) = 0;

    // Point cloud viewer access
    virtual IPointCloudViewer* getViewer() = 0;

    // Progress feedback
    virtual void showProgressDialog(const QString& title, const QString& message) = 0;
    virtual void updateProgressDialog(int percentage, const QString& stage) = 0;
    virtual void hideProgressDialog() = 0;

    // Memory and performance display
    virtual void updateMemoryDisplay(size_t totalBytes) = 0;
    virtual void updatePerformanceStats(float fps, int visiblePoints) = 0;

    // UI state management
    virtual void setLoadingState(bool isLoading) = 0;
    virtual void updateLoadingProgress(int percentage, const QString& stage) = 0;

    // File dialog operations
    virtual QString showOpenFileDialog(const QString& title, const QString& filter) = 0;
    virtual QString showOpenProjectDialog() = 0;
    virtual QString showSaveFileDialog(const QString& title, const QString& filter) = 0;

    // Settings and configuration
    virtual bool showLoadingSettingsDialog() = 0;
    virtual bool showCreateProjectDialog(QString& projectName, QString& projectPath) = 0;
    virtual bool showScanImportDialog() = 0;

    // Scan management (simplified)
    virtual void refreshScanList() = 0;

    // View controls
    virtual void enableViewControls(bool enabled) = 0;
    virtual void updateViewControlsState() = 0;

    // Application state
    virtual bool isProjectOpen() const = 0;
    virtual QString getCurrentProjectPath() const = 0;
    virtual Project* getCurrentProject() const = 0;

    // Cleanup and shutdown
    virtual void prepareForShutdown() = 0;
    virtual void cleanupResources() = 0;
};

#endif // IMAINVIEW_H
