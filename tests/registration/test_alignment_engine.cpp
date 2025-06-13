#include <QMatrix4x4>
#include <QVector3D>

#include "registration/AlignmentEngine.h"

#include <gtest/gtest.h>

class AlignmentEngineTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        alignmentEngine = std::make_unique<AlignmentEngine>();
    }

    std::unique_ptr<AlignmentEngine> alignmentEngine;
};

// Test basic alignment engine functionality
TEST_F(AlignmentEngineTest, BasicAlignment)
{
    // Create simple test point sets
    QList<QVector3D> sourcePoints = {
        QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)};

    QList<QVector3D> targetPoints = {
        QVector3D(1.0f, 1.0f, 0.0f), QVector3D(2.0f, 1.0f, 0.0f), QVector3D(1.0f, 2.0f, 0.0f)};

    bool success = alignmentEngine->setSourcePoints(sourcePoints);
    EXPECT_TRUE(success);

    success = alignmentEngine->setTargetPoints(targetPoints);
    EXPECT_TRUE(success);

    QMatrix4x4 transform = alignmentEngine->computeAlignment();
    EXPECT_FALSE(transform.isIdentity());
}

// Test alignment with empty point sets
TEST_F(AlignmentEngineTest, EmptyPointSets)
{
    QList<QVector3D> emptyPoints;

    bool success = alignmentEngine->setSourcePoints(emptyPoints);
    EXPECT_FALSE(success);

    success = alignmentEngine->setTargetPoints(emptyPoints);
    EXPECT_FALSE(success);
}

// Test alignment statistics
TEST_F(AlignmentEngineTest, AlignmentStatistics)
{
    QList<QVector3D> sourcePoints = {QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f)};

    QList<QVector3D> targetPoints = {QVector3D(0.1f, 0.0f, 0.0f), QVector3D(1.1f, 0.0f, 0.0f)};

    alignmentEngine->setSourcePoints(sourcePoints);
    alignmentEngine->setTargetPoints(targetPoints);

    QMatrix4x4 transform = alignmentEngine->computeAlignment();

    float rmsError = alignmentEngine->getRMSError();
    EXPECT_GT(rmsError, 0.0f);

    int iterations = alignmentEngine->getIterationCount();
    EXPECT_GT(iterations, 0);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
