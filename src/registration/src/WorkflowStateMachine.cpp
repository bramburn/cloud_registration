#include "registration/WorkflowStateMachine.h"
#include <QDebug>

// Define valid transitions matrix
const QMap<RegistrationStep, QList<RegistrationStep>> WorkflowStateMachine::validTransitions_ = {
    {RegistrationStep::SelectScans, {RegistrationStep::TargetDetection}},
    {RegistrationStep::TargetDetection, {RegistrationStep::SelectScans, RegistrationStep::ManualAlignment}},
    {RegistrationStep::ManualAlignment, {RegistrationStep::TargetDetection, RegistrationStep::ICPRegistration}},
    {RegistrationStep::ICPRegistration, {RegistrationStep::ManualAlignment, RegistrationStep::QualityReview}},
    {RegistrationStep::QualityReview, {RegistrationStep::ICPRegistration, RegistrationStep::Export}},
    {RegistrationStep::Export, {RegistrationStep::QualityReview}}
};

WorkflowStateMachine::WorkflowStateMachine(QObject* parent)
    : QObject(parent)
    , stateMachine_(new QStateMachine(this))
    , selectScansState_(nullptr)
    , targetDetectionState_(nullptr)
    , manualAlignmentState_(nullptr)
    , icpRegistrationState_(nullptr)
    , qualityReviewState_(nullptr)
    , exportState_(nullptr)
    , currentStep_(RegistrationStep::SelectScans)
{
    setupStateMachine();
    
    // Initialize step completion status
    for (const auto& step : getAllSteps()) {
        stepCompletionStatus_[step] = false;
    }
}

void WorkflowStateMachine::setupStateMachine()
{
    createStates();
    setupTransitions();
    connectSignals();
    
    stateMachine_->setInitialState(selectScansState_);
    stateMachine_->start();
}

void WorkflowStateMachine::createStates()
{
    selectScansState_ = new QState(stateMachine_);
    targetDetectionState_ = new QState(stateMachine_);
    manualAlignmentState_ = new QState(stateMachine_);
    icpRegistrationState_ = new QState(stateMachine_);
    qualityReviewState_ = new QState(stateMachine_);
    exportState_ = new QState(stateMachine_);
}

void WorkflowStateMachine::setupTransitions()
{
    // Define transitions between states
    // Note: In a real implementation, these would be triggered by specific signals
    // For now, we'll manage transitions manually through transitionTo()
}

void WorkflowStateMachine::connectSignals()
{
    connect(selectScansState_, &QState::entered, this, &WorkflowStateMachine::onStateEntered);
    connect(targetDetectionState_, &QState::entered, this, &WorkflowStateMachine::onStateEntered);
    connect(manualAlignmentState_, &QState::entered, this, &WorkflowStateMachine::onStateEntered);
    connect(icpRegistrationState_, &QState::entered, this, &WorkflowStateMachine::onStateEntered);
    connect(qualityReviewState_, &QState::entered, this, &WorkflowStateMachine::onStateEntered);
    connect(exportState_, &QState::entered, this, &WorkflowStateMachine::onStateEntered);
}

bool WorkflowStateMachine::canTransitionTo(RegistrationStep nextStep) const
{
    return validateTransition(currentStep_, nextStep);
}

void WorkflowStateMachine::transitionTo(RegistrationStep nextStep)
{
    if (!canTransitionTo(nextStep)) {
        QString reason = getTransitionBlockReason(currentStep_, nextStep);
        emit transitionBlocked(reason);
        return;
    }
    
    currentStep_ = nextStep;
    emit stepChanged(nextStep);
    
    if (nextStep == RegistrationStep::Export && isStepComplete(nextStep)) {
        emit workflowCompleted();
    }
}

void WorkflowStateMachine::goNext()
{
    RegistrationStep nextStep = getNextStep(currentStep_);
    if (nextStep != currentStep_) {
        transitionTo(nextStep);
    }
}

void WorkflowStateMachine::goBack()
{
    RegistrationStep prevStep = getPreviousStep(currentStep_);
    if (prevStep != currentStep_) {
        transitionTo(prevStep);
    }
}

void WorkflowStateMachine::reset()
{
    currentStep_ = RegistrationStep::SelectScans;
    
    // Reset completion status
    for (auto& status : stepCompletionStatus_) {
        status = false;
    }
    
    emit stepChanged(currentStep_);
    emit workflowReset();
}

bool WorkflowStateMachine::isStepComplete(RegistrationStep step) const
{
    return stepCompletionStatus_.value(step, false);
}

void WorkflowStateMachine::setStepComplete(RegistrationStep step, bool complete)
{
    stepCompletionStatus_[step] = complete;
}

bool WorkflowStateMachine::canGoNext() const
{
    RegistrationStep nextStep = getNextStep(currentStep_);
    return nextStep != currentStep_ && canTransitionTo(nextStep);
}

bool WorkflowStateMachine::canGoBack() const
{
    RegistrationStep prevStep = getPreviousStep(currentStep_);
    return prevStep != currentStep_ && canTransitionTo(prevStep);
}

bool WorkflowStateMachine::isWorkflowComplete() const
{
    return currentStep_ == RegistrationStep::Export && isStepComplete(RegistrationStep::Export);
}

QString WorkflowStateMachine::getStepName(RegistrationStep step) const
{
    return registrationStepToString(step);
}

QList<RegistrationStep> WorkflowStateMachine::getAllSteps() const
{
    return {
        RegistrationStep::SelectScans,
        RegistrationStep::TargetDetection,
        RegistrationStep::ManualAlignment,
        RegistrationStep::ICPRegistration,
        RegistrationStep::QualityReview,
        RegistrationStep::Export
    };
}

int WorkflowStateMachine::getStepIndex(RegistrationStep step) const
{
    QList<RegistrationStep> steps = getAllSteps();
    return steps.indexOf(step);
}

RegistrationStep WorkflowStateMachine::getStepByIndex(int index) const
{
    QList<RegistrationStep> steps = getAllSteps();
    if (index >= 0 && index < steps.size()) {
        return steps[index];
    }
    return RegistrationStep::SelectScans;
}

void WorkflowStateMachine::onStateEntered()
{
    // This slot is called when any state is entered
    // Additional state-specific logic can be added here
}

bool WorkflowStateMachine::validateTransition(RegistrationStep from, RegistrationStep to) const
{
    return validTransitions_.value(from).contains(to);
}

QString WorkflowStateMachine::getTransitionBlockReason(RegistrationStep from, RegistrationStep to) const
{
    if (!validateTransition(from, to)) {
        return QString("Invalid transition from %1 to %2")
               .arg(registrationStepToString(from))
               .arg(registrationStepToString(to));
    }
    return QString();
}

RegistrationStep WorkflowStateMachine::getNextStep(RegistrationStep current) const
{
    QList<RegistrationStep> steps = getAllSteps();
    int currentIndex = steps.indexOf(current);
    if (currentIndex >= 0 && currentIndex < steps.size() - 1) {
        return steps[currentIndex + 1];
    }
    return current;
}

RegistrationStep WorkflowStateMachine::getPreviousStep(RegistrationStep current) const
{
    QList<RegistrationStep> steps = getAllSteps();
    int currentIndex = steps.indexOf(current);
    if (currentIndex > 0) {
        return steps[currentIndex - 1];
    }
    return current;
}

QState* WorkflowStateMachine::getStateForStep(RegistrationStep step) const
{
    switch (step) {
        case RegistrationStep::SelectScans: return selectScansState_;
        case RegistrationStep::TargetDetection: return targetDetectionState_;
        case RegistrationStep::ManualAlignment: return manualAlignmentState_;
        case RegistrationStep::ICPRegistration: return icpRegistrationState_;
        case RegistrationStep::QualityReview: return qualityReviewState_;
        case RegistrationStep::Export: return exportState_;
    }
    return selectScansState_;
}

// Utility functions
QString registrationStepToString(RegistrationStep step)
{
    switch (step) {
        case RegistrationStep::SelectScans: return "Select Scans";
        case RegistrationStep::TargetDetection: return "Target Detection";
        case RegistrationStep::ManualAlignment: return "Manual Alignment";
        case RegistrationStep::ICPRegistration: return "ICP Registration";
        case RegistrationStep::QualityReview: return "Quality Review";
        case RegistrationStep::Export: return "Export";
    }
    return "Unknown";
}

RegistrationStep stringToRegistrationStep(const QString& stepString)
{
    if (stepString == "Select Scans") return RegistrationStep::SelectScans;
    if (stepString == "Target Detection") return RegistrationStep::TargetDetection;
    if (stepString == "Manual Alignment") return RegistrationStep::ManualAlignment;
    if (stepString == "ICP Registration") return RegistrationStep::ICPRegistration;
    if (stepString == "Quality Review") return RegistrationStep::QualityReview;
    if (stepString == "Export") return RegistrationStep::Export;
    return RegistrationStep::SelectScans;
}
