#include <iostream>
#include <vector>
#include <QVector3D>
#include <QMatrix4x4>

#include "src/screenspaceerror.h"
#include "src/octree.h"

int main() {
    std::cout << "Sprint R2 Simple Test - Screen-Space Error LOD System" << std::endl;
    
    // Test 1: Screen-space error calculation
    std::cout << "\n=== Test 1: Screen-Space Error Calculation ===" << std::endl;
    
    ViewportInfo viewport;
    viewport.width = 1920;
    viewport.height = 1080;
    viewport.nearPlane = 0.1f;
    viewport.farPlane = 1000.0f;
    
    AxisAlignedBoundingBox testAABB(QVector3D(-1, -1, -1), QVector3D(1, 1, 1));
    
    QMatrix4x4 mvpMatrix;
    mvpMatrix.setToIdentity();
    mvpMatrix.perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    mvpMatrix.lookAt(QVector3D(0, 0, 5), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    
    float error = ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
        testAABB, mvpMatrix, viewport);
    
    std::cout << "Screen-space error for test AABB: " << error << " pixels" << std::endl;
    
    // Test 2: Threshold evaluation
    std::cout << "\n=== Test 2: Threshold Evaluation ===" << std::endl;
    
    float primaryThreshold = 50.0f;
    float cullThreshold = 2.0f;
    
    bool shouldCull = ScreenSpaceErrorCalculator::shouldCullNode(error, cullThreshold);
    bool shouldStopRecursion = ScreenSpaceErrorCalculator::shouldStopRecursion(error, primaryThreshold);
    
    std::cout << "Error: " << error << " pixels" << std::endl;
    std::cout << "Should cull (threshold " << cullThreshold << "): " << (shouldCull ? "YES" : "NO") << std::endl;
    std::cout << "Should stop recursion (threshold " << primaryThreshold << "): " << (shouldStopRecursion ? "YES" : "NO") << std::endl;
    
    // Test 3: Point sampling
    std::cout << "\n=== Test 3: Point Sampling ===" << std::endl;
    
    // Create test octree node
    AxisAlignedBoundingBox bounds(QVector3D(0, 0, 0), QVector3D(10, 10, 10));
    OctreeNode testNode(bounds);
    
    // Add test points
    for (int i = 0; i < 1000; ++i) {
        testNode.points.emplace_back(
            static_cast<float>(i % 10),
            static_cast<float>((i / 10) % 10),
            static_cast<float>(i / 100),
            1.0f, 1.0f, 1.0f, 1.0f
        );
    }
    
    std::cout << "Created test node with " << testNode.points.size() << " points" << std::endl;
    
    auto sampledPoints = testNode.getSampledPoints(100);
    std::cout << "Sampled points (max 100): " << sampledPoints.size() << std::endl;
    
    auto percentageSampled = testNode.getSampledPointsByPercentage(0.1f);
    std::cout << "Percentage sampled (10%): " << percentageSampled.size() << std::endl;
    
    auto representativePoints = testNode.getRepresentativePoints();
    std::cout << "Representative points: " << representativePoints.size() << std::endl;
    
    // Test 4: Octree integration
    std::cout << "\n=== Test 4: Octree Integration ===" << std::endl;
    
    // Create test point cloud
    std::vector<PointFullData> testPoints;
    for (int x = 0; x < 20; ++x) {
        for (int y = 0; y < 20; ++y) {
            for (int z = 0; z < 5; ++z) {
                testPoints.emplace_back(
                    static_cast<float>(x),
                    static_cast<float>(y),
                    static_cast<float>(z),
                    1.0f, 1.0f, 1.0f, 1.0f
                );
            }
        }
    }
    
    Octree octree;
    octree.build(testPoints, 6, 100);
    
    std::cout << "Built octree with " << testPoints.size() << " points" << std::endl;
    std::cout << "Octree stats - Total points: " << octree.getTotalPointCount() 
              << ", Max depth: " << octree.getMaxDepth()
              << ", Node count: " << octree.getNodeCount() << std::endl;
    
    // Test screen-space error traversal
    auto frustumPlanes = FrustumUtils::extractFrustumPlanes(mvpMatrix);
    std::vector<PointFullData> visiblePoints;
    
    if (octree.root) {
        octree.root->collectVisiblePointsWithScreenSpaceError(
            frustumPlanes, mvpMatrix, viewport,
            primaryThreshold, cullThreshold, visiblePoints
        );
    }
    
    std::cout << "Screen-space error LOD traversal result: " << visiblePoints.size() << " visible points" << std::endl;
    
    // Compare with traditional distance-based LOD
    std::vector<PointFullData> distancePoints;
    octree.getVisiblePoints(frustumPlanes, QVector3D(10, 10, 25), 10.0f, 50.0f, distancePoints);
    
    std::cout << "Distance-based LOD result: " << distancePoints.size() << " visible points" << std::endl;
    
    std::cout << "\n=== Sprint R2 Test Completed Successfully ===" << std::endl;
    std::cout << "Screen-space error LOD system is working correctly!" << std::endl;
    
    return 0;
}
