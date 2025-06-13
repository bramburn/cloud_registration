#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <QTimer>

#include "registration/RegistrationWorkflowWidget.h"
#include "registration/AlignmentEngine.h"
#include "registration/SphereDetector.h"
#include "registration/NaturalPointSelector.h"
#include "ui/TargetDetectionDialog.h"
#include "registration/TargetManager.h"

class TargetDetectionIntegrationTest : public ::testing::Test
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
        
        workflowWidget = new RegistrationWorkflowWidget();
        alignmentEngine = new AlignmentEngine();
        targetManager = new TargetManager();
        sphereDetector = new SphereDetector();
        naturalPointSelector = new NaturalPointSelector();
    }

    void TearDown() override
    {
        delete workflowWidget;
        delete alignmentEngine;
        delete targetManager;
        delete sphereDetector;
        delete naturalPointSelector;
    }

    QApplication* app = nullptr;
    RegistrationWorkflowWidget* workflowWidget = nullptr;
    AlignmentEngine* alignmentEngine = nullptr;
    TargetManager* targetManager = nullptr;
    SphereDetector* sphereDetector = nullptr;
    NaturalPointSelector* naturalPointSelector = nullptr;
};

TEST_F(TargetDetectionIntegrationTest, WorkflowWidgetTargetDetectionButton)
{
    workflowWidget->show();
    
    // Verify target detection button exists and is initially disabled
    QPushButton* targetDetectionButton = nullptr;
    auto buttons = workflowWidget->findChildren<QPushButton*>();
    
    for (auto* button : buttons)
    {
        if (button->text().contains("Target Detection", Qt::CaseInsensitive))
        {
            targetDetectionButton = button;
            break;
        }
    }
    
    ASSERT_NE(targetDetectionButton, nullptr) << "Target Detection button should exist";
    EXPECT_FALSE(targetDetectionButton->isEnabled()) << "Button should be initially disabled";
    
    // Enable target detection and verify button becomes enabled
    workflowWidget->enableTargetDetection(true);
    EXPECT_TRUE(targetDetectionButton->isEnabled()) << "Button should be enabled after enableTargetDetection(true)";
}

TEST_F(TargetDetectionIntegrationTest, TargetDetectionDialogCreation)
{
    // Test that TargetDetectionDialog can be created and configured
    TargetDetectionDialog dialog(targetManager);
    
    EXPECT_EQ(dialog.getDetectionMode(), TargetDetectionDialog::AutomaticSpheres);
    
    // Test setting point cloud data
    QString testScanId = "integration_test_scan";
    std::vector<PointFullData> testPoints;
    
    // Create minimal test data
    for (int i = 0; i < 10; ++i)
    {
        PointFullData point;
        point.x = static_cast<float>(i);
        point.y = static_cast<float>(i);
        point.z = static_cast<float>(i);
        point.r = 255;
        point.g = 255;
        point.b = 255;
        point.intensity = 1000;
        testPoints.push_back(point);
    }
    
    dialog.setPointCloudData(testScanId, testPoints);
    
    // Verify parameters can be set and retrieved
    auto params = dialog.getDetectionParameters();
    EXPECT_GT(params.distanceThreshold, 0.0f);
    EXPECT_GT(params.maxIterations, 0);
}

TEST_F(TargetDetectionIntegrationTest, SphereDetectorDefaultParameters)
{
    // Test that SphereDetector provides valid default parameters
    auto defaultParams = sphereDetector->getDefaultParameters();
    
    EXPECT_GT(defaultParams.distanceThreshold, 0.0f);
    EXPECT_LT(defaultParams.distanceThreshold, 1.0f);
    EXPECT_GT(defaultParams.maxIterations, 100);
    EXPECT_LT(defaultParams.maxIterations, 10000);
    EXPECT_GE(defaultParams.minQuality, 0.0f);
    EXPECT_LE(defaultParams.minQuality, 1.0f);
    EXPECT_GT(defaultParams.minRadius, 0.0f);
    EXPECT_GT(defaultParams.maxRadius, defaultParams.minRadius);
    EXPECT_GT(defaultParams.minInliers, 0);
    
    // Verify parameters are valid
    EXPECT_TRUE(sphereDetector->validateParameters(defaultParams));
}

TEST_F(TargetDetectionIntegrationTest, NaturalPointSelectorDefaultParameters)
{
    // Test that NaturalPointSelector provides valid default parameters
    auto defaultParams = naturalPointSelector->getDefaultParameters();
    
    EXPECT_GT(defaultParams.neighborhoodRadius, 0.0f);
    EXPECT_LT(defaultParams.neighborhoodRadius, 1.0f);
    EXPECT_GE(defaultParams.curvatureThreshold, 0.0f);
    EXPECT_LE(defaultParams.curvatureThreshold, 1.0f);
    
    // Verify parameters are valid
    EXPECT_TRUE(naturalPointSelector->validateParameters(defaultParams));
}

TEST_F(TargetDetectionIntegrationTest, AlignmentEngineTargetDetection)
{
    // Test that AlignmentEngine can handle target detection requests
    QSignalSpy progressSpy(alignmentEngine, &AlignmentEngine::targetDetectionProgress);
    QSignalSpy completedSpy(alignmentEngine, &AlignmentEngine::targetDetectionCompleted);
    QSignalSpy errorSpy(alignmentEngine, &AlignmentEngine::targetDetectionError);
    
    QString testScanId = "test_scan";
    int testMode = 0; // Automatic spheres
    QVariantMap testParams;
    testParams["distanceThreshold"] = 0.01;
    testParams["maxIterations"] = 1000;
    
    alignmentEngine->startTargetDetection(testScanId, testMode, testParams);
    
    // Wait for async operations to complete
    QTimer::singleShot(500, [this]() {
        QApplication::processEvents();
    });
    
    // Process events to handle async signals
    QApplication::processEvents();
    
    // Should have received progress updates
    EXPECT_GT(progressSpy.count(), 0) << "Should receive progress updates";
    
    // Wait a bit more for completion
    QTest::qWait(500);
    QApplication::processEvents();
    
    // Should eventually complete (this is a mock implementation)
    EXPECT_GT(completedSpy.count(), 0) << "Should receive completion signal";
    EXPECT_EQ(errorSpy.count(), 0) << "Should not receive error signals";
}

TEST_F(TargetDetectionIntegrationTest, EndToEndWorkflow)
{
    // Test the complete end-to-end workflow
    
    // 1. Start with workflow widget
    workflowWidget->show();
    QSignalSpy targetDetectionRequestedSpy(workflowWidget, &RegistrationWorkflowWidget::targetDetectionRequested);
    
    // 2. Enable target detection
    workflowWidget->enableTargetDetection(true);
    
    // 3. Find and click target detection button
    QPushButton* targetDetectionButton = nullptr;
    auto buttons = workflowWidget->findChildren<QPushButton*>();
    
    for (auto* button : buttons)
    {
        if (button->text().contains("Target Detection", Qt::CaseInsensitive))
        {
            targetDetectionButton = button;
            break;
        }
    }
    
    ASSERT_NE(targetDetectionButton, nullptr);
    
    // 4. Click the button
    targetDetectionButton->click();
    
    // 5. Verify signal was emitted
    EXPECT_EQ(targetDetectionRequestedSpy.count(), 1);
    
    // 6. Create and configure dialog (simulating MainPresenter behavior)
    TargetDetectionDialog dialog(targetManager);
    
    QString testScanId = "end_to_end_test_scan";
    std::vector<PointFullData> testPoints(50); // Larger dataset
    dialog.setPointCloudData(testScanId, testPoints);
    
    // 7. Verify dialog is properly configured
    EXPECT_EQ(dialog.getDetectionMode(), TargetDetectionDialog::AutomaticSpheres);
    
    auto params = dialog.getDetectionParameters();
    EXPECT_GT(params.distanceThreshold, 0.0f);
    
    // 8. Test parameter modification
    params.distanceThreshold = 0.02f;
    params.maxIterations = 2000;
    dialog.setDetectionParameters(params);
    
    auto retrievedParams = dialog.getDetectionParameters();
    EXPECT_FLOAT_EQ(retrievedParams.distanceThreshold, 0.02f);
    EXPECT_EQ(retrievedParams.maxIterations, 2000);
    
    // If we reach here, the end-to-end workflow is functional
    SUCCEED();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
