#include <gtest/gtest.h>
#include <QVector3D>
#include <vector>
#include <cmath>
#include "detection/SphereDetector.h"
#include "registration/Target.h"
#include "pointdata.h"

class SphereDetectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        detector = std::make_unique<SphereDetector>();
        
        // Set up default parameters
        params.distanceThreshold = 0.01f;
        params.maxIterations = 1000;
        params.minQuality = 0.5f;
        params.minRadius = 0.05f;
        params.maxRadius = 0.5f;
        params.minInliers = 50;
        params.enablePreprocessing = false;  // Disable for controlled tests
    }
    
    // Generate synthetic sphere point cloud
    std::vector<PointFullData> generateSpherePoints(const QVector3D& center, float radius, int numPoints, float noise = 0.0f) {
        std::vector<PointFullData> points;
        points.reserve(numPoints);
        
        for (int i = 0; i < numPoints; ++i) {
            // Generate random point on sphere surface
            float theta = 2.0f * M_PI * static_cast<float>(rand()) / RAND_MAX;
            float phi = acos(1.0f - 2.0f * static_cast<float>(rand()) / RAND_MAX);
            
            float x = radius * sin(phi) * cos(theta);
            float y = radius * sin(phi) * sin(theta);
            float z = radius * cos(phi);
            
            // Add noise if specified
            if (noise > 0.0f) {
                x += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * noise;
                y += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * noise;
                z += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * noise;
            }
            
            PointFullData point;
            point.x = center.x() + x;
            point.y = center.y() + y;
            point.z = center.z() + z;
            point.intensity = 100.0f;
            point.hasIntensity = true;
            point.hasNormal = false;
            
            points.push_back(point);
        }
        
        return points;
    }
    
    // Generate random noise points
    std::vector<PointFullData> generateNoisePoints(int numPoints, const QVector3D& minBounds, const QVector3D& maxBounds) {
        std::vector<PointFullData> points;
        points.reserve(numPoints);
        
        for (int i = 0; i < numPoints; ++i) {
            PointFullData point;
            point.x = minBounds.x() + (maxBounds.x() - minBounds.x()) * static_cast<float>(rand()) / RAND_MAX;
            point.y = minBounds.y() + (maxBounds.y() - minBounds.y()) * static_cast<float>(rand()) / RAND_MAX;
            point.z = minBounds.z() + (maxBounds.z() - minBounds.z()) * static_cast<float>(rand()) / RAND_MAX;
            point.intensity = 50.0f;
            point.hasIntensity = true;
            point.hasNormal = false;
            
            points.push_back(point);
        }
        
        return points;
    }

    std::unique_ptr<SphereDetector> detector;
    TargetDetectionBase::DetectionParams params;
};

// Test basic sphere detection with perfect sphere
TEST_F(SphereDetectorTest, DetectPerfectSphere) {
    QVector3D sphereCenter(0.0f, 0.0f, 0.0f);
    float sphereRadius = 0.15f;
    
    auto points = generateSpherePoints(sphereCenter, sphereRadius, 200);
    
    auto result = detector->detect(points, params);
    
    EXPECT_TRUE(result.success);
    EXPECT_GE(result.targets.size(), 1);
    
    if (!result.targets.empty()) {
        auto sphereTarget = std::dynamic_pointer_cast<SphereTarget>(result.targets[0]);
        EXPECT_NE(sphereTarget, nullptr);
        
        // Check detected sphere parameters (allow some tolerance)
        QVector3D detectedCenter = sphereTarget->getPosition();
        float detectedRadius = sphereTarget->getRadius();
        
        EXPECT_NEAR(detectedCenter.x(), sphereCenter.x(), 0.02f);
        EXPECT_NEAR(detectedCenter.y(), sphereCenter.y(), 0.02f);
        EXPECT_NEAR(detectedCenter.z(), sphereCenter.z(), 0.02f);
        EXPECT_NEAR(detectedRadius, sphereRadius, 0.02f);
        EXPECT_GT(sphereTarget->getQuality(), 0.5f);
    }
}

// Test sphere detection with noise
TEST_F(SphereDetectorTest, DetectSphereWithNoise) {
    QVector3D sphereCenter(1.0f, 2.0f, 3.0f);
    float sphereRadius = 0.2f;
    
    // Generate sphere points with some noise
    auto spherePoints = generateSpherePoints(sphereCenter, sphereRadius, 150, 0.005f);
    
    // Add random noise points
    auto noisePoints = generateNoisePoints(50, QVector3D(-1.0f, -1.0f, -1.0f), QVector3D(3.0f, 4.0f, 5.0f));
    
    // Combine points
    std::vector<PointFullData> allPoints;
    allPoints.insert(allPoints.end(), spherePoints.begin(), spherePoints.end());
    allPoints.insert(allPoints.end(), noisePoints.begin(), noisePoints.end());
    
    auto result = detector->detect(allPoints, params);
    
    EXPECT_TRUE(result.success);
    EXPECT_GE(result.targets.size(), 1);
    
    if (!result.targets.empty()) {
        auto sphereTarget = std::dynamic_pointer_cast<SphereTarget>(result.targets[0]);
        EXPECT_NE(sphereTarget, nullptr);
        
        // Should still detect sphere reasonably well despite noise
        QVector3D detectedCenter = sphereTarget->getPosition();
        float detectedRadius = sphereTarget->getRadius();
        
        EXPECT_NEAR(detectedCenter.x(), sphereCenter.x(), 0.05f);
        EXPECT_NEAR(detectedCenter.y(), sphereCenter.y(), 0.05f);
        EXPECT_NEAR(detectedCenter.z(), sphereCenter.z(), 0.05f);
        EXPECT_NEAR(detectedRadius, sphereRadius, 0.05f);
    }
}

// Test detection of multiple spheres
TEST_F(SphereDetectorTest, DetectMultipleSpheres) {
    // Create two well-separated spheres
    QVector3D center1(0.0f, 0.0f, 0.0f);
    QVector3D center2(2.0f, 2.0f, 2.0f);
    float radius1 = 0.1f;
    float radius2 = 0.15f;
    
    auto points1 = generateSpherePoints(center1, radius1, 100);
    auto points2 = generateSpherePoints(center2, radius2, 120);
    
    // Combine points
    std::vector<PointFullData> allPoints;
    allPoints.insert(allPoints.end(), points1.begin(), points1.end());
    allPoints.insert(allPoints.end(), points2.begin(), points2.end());
    
    auto result = detector->detect(allPoints, params);
    
    EXPECT_TRUE(result.success);
    EXPECT_GE(result.targets.size(), 2);
    
    if (result.targets.size() >= 2) {
        // Check that we detected both spheres (order may vary)
        std::vector<QVector3D> detectedCenters;
        std::vector<float> detectedRadii;
        
        for (const auto& target : result.targets) {
            auto sphereTarget = std::dynamic_pointer_cast<SphereTarget>(target);
            if (sphereTarget) {
                detectedCenters.push_back(sphereTarget->getPosition());
                detectedRadii.push_back(sphereTarget->getRadius());
            }
        }
        
        EXPECT_EQ(detectedCenters.size(), 2);
        
        // Check that we can match detected spheres to original spheres
        bool found1 = false, found2 = false;
        for (size_t i = 0; i < detectedCenters.size(); ++i) {
            float dist1 = (detectedCenters[i] - center1).length();
            float dist2 = (detectedCenters[i] - center2).length();
            
            if (dist1 < 0.1f && std::abs(detectedRadii[i] - radius1) < 0.05f) {
                found1 = true;
            }
            if (dist2 < 0.1f && std::abs(detectedRadii[i] - radius2) < 0.05f) {
                found2 = true;
            }
        }
        
        EXPECT_TRUE(found1);
        EXPECT_TRUE(found2);
    }
}

// Test parameter validation
TEST_F(SphereDetectorTest, ParameterValidation) {
    // Test valid parameters
    EXPECT_TRUE(detector->validateParameters(params));
    
    // Test invalid parameters
    TargetDetectionBase::DetectionParams invalidParams = params;
    
    // Invalid radius range
    invalidParams.minRadius = 0.3f;
    invalidParams.maxRadius = 0.2f;  // max < min
    EXPECT_FALSE(detector->validateParameters(invalidParams));
    
    // Invalid distance threshold
    invalidParams = params;
    invalidParams.distanceThreshold = -0.01f;
    EXPECT_FALSE(detector->validateParameters(invalidParams));
    
    // Invalid iterations
    invalidParams = params;
    invalidParams.maxIterations = 0;
    EXPECT_FALSE(detector->validateParameters(invalidParams));
}

// Test insufficient points scenario
TEST_F(SphereDetectorTest, InsufficientPoints) {
    // Create very few points (less than minimum required)
    std::vector<PointFullData> fewPoints;
    for (int i = 0; i < 3; ++i) {
        PointFullData point;
        point.x = static_cast<float>(i);
        point.y = 0.0f;
        point.z = 0.0f;
        fewPoints.push_back(point);
    }
    
    auto result = detector->detect(fewPoints, params);
    
    EXPECT_FALSE(result.success);
    EXPECT_FALSE(result.errorMessage.isEmpty());
}

// Test sphere size filtering
TEST_F(SphereDetectorTest, SphereSizeFiltering) {
    // Create sphere outside parameter range
    QVector3D sphereCenter(0.0f, 0.0f, 0.0f);
    float sphereRadius = 0.02f;  // Smaller than minRadius (0.05f)
    
    auto points = generateSpherePoints(sphereCenter, sphereRadius, 100);
    
    auto result = detector->detect(points, params);
    
    // Should not detect sphere that's too small
    EXPECT_TRUE(result.success);  // Detection succeeds but finds no valid spheres
    EXPECT_EQ(result.targets.size(), 0);
}

// Test algorithm name and supported types
TEST_F(SphereDetectorTest, AlgorithmInfo) {
    EXPECT_EQ(detector->getAlgorithmName(), "RANSAC Sphere Detector");
    
    auto supportedTypes = detector->getSupportedTargetTypes();
    EXPECT_EQ(supportedTypes.size(), 1);
    EXPECT_EQ(supportedTypes[0], "Sphere");
}

// Test default parameters
TEST_F(SphereDetectorTest, DefaultParameters) {
    auto defaultParams = detector->getDefaultParameters();
    
    EXPECT_GT(defaultParams.distanceThreshold, 0.0f);
    EXPECT_GT(defaultParams.maxIterations, 0);
    EXPECT_GE(defaultParams.minQuality, 0.0f);
    EXPECT_LE(defaultParams.minQuality, 1.0f);
    EXPECT_GT(defaultParams.minRadius, 0.0f);
    EXPECT_GT(defaultParams.maxRadius, defaultParams.minRadius);
    EXPECT_GT(defaultParams.minInliers, 0);
}

// Test point count handling
TEST_F(SphereDetectorTest, PointCountHandling) {
    // Test with reasonable point count
    EXPECT_TRUE(detector->canHandlePointCount(10000));
    
    // Test with very large point count
    EXPECT_TRUE(detector->canHandlePointCount(1000000));
    
    // Test with extremely large point count (should still handle but might be slow)
    EXPECT_TRUE(detector->canHandlePointCount(50000000));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    
    // Seed random number generator for reproducible tests
    srand(42);
    
    return RUN_ALL_TESTS();
}
