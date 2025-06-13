#include "WorkflowProgressWidget.h"

#include <QDebug>

// Static style constants
const QString WorkflowProgressWidget::BUTTON_STYLE_INACTIVE = "QPushButton { "
                                                              "border-radius: 20px; "
                                                              "background-color: #E0E0E0; "
                                                              "color: #757575; "
                                                              "font-weight: normal; "
                                                              "border: 2px solid #E0E0E0; "
                                                              "min-width: 40px; "
                                                              "max-width: 40px; "
                                                              "min-height: 40px; "
                                                              "max-height: 40px; "
                                                              "} "
                                                              "QPushButton:hover { "
                                                              "background-color: #BDBDBD; "
                                                              "border-color: #BDBDBD; "
                                                              "}";

const QString WorkflowProgressWidget::BUTTON_STYLE_CURRENT = "QPushButton { "
                                                             "border-radius: 20px; "
                                                             "background-color: #2196F3; "
                                                             "color: white; "
                                                             "font-weight: bold; "
                                                             "border: 2px solid #2196F3; "
                                                             "min-width: 40px; "
                                                             "max-width: 40px; "
                                                             "min-height: 40px; "
                                                             "max-height: 40px; "
                                                             "}";

const QString WorkflowProgressWidget::BUTTON_STYLE_COMPLETE = "QPushButton { "
                                                              "border-radius: 20px; "
                                                              "background-color: #4CAF50; "
                                                              "color: white; "
                                                              "font-weight: bold; "
                                                              "border: 2px solid #4CAF50; "
                                                              "min-width: 40px; "
                                                              "max-width: 40px; "
                                                              "min-height: 40px; "
                                                              "max-height: 40px; "
                                                              "}";

const QString WorkflowProgressWidget::BUTTON_STYLE_DISABLED = "QPushButton { "
                                                              "border-radius: 20px; "
                                                              "background-color: #F5F5F5; "
                                                              "color: #BDBDBD; "
                                                              "font-weight: normal; "
                                                              "border: 2px solid #F5F5F5; "
                                                              "min-width: 40px; "
                                                              "max-width: 40px; "
                                                              "min-height: 40px; "
                                                              "max-height: 40px; "
                                                              "}";

const QString WorkflowProgressWidget::CONNECTOR_STYLE_INACTIVE = "QFrame { "
                                                                 "background-color: #E0E0E0; "
                                                                 "border: none; "
                                                                 "min-height: 2px; "
                                                                 "max-height: 2px; "
                                                                 "}";

const QString WorkflowProgressWidget::CONNECTOR_STYLE_COMPLETE = "QFrame { "
                                                                 "background-color: #4CAF50; "
                                                                 "border: none; "
                                                                 "min-height: 2px; "
                                                                 "max-height: 2px; "
                                                                 "}";

WorkflowProgressWidget::WorkflowProgressWidget(QWidget* parent)
    : QWidget(parent),
      mainLayout_(nullptr),
      currentStep_(RegistrationStep::SelectScans),
      compactMode_(false),
      showDescriptions_(true),
      animationsEnabled_(true)
{
    allSteps_ = {RegistrationStep::SelectScans,
                 RegistrationStep::TargetDetection,
                 RegistrationStep::ManualAlignment,
                 RegistrationStep::ICPRegistration,
                 RegistrationStep::QualityReview,
                 RegistrationStep::Export};

    setupUI();
}

void WorkflowProgressWidget::setupUI()
{
    mainLayout_ = new QHBoxLayout(this);
    mainLayout_->setContentsMargins(10, 10, 10, 10);
    mainLayout_->setSpacing(0);

    createStepIndicators();
    setupConnections();
    setupStyling();
    updateStepAppearance();
}

void WorkflowProgressWidget::createStepIndicators()
{
    for (int i = 0; i < allSteps_.size(); ++i)
    {
        RegistrationStep step = allSteps_[i];
        StepIndicator indicator;

        // Create vertical layout for each step
        indicator.layout = new QVBoxLayout();
        indicator.layout->setAlignment(Qt::AlignCenter);
        indicator.layout->setSpacing(5);

        // Create step button
        indicator.button = new QPushButton(QString::number(i + 1));
        indicator.button->setProperty("step", static_cast<int>(step));
        indicator.button->setToolTip(getStepDescription(step));

        // Create step label
        indicator.label = new QLabel(getStepName(step));
        indicator.label->setAlignment(Qt::AlignCenter);
        indicator.label->setWordWrap(true);
        indicator.label->setMaximumWidth(80);

        // Create step description (initially hidden in compact mode)
        indicator.description = new QLabel(getStepDescription(step));
        indicator.description->setAlignment(Qt::AlignCenter);
        indicator.description->setWordWrap(true);
        indicator.description->setMaximumWidth(100);
        indicator.description->setStyleSheet("color: #757575; font-size: 10px;");
        indicator.description->setVisible(showDescriptions_ && !compactMode_);

        // Add to layout
        indicator.layout->addWidget(indicator.button);
        indicator.layout->addWidget(indicator.label);
        if (showDescriptions_ && !compactMode_)
        {
            indicator.layout->addWidget(indicator.description);
        }

        // Create connector (except for last step)
        if (i < allSteps_.size() - 1)
        {
            indicator.connector = new QFrame();
            indicator.connector->setFrameShape(QFrame::HLine);
            indicator.connector->setFixedHeight(2);
            indicator.connector->setMinimumWidth(30);
        }
        else
        {
            indicator.connector = nullptr;
        }

        // Initialize state
        indicator.isComplete = false;
        indicator.isEnabled = (step == RegistrationStep::SelectScans);  // Only first step enabled initially

        stepIndicators_[step] = indicator;

        // Add to main layout
        mainLayout_->addLayout(indicator.layout);

        if (indicator.connector)
        {
            // Create a vertical layout to center the connector
            QVBoxLayout* connectorLayout = new QVBoxLayout();
            connectorLayout->addStretch();
            connectorLayout->addWidget(indicator.connector);
            connectorLayout->addStretch();
            mainLayout_->addLayout(connectorLayout);
        }
    }

    mainLayout_->addStretch();
}

void WorkflowProgressWidget::setupConnections()
{
    for (auto it = stepIndicators_.begin(); it != stepIndicators_.end(); ++it)
    {
        connect(it.value().button, &QPushButton::clicked, this, &WorkflowProgressWidget::onStepButtonClicked);
    }
}

void WorkflowProgressWidget::setupStyling()
{
    setStyleSheet("QLabel { "
                  "font-size: 12px; "
                  "color: #424242; "
                  "} "
                  "QLabel[current=\"true\"] { "
                  "font-weight: bold; "
                  "color: #2196F3; "
                  "}");
}

void WorkflowProgressWidget::updateCurrentStep(RegistrationStep step)
{
    if (currentStep_ != step)
    {
        currentStep_ = step;
        updateStepAppearance();
        qDebug() << "WorkflowProgressWidget: Updated current step to" << getStepName(step);
    }
}

void WorkflowProgressWidget::setStepComplete(RegistrationStep step, bool complete)
{
    auto it = stepIndicators_.find(step);
    if (it != stepIndicators_.end() && it.value().isComplete != complete)
    {
        it.value().isComplete = complete;
        updateStepButton(step);
        updateConnectors();
        qDebug() << "WorkflowProgressWidget: Step" << getStepName(step) << "completion status changed to" << complete;
    }
}

void WorkflowProgressWidget::setStepEnabled(RegistrationStep step, bool enabled)
{
    auto it = stepIndicators_.find(step);
    if (it != stepIndicators_.end() && it.value().isEnabled != enabled)
    {
        it.value().isEnabled = enabled;
        it.value().button->setEnabled(enabled);
        updateStepButton(step);
        qDebug() << "WorkflowProgressWidget: Step" << getStepName(step) << "enabled status changed to" << enabled;
    }
}

void WorkflowProgressWidget::setCompactMode(bool compact)
{
    if (compactMode_ != compact)
    {
        compactMode_ = compact;

        // Update description visibility
        for (auto it = stepIndicators_.begin(); it != stepIndicators_.end(); ++it)
        {
            it.value().description->setVisible(showDescriptions_ && !compactMode_);
        }

        qDebug() << "WorkflowProgressWidget: Compact mode set to" << compact;
    }
}

void WorkflowProgressWidget::setShowDescriptions(bool show)
{
    if (showDescriptions_ != show)
    {
        showDescriptions_ = show;

        // Update description visibility
        for (auto it = stepIndicators_.begin(); it != stepIndicators_.end(); ++it)
        {
            it.value().description->setVisible(showDescriptions_ && !compactMode_);
        }

        qDebug() << "WorkflowProgressWidget: Show descriptions set to" << show;
    }
}

void WorkflowProgressWidget::setAnimationsEnabled(bool enabled)
{
    animationsEnabled_ = enabled;
}

void WorkflowProgressWidget::onStepButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (button)
    {
        int stepInt = button->property("step").toInt();
        RegistrationStep step = static_cast<RegistrationStep>(stepInt);

        auto it = stepIndicators_.find(step);
        if (it != stepIndicators_.end() && it.value().isEnabled)
        {
            emit stepClicked(step);
            qDebug() << "WorkflowProgressWidget: Step clicked:" << getStepName(step);
        }
    }
}

void WorkflowProgressWidget::updateStepAppearance()
{
    for (auto step : allSteps_)
    {
        updateStepButton(step);

        // Update label styling
        auto it = stepIndicators_.find(step);
        if (it != stepIndicators_.end())
        {
            bool isCurrent = (step == currentStep_);
            it.value().label->setProperty("current", isCurrent);
            it.value().label->style()->unpolish(it.value().label);
            it.value().label->style()->polish(it.value().label);
        }
    }

    updateConnectors();
}

void WorkflowProgressWidget::updateStepButton(RegistrationStep step)
{
    auto it = stepIndicators_.find(step);
    if (it != stepIndicators_.end())
    {
        applyStepStyle(step);
    }
}

void WorkflowProgressWidget::updateConnectors()
{
    for (int i = 0; i < allSteps_.size() - 1; ++i)
    {
        RegistrationStep step = allSteps_[i];
        auto it = stepIndicators_.find(step);
        if (it != stepIndicators_.end() && it.value().connector)
        {
            QString style = (it.value().isComplete) ? CONNECTOR_STYLE_COMPLETE : CONNECTOR_STYLE_INACTIVE;
            it.value().connector->setStyleSheet(style);
        }
    }
}

QString WorkflowProgressWidget::getStepName(RegistrationStep step) const
{
    static const QMap<RegistrationStep, QString> stepNames = {{RegistrationStep::SelectScans, "Select\nScans"},
                                                              {RegistrationStep::TargetDetection, "Target\nDetection"},
                                                              {RegistrationStep::ManualAlignment, "Manual\nAlignment"},
                                                              {RegistrationStep::ICPRegistration, "ICP\nRegistration"},
                                                              {RegistrationStep::QualityReview, "Quality\nReview"},
                                                              {RegistrationStep::Export, "Export"}};
    return stepNames.value(step, "Unknown");
}

QString WorkflowProgressWidget::getStepDescription(RegistrationStep step) const
{
    static const QMap<RegistrationStep, QString> stepDescriptions = {
        {RegistrationStep::SelectScans, "Choose scans to register"},
        {RegistrationStep::TargetDetection, "Find registration targets"},
        {RegistrationStep::ManualAlignment, "Create correspondences"},
        {RegistrationStep::ICPRegistration, "Refine with ICP"},
        {RegistrationStep::QualityReview, "Check alignment quality"},
        {RegistrationStep::Export, "Save results"}};
    return stepDescriptions.value(step, "No description");
}

QString WorkflowProgressWidget::getStepIcon(RegistrationStep step) const
{
    // Icons could be added here in the future
    Q_UNUSED(step)
    return QString();
}

int WorkflowProgressWidget::getStepIndex(RegistrationStep step) const
{
    return allSteps_.indexOf(step);
}

void WorkflowProgressWidget::applyStepStyle(RegistrationStep step)
{
    auto it = stepIndicators_.find(step);
    if (it == stepIndicators_.end())
    {
        return;
    }

    QString style;

    if (!it.value().isEnabled)
    {
        style = BUTTON_STYLE_DISABLED;
    }
    else if (it.value().isComplete)
    {
        style = BUTTON_STYLE_COMPLETE;
        it.value().button->setText("✓");
    }
    else if (step == currentStep_)
    {
        style = BUTTON_STYLE_CURRENT;
        it.value().button->setText(QString::number(getStepIndex(step) + 1));
    }
    else
    {
        style = BUTTON_STYLE_INACTIVE;
        it.value().button->setText(QString::number(getStepIndex(step) + 1));
    }

    it.value().button->setStyleSheet(style);
}

QString WorkflowProgressWidget::getButtonStyle(RegistrationStep step) const
{
    auto it = stepIndicators_.find(step);
    if (it == stepIndicators_.end())
    {
        return BUTTON_STYLE_INACTIVE;
    }

    if (!it.value().isEnabled)
    {
        return BUTTON_STYLE_DISABLED;
    }
    else if (it.value().isComplete)
    {
        return BUTTON_STYLE_COMPLETE;
    }
    else if (step == currentStep_)
    {
        return BUTTON_STYLE_CURRENT;
    }
    else
    {
        return BUTTON_STYLE_INACTIVE;
    }
}

QString WorkflowProgressWidget::getConnectorStyle(RegistrationStep step) const
{
    auto it = stepIndicators_.find(step);
    if (it != stepIndicators_.end() && it.value().isComplete)
    {
        return CONNECTOR_STYLE_COMPLETE;
    }
    return CONNECTOR_STYLE_INACTIVE;
}
