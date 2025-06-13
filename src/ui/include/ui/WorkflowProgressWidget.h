#ifndef WORKFLOWPROGRESSWIDGET_H
#define WORKFLOWPROGRESSWIDGET_H

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "registration/WorkflowStateMachine.h"

/**
 * @brief Visual progress indicator for registration workflow
 *
 * This widget displays the current step in the registration workflow as a
 * visual stepper/breadcrumb interface. It shows all steps, highlights the
 * current step, and indicates completion status.
 *
 * Sprint 2 Implementation: Workflow progress visualization
 */
class WorkflowProgressWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WorkflowProgressWidget(QWidget* parent = nullptr);
    virtual ~WorkflowProgressWidget() = default;

    // Step management
    void updateCurrentStep(RegistrationStep step);
    void setStepComplete(RegistrationStep step, bool complete);
    void setStepEnabled(RegistrationStep step, bool enabled);

    // Visual configuration
    void setCompactMode(bool compact);
    void setShowDescriptions(bool show);
    void setAnimationsEnabled(bool enabled);

signals:
    void stepClicked(RegistrationStep step);

private slots:
    void onStepButtonClicked();

private:
    struct StepIndicator
    {
        QPushButton* button;
        QLabel* label;
        QLabel* description;
        QFrame* connector;
        QVBoxLayout* layout;
        bool isComplete;
        bool isEnabled;
    };

    // UI setup
    void setupUI();
    void createStepIndicators();
    void setupConnections();
    void setupStyling();

    // Step management
    void updateStepAppearance();
    void updateStepButton(RegistrationStep step);
    void updateConnectors();

    // Utility
    QString getStepName(RegistrationStep step) const;
    QString getStepDescription(RegistrationStep step) const;
    QString getStepIcon(RegistrationStep step) const;
    int getStepIndex(RegistrationStep step) const;

    // Styling
    void applyStepStyle(RegistrationStep step);
    QString getButtonStyle(RegistrationStep step) const;
    QString getConnectorStyle(RegistrationStep step) const;

    // Layout
    QHBoxLayout* mainLayout_;
    QMap<RegistrationStep, StepIndicator> stepIndicators_;

    // State
    RegistrationStep currentStep_;
    QList<RegistrationStep> allSteps_;

    // Configuration
    bool compactMode_;
    bool showDescriptions_;
    bool animationsEnabled_;

    // Styling constants
    static const QString BUTTON_STYLE_INACTIVE;
    static const QString BUTTON_STYLE_CURRENT;
    static const QString BUTTON_STYLE_COMPLETE;
    static const QString BUTTON_STYLE_DISABLED;
    static const QString CONNECTOR_STYLE_INACTIVE;
    static const QString CONNECTOR_STYLE_COMPLETE;
};

#endif  // WORKFLOWPROGRESSWIDGET_H
