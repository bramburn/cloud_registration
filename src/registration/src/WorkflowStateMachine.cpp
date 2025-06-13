#include "registration/WorkflowStateMachine.h"

#include <QDebug>

// Define valid transitions matrix
const QMap<RegistrationStep, QList<RegistrationStep>> WorkflowStateMachine::validTransitions_ = {
    {RegistrationStep::SelectScans, {RegistrationStep::TargetDetection}},
    {RegistrationStep::TargetDetection, {RegistrationStep::SelectScans, RegistrationStep::ManualAlignment}},
    {RegistrationStep::ManualAlignment, {RegistrationStep::TargetDetection, RegistrationStep::ICPRegistration}},
    {RegistrationStep::ICPRegistration, {RegistrationStep::ManualAlignment, RegistrationStep::QualityReview}},
    {RegistrationStep::QualityReview, {RegistrationStep::ICPRegistration, RegistrationStep::Export}},
    {RegistrationStep::Export, {RegistrationStep::QualityReview}}};

WorkflowStateMachine::WorkflowStateMachine(QObject* parent)
    : QObject(parent), currentStep_(RegistrationStep::SelectScans)
{
    // Initialize step completion status
    for (const auto& step : getAllSteps())
    {
        stepCompletionStatus_[step] = false;
    }
}

bool WorkflowStateMachine::canTransitionTo(RegistrationStep nextStep) const
{
    return validateTransition(currentStep_, nextStep);
}

void WorkflowStateMachine::transitionTo(RegistrationStep nextStep)
{
    if (!canTransitionTo(nextStep))
    {
        QString reason = getTransitionBlockReason(currentStep_, nextStep);
        emit transitionBlocked(reason);
        return;
    }

    currentStep_ = nextStep;
    emit stepChanged(nextStep);

    if (nextStep == RegistrationStep::Export && isStepComplete(nextStep))
    {
        emit workflowCompleted();
    }
}

void WorkflowStateMachine::goNext()
{
    RegistrationStep nextStep = getNextStep(currentStep_);
    if (nextStep != currentStep_)
    {
        transitionTo(nextStep);
    }
}

void WorkflowStateMachine::goBack()
{
    RegistrationStep prevStep = getPreviousStep(currentStep_);
    if (prevStep != currentStep_)
    {
        transitionTo(prevStep);
    }
}

void WorkflowStateMachine::reset()
{
    currentStep_ = RegistrationStep::SelectScans;

    // Reset completion status
    for (auto& status : stepCompletionStatus_)
    {
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
    return {RegistrationStep::SelectScans,
            RegistrationStep::TargetDetection,
            RegistrationStep::ManualAlignment,
            RegistrationStep::ICPRegistration,
            RegistrationStep::QualityReview,
            RegistrationStep::Export};
}

int WorkflowStateMachine::getStepIndex(RegistrationStep step) const
{
    QList<RegistrationStep> steps = getAllSteps();
    return steps.indexOf(step);
}

RegistrationStep WorkflowStateMachine::getStepByIndex(int index) const
{
    QList<RegistrationStep> steps = getAllSteps();
    if (index >= 0 && index < steps.size())
    {
        return steps[index];
    }
    return RegistrationStep::SelectScans;
}

bool WorkflowStateMachine::validateTransition(RegistrationStep from, RegistrationStep to) const
{
    return validTransitions_.value(from).contains(to);
}

QString WorkflowStateMachine::getTransitionBlockReason(RegistrationStep from, RegistrationStep to) const
{
    if (!validateTransition(from, to))
    {
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
    if (currentIndex >= 0 && currentIndex < steps.size() - 1)
    {
        return steps[currentIndex + 1];
    }
    return current;
}

RegistrationStep WorkflowStateMachine::getPreviousStep(RegistrationStep current) const
{
    QList<RegistrationStep> steps = getAllSteps();
    int currentIndex = steps.indexOf(current);
    if (currentIndex > 0)
    {
        return steps[currentIndex - 1];
    }
    return current;
}

// Utility functions
QString registrationStepToString(RegistrationStep step)
{
    switch (step)
    {
        case RegistrationStep::SelectScans:
            return "Select Scans";
        case RegistrationStep::TargetDetection:
            return "Target Detection";
        case RegistrationStep::ManualAlignment:
            return "Manual Alignment";
        case RegistrationStep::ICPRegistration:
            return "ICP Registration";
        case RegistrationStep::QualityReview:
            return "Quality Review";
        case RegistrationStep::Export:
            return "Export";
    }
    return "Unknown";
}

RegistrationStep stringToRegistrationStep(const QString& stepString)
{
    if (stepString == "Select Scans")
        return RegistrationStep::SelectScans;
    if (stepString == "Target Detection")
        return RegistrationStep::TargetDetection;
    if (stepString == "Manual Alignment")
        return RegistrationStep::ManualAlignment;
    if (stepString == "ICP Registration")
        return RegistrationStep::ICPRegistration;
    if (stepString == "Quality Review")
        return RegistrationStep::QualityReview;
    if (stepString == "Export")
        return RegistrationStep::Export;
    return RegistrationStep::SelectScans;
}
