#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/registration/PoseGraph.h"
#include "../src/registration/PoseGraphBuilder.h"
#include "../src/optimization/BundleAdjustment.h"
#include "../src/features/FeatureExtractor.h"
#include "../src/registration/FeatureBasedRegistration.h"
#include "../src/analysis/DifferenceAnalysis.h"

#include <QMatrix4x4>
#include <QVector3D>
#include <memory>

using namespace Registration;
using namespace Optimization;
using namespace Features;
using namespace Analysis;

class Sprint9RegistrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test pose graph
        testGraph = std::make_unique<PoseGraph>();
        
        // Add test nodes
        QMatrix4x4 identity;
        identity.setToIdentity();
        
        QMatrix4x4 transform1;
        transform1.setToIdentity();
        transform1.translate(1.0f, 0.0f, 0.0f);
        
        QMatrix4x4 transform2;
        transform2.setToIdentity();
        transform2.translate(2.0f, 0.0f, 0.0f);
        
        node1 = testGraph->addNode("scan1", identity);
        node2 = testGraph->addNode("scan2", transform1);
        node3 = testGraph->addNode("scan3", transform2);
        
        // Add test edges
        QMatrix4x4 relativeTransform;
        relativeTransform.setToIdentity();
        relativeTransform.translate(1.0f, 0.0f, 0.0f);
        
        testGraph->addEdge(node1, node2, relativeTransform, 0.01f);
        testGraph->addEdge(node2, node3, relativeTransform, 0.02f);
        
        // Create test point clouds
        createTestPointClouds();
    }
    
    void createTestPointClouds() {
        // Create a simple plane in XY at Z=0
        sourcePoints.clear();
        targetPoints.clear();
        
        for (int x = -5; x <= 5; ++x) {
            for (int y = -5; y <= 5; ++y) {
                Point3D point;
                point.x = static_cast<float>(x);
                point.y = static_cast<float>(y);
                point.z = 0.0f;
                point.intensity = 100;
                point.hasIntensity = true;
                
                sourcePoints.push_back(point);
                
                // Target points are slightly translated
                Point3D targetPoint = point;
                targetPoint.x += 0.1f;
                targetPoint.y += 0.05f;
                targetPoints.push_back(targetPoint);
            }
        }
    }
    
    std::unique_ptr<PoseGraph> testGraph;
    int node1, node2, node3;
    std::vector<Point3D> sourcePoints;
    std::vector<Point3D> targetPoints;
};

// PoseGraph Tests
TEST_F(Sprint9RegistrationTest, PoseGraphBasicOperations) {
    EXPECT_EQ(testGraph->nodeCount(), 3);
    EXPECT_EQ(testGraph->edgeCount(), 2);
    EXPECT_FALSE(testGraph->isEmpty());
    EXPECT_TRUE(testGraph->isValid());
    
    // Test node retrieval
    const PoseNode* node = testGraph->getNode(node1);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->scanId, "scan1");
    EXPECT_EQ(node->nodeIndex, node1);
    
    // Test node finding by scan ID
    int foundIndex = testGraph->findNodeByScanId("scan2");
    EXPECT_EQ(foundIndex, node2);
    
    // Test edge retrieval
    auto edgesFromNode1 = testGraph->getEdgesFromNode(node1);
    EXPECT_EQ(edgesFromNode1.size(), 1);
    EXPECT_EQ(edgesFromNode1[0].toNodeIndex, node2);
}

TEST_F(Sprint9RegistrationTest, PoseGraphEdgeManagement) {
    // Test edge addition
    QMatrix4x4 newTransform;
    newTransform.setToIdentity();
    newTransform.translate(0.0f, 1.0f, 0.0f);
    
    bool added = testGraph->addEdge(node1, node3, newTransform, 0.03f);
    EXPECT_TRUE(added);
    EXPECT_EQ(testGraph->edgeCount(), 3);
    
    // Test loop closure detection
    EXPECT_TRUE(testGraph->hasLoopClosures());
    
    // Test edge removal
    bool removed = testGraph->removeEdge(node1, node3);
    EXPECT_TRUE(removed);
    EXPECT_EQ(testGraph->edgeCount(), 2);
}

TEST_F(Sprint9RegistrationTest, PoseGraphNodeRemoval) {
    int initialEdgeCount = testGraph->edgeCount();
    
    // Remove middle node - should remove connected edges
    bool removed = testGraph->removeNode(node2);
    EXPECT_TRUE(removed);
    EXPECT_EQ(testGraph->nodeCount(), 2);
    EXPECT_LT(testGraph->edgeCount(), initialEdgeCount);
    
    // Verify node is gone
    const PoseNode* node = testGraph->getNode(node2);
    EXPECT_EQ(node, nullptr);
}

// PoseGraphBuilder Tests
TEST_F(Sprint9RegistrationTest, PoseGraphBuilderFromScans) {
    PoseGraphBuilder builder;
    QStringList scanIds = {"scan1", "scan2", "scan3", "scan4"};
    
    auto graph = builder.buildFromScans(scanIds);
    
    ASSERT_NE(graph, nullptr);
    EXPECT_EQ(graph->nodeCount(), 4);
    EXPECT_EQ(graph->edgeCount(), 0); // No edges in basic construction
    
    // Verify all scans are present
    for (const QString& scanId : scanIds) {
        int nodeIndex = graph->findNodeByScanId(scanId);
        EXPECT_GE(nodeIndex, 0);
    }
}

TEST_F(Sprint9RegistrationTest, PoseGraphBuilderValidation) {
    PoseGraphBuilder builder;
    
    auto result = builder.validateGraph(*testGraph);
    
    EXPECT_TRUE(result.isValid);
    EXPECT_EQ(result.connectedComponents, 1);
    EXPECT_TRUE(result.isolatedScans.isEmpty());
    EXPECT_EQ(result.errorMessage, "");
}

// BundleAdjustment Tests
TEST_F(Sprint9RegistrationTest, BundleAdjustmentParameters) {
    BundleAdjustment optimizer;
    
    auto params = optimizer.getRecommendedParameters(*testGraph);
    
    EXPECT_GT(params.maxIterations, 0);
    EXPECT_GT(params.convergenceThreshold, 0.0);
    EXPECT_LT(params.convergenceThreshold, 1.0);
    EXPECT_GT(params.initialLambda, 0.0);
}

TEST_F(Sprint9RegistrationTest, BundleAdjustmentBasicOptimization) {
    BundleAdjustment optimizer;
    BundleAdjustment::Parameters params;
    params.maxIterations = 10; // Small number for testing
    params.verbose = false;
    
    auto [optimizedGraph, result] = optimizer.optimize(*testGraph, params);
    
    ASSERT_NE(optimizedGraph, nullptr);
    EXPECT_EQ(optimizedGraph->nodeCount(), testGraph->nodeCount());
    EXPECT_EQ(optimizedGraph->edgeCount(), testGraph->edgeCount());
    
    // Should have attempted optimization
    EXPECT_GT(result.iterations, 0);
    EXPECT_GE(result.finalError, 0.0);
}

// FeatureExtractor Tests
TEST_F(Sprint9RegistrationTest, FeatureExtractorPlaneDetection) {
    FeatureExtractor extractor;
    FeatureExtractor::PlaneExtractionParams params;
    params.maxIterations = 100;
    params.minInliers = 10;
    params.maxPlanes = 5;
    
    auto planes = extractor.extractPlanes(sourcePoints, params);
    
    // Should find at least one plane (the XY plane we created)
    EXPECT_GE(planes.size(), 1);
    
    if (!planes.isEmpty()) {
        const Plane& plane = planes[0];
        EXPECT_GT(plane.inlierIndices.size(), 10);
        EXPECT_GT(plane.confidence, 0.0f);
        
        // The plane normal should be approximately (0, 0, 1) for XY plane
        EXPECT_NEAR(std::abs(plane.normal.z()), 1.0f, 0.1f);
    }
}

TEST_F(Sprint9RegistrationTest, FeatureExtractorPlaneValidation) {
    FeatureExtractor extractor;
    
    // Create a perfect plane
    Plane testPlane;
    testPlane.normal = QVector3D(0, 0, 1);
    testPlane.distance = 0.0f;
    testPlane.centroid = QVector3D(0, 0, 0);
    
    // Add all source points as inliers
    for (int i = 0; i < static_cast<int>(sourcePoints.size()); ++i) {
        testPlane.inlierIndices.append(i);
    }
    
    float quality = extractor.validatePlaneQuality(testPlane, sourcePoints);
    EXPECT_GT(quality, 0.8f); // Should be high quality for perfect plane
}

// FeatureBasedRegistration Tests
TEST_F(Sprint9RegistrationTest, FeatureBasedRegistrationParameters) {
    FeatureBasedRegistration registration;
    
    auto params = registration.getRecommendedParameters(sourcePoints, targetPoints);
    
    EXPECT_GT(params.maxAngleDifference, 0.0f);
    EXPECT_GT(params.maxDistanceDifference, 0.0f);
    EXPECT_GT(params.minCorrespondences, 0);
}

TEST_F(Sprint9RegistrationTest, FeatureBasedRegistrationBasic) {
    FeatureBasedRegistration registration;
    FeatureBasedRegistration::Parameters params;
    params.validateResult = false; // Skip validation for basic test
    
    auto result = registration.registerPointClouds(sourcePoints, targetPoints, params);
    
    // May not succeed due to insufficient features, but should not crash
    EXPECT_FALSE(result.errorMessage.isEmpty() && !result.success);
}

// DifferenceAnalysis Tests
TEST_F(Sprint9RegistrationTest, DifferenceAnalysisBasicCalculation) {
    DifferenceAnalysis analyzer;
    DifferenceAnalysis::Parameters params;
    params.maxSearchDistance = 1.0f;
    params.useKDTree = false; // Use brute force for testing
    
    auto distances = analyzer.calculateDistances(sourcePoints, targetPoints, QMatrix4x4(), params);
    
    EXPECT_EQ(distances.size(), sourcePoints.size());
    
    // All distances should be reasonable (points are close)
    for (float distance : distances) {
        EXPECT_GE(distance, 0.0f);
        EXPECT_LT(distance, 1.0f);
    }
}

TEST_F(Sprint9RegistrationTest, DifferenceAnalysisStatistics) {
    DifferenceAnalysis analyzer;
    
    // Create simple distance vector
    QVector<float> distances = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f};
    
    auto stats = analyzer.calculateStatistics(distances);
    
    EXPECT_EQ(stats.totalPoints, 5);
    EXPECT_EQ(stats.validDistances, 5);
    EXPECT_FLOAT_EQ(stats.meanDistance, 0.3f);
    EXPECT_FLOAT_EQ(stats.minDistance, 0.1f);
    EXPECT_FLOAT_EQ(stats.maxDistance, 0.5f);
    EXPECT_FLOAT_EQ(stats.medianDistance, 0.3f);
}

TEST_F(Sprint9RegistrationTest, DifferenceAnalysisColorMapping) {
    DifferenceAnalysis analyzer;
    
    QVector<float> distances = {0.0f, 0.5f, 1.0f, 1.5f, 2.0f};
    
    auto colorValues = analyzer.generateColorMapValues(distances, 2.0f);
    
    EXPECT_EQ(colorValues.size(), distances.size());
    EXPECT_FLOAT_EQ(colorValues[0], 0.0f);  // Min value
    EXPECT_FLOAT_EQ(colorValues[4], 1.0f);  // Max value
    EXPECT_FLOAT_EQ(colorValues[2], 0.5f);  // Middle value
}

TEST_F(Sprint9RegistrationTest, DifferenceAnalysisQualityAssessment) {
    DifferenceAnalysis analyzer;
    
    // Create statistics for good registration
    DifferenceAnalysis::Statistics goodStats;
    goodStats.totalPoints = 1000;
    goodStats.validDistances = 1000;
    goodStats.meanDistance = 0.01f;
    goodStats.rmsDistance = 0.015f;
    goodStats.outlierPercentage = 1.0f;
    goodStats.percentile95 = 0.02f;
    
    float goodQuality = analyzer.assessRegistrationQuality(goodStats);
    EXPECT_GT(goodQuality, 0.7f);
    
    // Create statistics for poor registration
    DifferenceAnalysis::Statistics poorStats;
    poorStats.totalPoints = 1000;
    poorStats.validDistances = 500;
    poorStats.meanDistance = 0.5f;
    poorStats.rmsDistance = 0.8f;
    poorStats.outlierPercentage = 50.0f;
    poorStats.percentile95 = 1.0f;
    
    float poorQuality = analyzer.assessRegistrationQuality(poorStats);
    EXPECT_LT(poorQuality, 0.3f);
}

// Integration Tests
TEST_F(Sprint9RegistrationTest, EndToEndWorkflow) {
    // Test the complete workflow: PoseGraph -> BundleAdjustment -> Analysis
    
    // 1. Build pose graph
    PoseGraphBuilder builder;
    auto result = builder.validateGraph(*testGraph);
    EXPECT_TRUE(result.isValid);
    
    // 2. Optimize with bundle adjustment
    BundleAdjustment optimizer;
    BundleAdjustment::Parameters optParams;
    optParams.maxIterations = 5; // Quick test
    
    auto [optimizedGraph, optResult] = optimizer.optimize(*testGraph, optParams);
    ASSERT_NE(optimizedGraph, nullptr);
    
    // 3. Analyze registration quality
    DifferenceAnalysis analyzer;
    auto distances = analyzer.calculateDistances(sourcePoints, targetPoints);
    auto stats = analyzer.calculateStatistics(distances);
    
    EXPECT_GT(stats.validDistances, 0);
    EXPECT_GE(stats.meanDistance, 0.0f);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
