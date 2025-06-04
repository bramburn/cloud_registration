#include <gtest/gtest.h>
#include <QApplication>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QVector3D>
#include <QMatrix4x4>
#include <chrono>
#include <random>

#include "../src/octree.h"
#include "../src/pointcloudviewerwidget.h"

class OctreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test point cloud - a simple cube
        testPoints.clear();
        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 10; y++) {
                for (int z = 0; z < 10; z++) {
                    testPoints.emplace_back(
                        static_cast<float>(x),
                        static_cast<float>(y),
                        static_cast<float>(z)
                    );
                }
            }
        }
        
        // Create flat array for compatibility testing
        testPointsFlat.clear();
        for (const auto& point : testPoints) {
            testPointsFlat.push_back(point.x);
            testPointsFlat.push_back(point.y);
            testPointsFlat.push_back(point.z);
        }
    }

    std::vector<PointFullData> testPoints;
    std::vector<float> testPointsFlat;
    Octree octree;
};

TEST_F(OctreeTest, OctreeConstruction) {
    octree.build(testPoints, 4, 50);
    
    ASSERT_NE(octree.root, nullptr);
    EXPECT_EQ(testPoints.size(), 1000);
    EXPECT_EQ(octree.getTotalPointCount(), 1000);
    EXPECT_GT(octree.getMaxDepth(), 0);
    EXPECT_GT(octree.getNodeCount(), 1);
}

TEST_F(OctreeTest, OctreeFromFloatArray) {
    octree.buildFromFloatArray(testPointsFlat, 4, 50);
    
    ASSERT_NE(octree.root, nullptr);
    EXPECT_EQ(octree.getTotalPointCount(), 1000);
    EXPECT_GT(octree.getMaxDepth(), 0);
}

TEST_F(OctreeTest, OctreeBuildPerformance) {
    // Generate larger dataset
    std::vector<PointFullData> largeDataset;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1000.0f);
    
    for (int i = 0; i < 100000; i++) {
        largeDataset.emplace_back(dis(gen), dis(gen), dis(gen));
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    octree.build(largeDataset, 8, 100);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_LT(duration.count(), 5000); // Should complete within 5 seconds
    EXPECT_EQ(octree.getTotalPointCount(), 100000);
}

TEST_F(OctreeTest, FrustumCulling) {
    octree.build(testPoints, 4, 50);
    
    // Create a frustum that should cull most points
    std::array<QVector4D, 6> frustumPlanes;
    // Set up frustum planes for a narrow view (only include points x > 5)
    frustumPlanes[0] = QVector4D(1, 0, 0, -5);   // Left: x > 5
    frustumPlanes[1] = QVector4D(-1, 0, 0, 6);   // Right: x < 6
    frustumPlanes[2] = QVector4D(0, 1, 0, -5);   // Bottom: y > 5
    frustumPlanes[3] = QVector4D(0, -1, 0, 6);   // Top: y < 6
    frustumPlanes[4] = QVector4D(0, 0, 1, -5);   // Near: z > 5
    frustumPlanes[5] = QVector4D(0, 0, -1, 6);   // Far: z < 6
    
    std::vector<PointFullData> visiblePoints;
    octree.getVisiblePoints(frustumPlanes, QVector3D(0, 0, 0), 100, 200, visiblePoints);
    
    // Should have significantly fewer points than the total
    EXPECT_LT(visiblePoints.size(), testPoints.size());
    EXPECT_GT(visiblePoints.size(), 0);
    
    // Verify that visible points are actually within the expected range
    for (const auto& point : visiblePoints) {
        EXPECT_GE(point.x, 5.0f);
        EXPECT_LE(point.x, 6.0f);
        EXPECT_GE(point.y, 5.0f);
        EXPECT_LE(point.y, 6.0f);
        EXPECT_GE(point.z, 5.0f);
        EXPECT_LE(point.z, 6.0f);
    }
}

TEST_F(OctreeTest, LODDistanceCulling) {
    octree.build(testPoints, 4, 50);
    
    // Create frustum that includes all points
    std::array<QVector4D, 6> frustumPlanes;
    for (auto& plane : frustumPlanes) {
        plane = QVector4D(0, 0, 0, 1000); // Very permissive planes
    }
    
    std::vector<PointFullData> closePoints, farPoints;
    
    // Test close camera (should get more points)
    octree.getVisiblePoints(frustumPlanes, QVector3D(5, 5, 5), 10, 20, closePoints);
    
    // Test far camera (should get fewer points due to LOD)
    octree.getVisiblePoints(frustumPlanes, QVector3D(100, 100, 100), 10, 20, farPoints);
    
    EXPECT_GE(closePoints.size(), farPoints.size());
}

TEST_F(OctreeTest, FrustumUtilities) {
    // Test frustum plane extraction
    QMatrix4x4 viewProjection;
    viewProjection.setToIdentity();
    viewProjection.perspective(45.0f, 1.0f, 0.1f, 1000.0f);
    
    auto planes = FrustumUtils::extractFrustumPlanes(viewProjection);
    EXPECT_EQ(planes.size(), 6);
    
    // Test point in frustum
    QVector3D pointInside(0, 0, -1);
    QVector3D pointOutside(0, 0, 1001);
    
    EXPECT_TRUE(FrustumUtils::pointInFrustum(pointInside, planes));
    EXPECT_FALSE(FrustumUtils::pointInFrustum(pointOutside, planes));
    
    // Test AABB in frustum
    AxisAlignedBoundingBox aabbInside(QVector3D(-1, -1, -2), QVector3D(1, 1, -0.5f));
    AxisAlignedBoundingBox aabbOutside(QVector3D(1000, 1000, 1000), QVector3D(1001, 1001, 1001));
    
    EXPECT_TRUE(FrustumUtils::aabbInFrustum(aabbInside, planes));
    EXPECT_FALSE(FrustumUtils::aabbInFrustum(aabbOutside, planes));
}

TEST_F(OctreeTest, AxisAlignedBoundingBox) {
    AxisAlignedBoundingBox aabb(QVector3D(0, 0, 0), QVector3D(10, 10, 10));
    
    // Test contains
    EXPECT_TRUE(aabb.contains(5, 5, 5));
    EXPECT_FALSE(aabb.contains(15, 5, 5));
    
    // Test center
    QVector3D center = aabb.center();
    EXPECT_EQ(center, QVector3D(5, 5, 5));
    
    // Test distance to point
    float distance = aabb.distanceToPoint(QVector3D(15, 5, 5));
    EXPECT_FLOAT_EQ(distance, 5.0f);
    
    distance = aabb.distanceToPoint(QVector3D(5, 5, 5)); // Point inside
    EXPECT_FLOAT_EQ(distance, 0.0f);
}

// Integration test with mock OpenGL context
class PointCloudViewerLODTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create QApplication if it doesn't exist
        if (!QApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = new QApplication(argc, argv);
        }
        
        // Create OpenGL context for testing
        context = new QOpenGLContext();
        surface = new QOffscreenSurface();
        
        if (context->create() && surface->create()) {
            context->makeCurrent(surface);
            hasOpenGL = true;
        } else {
            hasOpenGL = false;
        }
    }
    
    void TearDown() override {
        if (hasOpenGL && context) {
            context->doneCurrent();
        }
        delete context;
        delete surface;
    }
    
    QApplication* app = nullptr;
    QOpenGLContext* context = nullptr;
    QOffscreenSurface* surface = nullptr;
    bool hasOpenGL = false;
};

TEST_F(PointCloudViewerLODTest, LODSystemIntegration) {
    if (!hasOpenGL) {
        GTEST_SKIP() << "OpenGL context not available";
    }
    
    PointCloudViewerWidget viewer;
    
    // Test initial state
    EXPECT_FALSE(viewer.isLODEnabled());
    EXPECT_EQ(viewer.getVisiblePointCount(), 0);
    EXPECT_EQ(viewer.getOctreeNodeCount(), 0);
    
    // Enable LOD
    viewer.setLODEnabled(true);
    EXPECT_TRUE(viewer.isLODEnabled());
    
    // Test LOD distance settings
    viewer.setLODDistances(25.0f, 100.0f);
    float dist1, dist2;
    viewer.getLODDistances(dist1, dist2);
    EXPECT_FLOAT_EQ(dist1, 25.0f);
    EXPECT_FLOAT_EQ(dist2, 100.0f);
    
    // Create test point cloud
    std::vector<float> testPoints;
    for (int i = 0; i < 1000; i++) {
        testPoints.push_back(static_cast<float>(i % 10));
        testPoints.push_back(static_cast<float>((i / 10) % 10));
        testPoints.push_back(static_cast<float>(i / 100));
    }
    
    // Load point cloud
    viewer.loadPointCloud(testPoints);
    
    // Verify octree was built
    EXPECT_GT(viewer.getOctreeNodeCount(), 0);
    EXPECT_EQ(viewer.getPointCount(), 1000);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
