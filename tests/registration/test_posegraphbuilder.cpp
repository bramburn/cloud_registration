#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QApplication>
#include <QMatrix4x4>
#include <QSignalSpy>

#include "registration/PoseGraphBuilder.h"
#include "registration/PoseGraph.h"
#include "registration/RegistrationProject.h"

using namespace Registration;
using namespace testing;

class PoseGraphBuilderTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a test registration project
        project = std::make_unique<RegistrationProject>("TestProject", "/tmp/test");
        builder = std::make_unique<PoseGraphBuilder>();
        
        // Add test scans
        addTestScans();
    }

    void TearDown() override
    {
        project.reset();
        builder.reset();
    }

    void addTestScans()
    {
        // Add three test scans
        ScanInfo scanA;
        scanA.scanId = "ScanA";
        scanA.filePath = "/tmp/scanA.e57";
        scanA.transform.setToIdentity();
        project->addScan(scanA);

        ScanInfo scanB;
        scanB.scanId = "ScanB";
        scanB.filePath = "/tmp/scanB.e57";
        scanB.transform.setToIdentity();
        project->addScan(scanB);

        ScanInfo scanC;
        scanC.scanId = "ScanC";
        scanC.filePath = "/tmp/scanC.e57";
        scanC.transform.setToIdentity();
        project->addScan(scanC);
    }

    void addRegistrationResult(const QString& source, const QString& target, float rmsError = 0.01f)
    {
        RegistrationProject::RegistrationResult result;
        result.sourceScanId = source;
        result.targetScanId = target;
        result.transformation.setToIdentity();
        result.rmsError = rmsError;
        result.correspondenceCount = 100;
        result.isValid = true;
        result.algorithm = "Test";
        
        project->addRegistrationResult(result);
    }

    std::unique_ptr<RegistrationProject> project;
    std::unique_ptr<PoseGraphBuilder> builder;
};

// Test Case 1: Simple chain (A-B, B-C)
TEST_F(PoseGraphBuilderTest, BuildGraphSimpleChain)
{
    // Add registration results for a simple chain
    addRegistrationResult("ScanA", "ScanB", 0.01f);
    addRegistrationResult("ScanB", "ScanC", 0.02f);

    // Build the pose graph
    auto graph = builder->build(*project);

    // Verify graph structure
    ASSERT_NE(graph, nullptr);
    EXPECT_EQ(graph->nodeCount(), 3);
    EXPECT_EQ(graph->edgeCount(), 2);

    // Verify nodes exist
    EXPECT_GE(graph->findNodeByScanId("ScanA"), 0);
    EXPECT_GE(graph->findNodeByScanId("ScanB"), 0);
    EXPECT_GE(graph->findNodeByScanId("ScanC"), 0);

    // Verify graph is valid
    EXPECT_TRUE(graph->isValid());
}

// Test Case 2: Loop closure (A-B, B-C, C-A)
TEST_F(PoseGraphBuilderTest, BuildGraphWithLoopClosure)
{
    // Add registration results forming a loop
    addRegistrationResult("ScanA", "ScanB", 0.01f);
    addRegistrationResult("ScanB", "ScanC", 0.02f);
    addRegistrationResult("ScanC", "ScanA", 0.015f);

    // Build the pose graph
    auto graph = builder->build(*project);

    // Verify graph structure
    ASSERT_NE(graph, nullptr);
    EXPECT_EQ(graph->nodeCount(), 3);
    EXPECT_EQ(graph->edgeCount(), 3);

    // Verify loop closure detection
    EXPECT_TRUE(graph->hasLoopClosures());

    // Verify graph is valid
    EXPECT_TRUE(graph->isValid());
}

// Test Case 3: Disconnected components (A-B, C-D)
TEST_F(PoseGraphBuilderTest, BuildGraphDisconnectedComponents)
{
    // Add a fourth scan
    ScanInfo scanD;
    scanD.scanId = "ScanD";
    scanD.filePath = "/tmp/scanD.e57";
    scanD.transform.setToIdentity();
    project->addScan(scanD);

    // Add registration results for disconnected components
    addRegistrationResult("ScanA", "ScanB", 0.01f);
    addRegistrationResult("ScanC", "ScanD", 0.02f);

    // Build the pose graph
    auto graph = builder->build(*project);

    // Verify graph structure
    ASSERT_NE(graph, nullptr);
    EXPECT_EQ(graph->nodeCount(), 4);
    EXPECT_EQ(graph->edgeCount(), 2);

    // Verify graph is valid (disconnected components are allowed)
    EXPECT_TRUE(graph->isValid());

    // Validate the graph and check for disconnected components
    auto validation = builder->validateGraph(*graph);
    EXPECT_GT(validation.connectedComponents, 1);
}

// Test Case 4: Empty project
TEST_F(PoseGraphBuilderTest, BuildGraphEmptyProject)
{
    // Create empty project
    auto emptyProject = std::make_unique<RegistrationProject>("Empty", "/tmp/empty");

    // Build the pose graph
    auto graph = builder->build(*emptyProject);

    // Verify empty graph
    ASSERT_NE(graph, nullptr);
    EXPECT_EQ(graph->nodeCount(), 0);
    EXPECT_EQ(graph->edgeCount(), 0);
    EXPECT_TRUE(graph->isEmpty());
}

// Test Case 5: Project with scans but no registrations
TEST_F(PoseGraphBuilderTest, BuildGraphNoRegistrations)
{
    // Build the pose graph (project has scans but no registrations)
    auto graph = builder->build(*project);

    // Verify graph has nodes but no edges
    ASSERT_NE(graph, nullptr);
    EXPECT_EQ(graph->nodeCount(), 3);
    EXPECT_EQ(graph->edgeCount(), 0);
    EXPECT_TRUE(graph->isValid());
}

// Test Case 6: Invalid registration results are ignored
TEST_F(PoseGraphBuilderTest, BuildGraphIgnoresInvalidResults)
{
    // Add valid registration
    addRegistrationResult("ScanA", "ScanB", 0.01f);

    // Add invalid registration result
    RegistrationProject::RegistrationResult invalidResult;
    invalidResult.sourceScanId = "ScanB";
    invalidResult.targetScanId = "ScanC";
    invalidResult.transformation.setToIdentity();
    invalidResult.rmsError = 0.02f;
    invalidResult.correspondenceCount = 50;
    invalidResult.isValid = false;  // Mark as invalid
    invalidResult.algorithm = "Test";
    project->addRegistrationResult(invalidResult);

    // Build the pose graph
    auto graph = builder->build(*project);

    // Verify only valid registration is included
    ASSERT_NE(graph, nullptr);
    EXPECT_EQ(graph->nodeCount(), 3);
    EXPECT_EQ(graph->edgeCount(), 1);  // Only one valid edge
}

// Test Case 7: Signal emission during build
TEST_F(PoseGraphBuilderTest, BuildGraphEmitsSignals)
{
    // Set up signal spies
    QSignalSpy progressSpy(builder.get(), &PoseGraphBuilder::buildProgress);
    QSignalSpy completedSpy(builder.get(), &PoseGraphBuilder::buildCompleted);

    // Add registration results
    addRegistrationResult("ScanA", "ScanB", 0.01f);
    addRegistrationResult("ScanB", "ScanC", 0.02f);

    // Build the pose graph
    auto graph = builder->build(*project);

    // Verify signals were emitted
    EXPECT_GT(progressSpy.count(), 0);
    EXPECT_EQ(completedSpy.count(), 1);

    // Verify completion signal indicates success
    QList<QVariant> completedArgs = completedSpy.takeFirst();
    EXPECT_TRUE(completedArgs.at(0).toBool());
}

// Test Case 8: Graph validation
TEST_F(PoseGraphBuilderTest, ValidateGraph)
{
    // Add registration results
    addRegistrationResult("ScanA", "ScanB", 0.01f);
    addRegistrationResult("ScanB", "ScanC", 0.02f);

    // Build the pose graph
    auto graph = builder->build(*project);

    // Validate the graph
    auto validation = builder->validateGraph(*graph);

    // Verify validation results
    EXPECT_TRUE(validation.isValid);
    EXPECT_EQ(validation.connectedComponents, 1);
    EXPECT_FALSE(validation.hasLoops);
    EXPECT_TRUE(validation.isolatedScans.isEmpty());
    EXPECT_TRUE(validation.errorMessage.isEmpty());
}

// Test Case 9: Add registration edge to existing graph
TEST_F(PoseGraphBuilderTest, AddRegistrationEdge)
{
    // Build initial graph
    auto graph = builder->build(*project);
    EXPECT_EQ(graph->edgeCount(), 0);

    // Add registration edge
    QMatrix4x4 transform;
    transform.setToIdentity();
    bool success = builder->addRegistrationEdge(*graph, "ScanA", "ScanB", transform, 0.01f);

    // Verify edge was added
    EXPECT_TRUE(success);
    EXPECT_EQ(graph->edgeCount(), 1);
}

// Main function for running tests
int main(int argc, char** argv)
{
    QApplication app(argc, argv);  // Required for Qt objects
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
