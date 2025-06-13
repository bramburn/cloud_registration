#include "ui/RegistrationTabWidget.h"
#include "ui/PoseGraphViewerWidget.h"
#include "registration/RegistrationWorkflowWidget.h"
#include "registration/RegistrationProject.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>

RegistrationTabWidget::RegistrationTabWidget(QWidget* parent)
    : QTabWidget(parent),
      m_workflowWidget(nullptr),
      m_poseGraphViewer(nullptr),
      m_project(nullptr),
      m_workflowTabIndex(-1),
      m_poseGraphTabIndex(-1)
{
    setupUI();
}

void RegistrationTabWidget::setupUI()
{
    // Configure tab widget
    setTabPosition(QTabWidget::North);
    setMovable(false);
    setTabsClosable(false);

    createWorkflowTab();
    createPoseGraphTab();
    setupConnections();

    // Set initial tab
    setCurrentIndex(m_workflowTabIndex);
}

void RegistrationTabWidget::createWorkflowTab()
{
    m_workflowWidget = new Registration::RegistrationWorkflowWidget(this);
    m_workflowTabIndex = addTab(m_workflowWidget, "Registration Workflow");
    
    // Set tab icon (optional)
    setTabIcon(m_workflowTabIndex, QIcon(":/icons/workflow.png"));
    
    qDebug() << "RegistrationTabWidget: Created workflow tab at index" << m_workflowTabIndex;
}

void RegistrationTabWidget::createPoseGraphTab()
{
    m_poseGraphViewer = new PoseGraphViewerWidget(this);
    m_poseGraphTabIndex = addTab(m_poseGraphViewer, "Pose Graph");
    
    // Set tab icon (optional)
    setTabIcon(m_poseGraphTabIndex, QIcon(":/icons/graph.png"));
    
    qDebug() << "RegistrationTabWidget: Created pose graph tab at index" << m_poseGraphTabIndex;
}

void RegistrationTabWidget::setupConnections()
{
    // Tab change connections
    connect(this, &QTabWidget::currentChanged, this, &RegistrationTabWidget::onTabChanged);

    // Workflow connections
    if (m_workflowWidget) {
        connect(m_workflowWidget, &Registration::RegistrationWorkflowWidget::workflowCompleted,
                this, &RegistrationTabWidget::onWorkflowCompleted);
        
        connect(m_workflowWidget, &Registration::RegistrationWorkflowWidget::projectChanged,
                this, &RegistrationTabWidget::projectChanged);
    }

    // Pose graph connections
    if (m_poseGraphViewer) {
        connect(m_poseGraphViewer, &PoseGraphViewerWidget::nodeSelected,
                this, &RegistrationTabWidget::onPoseGraphNodeSelected);
        
        connect(m_poseGraphViewer, &PoseGraphViewerWidget::edgeSelected,
                this, &RegistrationTabWidget::onPoseGraphEdgeSelected);
    }
}

void RegistrationTabWidget::setRegistrationProject(Registration::RegistrationProject* project)
{
    if (m_project != project) {
        m_project = project;
        
        // Set project for workflow widget
        if (m_workflowWidget) {
            m_workflowWidget->setProject(project);
        }
        
        // Note: Pose graph viewer gets updated through MainPresenter
        // when the project changes and pose graph is rebuilt
        
        updateTabStates();
        emit projectChanged();
        
        qDebug() << "RegistrationTabWidget: Project set";
    }
}

Registration::RegistrationWorkflowWidget* RegistrationTabWidget::getWorkflowWidget() const
{
    return m_workflowWidget;
}

PoseGraphViewerWidget* RegistrationTabWidget::getPoseGraphViewer() const
{
    return m_poseGraphViewer;
}

void RegistrationTabWidget::showWorkflowTab()
{
    if (m_workflowTabIndex >= 0) {
        setCurrentIndex(m_workflowTabIndex);
    }
}

void RegistrationTabWidget::showPoseGraphTab()
{
    if (m_poseGraphTabIndex >= 0) {
        setCurrentIndex(m_poseGraphTabIndex);
    }
}

void RegistrationTabWidget::onTabChanged(int index)
{
    QString tabName = getTabName(index);
    
    qDebug() << "RegistrationTabWidget: Tab changed to" << tabName << "at index" << index;
    
    // Update tab states when switching
    updateTabStates();
    
    emit activeTabChanged(tabName);
}

void RegistrationTabWidget::onWorkflowCompleted()
{
    qDebug() << "RegistrationTabWidget: Workflow completed, switching to pose graph tab";
    
    // Automatically switch to pose graph tab when workflow is completed
    showPoseGraphTab();
}

void RegistrationTabWidget::onPoseGraphNodeSelected(const QString& scanId)
{
    qDebug() << "RegistrationTabWidget: Pose graph node selected:" << scanId;
    
    // Could trigger actions like highlighting the scan in the workflow
    // or switching to a specific workflow step
}

void RegistrationTabWidget::onPoseGraphEdgeSelected(const QString& sourceScanId, const QString& targetScanId)
{
    qDebug() << "RegistrationTabWidget: Pose graph edge selected:" << sourceScanId << "to" << targetScanId;
    
    // Could trigger actions like showing registration details
    // or switching to quality review step
}

void RegistrationTabWidget::updateTabStates()
{
    // Enable/disable tabs based on project state
    bool hasProject = (m_project != nullptr);
    
    // Workflow tab is always available if there's a project
    setTabEnabled(m_workflowTabIndex, hasProject);
    
    // Pose graph tab is available if there's a project
    // (even if empty, it can show "no registrations" message)
    setTabEnabled(m_poseGraphTabIndex, hasProject);
    
    // Update tab tooltips
    if (hasProject) {
        setTabToolTip(m_workflowTabIndex, "Step-by-step registration workflow");
        setTabToolTip(m_poseGraphTabIndex, "Visualization of scan connectivity");
    } else {
        setTabToolTip(m_workflowTabIndex, "Load a project to access the registration workflow");
        setTabToolTip(m_poseGraphTabIndex, "Load a project to view the pose graph");
    }
}

QString RegistrationTabWidget::getTabName(int index) const
{
    if (index == m_workflowTabIndex) {
        return "Workflow";
    } else if (index == m_poseGraphTabIndex) {
        return "Pose Graph";
    } else {
        return "Unknown";
    }
}
