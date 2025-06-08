#ifndef REGISTRATIONWORKFLOWWIDGET_H
#define REGISTRATIONWORKFLOWWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <memory>
#include "WorkflowStateMachine.h"
#include "../ui/WorkflowProgressWidget.h"

// Forward declarations
class TargetManager;
class RegistrationProject;
class ScanComparisonView;
class TargetManagementPanel;

/**
 * @brief Main widget for registration workflow management
 *
 * This widget provides the complete user interface for the registration workflow.
 * It contains the progress indicator, step-specific content areas, and navigation
 * controls. It coordinates between the state machine and the various UI components.
 *
 * Sprint 2 Implementation: Registration workflow UI foundation
 */
class RegistrationWorkflowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RegistrationWorkflowWidget(QWidget* parent = nullptr);
    virtual ~RegistrationWorkflowWidget() = default;

    // Project management
    void setProject(RegistrationProject* project);
    RegistrationProject* project() const { return project_; }

    // Workflow control
    void startWorkflow();
    void resetWorkflow();
    void goToStep(RegistrationStep step);
    RegistrationStep currentStep() const;

    // Step completion
    void setStepComplete(RegistrationStep step, bool complete);
    bool isStepComplete(RegistrationStep step) const;

    // Navigation
    bool canGoNext() const;
    bool canGoBack() const;
    void enableNavigation(bool enabled);

signals:
    void workflowStarted();
    void workflowCompleted();
    void stepChanged(RegistrationStep step);
    void projectChanged();

public slots:
    void goNext();
    void goBack();

private slots:
    void onStateMachineStepChanged(RegistrationStep step);
    void onStateMachineTransitionBlocked(const QString& reason);
    void onProgressWidgetStepClicked(RegistrationStep step);
    void onStepValidationChanged(RegistrationStep step, bool isValid);

private:
    // UI setup
    void setupUI();
    void createProgressArea();
    void createContentArea();
    void createNavigationArea();
    void createStepWidgets();
    void setupConnections();
    void setupStyling();

    // Step management
    void updateCurrentStepWidget();
    void updateNavigationButtons();
    void updateStepValidation();

    // Step widget creation
    QWidget* createScanSelectionWidget();
    QWidget* createTargetDetectionWidget();
    QWidget* createManualAlignmentWidget();
    QWidget* createICPRegistrationWidget();
    QWidget* createQualityReviewWidget();
    QWidget* createExportWidget();

    // Validation
    bool validateCurrentStep() const;
    bool validateScanSelection() const;
    bool validateTargetDetection() const;
    bool validateManualAlignment() const;
    bool validateICPRegistration() const;
    bool validateQualityReview() const;

    // Utility
    QString getStepTitle(RegistrationStep step) const;
    QString getStepInstructions(RegistrationStep step) const;

    // Main layout components
    QVBoxLayout* mainLayout_;
    WorkflowProgressWidget* progressWidget_;
    QStackedWidget* contentStack_;
    QHBoxLayout* navigationLayout_;

    // Navigation controls
    QPushButton* backButton_;
    QPushButton* nextButton_;
    QPushButton* cancelButton_;
    QLabel* statusLabel_;

    // Step widgets
    QWidget* scanSelectionWidget_;
    QWidget* targetDetectionWidget_;
    QWidget* manualAlignmentWidget_;
    QWidget* icpRegistrationWidget_;
    QWidget* qualityReviewWidget_;
    QWidget* exportWidget_;

    // Core components
    std::unique_ptr<WorkflowStateMachine> stateMachine_;
    std::unique_ptr<TargetManager> targetManager_;
    RegistrationProject* project_;

    // Specialized UI components (to be implemented in future sprints)
    ScanComparisonView* scanComparisonView_;
    TargetManagementPanel* targetManagementPanel_;

    // State
    bool navigationEnabled_;
    QMap<RegistrationStep, bool> stepCompletionStatus_;
};

#endif // REGISTRATIONWORKFLOWWIDGET_H
