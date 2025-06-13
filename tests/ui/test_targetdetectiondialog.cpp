#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <QTest>

#include "ui/TargetDetectionDialog.h"
#include "registration/TargetManager.h"
#include "registration/SphereDetector.h"
#include "registration/NaturalPointSelector.h"

class TargetDetectionDialogTest : public ::testing::Test
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
        
        targetManager = new TargetManager();
        dialog = new TargetDetectionDialog(targetManager);
    }

    void TearDown() override
    {
        delete dialog;
        delete targetManager;
    }

    QApplication* app = nullptr;
    TargetManager* targetManager = nullptr;
    TargetDetectionDialog* dialog = nullptr;
};

TEST_F(TargetDetectionDialogTest, InitialState)
{
    EXPECT_NE(dialog, nullptr);
    EXPECT_EQ(dialog->getDetectionMode(), TargetDetectionDialog::AutomaticSpheres);
    
    // Dialog should be initially disabled for detection
    // (no point cloud data loaded)
    auto params = dialog->getDetectionParameters();
    EXPECT_GT(params.distanceThreshold, 0.0f);
    EXPECT_GT(params.maxIterations, 0);
    EXPECT_GT(params.minQuality, 0.0f);
}

TEST_F(TargetDetectionDialogTest, DetectionModeChange)
{
    // Test mode switching
    dialog->show();
    
    // Find the detection mode combo box
    auto* modeCombo = dialog->findChild<QComboBox*>();
    ASSERT_NE(modeCombo, nullptr);
    
    // Test switching to manual mode
    modeCombo->setCurrentIndex(static_cast<int>(TargetDetectionDialog::ManualNaturalPoints));
    EXPECT_EQ(dialog->getDetectionMode(), TargetDetectionDialog::ManualNaturalPoints);
    
    // Test switching to both mode
    modeCombo->setCurrentIndex(static_cast<int>(TargetDetectionDialog::Both));
    EXPECT_EQ(dialog->getDetectionMode(), TargetDetectionDialog::Both);
}

TEST_F(TargetDetectionDialogTest, ParameterGetSet)
{
    TargetDetectionBase::DetectionParams testParams;
    testParams.distanceThreshold = 0.02f;
    testParams.maxIterations = 2000;
    testParams.minQuality = 0.8f;
    testParams.enablePreprocessing = false;
    testParams.minRadius = 0.1f;
    testParams.maxRadius = 1.0f;
    testParams.minInliers = 100;
    testParams.neighborhoodRadius = 0.2f;
    testParams.curvatureThreshold = 0.2f;
    
    dialog->setDetectionParameters(testParams);
    auto retrievedParams = dialog->getDetectionParameters();
    
    EXPECT_FLOAT_EQ(retrievedParams.distanceThreshold, testParams.distanceThreshold);
    EXPECT_EQ(retrievedParams.maxIterations, testParams.maxIterations);
    EXPECT_FLOAT_EQ(retrievedParams.minQuality, testParams.minQuality);
    EXPECT_EQ(retrievedParams.enablePreprocessing, testParams.enablePreprocessing);
    EXPECT_FLOAT_EQ(retrievedParams.minRadius, testParams.minRadius);
    EXPECT_FLOAT_EQ(retrievedParams.maxRadius, testParams.maxRadius);
    EXPECT_EQ(retrievedParams.minInliers, testParams.minInliers);
    EXPECT_FLOAT_EQ(retrievedParams.neighborhoodRadius, testParams.neighborhoodRadius);
    EXPECT_FLOAT_EQ(retrievedParams.curvatureThreshold, testParams.curvatureThreshold);
}

TEST_F(TargetDetectionDialogTest, DefaultParameters)
{
    // Test that dialog initializes with reasonable defaults
    dialog->resetToDefaults();
    auto params = dialog->getDetectionParameters();
    
    EXPECT_GT(params.distanceThreshold, 0.0f);
    EXPECT_LT(params.distanceThreshold, 1.0f);
    EXPECT_GT(params.maxIterations, 100);
    EXPECT_LT(params.maxIterations, 10000);
    EXPECT_GE(params.minQuality, 0.0f);
    EXPECT_LE(params.minQuality, 1.0f);
    EXPECT_GT(params.minRadius, 0.0f);
    EXPECT_GT(params.maxRadius, params.minRadius);
    EXPECT_GT(params.minInliers, 0);
    EXPECT_GT(params.neighborhoodRadius, 0.0f);
    EXPECT_GE(params.curvatureThreshold, 0.0f);
}

TEST_F(TargetDetectionDialogTest, PointCloudDataSetting)
{
    QString testScanId = "test_scan_001";
    std::vector<PointFullData> testPoints;
    
    // Create some test points
    for (int i = 0; i < 100; ++i)
    {
        PointFullData point;
        point.x = static_cast<float>(i) * 0.1f;
        point.y = static_cast<float>(i) * 0.1f;
        point.z = static_cast<float>(i) * 0.1f;
        point.r = 255;
        point.g = 255;
        point.b = 255;
        point.intensity = 1000;
        testPoints.push_back(point);
    }
    
    dialog->setPointCloudData(testScanId, testPoints);
    
    // After setting point cloud data, the start button should be enabled
    auto* startButton = dialog->findChild<QPushButton*>("Start Detection");
    if (startButton)
    {
        EXPECT_TRUE(startButton->isEnabled());
    }
}

TEST_F(TargetDetectionDialogTest, SignalEmission)
{
    // Test signal emission for manual selection
    QSignalSpy manualSelectionSpy(dialog, &TargetDetectionDialog::manualSelectionRequested);
    
    QString testScanId = "test_scan_001";
    std::vector<PointFullData> testPoints(10); // Small test dataset
    dialog->setPointCloudData(testScanId, testPoints);
    
    // Switch to manual mode and start detection
    auto* modeCombo = dialog->findChild<QComboBox*>();
    if (modeCombo)
    {
        modeCombo->setCurrentIndex(static_cast<int>(TargetDetectionDialog::ManualNaturalPoints));
        dialog->startDetection();
        
        // Should emit manual selection signal
        EXPECT_EQ(manualSelectionSpy.count(), 1);
        auto arguments = manualSelectionSpy.takeFirst();
        EXPECT_EQ(arguments.at(0).toString(), testScanId);
    }
}

// Test the detection completion workflow
TEST_F(TargetDetectionDialogTest, DetectionCompletion)
{
    QSignalSpy detectionCompletedSpy(dialog, &TargetDetectionDialog::detectionCompleted);
    
    QString testScanId = "test_scan_001";
    std::vector<PointFullData> testPoints(10);
    dialog->setPointCloudData(testScanId, testPoints);
    
    // Create a mock detection result
    TargetDetectionBase::DetectionResult mockResult;
    mockResult.success = true;
    mockResult.processedPoints = 10;
    mockResult.processingTime = 1.5;
    
    // Simulate detection completion
    dialog->onDetectionCompleted(mockResult);
    
    // Accept the targets
    dialog->onAcceptTargets();
    
    // Should emit detection completed signal
    EXPECT_EQ(detectionCompletedSpy.count(), 1);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
