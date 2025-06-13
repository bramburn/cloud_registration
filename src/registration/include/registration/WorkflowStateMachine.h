#pragma once

#include <QObject>
#include <QStateMachine>
#include <QState>
#include <QMap>
#include <QList>

/**
 * @brief Enumeration of registration workflow steps
 */
enum class RegistrationStep {
    SelectScans,
    TargetDetection,
    ManualAlignment,
    ICPRegistration,
    QualityReview,
    Export
};

/**
 * @brief State machine for managing registration workflow transitions
 * 
 * This class implements a finite state machine that controls the flow
 * of the registration process. It ensures that users follow a logical
 * sequence of steps and validates transitions between workflow states.
 * 
 * Sprint 2 Implementation: Workflow state management
 */
class WorkflowStateMachine : public QObject
{
    Q_OBJECT

public:
    explicit WorkflowStateMachine(QObject* parent = nullptr);
    virtual ~WorkflowStateMachine() = default;

    // State queries
    RegistrationStep currentStep() const { return currentStep_; }
    bool canTransitionTo(RegistrationStep nextStep) const;
    
    // State transitions
    void transitionTo(RegistrationStep nextStep);
    void goNext();
    void goBack();
    void reset();
    
    // Step validation
    bool isStepComplete(RegistrationStep step) const;
    void setStepComplete(RegistrationStep step, bool complete);
    
    // Workflow queries
    bool canGoNext() const;
    bool canGoBack() const;
    bool isWorkflowComplete() const;
    
    // Utility
    QString getStepName(RegistrationStep step) const;
    QList<RegistrationStep> getAllSteps() const;
    int getStepIndex(RegistrationStep step) const;
    RegistrationStep getStepByIndex(int index) const;

signals:
    void stepChanged(RegistrationStep newStep);
    void transitionBlocked(const QString& reason);
    void workflowCompleted();
    void workflowReset();

private slots:
    void onStateEntered();

private:
    // State machine setup
    void setupStateMachine();
    void createStates();
    void setupTransitions();
    void connectSignals();
    
    // Validation
    bool validateTransition(RegistrationStep from, RegistrationStep to) const;
    QString getTransitionBlockReason(RegistrationStep from, RegistrationStep to) const;
    
    // Helper methods
    RegistrationStep getNextStep(RegistrationStep current) const;
    RegistrationStep getPreviousStep(RegistrationStep current) const;
    QState* getStateForStep(RegistrationStep step) const;

    // Qt State Machine components
    QStateMachine* stateMachine_;
    QState* selectScansState_;
    QState* targetDetectionState_;
    QState* manualAlignmentState_;
    QState* icpRegistrationState_;
    QState* qualityReviewState_;
    QState* exportState_;
    
    // Current state tracking
    RegistrationStep currentStep_;
    
    // Step completion tracking
    QMap<RegistrationStep, bool> stepCompletionStatus_;
    
    // Valid transitions matrix
    static const QMap<RegistrationStep, QList<RegistrationStep>> validTransitions_;
};

// Utility functions for enum conversion
QString registrationStepToString(RegistrationStep step);
RegistrationStep stringToRegistrationStep(const QString& stepString);

// Register metatype for Qt signals/slots
Q_DECLARE_METATYPE(RegistrationStep)

#endif // WORKFLOWSTATEMACHINE_H
