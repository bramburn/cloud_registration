#include <gtest/gtest.h>
#include <QApplication>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QVector3D>
#include <QMatrix4x4>
#include <chrono>
#include <random>

#include "../src/screenspaceerror.h"
#include "../src/octree.h"
#include "../src/pointcloudviewerwidget.h"

class ScreenSpaceErrorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test viewport
        viewport.width = 1920;
        viewport.height = 1080;
        viewport.nearPlane = 0.1f;
        viewport.farPlane = 1000.0f;
        
        // Setup test AABB
        testAABB = AxisAlignedBoundingBox(
            QVector3D(-1, -1, -1), 
            QVector3D(1, 1, 1)
        );
        
        // Setup test MVP matrix (identity for simplicity)
        mvpMatrix.setToIdentity();
        mvpMatrix.perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
        mvpMatrix.lookAt(QVector3D(0, 0, 5), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    }
    
    ViewportInfo viewport;
    AxisAlignedBoundingBox testAABB;
    QMatrix4x4 mvpMatrix;
};

TEST_F(ScreenSpaceErrorTest, CalculateScreenSpaceErrorBasic) {
    float error = ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
        testAABB, mvpMatrix, viewport);
    
    EXPECT_GT(error, 0.0f);
    EXPECT_LT(error, viewport.width); // Should be reasonable
}

TEST_F(ScreenSpaceErrorTest, DistantObjectHasSmallerError) {
    // Close AABB
    QMatrix4x4 closeMVP;
    closeMVP.setToIdentity();
    closeMVP.perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    closeMVP.lookAt(QVector3D(0, 0, 3), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    
    // Distant AABB
    QMatrix4x4 distantMVP;
    distantMVP.setToIdentity();
    distantMVP.perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    distantMVP.lookAt(QVector3D(0, 0, 20), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    
    float closeError = ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
        testAABB, closeMVP, viewport);
    float distantError = ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
        testAABB, distantMVP, viewport);
    
    EXPECT_GT(closeError, distantError);
}

TEST_F(ScreenSpaceErrorTest, CullingThresholds) {
    float error = 10.0f;
    
    EXPECT_TRUE(ScreenSpaceErrorCalculator::shouldCullNode(error, 15.0f));
    EXPECT_FALSE(ScreenSpaceErrorCalculator::shouldCullNode(error, 5.0f));
    
    EXPECT_TRUE(ScreenSpaceErrorCalculator::shouldStopRecursion(error, 15.0f));
    EXPECT_FALSE(ScreenSpaceErrorCalculator::shouldStopRecursion(error, 5.0f));
}

class RefinedPointSelectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test octree node with known points
        AxisAlignedBoundingBox bounds(QVector3D(0, 0, 0), QVector3D(10, 10, 10));
        testNode = std::make_unique<OctreeNode>(bounds);
        
        // Add test points
        for (int i = 0; i < 1000; ++i) {
            testNode->points.emplace_back(
                static_cast<float>(i % 10),
                static_cast<float>((i / 10) % 10),
                static_cast<float>(i / 100),
                1.0f, 1.0f, 1.0f, 1.0f
            );
        }
    }
    
    std::unique_ptr<OctreeNode> testNode;
};

TEST_F(RefinedPointSelectionTest, SampledPointsRespectMaxCount) {
    auto sampledPoints = testNode->getSampledPoints(100);
    
    EXPECT_EQ(sampledPoints.size(), 100);
}

TEST_F(RefinedPointSelectionTest, SampledPointsByPercentage) {
    auto sampledPoints = testNode->getSampledPointsByPercentage(0.1f); // 10%
    
    EXPECT_EQ(sampledPoints.size(), 100); // 10% of 1000
}

TEST_F(RefinedPointSelectionTest, RepresentativePointsConsistent) {
    auto rep1 = testNode->getRepresentativePoints();
    auto rep2 = testNode->getRepresentativePoints();
    
    EXPECT_EQ(rep1.size(), rep2.size());
    // Should be identical due to caching
    for (size_t i = 0; i < rep1.size(); ++i) {
        EXPECT_FLOAT_EQ(rep1[i].x, rep2[i].x);
        EXPECT_FLOAT_EQ(rep1[i].y, rep2[i].y);
        EXPECT_FLOAT_EQ(rep1[i].z, rep2[i].z);
    }
}

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test point cloud - a 50x50x10 grid
        testPoints.clear();
        for (int x = 0; x < 50; ++x) {
            for (int y = 0; y < 50; ++y) {
                for (int z = 0; z < 10; ++z) {
                    testPoints.emplace_back(
                        static_cast<float>(x),
                        static_cast<float>(y),
                        static_cast<float>(z),
                        1.0f, 1.0f, 1.0f, 1.0f
                    );
                }
            }
        }
        
        octree.build(testPoints, 6, 100);
        
        // Setup viewport and matrices
        viewport.width = 1920;
        viewport.height = 1080;
        viewport.nearPlane = 0.1f;
        viewport.farPlane = 1000.0f;
        
        mvpMatrix.setToIdentity();
        mvpMatrix.perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
        mvpMatrix.lookAt(QVector3D(25, 25, 50), QVector3D(25, 25, 5), QVector3D(0, 1, 0));
        
        // Setup frustum
        frustumPlanes = FrustumUtils::extractFrustumPlanes(mvpMatrix);
    }
    
    std::vector<PointFullData> testPoints;
    Octree octree;
    ViewportInfo viewport;
    QMatrix4x4 mvpMatrix;
    std::array<QVector4D, 6> frustumPlanes;
};

TEST_F(IntegrationTest, ScreenSpaceErrorLODReducesPoints) {
    std::vector<PointFullData> visiblePoints;
    
    // High threshold - should get fewer points
    octree.root->collectVisiblePointsWithScreenSpaceError(
        frustumPlanes, mvpMatrix, viewport,
        100.0f, 5.0f, visiblePoints);
    
    size_t highThresholdCount = visiblePoints.size();
    visiblePoints.clear();
    
    // Low threshold - should get more points
    octree.root->collectVisiblePointsWithScreenSpaceError(
        frustumPlanes, mvpMatrix, viewport,
        10.0f, 1.0f, visiblePoints);
    
    size_t lowThresholdCount = visiblePoints.size();
    
    EXPECT_LT(highThresholdCount, lowThresholdCount);
    EXPECT_GT(highThresholdCount, 0);
}

TEST_F(IntegrationTest, ScreenSpaceErrorVsDistanceLOD) {
    std::vector<PointFullData> screenSpacePoints, distancePoints;
    
    // Screen-space error LOD
    octree.root->collectVisiblePointsWithScreenSpaceError(
        frustumPlanes, mvpMatrix, viewport,
        50.0f, 2.0f, screenSpacePoints);
    
    // Distance-based LOD
    octree.getVisiblePoints(frustumPlanes, QVector3D(25, 25, 50),
                           25.0f, 100.0f, distancePoints);
    
    // Both should return reasonable point counts
    EXPECT_GT(screenSpacePoints.size(), 0);
    EXPECT_GT(distancePoints.size(), 0);
    
    // Screen-space error should generally be more efficient
    // (This is a heuristic test - actual results may vary)
    qDebug() << "Screen-space LOD points:" << screenSpacePoints.size()
             << "Distance LOD points:" << distancePoints.size();
}

// Integration test with mock OpenGL context
class PointCloudViewerLODR2Test : public ::testing::Test {
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

TEST_F(PointCloudViewerLODR2Test, ScreenSpaceErrorThresholdControl) {
    if (!hasOpenGL) {
        GTEST_SKIP() << "OpenGL context not available";
    }
    
    PointCloudViewerWidget viewer;
    
    // Test initial state
    EXPECT_FALSE(viewer.isLODEnabled());
    
    // Enable LOD
    viewer.setLODEnabled(true);
    EXPECT_TRUE(viewer.isLODEnabled());
    
    // Test screen-space error threshold settings
    viewer.setPrimaryScreenSpaceErrorThreshold(75.0f);
    viewer.setCullScreenSpaceErrorThreshold(3.0f);
    
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
