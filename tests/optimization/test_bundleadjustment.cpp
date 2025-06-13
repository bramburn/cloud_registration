#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QSignalSpy>
#include <QMatrix4x4>
#include <QVector3D>

#include "optimization/BundleAdjustment.h"
#include "registration/PoseGraph.h"

using namespace Optimization;
using namespace Registration;

class BundleAdjustmentTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        bundleAdjustment = std::make_unique<BundleAdjustment>();
    }

    void TearDown() override
    {
        bundleAdjustment.reset();
    }

    // Helper method to create a simple test pose graph
    std::unique_ptr<PoseGraph> createTestPoseGraph()
    {
        auto graph = std::make_unique<PoseGraph>();
        
        // Add 3 nodes in a triangle configuration
        QMatrix4x4 transform1;
        transform1.setToIdentity();
        int node1 = graph->addNode("scan1", transform1);
        
        QMatrix4x4 transform2;
        transform2.setToIdentity();
        transform2.translate(1.0f, 0.0f, 0.0f);
        int node2 = graph->addNode("scan2", transform2);
        
        QMatrix4x4 transform3;
        transform3.setToIdentity();
        transform3.translate(0.5f, 1.0f, 0.0f);
        int node3 = graph->addNode("scan3", transform3);
        
        // Add edges with some noise to create optimization opportunity
        QMatrix4x4 edge12;
        edge12.setToIdentity();
        edge12.translate(1.01f, 0.01f, 0.0f); // Slightly noisy
        graph->addEdge(node1, node2, edge12, 0.01f);
        
        QMatrix4x4 edge23;
        edge23.setToIdentity();
        edge23.translate(-0.49f, 1.01f, 0.0f); // Slightly noisy
        graph->addEdge(node2, node3, edge23, 0.01f);
        
        QMatrix4x4 edge31;
        edge31.setToIdentity();
        edge31.translate(-0.51f, -0.99f, 0.0f); // Slightly noisy
        graph->addEdge(node3, node1, edge31, 0.01f);
        
        return graph;
    }

    std::unique_ptr<BundleAdjustment> bundleAdjustment;
};

TEST_F(BundleAdjustmentTest, ConstructorInitialization)
{
    EXPECT_NE(bundleAdjustment, nullptr);
}

TEST_F(BundleAdjustmentTest, OptimizeEmptyGraph)
{
    PoseGraph emptyGraph;
    BundleAdjustment::Parameters params;
    
    auto [optimizedGraph, result] = bundleAdjustment->optimize(emptyGraph, params);
    
    EXPECT_FALSE(result.converged);
    EXPECT_EQ(result.iterations, 0);
    EXPECT_TRUE(result.statusMessage.contains("Empty"));
}

TEST_F(BundleAdjustmentTest, OptimizeValidGraph)
{
    auto testGraph = createTestPoseGraph();
    BundleAdjustment::Parameters params;
    params.maxIterations = 10;
    params.convergenceThreshold = 1e-6;
    params.verbose = true;
    
    // Set up signal spy to monitor progress
    QSignalSpy progressSpy(bundleAdjustment.get(), &BundleAdjustment::optimizationProgress);
    QSignalSpy completedSpy(bundleAdjustment.get(), &BundleAdjustment::optimizationCompleted);
    
    auto [optimizedGraph, result] = bundleAdjustment->optimize(*testGraph, params);
    
    // Check that optimization ran
    EXPECT_GT(result.iterations, 0);
    EXPECT_LE(result.iterations, params.maxIterations);
    EXPECT_GE(result.finalError, 0.0);
    EXPECT_GE(result.improvementRatio, 0.0);
    EXPECT_GT(result.optimizationTimeSeconds, 0.0);
    
    // Check that optimized graph is valid
    EXPECT_NE(optimizedGraph, nullptr);
    EXPECT_EQ(optimizedGraph->nodeCount(), testGraph->nodeCount());
    EXPECT_EQ(optimizedGraph->edgeCount(), testGraph->edgeCount());
    
    // Check that signals were emitted
    EXPECT_GT(progressSpy.count(), 0);
    EXPECT_EQ(completedSpy.count(), 1);
}

TEST_F(BundleAdjustmentTest, CancellationSupport)
{
    auto testGraph = createTestPoseGraph();
    BundleAdjustment::Parameters params;
    params.maxIterations = 1000; // Long running
    
    // Start optimization in a separate thread (simulated)
    bundleAdjustment->cancel(); // Cancel immediately
    
    auto [optimizedGraph, result] = bundleAdjustment->optimize(*testGraph, params);
    
    // Should handle cancellation gracefully
    EXPECT_NE(optimizedGraph, nullptr);
}

TEST_F(BundleAdjustmentTest, RecommendedParameters)
{
    auto testGraph = createTestPoseGraph();
    
    auto params = bundleAdjustment->getRecommendedParameters(*testGraph);
    
    EXPECT_GT(params.maxIterations, 0);
    EXPECT_GT(params.convergenceThreshold, 0.0);
    EXPECT_GT(params.initialLambda, 0.0);
    EXPECT_GT(params.lambdaFactor, 1.0);
    EXPECT_GT(params.maxLambda, params.initialLambda);
}

TEST_F(BundleAdjustmentTest, ParameterValidation)
{
    auto testGraph = createTestPoseGraph();
    BundleAdjustment::Parameters params;
    
    // Test with invalid parameters
    params.maxIterations = 0;
    auto [optimizedGraph1, result1] = bundleAdjustment->optimize(*testGraph, params);
    EXPECT_EQ(result1.iterations, 0);
    
    // Test with very small convergence threshold
    params.maxIterations = 100;
    params.convergenceThreshold = 1e-15;
    auto [optimizedGraph2, result2] = bundleAdjustment->optimize(*testGraph, params);
    EXPECT_GE(result2.iterations, 0);
}

TEST_F(BundleAdjustmentTest, ErrorReduction)
{
    auto testGraph = createTestPoseGraph();
    BundleAdjustment::Parameters params;
    params.maxIterations = 50;
    params.convergenceThreshold = 1e-8;
    
    auto [optimizedGraph, result] = bundleAdjustment->optimize(*testGraph, params);
    
    // Should achieve some error reduction for noisy graph
    if (result.converged) {
        EXPECT_LT(result.finalError, result.initialError);
        EXPECT_GT(result.improvementRatio, 0.0);
    }
}

TEST_F(BundleAdjustmentTest, ProgressSignalEmission)
{
    auto testGraph = createTestPoseGraph();
    BundleAdjustment::Parameters params;
    params.maxIterations = 5;
    params.verbose = true;
    
    QSignalSpy progressSpy(bundleAdjustment.get(), &BundleAdjustment::optimizationProgress);
    
    auto [optimizedGraph, result] = bundleAdjustment->optimize(*testGraph, params);
    
    // Should emit progress signals
    EXPECT_GE(progressSpy.count(), 1);
    
    // Check signal parameters
    if (progressSpy.count() > 0) {
        auto firstSignal = progressSpy.first();
        EXPECT_EQ(firstSignal.size(), 3); // iteration, currentError, lambda
        EXPECT_GE(firstSignal[0].toInt(), 0); // iteration >= 0
        EXPECT_GE(firstSignal[1].toDouble(), 0.0); // currentError >= 0
        EXPECT_GT(firstSignal[2].toDouble(), 0.0); // lambda > 0
    }
}

TEST_F(BundleAdjustmentTest, CompletionSignalEmission)
{
    auto testGraph = createTestPoseGraph();
    BundleAdjustment::Parameters params;
    params.maxIterations = 5;
    
    QSignalSpy completedSpy(bundleAdjustment.get(), &BundleAdjustment::optimizationCompleted);
    
    auto [optimizedGraph, result] = bundleAdjustment->optimize(*testGraph, params);
    
    // Should emit completion signal exactly once
    EXPECT_EQ(completedSpy.count(), 1);
    
    // Check signal parameter
    if (completedSpy.count() > 0) {
        auto signal = completedSpy.first();
        EXPECT_EQ(signal.size(), 1); // BundleAdjustment::Result
    }
}

// Test fixture for testing with different graph sizes
class BundleAdjustmentGraphSizeTest : public BundleAdjustmentTest,
                                      public ::testing::WithParamInterface<int>
{
};

TEST_P(BundleAdjustmentGraphSizeTest, RecommendedParametersScaling)
{
    int nodeCount = GetParam();
    
    // Create graph with specified number of nodes
    auto graph = std::make_unique<PoseGraph>();
    for (int i = 0; i < nodeCount; ++i) {
        QMatrix4x4 transform;
        transform.setToIdentity();
        transform.translate(i * 1.0f, 0.0f, 0.0f);
        graph->addNode(QString("scan%1").arg(i), transform);
    }
    
    // Add edges to form a chain
    for (int i = 0; i < nodeCount - 1; ++i) {
        QMatrix4x4 edge;
        edge.setToIdentity();
        edge.translate(1.0f, 0.0f, 0.0f);
        graph->addEdge(i, i + 1, edge, 0.01f);
    }
    
    auto params = bundleAdjustment->getRecommendedParameters(*graph);
    
    // Larger graphs should have more iterations
    if (nodeCount > 20) {
        EXPECT_GE(params.maxIterations, 200);
    } else if (nodeCount > 10) {
        EXPECT_GE(params.maxIterations, 150);
    } else {
        EXPECT_GE(params.maxIterations, 100);
    }
}

INSTANTIATE_TEST_SUITE_P(GraphSizes, BundleAdjustmentGraphSizeTest,
                         ::testing::Values(3, 5, 10, 15, 25));
