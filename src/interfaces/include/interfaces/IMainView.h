#ifndef IMAINVIEW_H
#define IMAINVIEW_H

#include <QObject>
#include <QString>
#include <QStringList>

#include <vector>

// Forward declarations
class IPointCloudViewer;

/**
 * @brief IMainView - Abstract interface for the main application window
 *
 * This interface defines the contract for the main view component of the application.
 * It enables loose coupling between the presentation logic (MainPresenter) and the
 * UI implementation (MainWindow), allowing for easy testing with mock implementations
 * and future substitution of different UI frameworks.
 *
 * Sprint 4 Decoupling Requirements:
 * - Provides abstraction layer for main window UI operations
 * - Enables dependency injection and polymorphic usage
 * - Supports unit testing with mock implementations
 * - Separates presentation logic from UI implementation
 */
class IMainView : public QObject
{
    Q_OBJECT

public:
    explicit IMainView(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IMainView() = default;

    /**
     * @brief Display an error message to the user
     * @param title Error dialog title
     * @param message Error message content
     */
    virtual void displayErrorMessage(const QString& title, const QString& message) = 0;

    /**
     * @brief Display an information message to the user
     * @param title Information dialog title
     * @param message Information message content
     */
    virtual void displayInfoMessage(const QString& title, const QString& message) = 0;

    /**
     * @brief Display a warning message to the user
     * @param title Warning dialog title
     * @param message Warning message content
     */
    virtual void displayWarningMessage(const QString& title, const QString& message) = 0;

    /**
     * @brief Update the status bar text
     * @param text Status message to display
     */
    virtual void updateStatusBar(const QString& text) = 0;

    /**
     * @brief Set the main window title
     * @param title Window title
     */
    virtual void setWindowTitle(const QString& title) = 0;

    /**
     * @brief Get access to the point cloud viewer
     * @return Pointer to the point cloud viewer interface
     */
    virtual IPointCloudViewer* getViewer() = 0;

    /**
     * @brief Get the sidebar widget (Sprint 3)
     * @return Pointer to the sidebar widget
     */
    virtual class SidebarWidget* getSidebar() = 0;

    /**
     * @brief Get the alignment control panel (Sprint 2.1)
     * @return Pointer to the alignment control panel
     */
    virtual class AlignmentControlPanel* getAlignmentControlPanel() = 0;

    /**
     * @brief Show or hide the progress dialog
     * @param show true to show, false to hide
     * @param title Progress dialog title
     * @param message Progress message
     */
    virtual void
    showProgressDialog(bool show, const QString& title = QString(), const QString& message = QString()) = 0;

    /**
     * @brief Update progress dialog
     * @param percentage Progress percentage (0-100)
     * @param message Progress message
     */
    virtual void updateProgress(int percentage, const QString& message) = 0;

    /**
     * @brief Enable or disable UI actions
     * @param enabled true to enable, false to disable
     */
    virtual void setActionsEnabled(bool enabled) = 0;

    /**
     * @brief Set the project title in the UI
     * @param projectName Name of the current project
     */
    virtual void setProjectTitle(const QString& projectName) = 0;

    /**
     * @brief Update the scan list in the sidebar
     * @param scanNames List of scan names
     */
    virtual void updateScanList(const QStringList& scanNames) = 0;

    /**
     * @brief Highlight a specific scan in the UI
     * @param scanName Name of the scan to highlight
     */
    virtual void highlightScan(const QString& scanName) = 0;

    /**
     * @brief Show the project hub view
     */
    virtual void showProjectHub() = 0;

    /**
     * @brief Show the project view
     */
    virtual void showProjectView() = 0;

    /**
     * @brief Update memory usage display
     * @param totalBytes Total memory usage in bytes
     */
    virtual void updateMemoryUsage(size_t totalBytes) = 0;

    /**
     * @brief Update rendering statistics display
     * @param fps Frames per second
     * @param visiblePoints Number of visible points
     */
    virtual void updateRenderingStats(float fps, int visiblePoints) = 0;

    /**
     * @brief Ask user for file path to open
     * @param title Dialog title
     * @param filter File filter (e.g., "E57 Files (*.e57)")
     * @return Selected file path, empty if cancelled
     */
    virtual QString askForOpenFilePath(const QString& title, const QString& filter) = 0;

    /**
     * @brief Ask user for file path to save
     * @param title Dialog title
     * @param filter File filter (e.g., "E57 Files (*.e57)")
     * @param defaultName Default file name
     * @return Selected file path, empty if cancelled
     */
    virtual QString
    askForSaveFilePath(const QString& title, const QString& filter, const QString& defaultName = QString()) = 0;

    /**
     * @brief Ask user for confirmation
     * @param title Dialog title
     * @param message Confirmation message
     * @return true if user confirmed, false otherwise
     */
    virtual bool askForConfirmation(const QString& title, const QString& message) = 0;

    // Sprint 3: Sidebar operation interface methods
    /**
     * @brief Prompt user for cluster name
     * @param title Dialog title
     * @param defaultName Default cluster name
     * @return Entered cluster name, empty if cancelled
     */
    virtual QString promptForClusterName(const QString& title, const QString& defaultName = QString()) = 0;

    /**
     * @brief Load a scan through the view's load manager
     * @param scanId ID of scan to load
     */
    virtual void loadScan(const QString& scanId) = 0;

    /**
     * @brief Unload a scan through the view's load manager
     * @param scanId ID of scan to unload
     */
    virtual void unloadScan(const QString& scanId) = 0;

    /**
     * @brief Load a cluster through the view's load manager
     * @param clusterId ID of cluster to load
     */
    virtual void loadCluster(const QString& clusterId) = 0;

    /**
     * @brief Unload a cluster through the view's load manager
     * @param clusterId ID of cluster to unload
     */
    virtual void unloadCluster(const QString& clusterId) = 0;

    /**
     * @brief View point cloud for item
     * @param itemId ID of item to view
     * @param itemType Type of item (scan/cluster)
     */
    virtual void viewPointCloud(const QString& itemId, const QString& itemType) = 0;

    /**
     * @brief Delete a scan
     * @param scanId ID of scan to delete
     * @param deletePhysicalFile Whether to delete physical file
     */
    virtual void deleteScan(const QString& scanId, bool deletePhysicalFile) = 0;

    /**
     * @brief Perform batch operation on scans
     * @param operation Operation type (load/unload)
     * @param scanIds List of scan IDs
     */
    virtual void performBatchOperation(const QString& operation, const QStringList& scanIds) = 0;

signals:
    /**
     * @brief Emitted when user requests to create a new project
     */
    void newProjectRequested();

    /**
     * @brief Emitted when user requests to open a project
     */
    void openProjectRequested();

    /**
     * @brief Emitted when user requests to close the current project
     */
    void closeProjectRequested();

    /**
     * @brief Emitted when user requests to import scans
     */
    void importScansRequested();

    /**
     * @brief Emitted when user requests to open a file
     * @param filePath Path of the file to open
     */
    void openFileRequested(const QString& filePath);

    /**
     * @brief Emitted when user requests to save a file
     * @param filePath Path where to save the file
     */
    void saveFileRequested(const QString& filePath);

    /**
     * @brief Emitted when user activates a scan
     * @param scanId ID of the activated scan
     */
    void scanActivated(const QString& scanId);

    /**
     * @brief Emitted when user changes viewer settings
     */
    void viewerSettingsChanged();

    /**
     * @brief Emitted when application should exit
     */
    void exitRequested();
};

#endif  // IMAINVIEW_H
