#include "RegistrationWorkflowWidget.h"
#include "TargetManager.h"
#include <QDebug>
#include <QMessageBox>
#include <QGroupBox>
#include <QTextEdit>

RegistrationWorkflowWidget::RegistrationWorkflowWidget(QWidget* parent)
    : QWidget(parent)
    , mainLayout_(nullptr)
    , progressWidget_(nullptr)
    , contentStack_(nullptr)
    , navigationLayout_(nullptr)
    , backButton_(nullptr)
    , nextButton_(nullptr)
    , cancelButton_(nullptr)
    , statusLabel_(nullptr)
    , scanSelectionWidget_(nullptr)
    , targetDetectionWidget_(nullptr)
    , manualAlignmentWidget_(nullptr)
    , icpRegistrationWidget_(nullptr)
    , qualityReviewWidget_(nullptr)
    , exportWidget_(nullptr)
    , stateMachine_(std::make_unique<WorkflowStateMachine>(this))
    , targetManager_(std::make_unique<TargetManager>(this))
    , project_(nullptr)
    , scanComparisonView_(nullptr)
    , targetManagementPanel_(nullptr)
    , navigationEnabled_(true)
{
    setupUI();
}

void RegistrationWorkflowWidget::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(10, 10, 10, 10);
    mainLayout_->setSpacing(10);
    
    createProgressArea();
    createContentArea();
    createNavigationArea();
    createStepWidgets();
    setupConnections();
    setupStyling();
    
    // Initialize state
    updateCurrentStepWidget();
    updateNavigationButtons();
}

void RegistrationWorkflowWidget::createProgressArea()
{
    progressWidget_ = new WorkflowProgressWidget(this);
    mainLayout_->addWidget(progressWidget_);
}

void RegistrationWorkflowWidget::createContentArea()
{
    contentStack_ = new QStackedWidget(this);
    contentStack_->setMinimumHeight(400);
    mainLayout_->addWidget(contentStack_, 1); // Give it stretch factor
}

void RegistrationWorkflowWidget::createNavigationArea()
{
    navigationLayout_ = new QHBoxLayout();
    
    // Status label
    statusLabel_ = new QLabel("Ready to start registration workflow");
    statusLabel_->setStyleSheet("color: #666; font-style: italic;");
    
    // Navigation buttons
    backButton_ = new QPushButton("← Back");
    nextButton_ = new QPushButton("Next →");
    cancelButton_ = new QPushButton("Cancel");
    
    backButton_->setMinimumWidth(100);
    nextButton_->setMinimumWidth(100);
    cancelButton_->setMinimumWidth(100);
    
    // Layout
    navigationLayout_->addWidget(statusLabel_);
    navigationLayout_->addStretch();
    navigationLayout_->addWidget(backButton_);
    navigationLayout_->addWidget(nextButton_);
    navigationLayout_->addWidget(cancelButton_);
    
    mainLayout_->addLayout(navigationLayout_);
}

void RegistrationWorkflowWidget::createStepWidgets()
{
    scanSelectionWidget_ = createScanSelectionWidget();
    targetDetectionWidget_ = createTargetDetectionWidget();
    manualAlignmentWidget_ = createManualAlignmentWidget();
    icpRegistrationWidget_ = createICPRegistrationWidget();
    qualityReviewWidget_ = createQualityReviewWidget();
    exportWidget_ = createExportWidget();
    
    contentStack_->addWidget(scanSelectionWidget_);
    contentStack_->addWidget(targetDetectionWidget_);
    contentStack_->addWidget(manualAlignmentWidget_);
    contentStack_->addWidget(icpRegistrationWidget_);
    contentStack_->addWidget(qualityReviewWidget_);
    contentStack_->addWidget(exportWidget_);
}

void RegistrationWorkflowWidget::setupConnections()
{
    // State machine connections
    connect(stateMachine_.get(), &WorkflowStateMachine::stepChanged,
            this, &RegistrationWorkflowWidget::onStateMachineStepChanged);
    connect(stateMachine_.get(), &WorkflowStateMachine::transitionBlocked,
            this, &RegistrationWorkflowWidget::onStateMachineTransitionBlocked);
    connect(stateMachine_.get(), &WorkflowStateMachine::stepValidationChanged,
            this, &RegistrationWorkflowWidget::onStepValidationChanged);
    
    // Progress widget connections
    connect(progressWidget_, &WorkflowProgressWidget::stepClicked,
            this, &RegistrationWorkflowWidget::onProgressWidgetStepClicked);
    
    // Navigation button connections
    connect(backButton_, &QPushButton::clicked, this, &RegistrationWorkflowWidget::goBack);
    connect(nextButton_, &QPushButton::clicked, this, &RegistrationWorkflowWidget::goNext);
    connect(cancelButton_, &QPushButton::clicked, this, &RegistrationWorkflowWidget::resetWorkflow);
}

void RegistrationWorkflowWidget::setupStyling()
{
    setStyleSheet(
        "QGroupBox { "
        "font-weight: bold; "
        "border: 2px solid #CCCCCC; "
        "border-radius: 5px; "
        "margin-top: 1ex; "
        "} "
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "left: 10px; "
        "padding: 0 5px 0 5px; "
        "} "
        "QPushButton { "
        "padding: 8px 16px; "
        "border: 1px solid #CCCCCC; "
        "border-radius: 4px; "
        "background-color: #F5F5F5; "
        "} "
        "QPushButton:hover { "
        "background-color: #E0E0E0; "
        "} "
        "QPushButton:pressed { "
        "background-color: #D0D0D0; "
        "} "
        "QPushButton:disabled { "
        "background-color: #F9F9F9; "
        "color: #CCCCCC; "
        "}"
    );
}

QWidget* RegistrationWorkflowWidget::createScanSelectionWidget()
{
    QGroupBox* groupBox = new QGroupBox("Select Scans for Registration");
    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    
    QLabel* instructions = new QLabel(getStepInstructions(RegistrationStep::SelectScans));
    instructions->setWordWrap(true);
    layout->addWidget(instructions);
    
    // Placeholder for scan selection UI
    QTextEdit* placeholder = new QTextEdit();
    placeholder->setPlainText("Scan selection interface will be implemented here.\n\n"
                             "This will include:\n"
                             "- List of available scans in the project\n"
                             "- Multi-selection capability\n"
                             "- Scan preview thumbnails\n"
                             "- Scan metadata display");
    placeholder->setMaximumHeight(200);
    placeholder->setReadOnly(true);
    layout->addWidget(placeholder);
    
    layout->addStretch();
    return groupBox;
}

QWidget* RegistrationWorkflowWidget::createTargetDetectionWidget()
{
    QGroupBox* groupBox = new QGroupBox("Target Detection");
    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    
    QLabel* instructions = new QLabel(getStepInstructions(RegistrationStep::TargetDetection));
    instructions->setWordWrap(true);
    layout->addWidget(instructions);
    
    // Placeholder for target detection UI
    QTextEdit* placeholder = new QTextEdit();
    placeholder->setPlainText("Target detection interface will be implemented here.\n\n"
                             "This will include:\n"
                             "- Automatic sphere detection\n"
                             "- Checkerboard pattern detection\n"
                             "- Manual point selection tools\n"
                             "- Target quality assessment");
    placeholder->setMaximumHeight(200);
    placeholder->setReadOnly(true);
    layout->addWidget(placeholder);
    
    layout->addStretch();
    return groupBox;
}

QWidget* RegistrationWorkflowWidget::createManualAlignmentWidget()
{
    QGroupBox* groupBox = new QGroupBox("Manual Alignment");
    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    
    QLabel* instructions = new QLabel(getStepInstructions(RegistrationStep::ManualAlignment));
    instructions->setWordWrap(true);
    layout->addWidget(instructions);
    
    // Placeholder for manual alignment UI
    QTextEdit* placeholder = new QTextEdit();
    placeholder->setPlainText("Manual alignment interface will be implemented here.\n\n"
                             "This will include:\n"
                             "- Side-by-side scan comparison\n"
                             "- Target correspondence creation\n"
                             "- Real-time transformation preview\n"
                             "- Alignment quality metrics");
    placeholder->setMaximumHeight(200);
    placeholder->setReadOnly(true);
    layout->addWidget(placeholder);
    
    layout->addStretch();
    return groupBox;
}

QWidget* RegistrationWorkflowWidget::createICPRegistrationWidget()
{
    QGroupBox* groupBox = new QGroupBox("ICP Registration");
    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    
    QLabel* instructions = new QLabel(getStepInstructions(RegistrationStep::ICPRegistration));
    instructions->setWordWrap(true);
    layout->addWidget(instructions);
    
    // Placeholder for ICP registration UI
    QTextEdit* placeholder = new QTextEdit();
    placeholder->setPlainText("ICP registration interface will be implemented here.\n\n"
                             "This will include:\n"
                             "- ICP algorithm configuration\n"
                             "- Progress monitoring\n"
                             "- Convergence visualization\n"
                             "- Result validation");
    placeholder->setMaximumHeight(200);
    placeholder->setReadOnly(true);
    layout->addWidget(placeholder);
    
    layout->addStretch();
    return groupBox;
}

QWidget* RegistrationWorkflowWidget::createQualityReviewWidget()
{
    QGroupBox* groupBox = new QGroupBox("Quality Review");
    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    
    QLabel* instructions = new QLabel(getStepInstructions(RegistrationStep::QualityReview));
    instructions->setWordWrap(true);
    layout->addWidget(instructions);
    
    // Placeholder for quality review UI
    QTextEdit* placeholder = new QTextEdit();
    placeholder->setPlainText("Quality review interface will be implemented here.\n\n"
                             "This will include:\n"
                             "- Registration accuracy metrics\n"
                             "- Error visualization\n"
                             "- Quality assessment reports\n"
                             "- Acceptance/rejection controls");
    placeholder->setMaximumHeight(200);
    placeholder->setReadOnly(true);
    layout->addWidget(placeholder);
    
    layout->addStretch();
    return groupBox;
}

QWidget* RegistrationWorkflowWidget::createExportWidget()
{
    QGroupBox* groupBox = new QGroupBox("Export Results");
    QVBoxLayout* layout = new QVBoxLayout(groupBox);
    
    QLabel* instructions = new QLabel(getStepInstructions(RegistrationStep::Export));
    instructions->setWordWrap(true);
    layout->addWidget(instructions);
    
    // Placeholder for export UI
    QTextEdit* placeholder = new QTextEdit();
    placeholder->setPlainText("Export interface will be implemented here.\n\n"
                             "This will include:\n"
                             "- File format selection\n"
                             "- Export options configuration\n"
                             "- Progress monitoring\n"
                             "- Result validation");
    placeholder->setMaximumHeight(200);
    placeholder->setReadOnly(true);
    layout->addWidget(placeholder);
    
    layout->addStretch();
    return groupBox;
}

void RegistrationWorkflowWidget::setProject(RegistrationProject* project)
{
    if (project_ != project) {
        project_ = project;
        emit projectChanged();
        qDebug() << "RegistrationWorkflowWidget: Project set";
    }
}

void RegistrationWorkflowWidget::startWorkflow()
{
    resetWorkflow();
    emit workflowStarted();
    qDebug() << "RegistrationWorkflowWidget: Workflow started";
}

void RegistrationWorkflowWidget::resetWorkflow()
{
    stateMachine_->transitionTo(RegistrationStep::SelectScans);
    
    // Reset step completion status
    for (auto step : {RegistrationStep::SelectScans, RegistrationStep::TargetDetection,
                     RegistrationStep::ManualAlignment, RegistrationStep::ICPRegistration,
                     RegistrationStep::QualityReview, RegistrationStep::Export}) {
        setStepComplete(step, false);
    }
    
    updateNavigationButtons();
    statusLabel_->setText("Workflow reset - ready to start");
    qDebug() << "RegistrationWorkflowWidget: Workflow reset";
}

void RegistrationWorkflowWidget::goToStep(RegistrationStep step)
{
    stateMachine_->transitionTo(step);
}

RegistrationStep RegistrationWorkflowWidget::currentStep() const
{
    return stateMachine_->currentStep();
}

void RegistrationWorkflowWidget::setStepComplete(RegistrationStep step, bool complete)
{
    if (stepCompletionStatus_.value(step, false) != complete) {
        stepCompletionStatus_[step] = complete;
        stateMachine_->setStepComplete(step, complete);
        progressWidget_->setStepComplete(step, complete);
        updateNavigationButtons();
        qDebug() << "RegistrationWorkflowWidget: Step" << static_cast<int>(step) 
                 << "completion set to" << complete;
    }
}

bool RegistrationWorkflowWidget::isStepComplete(RegistrationStep step) const
{
    return stepCompletionStatus_.value(step, false);
}

bool RegistrationWorkflowWidget::canGoNext() const
{
    return navigationEnabled_ && stateMachine_->canTransitionTo(stateMachine_->getNextStep());
}

bool RegistrationWorkflowWidget::canGoBack() const
{
    return navigationEnabled_ && stateMachine_->canTransitionTo(stateMachine_->getPreviousStep());
}

void RegistrationWorkflowWidget::enableNavigation(bool enabled)
{
    navigationEnabled_ = enabled;
    updateNavigationButtons();
}

void RegistrationWorkflowWidget::goNext()
{
    if (canGoNext() && validateCurrentStep()) {
        RegistrationStep nextStep = stateMachine_->getNextStep();
        stateMachine_->transitionTo(nextStep);
    }
}

void RegistrationWorkflowWidget::goBack()
{
    if (canGoBack()) {
        RegistrationStep previousStep = stateMachine_->getPreviousStep();
        stateMachine_->transitionTo(previousStep);
    }
}

void RegistrationWorkflowWidget::onStateMachineStepChanged(RegistrationStep step)
{
    progressWidget_->updateCurrentStep(step);
    updateCurrentStepWidget();
    updateNavigationButtons();
    
    statusLabel_->setText(QString("Current step: %1").arg(getStepTitle(step)));
    emit stepChanged(step);
    
    qDebug() << "RegistrationWorkflowWidget: Step changed to" << static_cast<int>(step);
}

void RegistrationWorkflowWidget::onStateMachineTransitionBlocked(const QString& reason)
{
    QMessageBox::warning(this, "Transition Blocked", reason);
    statusLabel_->setText(QString("Transition blocked: %1").arg(reason));
    qWarning() << "RegistrationWorkflowWidget: Transition blocked:" << reason;
}

void RegistrationWorkflowWidget::onProgressWidgetStepClicked(RegistrationStep step)
{
    if (stateMachine_->canTransitionTo(step)) {
        stateMachine_->transitionTo(step);
    } else {
        statusLabel_->setText("Cannot navigate to that step yet");
    }
}

void RegistrationWorkflowWidget::onStepValidationChanged(RegistrationStep step, bool isValid)
{
    progressWidget_->setStepEnabled(step, isValid);
    updateNavigationButtons();
}

void RegistrationWorkflowWidget::updateCurrentStepWidget()
{
    RegistrationStep currentStep = stateMachine_->currentStep();
    
    int index = 0;
    switch (currentStep) {
        case RegistrationStep::SelectScans: index = 0; break;
        case RegistrationStep::TargetDetection: index = 1; break;
        case RegistrationStep::ManualAlignment: index = 2; break;
        case RegistrationStep::ICPRegistration: index = 3; break;
        case RegistrationStep::QualityReview: index = 4; break;
        case RegistrationStep::Export: index = 5; break;
    }
    
    contentStack_->setCurrentIndex(index);
}

void RegistrationWorkflowWidget::updateNavigationButtons()
{
    backButton_->setEnabled(canGoBack());
    nextButton_->setEnabled(canGoNext());
    
    // Update button text based on current step
    RegistrationStep currentStep = stateMachine_->currentStep();
    if (currentStep == RegistrationStep::Export) {
        nextButton_->setText("Finish");
    } else {
        nextButton_->setText("Next →");
    }
}

bool RegistrationWorkflowWidget::validateCurrentStep() const
{
    RegistrationStep currentStep = stateMachine_->currentStep();
    
    switch (currentStep) {
        case RegistrationStep::SelectScans: return validateScanSelection();
        case RegistrationStep::TargetDetection: return validateTargetDetection();
        case RegistrationStep::ManualAlignment: return validateManualAlignment();
        case RegistrationStep::ICPRegistration: return validateICPRegistration();
        case RegistrationStep::QualityReview: return validateQualityReview();
        case RegistrationStep::Export: return true; // Export is always valid
    }
    
    return false;
}

bool RegistrationWorkflowWidget::validateScanSelection() const
{
    // For now, always return true - actual validation will be implemented
    // when scan selection UI is added
    return true;
}

bool RegistrationWorkflowWidget::validateTargetDetection() const
{
    // For now, always return true - actual validation will be implemented
    // when target detection is added
    return true;
}

bool RegistrationWorkflowWidget::validateManualAlignment() const
{
    // For now, always return true - actual validation will be implemented
    // when manual alignment is added
    return true;
}

bool RegistrationWorkflowWidget::validateICPRegistration() const
{
    // For now, always return true - actual validation will be implemented
    // when ICP registration is added
    return true;
}

bool RegistrationWorkflowWidget::validateQualityReview() const
{
    // For now, always return true - actual validation will be implemented
    // when quality review is added
    return true;
}

QString RegistrationWorkflowWidget::getStepTitle(RegistrationStep step) const
{
    return stateMachine_->getStepName(step);
}

QString RegistrationWorkflowWidget::getStepInstructions(RegistrationStep step) const
{
    return stateMachine_->getStepDescription(step);
}
