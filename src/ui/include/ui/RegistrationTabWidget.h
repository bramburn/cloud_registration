#pragma once

#include <QTabWidget>
#include <QWidget>
#include <memory>

// Forward declarations
class PoseGraphViewerWidget;
namespace Registration {
    class RegistrationWorkflowWidget;
    class RegistrationProject;
}

/**
 * @brief Tab widget that combines registration workflow and pose graph visualization
 * 
 * This widget provides a tabbed interface that includes:
 * - Registration Workflow tab: Step-by-step registration process
 * - Pose Graph tab: Visualization of scan connectivity and relationships
 * 
 * This is the main registration interface that users will interact with.
 */
class RegistrationTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit RegistrationTabWidget(QWidget* parent = nullptr);
    virtual ~RegistrationTabWidget() = default;

    /**
     * @brief Set the registration project for both workflow and pose graph
     * @param project Pointer to the registration project
     */
    void setRegistrationProject(Registration::RegistrationProject* project);

    /**
     * @brief Get the registration workflow widget
     * @return Pointer to the workflow widget
     */
    Registration::RegistrationWorkflowWidget* getWorkflowWidget() const;

    /**
     * @brief Get the pose graph viewer widget
     * @return Pointer to the pose graph viewer
     */
    PoseGraphViewerWidget* getPoseGraphViewer() const;

    /**
     * @brief Switch to the workflow tab
     */
    void showWorkflowTab();

    /**
     * @brief Switch to the pose graph tab
     */
    void showPoseGraphTab();

signals:
    /**
     * @brief Emitted when the active tab changes
     * @param tabName Name of the active tab
     */
    void activeTabChanged(const QString& tabName);

    /**
     * @brief Emitted when the project changes
     */
    void projectChanged();

private slots:
    /**
     * @brief Handle tab change events
     * @param index New tab index
     */
    void onTabChanged(int index);

    /**
     * @brief Handle workflow completion
     */
    void onWorkflowCompleted();

    /**
     * @brief Handle pose graph node selection
     * @param scanId Selected scan ID
     */
    void onPoseGraphNodeSelected(const QString& scanId);

    /**
     * @brief Handle pose graph edge selection
     * @param sourceScanId Source scan ID
     * @param targetScanId Target scan ID
     */
    void onPoseGraphEdgeSelected(const QString& sourceScanId, const QString& targetScanId);

private:
    // UI setup
    void setupUI();
    void createWorkflowTab();
    void createPoseGraphTab();
    void setupConnections();

    // Tab management
    void updateTabStates();
    QString getTabName(int index) const;

    // Widgets
    Registration::RegistrationWorkflowWidget* m_workflowWidget;
    PoseGraphViewerWidget* m_poseGraphViewer;

    // Project reference
    Registration::RegistrationProject* m_project;

    // Tab indices
    int m_workflowTabIndex;
    int m_poseGraphTabIndex;
};
