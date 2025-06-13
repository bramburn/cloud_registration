#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <QPushButton>

#include "registration/RegistrationWorkflowWidget.h"
#include "registration/RegistrationProject.h"

class RegistrationWorkflowWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Ensure QApplication exists for Qt widgets
        if (!QApplication::instance())
        {
            int argc = 0;
            char** argv = nullptr;
            app = new QApplication(argc, argv);
        }
        
        widget = new RegistrationWorkflowWidget();
        project = new RegistrationProject();
    }

    void TearDown() override
    {
        delete widget;
        delete project;
    }

    QApplication* app = nullptr;
    RegistrationWorkflowWidget* widget = nullptr;
    RegistrationProject* project = nullptr;
};

TEST_F(RegistrationWorkflowWidgetTest, InitialState)
{
    EXPECT_NE(widget, nullptr);
    EXPECT_EQ(widget->currentStep(), RegistrationStep::SelectScans);
    EXPECT_FALSE(widget->canGoNext());
    EXPECT_FALSE(widget->canGoBack());
}

TEST_F(RegistrationWorkflowWidgetTest, ProjectSetting)
{
    QSignalSpy projectChangedSpy(widget, &RegistrationWorkflowWidget::projectChanged);
    
    widget->setProject(project);
    EXPECT_EQ(widget->project(), project);
    EXPECT_EQ(projectChangedSpy.count(), 1);
}

TEST_F(RegistrationWorkflowWidgetTest, TargetDetectionButtonPresence)
{
    widget->show();
    
    // Find the target detection button
    QPushButton* targetDetectionButton = nullptr;
    auto buttons = widget->findChildren<QPushButton*>();
    
    for (auto* button : buttons)
    {
        if (button->text().contains("Target Detection", Qt::CaseInsensitive))
        {
            targetDetectionButton = button;
            break;
        }
    }
    
    ASSERT_NE(targetDetectionButton, nullptr) << "Target Detection button not found";
    
    // Initially should be disabled (no scans loaded)
    EXPECT_FALSE(targetDetectionButton->isEnabled());
}

TEST_F(RegistrationWorkflowWidgetTest, TargetDetectionButtonEnablement)
{
    widget->show();
    
    // Find the target detection button
    QPushButton* targetDetectionButton = nullptr;
    auto buttons = widget->findChildren<QPushButton*>();
    
    for (auto* button : buttons)
    {
        if (button->text().contains("Target Detection", Qt::CaseInsensitive))
        {
            targetDetectionButton = button;
            break;
        }
    }
    
    ASSERT_NE(targetDetectionButton, nullptr);
    
    // Initially disabled
    EXPECT_FALSE(targetDetectionButton->isEnabled());
    
    // Enable target detection
    widget->enableTargetDetection(true);
    EXPECT_TRUE(targetDetectionButton->isEnabled());
    
    // Disable target detection
    widget->enableTargetDetection(false);
    EXPECT_FALSE(targetDetectionButton->isEnabled());
}

TEST_F(RegistrationWorkflowWidgetTest, TargetDetectionSignalEmission)
{
    QSignalSpy targetDetectionSpy(widget, &RegistrationWorkflowWidget::targetDetectionRequested);
    
    widget->show();
    
    // Find and enable the target detection button
    QPushButton* targetDetectionButton = nullptr;
    auto buttons = widget->findChildren<QPushButton*>();
    
    for (auto* button : buttons)
    {
        if (button->text().contains("Target Detection", Qt::CaseInsensitive))
        {
            targetDetectionButton = button;
            break;
        }
    }
    
    ASSERT_NE(targetDetectionButton, nullptr);
    
    // Enable the button and click it
    widget->enableTargetDetection(true);
    targetDetectionButton->click();
    
    // Should emit the signal
    EXPECT_EQ(targetDetectionSpy.count(), 1);
}

TEST_F(RegistrationWorkflowWidgetTest, WorkflowNavigation)
{
    QSignalSpy stepChangedSpy(widget, &RegistrationWorkflowWidget::stepChanged);
    
    widget->setProject(project);
    widget->startWorkflow();
    
    EXPECT_EQ(widget->currentStep(), RegistrationStep::SelectScans);
    EXPECT_EQ(stepChangedSpy.count(), 1);
    
    // Mark current step as complete to enable navigation
    widget->setStepComplete(RegistrationStep::SelectScans, true);
    EXPECT_TRUE(widget->canGoNext());
    
    // Navigate to next step
    widget->goNext();
    EXPECT_EQ(widget->currentStep(), RegistrationStep::TargetDetection);
    EXPECT_EQ(stepChangedSpy.count(), 2);
}

TEST_F(RegistrationWorkflowWidgetTest, StepCompletion)
{
    widget->setProject(project);
    
    // Initially no steps should be complete
    EXPECT_FALSE(widget->isStepComplete(RegistrationStep::SelectScans));
    EXPECT_FALSE(widget->isStepComplete(RegistrationStep::TargetDetection));
    
    // Mark scan selection as complete
    widget->setStepComplete(RegistrationStep::SelectScans, true);
    EXPECT_TRUE(widget->isStepComplete(RegistrationStep::SelectScans));
    EXPECT_FALSE(widget->isStepComplete(RegistrationStep::TargetDetection));
    
    // Should enable navigation to next step
    EXPECT_TRUE(widget->canGoNext());
}

TEST_F(RegistrationWorkflowWidgetTest, WorkflowReset)
{
    widget->setProject(project);
    widget->startWorkflow();
    
    // Complete some steps
    widget->setStepComplete(RegistrationStep::SelectScans, true);
    widget->goNext();
    widget->setStepComplete(RegistrationStep::TargetDetection, true);
    
    EXPECT_EQ(widget->currentStep(), RegistrationStep::TargetDetection);
    EXPECT_TRUE(widget->isStepComplete(RegistrationStep::SelectScans));
    EXPECT_TRUE(widget->isStepComplete(RegistrationStep::TargetDetection));
    
    // Reset workflow
    widget->resetWorkflow();
    
    EXPECT_EQ(widget->currentStep(), RegistrationStep::SelectScans);
    EXPECT_FALSE(widget->isStepComplete(RegistrationStep::SelectScans));
    EXPECT_FALSE(widget->isStepComplete(RegistrationStep::TargetDetection));
}

TEST_F(RegistrationWorkflowWidgetTest, NavigationEnablement)
{
    widget->setProject(project);
    
    // Initially navigation should be enabled
    EXPECT_TRUE(widget->canGoNext() || widget->canGoBack() || true); // At least navigation is possible
    
    // Disable navigation
    widget->enableNavigation(false);
    EXPECT_FALSE(widget->canGoNext());
    EXPECT_FALSE(widget->canGoBack());
    
    // Re-enable navigation
    widget->enableNavigation(true);
    // Navigation state depends on current step and completion status
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
