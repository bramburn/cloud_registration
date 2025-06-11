#include <gtest/gtest.h>
#include <QVector3D>
#include <QMatrix4x4>
#include "registration/ErrorAnalysis.h"

class ErrorAnalysisTest : public ::testing::Test {
protected:
    void SetUp() override {
        errorAnalysis = std::make_unique<ErrorAnalysis>();
    }

    std::unique_ptr<ErrorAnalysis> errorAnalysis;
};

// Test RMS error calculation
TEST_F(ErrorAnalysisTest, RMSErrorCalculation) {
    QList<QVector3D> sourcePoints = {
        QVector3D(0.0f, 0.0f, 0.0f),
        QVector3D(1.0f, 0.0f, 0.0f),
        QVector3D(0.0f, 1.0f, 0.0f)
    };
    
    QList<QVector3D> targetPoints = {
        QVector3D(0.1f, 0.0f, 0.0f),
        QVector3D(1.1f, 0.0f, 0.0f),
        QVector3D(0.0f, 1.1f, 0.0f)
    };
    
    float rmsError = errorAnalysis->calculateRMSError(sourcePoints, targetPoints);
    
    EXPECT_GT(rmsError, 0.0f);
    EXPECT_LT(rmsError, 1.0f); // Should be reasonable error
}

// Test mean error calculation
TEST_F(ErrorAnalysisTest, MeanErrorCalculation) {
    QList<QVector3D> sourcePoints = {
        QVector3D(0.0f, 0.0f, 0.0f),
        QVector3D(1.0f, 0.0f, 0.0f)
    };
    
    QList<QVector3D> targetPoints = {
        QVector3D(0.1f, 0.0f, 0.0f),
        QVector3D(1.1f, 0.0f, 0.0f)
    };
    
    float meanError = errorAnalysis->calculateMeanError(sourcePoints, targetPoints);
    
    EXPECT_FLOAT_EQ(meanError, 0.1f); // Expected mean error
}

// Test maximum error calculation
TEST_F(ErrorAnalysisTest, MaxErrorCalculation) {
    QList<QVector3D> sourcePoints = {
        QVector3D(0.0f, 0.0f, 0.0f),
        QVector3D(1.0f, 0.0f, 0.0f)
    };
    
    QList<QVector3D> targetPoints = {
        QVector3D(0.1f, 0.0f, 0.0f),
        QVector3D(1.2f, 0.0f, 0.0f)
    };
    
    float maxError = errorAnalysis->calculateMaxError(sourcePoints, targetPoints);
    
    EXPECT_FLOAT_EQ(maxError, 0.2f); // Expected max error
}

// Test error statistics
TEST_F(ErrorAnalysisTest, ErrorStatistics) {
    QList<QVector3D> sourcePoints = {
        QVector3D(0.0f, 0.0f, 0.0f),
        QVector3D(1.0f, 0.0f, 0.0f),
        QVector3D(0.0f, 1.0f, 0.0f)
    };
    
    QList<QVector3D> targetPoints = {
        QVector3D(0.1f, 0.0f, 0.0f),
        QVector3D(1.1f, 0.0f, 0.0f),
        QVector3D(0.0f, 1.1f, 0.0f)
    };
    
    ErrorAnalysis::Statistics stats = errorAnalysis->calculateStatistics(sourcePoints, targetPoints);
    
    EXPECT_GT(stats.rmsError, 0.0f);
    EXPECT_GT(stats.meanError, 0.0f);
    EXPECT_GT(stats.maxError, 0.0f);
    EXPECT_EQ(stats.pointCount, 3);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
