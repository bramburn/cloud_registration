#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <QTest>

#include "ui/ICPParameterDialog.h"
#include "algorithms/ICPRegistration.h"

class ICPParameterDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create test point clouds
        std::vector<float> sourcePoints = {
            0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f
        };
        
        std::vector<float> targetPoints = {
            0.1f, 0.1f, 0.1f,
            1.1f, 0.1f, 0.1f,
            0.1f, 1.1f, 0.1f,
            0.1f, 0.1f, 1.1f
        };
        
        sourceCloud = PointCloud(sourcePoints);
        targetCloud = PointCloud(targetPoints);
    }

    PointCloud sourceCloud;
    PointCloud targetCloud;
};

TEST_F(ICPParameterDialogTest, DialogCreation)
{
    ICPParameterDialog dialog(sourceCloud, targetCloud);
    
    // Check that dialog is created successfully
    EXPECT_EQ(dialog.windowTitle(), "ICP Parameter Configuration");
    EXPECT_TRUE(dialog.isModal());
}

TEST_F(ICPParameterDialogTest, DefaultParametersLoaded)
{
    ICPParameterDialog dialog(sourceCloud, targetCloud);
    
    ICPParams params = dialog.getICPParameters();
    
    // Check that default parameters are reasonable
    EXPECT_GT(params.maxIterations, 0);
    EXPECT_GT(params.convergenceThreshold, 0.0f);
    EXPECT_GT(params.maxCorrespondenceDistance, 0.0f);
    EXPECT_EQ(params.useOutlierRejection, true);
    EXPECT_GT(params.outlierThreshold, 0.0f);
}

TEST_F(ICPParameterDialogTest, ParameterSetAndGet)
{
    ICPParameterDialog dialog(sourceCloud, targetCloud);
    
    ICPParams testParams;
    testParams.maxIterations = 75;
    testParams.convergenceThreshold = 1e-6f;
    testParams.maxCorrespondenceDistance = 0.05f;
    testParams.useOutlierRejection = false;
    testParams.outlierThreshold = 3.0f;
    
    dialog.setICPParameters(testParams);
    ICPParams retrievedParams = dialog.getICPParameters();
    
    EXPECT_EQ(retrievedParams.maxIterations, testParams.maxIterations);
    EXPECT_FLOAT_EQ(retrievedParams.convergenceThreshold, testParams.convergenceThreshold);
    EXPECT_FLOAT_EQ(retrievedParams.maxCorrespondenceDistance, testParams.maxCorrespondenceDistance);
    EXPECT_EQ(retrievedParams.useOutlierRejection, testParams.useOutlierRejection);
    EXPECT_FLOAT_EQ(retrievedParams.outlierThreshold, testParams.outlierThreshold);
}

TEST_F(ICPParameterDialogTest, ScanIdSetAndGet)
{
    ICPParameterDialog dialog(sourceCloud, targetCloud);
    
    QString sourceScanId = "scan_001";
    QString targetScanId = "scan_002";
    
    dialog.setScanIds(sourceScanId, targetScanId);
    
    EXPECT_EQ(dialog.getSourceScanId(), sourceScanId);
    EXPECT_EQ(dialog.getTargetScanId(), targetScanId);
    
    // Check that window title is updated
    QString expectedTitle = QString("ICP Configuration - %1 → %2").arg(sourceScanId, targetScanId);
    EXPECT_EQ(dialog.windowTitle(), expectedTitle);
}

TEST_F(ICPParameterDialogTest, RunICPSignalEmission)
{
    ICPParameterDialog dialog(sourceCloud, targetCloud);
    dialog.setScanIds("source", "target");
    
    QSignalSpy spy(&dialog, &ICPParameterDialog::runICPRequested);
    
    // Simulate clicking the "Run ICP" button
    // Note: This would require accessing the button directly or using QTest::mouseClick
    // For now, we'll test the slot directly
    dialog.onRunICPClicked();
    
    // Check that signal was emitted
    EXPECT_EQ(spy.count(), 1);
    
    // Check signal parameters
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(1).toString(), "source");
    EXPECT_EQ(arguments.at(2).toString(), "target");
}

TEST_F(ICPParameterDialogTest, ResetToDefaults)
{
    ICPParameterDialog dialog(sourceCloud, targetCloud);
    
    // Get initial default parameters
    ICPParams defaultParams = dialog.getICPParameters();
    
    // Modify parameters
    ICPParams modifiedParams;
    modifiedParams.maxIterations = 999;
    modifiedParams.convergenceThreshold = 0.1f;
    modifiedParams.maxCorrespondenceDistance = 5.0f;
    modifiedParams.useOutlierRejection = false;
    modifiedParams.outlierThreshold = 10.0f;
    
    dialog.setICPParameters(modifiedParams);
    
    // Reset to defaults
    dialog.onResetToDefaultsClicked();
    
    // Check that parameters are back to defaults
    ICPParams resetParams = dialog.getICPParameters();
    EXPECT_EQ(resetParams.maxIterations, defaultParams.maxIterations);
    EXPECT_FLOAT_EQ(resetParams.convergenceThreshold, defaultParams.convergenceThreshold);
    EXPECT_FLOAT_EQ(resetParams.maxCorrespondenceDistance, defaultParams.maxCorrespondenceDistance);
    EXPECT_EQ(resetParams.useOutlierRejection, defaultParams.useOutlierRejection);
    EXPECT_FLOAT_EQ(resetParams.outlierThreshold, defaultParams.outlierThreshold);
}

// Test the ICPRegistration::getRecommendedParameters method
TEST_F(ICPParameterDialogTest, RecommendedParametersCalculation)
{
    ICPParams params = ICPRegistration::getRecommendedParameters(sourceCloud, targetCloud);
    
    // Check that recommended parameters are reasonable
    EXPECT_GT(params.maxIterations, 0);
    EXPECT_LE(params.maxIterations, 1000);
    
    EXPECT_GT(params.convergenceThreshold, 0.0f);
    EXPECT_LE(params.convergenceThreshold, 1e-2f);
    
    EXPECT_GT(params.maxCorrespondenceDistance, 0.0f);
    EXPECT_LE(params.maxCorrespondenceDistance, 10.0f);
    
    EXPECT_EQ(params.useOutlierRejection, true);
    EXPECT_GT(params.outlierThreshold, 0.0f);
    EXPECT_LE(params.outlierThreshold, 5.0f);
    
    EXPECT_GT(params.subsamplingRatio, 0.0f);
    EXPECT_LE(params.subsamplingRatio, 1.0f);
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
