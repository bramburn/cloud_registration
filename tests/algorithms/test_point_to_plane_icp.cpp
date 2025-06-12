#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QMatrix4x4>
#include <QVector3D>
#include <cmath>
#include "../../src/algorithms/PointToPlaneICP.h"

class PointToPlaneICPTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create QCoreApplication if it doesn't exist
        if (!QCoreApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = new QCoreApplication(argc, argv);
        }
    }
    
    void TearDown() override {
        // Don't delete the application as it might be used by other tests
    }
    
    // Helper method to create a planar point cloud with normals
    PointCloud createPlanarPointCloud(int numPoints = 25) {
        PointCloud cloud;
        cloud.points.reserve(numPoints);
        cloud.normals.reserve(numPoints);
        
        // Create points on a plane (z = 0) with normal pointing up
        int gridSize = static_cast<int>(std::sqrt(numPoints));
        for (int i = 0; i < gridSize; ++i) {
            for (int j = 0; j < gridSize; ++j) {
                float x = static_cast<float>(i) * 0.2f;
                float y = static_cast<float>(j) * 0.2f;
                float z = 0.0f;
                
                cloud.points.emplace_back(x, y, z);
                cloud.normals.emplace_back(0.0f, 0.0f, 1.0f);  // Normal pointing up
            }
        }
        
        return cloud;
    }
    
    // Helper method to create a point cloud with mixed surface orientations
    PointCloud createMixedSurfacePointCloud() {
        PointCloud cloud;
        
        // Floor points (z = 0, normal up)
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                cloud.points.emplace_back(i * 0.5f, j * 0.5f, 0.0f);
                cloud.normals.emplace_back(0.0f, 0.0f, 1.0f);
            }
        }
        
        // Wall points (x = 0, normal in +x direction)
        for (int i = 0; i < 3; ++i) {
            for (int k = 0; k < 3; ++k) {
                cloud.points.emplace_back(0.0f, i * 0.5f, k * 0.5f + 0.5f);
                cloud.normals.emplace_back(1.0f, 0.0f, 0.0f);
            }
        }
        
        // Another wall points (y = 0, normal in +y direction)
        for (int i = 0; i < 3; ++i) {
            for (int k = 0; k < 3; ++k) {
                cloud.points.emplace_back(i * 0.5f, 0.0f, k * 0.5f + 0.5f);
                cloud.normals.emplace_back(0.0f, 1.0f, 0.0f);
            }
        }
        
        return cloud;
    }
    
    QCoreApplication* app = nullptr;
};

// Test PointToPlaneICP with planar surfaces
TEST_F(PointToPlaneICPTest, PlanarSurfaceAlignment) {
    // Create source and target planar clouds
    PointCloud source = createPlanarPointCloud(25);
    PointCloud target = source;  // Copy
    
    // Apply small transformation to target
    QMatrix4x4 knownTransform;
    knownTransform.setToIdentity();
    knownTransform.translate(0.05f, 0.03f, 0.02f);
    knownTransform.rotate(2.0f, QVector3D(0, 0, 1));  // Small rotation around Z
    
    target.transform(knownTransform);
    
    // Run Point-to-Plane ICP
    PointToPlaneICP icp;
    ICPParams params;
    params.maxIterations = 30;
    params.convergenceThreshold = 1e-6f;
    params.maxCorrespondenceDistance = 1.0f;
    
    QSignalSpy finishedSpy(&icp, &ICPRegistration::computationFinished);
    
    QMatrix4x4 result = icp.compute(source, target, QMatrix4x4(), params);
    
    // Wait for completion
    EXPECT_TRUE(finishedSpy.wait(5000));
    EXPECT_EQ(finishedSpy.count(), 1);
    
    // Check that computation was successful
    QList<QVariant> arguments = finishedSpy.takeFirst();
    bool success = arguments.at(0).toBool();
    EXPECT_TRUE(success);
    
    // Verify the result is close to the known transformation
    // Apply result to source and check if it matches target
    PointCloud transformed = source;
    transformed.transform(result);
    
    // Check that transformed source is close to target
    ASSERT_EQ(transformed.size(), target.size());
    
    float totalError = 0.0f;
    for (size_t i = 0; i < transformed.size(); ++i) {
        QVector3D diff = transformed.points[i] - target.points[i];
        totalError += diff.length();
    }
    float avgError = totalError / transformed.size();
    
    EXPECT_LT(avgError, 0.01f);  // Average error should be very small
}

// Test PointToPlaneICP with mixed surface orientations
TEST_F(PointToPlaneICPTest, MixedSurfaceAlignment) {
    // Create source and target with mixed surfaces
    PointCloud source = createMixedSurfacePointCloud();
    PointCloud target = source;  // Copy
    
    // Apply transformation to target
    QMatrix4x4 knownTransform;
    knownTransform.setToIdentity();
    knownTransform.translate(0.02f, 0.03f, 0.01f);
    knownTransform.rotate(1.0f, QVector3D(1, 0, 0));  // Small rotation around X
    knownTransform.rotate(0.5f, QVector3D(0, 1, 0));  // Small rotation around Y
    
    target.transform(knownTransform);
    
    // Run Point-to-Plane ICP
    PointToPlaneICP icp;
    ICPParams params;
    params.maxIterations = 50;
    params.convergenceThreshold = 1e-5f;
    params.maxCorrespondenceDistance = 1.0f;
    
    QSignalSpy progressSpy(&icp, &ICPRegistration::progressUpdated);
    QSignalSpy finishedSpy(&icp, &ICPRegistration::computationFinished);
    
    QMatrix4x4 result = icp.compute(source, target, QMatrix4x4(), params);
    
    // Wait for completion
    EXPECT_TRUE(finishedSpy.wait(5000));
    
    // Should have received progress updates
    EXPECT_GT(progressSpy.count(), 0);
    
    // Check final result
    QList<QVariant> arguments = finishedSpy.takeFirst();
    bool success = arguments.at(0).toBool();
    float finalRMSError = arguments.at(2).toFloat();
    int iterations = arguments.at(3).toInt();
    
    EXPECT_TRUE(success);
    EXPECT_LT(finalRMSError, 0.05f);  // Should achieve good accuracy
    EXPECT_GT(iterations, 0);
    EXPECT_LE(iterations, params.maxIterations);
}

// Test PointToPlaneICP fallback when no normals are provided
TEST_F(PointToPlaneICPTest, FallbackWithoutNormals) {
    // Create point cloud without normals
    PointCloud source;
    source.points = {
        QVector3D(0.0f, 0.0f, 0.0f),
        QVector3D(1.0f, 0.0f, 0.0f),
        QVector3D(0.0f, 1.0f, 0.0f),
        QVector3D(1.0f, 1.0f, 0.0f)
    };
    // Note: no normals added
    
    PointCloud target = source;
    QMatrix4x4 transform;
    transform.setToIdentity();
    transform.translate(0.1f, 0.1f, 0.0f);
    target.transform(transform);
    
    // Run Point-to-Plane ICP (should fall back to point-to-point or estimate normals)
    PointToPlaneICP icp;
    ICPParams params;
    params.maxIterations = 20;
    params.convergenceThreshold = 1e-5f;
    
    QSignalSpy finishedSpy(&icp, &ICPRegistration::computationFinished);
    
    QMatrix4x4 result = icp.compute(source, target, QMatrix4x4(), params);
    
    // Should still complete (either with estimated normals or fallback)
    EXPECT_TRUE(finishedSpy.wait(5000));
    
    QList<QVariant> arguments = finishedSpy.takeFirst();
    bool success = arguments.at(0).toBool();
    
    // Should succeed even without initial normals
    EXPECT_TRUE(success);
}

// Test point-to-plane error calculation
TEST_F(PointToPlaneICPTest, PointToPlaneErrorCalculation) {
    PointToPlaneICP icp;
    
    // Create correspondences with known point-to-plane distances
    std::vector<Correspondence> correspondences;
    
    // Point on plane z=0 with normal (0,0,1)
    Correspondence corr1;
    corr1.sourcePoint = QVector3D(1.0f, 1.0f, 0.1f);  // 0.1 units above plane
    corr1.targetPoint = QVector3D(1.0f, 1.0f, 0.0f);  // Point on plane
    corr1.targetNormal = QVector3D(0.0f, 0.0f, 1.0f); // Normal pointing up
    corr1.isValid = true;
    correspondences.push_back(corr1);
    
    // Another point
    Correspondence corr2;
    corr2.sourcePoint = QVector3D(2.0f, 2.0f, -0.05f); // 0.05 units below plane
    corr2.targetPoint = QVector3D(2.0f, 2.0f, 0.0f);   // Point on plane
    corr2.targetNormal = QVector3D(0.0f, 0.0f, 1.0f);  // Normal pointing up
    corr2.isValid = true;
    correspondences.push_back(corr2);
    
    // Calculate point-to-plane RMS error
    float rmsError = icp.calculateRMSError(correspondences);
    
    // Expected: sqrt((0.1^2 + 0.05^2) / 2) = sqrt(0.0125 / 2) = sqrt(0.00625) ≈ 0.079
    float expected = std::sqrt((0.1f * 0.1f + 0.05f * 0.05f) / 2.0f);
    
    EXPECT_NEAR(rmsError, expected, 0.001f);
}

// Test normal estimation functionality
TEST_F(PointToPlaneICPTest, NormalEstimation) {
    // Create a simple planar point cloud without normals
    PointCloud cloud;
    
    // Add points on the XY plane (z = 0)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            cloud.points.emplace_back(i * 0.1f, j * 0.1f, 0.0f);
        }
    }
    
    EXPECT_TRUE(cloud.normals.empty());
    
    // Create PointToPlaneICP and trigger normal estimation
    PointToPlaneICP icp;
    
    // Run ICP which should estimate normals
    PointCloud target = cloud;
    QMatrix4x4 smallTransform;
    smallTransform.setToIdentity();
    smallTransform.translate(0.01f, 0.01f, 0.0f);
    target.transform(smallTransform);
    
    ICPParams params;
    params.maxIterations = 5;  // Just a few iterations to test normal estimation
    
    QSignalSpy finishedSpy(&icp, &ICPRegistration::computationFinished);
    
    icp.compute(cloud, target, QMatrix4x4(), params);
    
    EXPECT_TRUE(finishedSpy.wait(3000));
    
    // The algorithm should have completed (whether with estimated normals or fallback)
    QList<QVariant> arguments = finishedSpy.takeFirst();
    bool success = arguments.at(0).toBool();
    
    // Should complete successfully
    EXPECT_TRUE(success);
}

// Test performance comparison hint (not a strict requirement)
TEST_F(PointToPlaneICPTest, PerformanceHint) {
    // This test provides a hint about expected performance characteristics
    // Point-to-plane ICP should generally converge faster than point-to-point
    
    PointCloud source = createPlanarPointCloud(16);  // 4x4 grid
    PointCloud target = source;
    
    // Apply moderate transformation
    QMatrix4x4 transform;
    transform.setToIdentity();
    transform.translate(0.1f, 0.05f, 0.02f);
    transform.rotate(5.0f, QVector3D(0, 0, 1));
    target.transform(transform);
    
    PointToPlaneICP icp;
    ICPParams params;
    params.maxIterations = 50;
    params.convergenceThreshold = 1e-5f;
    
    QSignalSpy finishedSpy(&icp, &ICPRegistration::computationFinished);
    
    auto startTime = std::chrono::high_resolution_clock::now();
    icp.compute(source, target, QMatrix4x4(), params);
    
    EXPECT_TRUE(finishedSpy.wait(5000));
    auto endTime = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    QList<QVariant> arguments = finishedSpy.takeFirst();
    bool success = arguments.at(0).toBool();
    int iterations = arguments.at(3).toInt();
    
    EXPECT_TRUE(success);
    
    // Performance hint: should converge reasonably quickly
    EXPECT_LT(duration.count(), 2000);  // Should complete within 2 seconds
    EXPECT_LT(iterations, 30);          // Should converge in reasonable iterations
    
    qDebug() << "Point-to-Plane ICP completed in" << duration.count() << "ms with" 
             << iterations << "iterations";
}
