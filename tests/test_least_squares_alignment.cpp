#include <QMatrix4x4>
#include <QVector3D>

#include "algorithms/LeastSquaresAlignment.h"

#include <gtest/gtest.h>

class LeastSquaresAlignmentTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Set up test data
    }
};

// Test basic alignment functionality
TEST_F(LeastSquaresAlignmentTest, BasicAlignment)
{
    // Create simple test point sets
    QList<QVector3D> sourcePoints = {
        QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)};

    QList<QVector3D> targetPoints = {
        QVector3D(1.0f, 1.0f, 0.0f), QVector3D(2.0f, 1.0f, 0.0f), QVector3D(1.0f, 2.0f, 0.0f)};

    LeastSquaresAlignment alignment;
    QMatrix4x4 transform = alignment.computeAlignment(sourcePoints, targetPoints);

    // Check that transform is not identity (some transformation occurred)
    EXPECT_FALSE(transform.isIdentity());
}

// Test alignment with identical point sets
TEST_F(LeastSquaresAlignmentTest, IdenticalPointSets)
{
    QList<QVector3D> points = {QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f)};

    LeastSquaresAlignment alignment;
    QMatrix4x4 transform = alignment.computeAlignment(points, points);

    // Transform should be close to identity for identical point sets
    EXPECT_TRUE(transform.isIdentity());
}

// Test error calculation
TEST_F(LeastSquaresAlignmentTest, ErrorCalculation)
{
    QList<QVector3D> sourcePoints = {QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f)};

    QList<QVector3D> targetPoints = {QVector3D(0.1f, 0.0f, 0.0f), QVector3D(1.1f, 0.0f, 0.0f)};

    LeastSquaresAlignment alignment;
    QMatrix4x4 transform = alignment.computeAlignment(sourcePoints, targetPoints);
    float error = alignment.computeAlignmentError(sourcePoints, targetPoints, transform);

    EXPECT_GT(error, 0.0f);
    EXPECT_LT(error, 1.0f);  // Should be reasonable error
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
