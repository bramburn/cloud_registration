#include <QCoreApplication>
#include <QMatrix4x4>
#include <QSignalSpy>
#include <QVector3D>

#include <cmath>

#include "../../src/algorithms/ICPRegistration.h"
#include "../../src/algorithms/LeastSquaresAlignment.h"
#include "../../src/algorithms/PointToPlaneICP.h"
#include "../../src/registration/AlignmentEngine.h"

#include <gtest/gtest.h>

class ICPRegistrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create QCoreApplication if it doesn't exist
        if (!QCoreApplication::instance())
        {
            int argc = 0;
            char** argv = nullptr;
            app = new QCoreApplication(argc, argv);
        }
    }

    void TearDown() override
    {
        // Don't delete the application as it might be used by other tests
    }

    // Helper method to create a simple point cloud
    PointCloud createTestPointCloud(int numPoints = 100)
    {
        PointCloud cloud;
        cloud.points.reserve(numPoints);

        // Create a simple grid of points
        int gridSize = static_cast<int>(std::sqrt(numPoints));
        for (int i = 0; i < gridSize; ++i)
        {
            for (int j = 0; j < gridSize; ++j)
            {
                float x = static_cast<float>(i) * 0.1f;
                float y = static_cast<float>(j) * 0.1f;
                float z = 0.0f;
                cloud.points.emplace_back(x, y, z);
            }
        }

        return cloud;
    }

    // Helper method to transform a point cloud
    PointCloud transformPointCloud(const PointCloud& cloud, const QMatrix4x4& transform)
    {
        PointCloud transformed = cloud;
        transformed.transform(transform);
        return transformed;
    }

    // Helper method to create a known transformation
    QMatrix4x4 createTestTransformation(
        float tx = 0.1f, float ty = 0.05f, float tz = 0.02f, float rotX = 0.1f, float rotY = 0.05f, float rotZ = 0.02f)
    {
        QMatrix4x4 transform;
        transform.setToIdentity();

        // Apply translation
        transform.translate(tx, ty, tz);

        // Apply rotations (in degrees)
        transform.rotate(rotX * 180.0f / M_PI, QVector3D(1, 0, 0));
        transform.rotate(rotY * 180.0f / M_PI, QVector3D(0, 1, 0));
        transform.rotate(rotZ * 180.0f / M_PI, QVector3D(0, 0, 1));

        return transform;
    }

    QCoreApplication* app = nullptr;
};

// Test PointCloud basic functionality
TEST_F(ICPRegistrationTest, PointCloudBasicOperations)
{
    // Test construction from float vector
    std::vector<float> pointData = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    PointCloud cloud(pointData);

    EXPECT_EQ(cloud.size(), 4);
    EXPECT_FALSE(cloud.empty());

    EXPECT_FLOAT_EQ(cloud.points[0].x(), 0.0f);
    EXPECT_FLOAT_EQ(cloud.points[0].y(), 0.0f);
    EXPECT_FLOAT_EQ(cloud.points[0].z(), 0.0f);

    EXPECT_FLOAT_EQ(cloud.points[1].x(), 1.0f);
    EXPECT_FLOAT_EQ(cloud.points[1].y(), 0.0f);
    EXPECT_FLOAT_EQ(cloud.points[1].z(), 0.0f);
}

// Test PointCloud transformation
TEST_F(ICPRegistrationTest, PointCloudTransformation)
{
    PointCloud cloud = createTestPointCloud(4);
    QVector3D originalPoint = cloud.points[0];

    QMatrix4x4 transform;
    transform.setToIdentity();
    transform.translate(1.0f, 2.0f, 3.0f);

    cloud.transform(transform);

    QVector3D expectedPoint = transform.map(originalPoint);
    EXPECT_FLOAT_EQ(cloud.points[0].x(), expectedPoint.x());
    EXPECT_FLOAT_EQ(cloud.points[0].y(), expectedPoint.y());
    EXPECT_FLOAT_EQ(cloud.points[0].z(), expectedPoint.z());
}

// Test PointCloud subsampling
TEST_F(ICPRegistrationTest, PointCloudSubsampling)
{
    PointCloud cloud = createTestPointCloud(100);

    // Test 50% subsampling
    PointCloud subsampled = cloud.subsample(0.5f);
    EXPECT_LT(subsampled.size(), cloud.size());
    EXPECT_GT(subsampled.size(), 0);

    // Test 100% subsampling (should return original)
    PointCloud full = cloud.subsample(1.0f);
    EXPECT_EQ(full.size(), cloud.size());

    // Test 0% subsampling (should return empty)
    PointCloud empty = cloud.subsample(0.0f);
    EXPECT_TRUE(empty.empty());
}

// Test KDTree nearest neighbor search
TEST_F(ICPRegistrationTest, KDTreeNearestNeighbor)
{
    PointCloud cloud = createTestPointCloud(25);  // 5x5 grid
    KDTree kdTree(cloud);

    // Query for a point that should be in the cloud
    QVector3D query(0.1f, 0.1f, 0.0f);  // Should match cloud.points[6] (1,1 in grid)
    QVector3D nearest;
    float distance;

    bool found = kdTree.findNearestNeighbor(query, nearest, distance);

    EXPECT_TRUE(found);
    EXPECT_LT(distance, 0.01f);  // Should be very close
    EXPECT_FLOAT_EQ(nearest.x(), 0.1f);
    EXPECT_FLOAT_EQ(nearest.y(), 0.1f);
    EXPECT_FLOAT_EQ(nearest.z(), 0.0f);
}

// Test LeastSquaresAlignment
TEST_F(ICPRegistrationTest, LeastSquaresAlignment)
{
    // Create corresponding points with known transformation
    QList<QPair<QVector3D, QVector3D>> correspondences;

    // Original points
    QVector3D p1(0.0f, 0.0f, 0.0f);
    QVector3D p2(1.0f, 0.0f, 0.0f);
    QVector3D p3(0.0f, 1.0f, 0.0f);
    QVector3D p4(0.0f, 0.0f, 1.0f);

    // Known transformation: translate by (1, 2, 3)
    QMatrix4x4 knownTransform;
    knownTransform.setToIdentity();
    knownTransform.translate(1.0f, 2.0f, 3.0f);

    // Transformed points
    QVector3D t1 = knownTransform.map(p1);
    QVector3D t2 = knownTransform.map(p2);
    QVector3D t3 = knownTransform.map(p3);
    QVector3D t4 = knownTransform.map(p4);

    correspondences.append(qMakePair(p1, t1));
    correspondences.append(qMakePair(p2, t2));
    correspondences.append(qMakePair(p3, t3));
    correspondences.append(qMakePair(p4, t4));

    // Compute transformation
    QMatrix4x4 computed = LeastSquaresAlignment::computeTransformation(correspondences);

    // Verify the computed transformation is close to the known one
    for (const auto& pair : correspondences)
    {
        QVector3D transformed = computed.map(pair.first);
        QVector3D expected = pair.second;

        EXPECT_NEAR(transformed.x(), expected.x(), 0.01f);
        EXPECT_NEAR(transformed.y(), expected.y(), 0.01f);
        EXPECT_NEAR(transformed.z(), expected.z(), 0.01f);
    }
}

// Test ICP convergence with perfect data
TEST_F(ICPRegistrationTest, ICPConvergenceAccuracy)
{
    // Create source and target clouds
    PointCloud source = createTestPointCloud(25);

    // Apply known transformation to create target
    QMatrix4x4 knownTransform = createTestTransformation(0.05f, 0.03f, 0.02f, 0.02f, 0.01f, 0.015f);
    PointCloud target = transformPointCloud(source, knownTransform);

    // Run ICP
    ICPRegistration icp;
    ICPParams params;
    params.maxIterations = 50;
    params.convergenceThreshold = 1e-6f;
    params.maxCorrespondenceDistance = 1.0f;

    QSignalSpy finishedSpy(&icp, &ICPRegistration::computationFinished);

    QMatrix4x4 result = icp.compute(source, target, QMatrix4x4(), params);

    // Wait for completion signal
    EXPECT_TRUE(finishedSpy.wait(5000));  // 5 second timeout
    EXPECT_EQ(finishedSpy.count(), 1);

    // Verify the result is close to the known transformation
    QMatrix4x4 inverse = knownTransform.inverted();
    QMatrix4x4 error = result * inverse;

    // Check that the error matrix is close to identity
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            if (i == j)
            {
                EXPECT_NEAR(error(i, j), 1.0f, 0.01f);  // Diagonal should be ~1
            }
            else
            {
                EXPECT_NEAR(error(i, j), 0.0f, 0.01f);  // Off-diagonal should be ~0
            }
        }
    }
}

// Test ICP with partial overlap
TEST_F(ICPRegistrationTest, ICPPartialOverlap)
{
    // Create source cloud
    PointCloud source = createTestPointCloud(25);

    // Create target cloud with only partial overlap
    PointCloud target;
    target.points.reserve(15);

    // Only include some points from the source (simulating partial overlap)
    for (size_t i = 0; i < std::min(size_t(15), source.size()); ++i)
    {
        target.points.push_back(source.points[i]);
    }

    // Apply small transformation to target
    QMatrix4x4 transform = createTestTransformation(0.02f, 0.01f, 0.005f, 0.01f, 0.005f, 0.008f);
    target.transform(transform);

    // Run ICP
    ICPRegistration icp;
    ICPParams params;
    params.maxIterations = 30;
    params.convergenceThreshold = 1e-5f;
    params.maxCorrespondenceDistance = 0.5f;

    QSignalSpy finishedSpy(&icp, &ICPRegistration::computationFinished);

    QMatrix4x4 result = icp.compute(source, target, QMatrix4x4(), params);

    // Wait for completion
    EXPECT_TRUE(finishedSpy.wait(5000));

    // Should still converge, though maybe not as accurately
    QList<QVariant> arguments = finishedSpy.takeFirst();
    bool success = arguments.at(0).toBool();
    EXPECT_TRUE(success);
}

// Test ICP cancellation
TEST_F(ICPRegistrationTest, ICPCancellation)
{
    PointCloud source = createTestPointCloud(100);
    PointCloud target = createTestPointCloud(100);

    ICPRegistration icp;
    ICPParams params;
    params.maxIterations = 1000;           // Many iterations
    params.convergenceThreshold = 1e-10f;  // Very strict convergence

    QSignalSpy finishedSpy(&icp, &ICPRegistration::computationFinished);

    // Start ICP computation
    icp.compute(source, target, QMatrix4x4(), params);

    // Cancel immediately
    icp.cancel();

    // Should finish quickly due to cancellation
    EXPECT_TRUE(finishedSpy.wait(2000));

    QList<QVariant> arguments = finishedSpy.takeFirst();
    bool success = arguments.at(0).toBool();
    EXPECT_FALSE(success);  // Should be false due to cancellation
}

// Test AlignmentEngine integration
TEST_F(ICPRegistrationTest, AlignmentEngineIntegration)
{
    AlignmentEngine engine;

    // Set up manual correspondences
    QList<QPair<QVector3D, QVector3D>> correspondences;
    correspondences.append(qMakePair(QVector3D(0, 0, 0), QVector3D(0.1f, 0.1f, 0.1f)));
    correspondences.append(qMakePair(QVector3D(1, 0, 0), QVector3D(1.1f, 0.1f, 0.1f)));
    correspondences.append(qMakePair(QVector3D(0, 1, 0), QVector3D(0.1f, 1.1f, 0.1f)));
    correspondences.append(qMakePair(QVector3D(0, 0, 1), QVector3D(0.1f, 0.1f, 1.1f)));

    QSignalSpy transformSpy(&engine, &AlignmentEngine::transformationUpdated);
    QSignalSpy qualitySpy(&engine, &AlignmentEngine::qualityMetricsUpdated);

    engine.setCorrespondences(correspondences);

    // Should emit transformation and quality updates
    EXPECT_GE(transformSpy.count(), 1);
    EXPECT_GE(qualitySpy.count(), 1);

    // Should have valid transformation
    QMatrix4x4 transform = engine.getCurrentTransformation();
    EXPECT_FALSE(transform.isIdentity());

    // Should have reasonable RMS error
    float rmsError = engine.getCurrentRMSError();
    EXPECT_GT(rmsError, 0.0f);
    EXPECT_LT(rmsError, 1.0f);  // Should be reasonable for this test case
}
