#include <QApplication>
#include <QSignalSpy>
#include <QTest>

#include "../src/registration/RegistrationProject.h"
#include "../src/registration/RegistrationWorkflowWidget.h"
#include "../src/registration/Target.h"
#include "../src/registration/TargetCorrespondence.h"
#include "../src/registration/TargetManager.h"
#include "../src/registration/WorkflowStateMachine.h"
#include "../src/ui/WorkflowProgressWidget.h"

#include <gtest/gtest.h>

class RegistrationWorkflowTest : public ::testing::Test
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
    }

    void TearDown() override
    {
        // Don't delete QApplication as it might be used by other tests
    }

    QApplication* app = nullptr;
};

// WorkflowStateMachine Tests
TEST_F(RegistrationWorkflowTest, WorkflowStateMachine_InitialState)
{
    WorkflowStateMachine stateMachine;

    EXPECT_EQ(stateMachine.currentStep(), RegistrationStep::SelectScans);
    EXPECT_FALSE(stateMachine.isStepComplete(RegistrationStep::SelectScans));
}

TEST_F(RegistrationWorkflowTest, WorkflowStateMachine_ValidTransitions)
{
    WorkflowStateMachine stateMachine;

    // Test valid forward transition
    EXPECT_TRUE(stateMachine.canTransitionTo(RegistrationStep::TargetDetection));

    stateMachine.transitionTo(RegistrationStep::TargetDetection);
    EXPECT_EQ(stateMachine.currentStep(), RegistrationStep::TargetDetection);

    // Test valid backward transition
    EXPECT_TRUE(stateMachine.canTransitionTo(RegistrationStep::SelectScans));
}

TEST_F(RegistrationWorkflowTest, WorkflowStateMachine_InvalidTransitions)
{
    WorkflowStateMachine stateMachine;

    // Cannot skip steps
    EXPECT_FALSE(stateMachine.canTransitionTo(RegistrationStep::ICPRegistration));

    QSignalSpy transitionBlockedSpy(&stateMachine, &WorkflowStateMachine::transitionBlocked);
    stateMachine.transitionTo(RegistrationStep::ICPRegistration);

    EXPECT_EQ(transitionBlockedSpy.count(), 1);
    EXPECT_EQ(stateMachine.currentStep(), RegistrationStep::SelectScans);
}

TEST_F(RegistrationWorkflowTest, WorkflowStateMachine_StepCompletion)
{
    WorkflowStateMachine stateMachine;

    QSignalSpy validationChangedSpy(&stateMachine, &WorkflowStateMachine::stepValidationChanged);

    stateMachine.setStepComplete(RegistrationStep::SelectScans, true);
    EXPECT_TRUE(stateMachine.isStepComplete(RegistrationStep::SelectScans));
    EXPECT_EQ(validationChangedSpy.count(), 1);
}

// Target Tests
TEST_F(RegistrationWorkflowTest, SphereTarget_Creation)
{
    QVector3D position(1.0f, 2.0f, 3.0f);
    SphereTarget target("sphere1", position, 0.1f);

    EXPECT_EQ(target.targetId(), "sphere1");
    EXPECT_EQ(target.position(), position);
    EXPECT_EQ(target.radius(), 0.1f);
    EXPECT_EQ(target.getType(), "Sphere");
    EXPECT_TRUE(target.validate());
}

TEST_F(RegistrationWorkflowTest, SphereTarget_Serialization)
{
    QVector3D position(1.0f, 2.0f, 3.0f);
    SphereTarget original("sphere1", position, 0.1f);
    original.setConfidence(0.8f);
    original.setRmsError(0.01f);

    QVariantMap data = original.serialize();
    SphereTarget deserialized("", QVector3D(), 0.0f);

    EXPECT_TRUE(deserialized.deserialize(data));
    EXPECT_EQ(deserialized.targetId(), original.targetId());
    EXPECT_EQ(deserialized.position(), original.position());
    EXPECT_EQ(deserialized.radius(), original.radius());
    EXPECT_EQ(deserialized.confidence(), original.confidence());
    EXPECT_EQ(deserialized.rmsError(), original.rmsError());
}

TEST_F(RegistrationWorkflowTest, CheckerboardTarget_Creation)
{
    QVector3D position(0.0f, 0.0f, 0.0f);
    QList<QVector3D> corners = {QVector3D(0.0f, 0.0f, 0.0f),
                                QVector3D(1.0f, 0.0f, 0.0f),
                                QVector3D(1.0f, 1.0f, 0.0f),
                                QVector3D(0.0f, 1.0f, 0.0f)};

    CheckerboardTarget target("checkerboard1", position, corners);

    EXPECT_EQ(target.targetId(), "checkerboard1");
    EXPECT_EQ(target.getType(), "Checkerboard");
    EXPECT_EQ(target.cornerCount(), 4);
    EXPECT_TRUE(target.validate());
}

TEST_F(RegistrationWorkflowTest, NaturalPointTarget_Creation)
{
    QVector3D position(5.0f, 10.0f, 15.0f);
    NaturalPointTarget target("natural1", position, "Corner of building");

    EXPECT_EQ(target.targetId(), "natural1");
    EXPECT_EQ(target.position(), position);
    EXPECT_EQ(target.getType(), "NaturalPoint");
    EXPECT_EQ(target.description(), "Corner of building");
    EXPECT_TRUE(target.validate());
}

// TargetCorrespondence Tests
TEST_F(RegistrationWorkflowTest, TargetCorrespondence_Creation)
{
    TargetCorrespondence correspondence("target1", "target2", "scan1", "scan2");

    EXPECT_EQ(correspondence.targetId1(), "target1");
    EXPECT_EQ(correspondence.targetId2(), "target2");
    EXPECT_EQ(correspondence.scanId1(), "scan1");
    EXPECT_EQ(correspondence.scanId2(), "scan2");
    EXPECT_TRUE(correspondence.validate());
}

TEST_F(RegistrationWorkflowTest, TargetCorrespondence_Validation)
{
    // Invalid: same scan
    TargetCorrespondence invalidCorr("target1", "target2", "scan1", "scan1");
    EXPECT_FALSE(invalidCorr.validate());

    // Invalid: same target and scan
    TargetCorrespondence invalidCorr2("target1", "target1", "scan1", "scan1");
    EXPECT_FALSE(invalidCorr2.validate());

    // Valid correspondence
    TargetCorrespondence validCorr("target1", "target2", "scan1", "scan2");
    EXPECT_TRUE(validCorr.validate());
}

TEST_F(RegistrationWorkflowTest, TargetCorrespondence_Serialization)
{
    TargetCorrespondence original("target1", "target2", "scan1", "scan2");
    original.setConfidence(0.9f);
    original.setDistance(1.5f);
    original.setManual(true);

    QVariantMap data = original.serialize();
    TargetCorrespondence deserialized;

    EXPECT_TRUE(deserialized.deserialize(data));
    EXPECT_EQ(deserialized.targetId1(), original.targetId1());
    EXPECT_EQ(deserialized.targetId2(), original.targetId2());
    EXPECT_EQ(deserialized.confidence(), original.confidence());
    EXPECT_EQ(deserialized.distance(), original.distance());
    EXPECT_EQ(deserialized.isManual(), original.isManual());
}

// TargetManager Tests
TEST_F(RegistrationWorkflowTest, TargetManager_AddTarget)
{
    TargetManager manager;

    auto target = std::make_unique<SphereTarget>("sphere1", QVector3D(1, 2, 3), 0.1f);
    QString targetId = target->targetId();

    QSignalSpy targetAddedSpy(&manager, &TargetManager::targetAdded);

    manager.addTarget("scan1", std::move(target));

    EXPECT_EQ(targetAddedSpy.count(), 1);
    EXPECT_EQ(manager.getTargetCount(), 1);
    EXPECT_TRUE(manager.hasTarget(targetId));
    EXPECT_EQ(manager.getTargetCountForScan("scan1"), 1);
}

TEST_F(RegistrationWorkflowTest, TargetManager_RemoveTarget)
{
    TargetManager manager;

    auto target = std::make_unique<SphereTarget>("sphere1", QVector3D(1, 2, 3), 0.1f);
    QString targetId = target->targetId();

    manager.addTarget("scan1", std::move(target));

    QSignalSpy targetRemovedSpy(&manager, &TargetManager::targetRemoved);

    manager.removeTarget(targetId);

    EXPECT_EQ(targetRemovedSpy.count(), 1);
    EXPECT_EQ(manager.getTargetCount(), 0);
    EXPECT_FALSE(manager.hasTarget(targetId));
}

TEST_F(RegistrationWorkflowTest, TargetManager_Correspondences)
{
    TargetManager manager;

    // Add targets
    auto target1 = std::make_unique<SphereTarget>("sphere1", QVector3D(1, 2, 3), 0.1f);
    auto target2 = std::make_unique<SphereTarget>("sphere2", QVector3D(4, 5, 6), 0.1f);

    manager.addTarget("scan1", std::move(target1));
    manager.addTarget("scan2", std::move(target2));

    // Add correspondence
    TargetCorrespondence correspondence("sphere1", "sphere2", "scan1", "scan2");

    QSignalSpy correspondenceAddedSpy(&manager, &TargetManager::correspondenceAdded);

    manager.addCorrespondence(correspondence);

    EXPECT_EQ(correspondenceAddedSpy.count(), 1);
    EXPECT_EQ(manager.getCorrespondenceCount(), 1);
    EXPECT_TRUE(manager.hasCorrespondence("sphere1", "sphere2"));
}

// RegistrationProject Tests
TEST_F(RegistrationWorkflowTest, RegistrationProject_ScanManagement)
{
    RegistrationProject project("Test Project", "/tmp/test");

    ScanInfo scanInfo;
    scanInfo.scanId = "scan1";
    scanInfo.filePath = "/path/to/scan1.las";
    scanInfo.name = "First Scan";
    scanInfo.pointCount = 1000000;

    QSignalSpy scanAddedSpy(&project, &RegistrationProject::scanAdded);

    project.addScan(scanInfo);

    EXPECT_EQ(scanAddedSpy.count(), 1);
    EXPECT_EQ(project.getScanCount(), 1);
    EXPECT_TRUE(project.hasScan("scan1"));

    ScanInfo retrieved = project.getScan("scan1");
    EXPECT_EQ(retrieved.scanId, scanInfo.scanId);
    EXPECT_EQ(retrieved.filePath, scanInfo.filePath);
    EXPECT_EQ(retrieved.pointCount, scanInfo.pointCount);
}

TEST_F(RegistrationWorkflowTest, RegistrationProject_ReferenceScan)
{
    RegistrationProject project("Test Project", "/tmp/test");

    ScanInfo scan1;
    scan1.scanId = "scan1";
    scan1.filePath = "/path/to/scan1.las";

    ScanInfo scan2;
    scan2.scanId = "scan2";
    scan2.filePath = "/path/to/scan2.las";

    project.addScan(scan1);
    project.addScan(scan2);

    // First scan should be reference by default
    EXPECT_EQ(project.getReferenceScan().scanId, "scan1");

    QSignalSpy referenceScanChangedSpy(&project, &RegistrationProject::referenceScanChanged);

    project.setReferenceScan("scan2");

    EXPECT_EQ(referenceScanChangedSpy.count(), 1);
    EXPECT_EQ(project.getReferenceScan().scanId, "scan2");
}

TEST_F(RegistrationWorkflowTest, RegistrationProject_Serialization)
{
    RegistrationProject original("Test Project", "/tmp/test");

    // Add scan
    ScanInfo scanInfo;
    scanInfo.scanId = "scan1";
    scanInfo.filePath = "/path/to/scan1.las";
    scanInfo.name = "Test Scan";
    original.addScan(scanInfo);

    // Add target
    auto target = std::make_unique<SphereTarget>("sphere1", QVector3D(1, 2, 3), 0.1f);
    original.targetManager()->addTarget("scan1", std::move(target));

    // Serialize and deserialize
    QVariantMap data = original.serialize();
    RegistrationProject deserialized;

    EXPECT_TRUE(deserialized.deserialize(data));
    EXPECT_EQ(deserialized.name(), original.name());
    EXPECT_EQ(deserialized.getScanCount(), original.getScanCount());
    EXPECT_EQ(deserialized.targetManager()->getTargetCount(), original.targetManager()->getTargetCount());
}

// WorkflowProgressWidget Tests
TEST_F(RegistrationWorkflowTest, WorkflowProgressWidget_Creation)
{
    WorkflowProgressWidget widget;

    // Widget should be created successfully
    EXPECT_TRUE(widget.isVisible() == false);  // Not shown by default
}

TEST_F(RegistrationWorkflowTest, WorkflowProgressWidget_StepUpdates)
{
    WorkflowProgressWidget widget;

    QSignalSpy stepClickedSpy(&widget, &WorkflowProgressWidget::stepClicked);

    widget.updateCurrentStep(RegistrationStep::TargetDetection);
    widget.setStepComplete(RegistrationStep::SelectScans, true);
    widget.setStepEnabled(RegistrationStep::ManualAlignment, true);

    // No crashes should occur
    EXPECT_TRUE(true);
}

// RegistrationWorkflowWidget Tests
TEST_F(RegistrationWorkflowTest, RegistrationWorkflowWidget_Creation)
{
    RegistrationWorkflowWidget widget;

    EXPECT_EQ(widget.currentStep(), RegistrationStep::SelectScans);
    EXPECT_FALSE(widget.isStepComplete(RegistrationStep::SelectScans));
}

TEST_F(RegistrationWorkflowTest, RegistrationWorkflowWidget_ProjectAssignment)
{
    RegistrationWorkflowWidget widget;
    RegistrationProject project("Test Project", "/tmp/test");

    QSignalSpy projectChangedSpy(&widget, &RegistrationWorkflowWidget::projectChanged);

    widget.setProject(&project);

    EXPECT_EQ(projectChangedSpy.count(), 1);
    EXPECT_EQ(widget.project(), &project);
}

TEST_F(RegistrationWorkflowTest, RegistrationWorkflowWidget_WorkflowControl)
{
    RegistrationWorkflowWidget widget;

    QSignalSpy workflowStartedSpy(&widget, &RegistrationWorkflowWidget::workflowStarted);

    widget.startWorkflow();

    EXPECT_EQ(workflowStartedSpy.count(), 1);
    EXPECT_EQ(widget.currentStep(), RegistrationStep::SelectScans);
}

// Integration Tests
TEST_F(RegistrationWorkflowTest, Integration_CompleteWorkflow)
{
    // Create project
    RegistrationProject project("Integration Test", "/tmp/integration");

    // Add scans
    ScanInfo scan1;
    scan1.scanId = "scan1";
    scan1.filePath = "/path/to/scan1.las";
    scan1.name = "Scan 1";

    ScanInfo scan2;
    scan2.scanId = "scan2";
    scan2.filePath = "/path/to/scan2.las";
    scan2.name = "Scan 2";

    project.addScan(scan1);
    project.addScan(scan2);

    // Add targets
    auto target1 = std::make_unique<SphereTarget>("sphere1", QVector3D(1, 2, 3), 0.1f);
    auto target2 = std::make_unique<SphereTarget>("sphere2", QVector3D(4, 5, 6), 0.1f);

    project.targetManager()->addTarget("scan1", std::move(target1));
    project.targetManager()->addTarget("scan2", std::move(target2));

    // Add correspondence
    TargetCorrespondence correspondence("sphere1", "sphere2", "scan1", "scan2");
    project.targetManager()->addCorrespondence(correspondence);

    // Create workflow widget
    RegistrationWorkflowWidget widget;
    widget.setProject(&project);

    // Verify integration
    EXPECT_EQ(widget.project()->getScanCount(), 2);
    EXPECT_EQ(widget.project()->targetManager()->getTargetCount(), 2);
    EXPECT_EQ(widget.project()->targetManager()->getCorrespondenceCount(), 1);

    // Test workflow progression
    widget.startWorkflow();
    EXPECT_EQ(widget.currentStep(), RegistrationStep::SelectScans);

    // Mark step complete and try to advance
    widget.setStepComplete(RegistrationStep::SelectScans, true);
    EXPECT_TRUE(widget.isStepComplete(RegistrationStep::SelectScans));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    // Initialize Qt application for widget tests
    QApplication app(argc, argv);

    return RUN_ALL_TESTS();
}
