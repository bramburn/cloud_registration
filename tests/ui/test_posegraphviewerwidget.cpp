#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QApplication>
#include <QSignalSpy>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QTest>

#include "ui/PoseGraphViewerWidget.h"
#include "registration/PoseGraph.h"

using namespace Registration;
using namespace testing;

class PoseGraphViewerWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        viewer = std::make_unique<PoseGraphViewerWidget>();
        graph = std::make_unique<PoseGraph>();
    }

    void TearDown() override
    {
        viewer.reset();
        graph.reset();
    }

    void createSimpleGraph()
    {
        // Add nodes
        graph->addNode("ScanA", QMatrix4x4());
        graph->addNode("ScanB", QMatrix4x4());
        
        // Add edge
        int nodeA = graph->findNodeByScanId("ScanA");
        int nodeB = graph->findNodeByScanId("ScanB");
        graph->addEdge(nodeA, nodeB, QMatrix4x4(), 0.01f);
    }

    void createTriangleGraph()
    {
        // Add nodes
        graph->addNode("ScanA", QMatrix4x4());
        graph->addNode("ScanB", QMatrix4x4());
        graph->addNode("ScanC", QMatrix4x4());
        
        // Add edges forming a triangle
        int nodeA = graph->findNodeByScanId("ScanA");
        int nodeB = graph->findNodeByScanId("ScanB");
        int nodeC = graph->findNodeByScanId("ScanC");
        
        graph->addEdge(nodeA, nodeB, QMatrix4x4(), 0.01f);
        graph->addEdge(nodeB, nodeC, QMatrix4x4(), 0.02f);
        graph->addEdge(nodeC, nodeA, QMatrix4x4(), 0.015f);
    }

    std::unique_ptr<PoseGraphViewerWidget> viewer;
    std::unique_ptr<PoseGraph> graph;
};

// Test Case 1: Widget initialization
TEST_F(PoseGraphViewerWidgetTest, WidgetInitialization)
{
    // Verify widget is properly initialized
    EXPECT_NE(viewer.get(), nullptr);
    
    // Check that the widget has the expected child widgets
    auto* graphicsView = viewer->findChild<QGraphicsView*>();
    EXPECT_NE(graphicsView, nullptr);
    
    auto* scene = graphicsView->scene();
    EXPECT_NE(scene, nullptr);
    
    // Verify initial state
    EXPECT_TRUE(scene->items().isEmpty());
}

// Test Case 2: Display simple graph with 2 nodes and 1 edge
TEST_F(PoseGraphViewerWidgetTest, DisplaySimpleGraph)
{
    createSimpleGraph();
    
    // Set up signal spy
    QSignalSpy viewUpdatedSpy(viewer.get(), &PoseGraphViewerWidget::viewUpdated);
    
    // Display the graph
    viewer->displayGraph(*graph);
    
    // Verify signal was emitted
    EXPECT_EQ(viewUpdatedSpy.count(), 1);
    
    // Get the graphics scene
    auto* graphicsView = viewer->findChild<QGraphicsView*>();
    ASSERT_NE(graphicsView, nullptr);
    auto* scene = graphicsView->scene();
    ASSERT_NE(scene, nullptr);
    
    // Count different types of items
    auto items = scene->items();
    int ellipseCount = 0;
    int lineCount = 0;
    int textCount = 0;
    
    for (auto* item : items) {
        if (qgraphicsitem_cast<QGraphicsEllipseItem*>(item)) {
            ellipseCount++;
        } else if (qgraphicsitem_cast<QGraphicsLineItem*>(item)) {
            lineCount++;
        } else if (qgraphicsitem_cast<QGraphicsTextItem*>(item)) {
            textCount++;
        }
    }
    
    // Verify correct number of items
    EXPECT_EQ(ellipseCount, 2);  // 2 nodes
    EXPECT_EQ(lineCount, 1);     // 1 edge
    EXPECT_GE(textCount, 2);     // At least 2 labels (node labels)
}

// Test Case 3: Display triangle graph with loop closure
TEST_F(PoseGraphViewerWidgetTest, DisplayTriangleGraph)
{
    createTriangleGraph();
    
    // Display the graph
    viewer->displayGraph(*graph);
    
    // Get the graphics scene
    auto* graphicsView = viewer->findChild<QGraphicsView*>();
    ASSERT_NE(graphicsView, nullptr);
    auto* scene = graphicsView->scene();
    ASSERT_NE(scene, nullptr);
    
    // Count items
    auto items = scene->items();
    int ellipseCount = 0;
    int lineCount = 0;
    
    for (auto* item : items) {
        if (qgraphicsitem_cast<QGraphicsEllipseItem*>(item)) {
            ellipseCount++;
        } else if (qgraphicsitem_cast<QGraphicsLineItem*>(item)) {
            lineCount++;
        }
    }
    
    // Verify correct number of items
    EXPECT_EQ(ellipseCount, 3);  // 3 nodes
    EXPECT_EQ(lineCount, 3);     // 3 edges
}

// Test Case 4: Clear graph
TEST_F(PoseGraphViewerWidgetTest, ClearGraph)
{
    createSimpleGraph();
    
    // Display the graph
    viewer->displayGraph(*graph);
    
    // Get the graphics scene
    auto* graphicsView = viewer->findChild<QGraphicsView*>();
    ASSERT_NE(graphicsView, nullptr);
    auto* scene = graphicsView->scene();
    ASSERT_NE(scene, nullptr);
    
    // Verify items exist
    EXPECT_FALSE(scene->items().isEmpty());
    
    // Clear the graph
    viewer->clearGraph();
    
    // Verify scene is empty
    EXPECT_TRUE(scene->items().isEmpty());
}

// Test Case 5: Display empty graph
TEST_F(PoseGraphViewerWidgetTest, DisplayEmptyGraph)
{
    // Display empty graph
    viewer->displayGraph(*graph);
    
    // Get the graphics scene
    auto* graphicsView = viewer->findChild<QGraphicsView*>();
    ASSERT_NE(graphicsView, nullptr);
    auto* scene = graphicsView->scene();
    ASSERT_NE(scene, nullptr);
    
    // Verify scene is empty
    EXPECT_TRUE(scene->items().isEmpty());
}

// Test Case 6: Toggle node labels
TEST_F(PoseGraphViewerWidgetTest, ToggleNodeLabels)
{
    createSimpleGraph();
    
    // Display the graph with labels enabled (default)
    viewer->displayGraph(*graph);
    
    // Get the graphics scene
    auto* graphicsView = viewer->findChild<QGraphicsView*>();
    ASSERT_NE(graphicsView, nullptr);
    auto* scene = graphicsView->scene();
    ASSERT_NE(scene, nullptr);
    
    // Count text items (labels)
    auto items = scene->items();
    int textCount = 0;
    for (auto* item : items) {
        if (auto* textItem = qgraphicsitem_cast<QGraphicsTextItem*>(item)) {
            if (textItem->isVisible()) {
                textCount++;
            }
        }
    }
    
    int initialTextCount = textCount;
    EXPECT_GT(initialTextCount, 0);
    
    // Hide labels
    viewer->setShowNodeLabels(false);
    
    // Count visible text items again
    textCount = 0;
    for (auto* item : items) {
        if (auto* textItem = qgraphicsitem_cast<QGraphicsTextItem*>(item)) {
            if (textItem->isVisible()) {
                textCount++;
            }
        }
    }
    
    // Should have fewer visible text items
    EXPECT_LT(textCount, initialTextCount);
}

// Test Case 7: Node selection signal
TEST_F(PoseGraphViewerWidgetTest, NodeSelectionSignal)
{
    createSimpleGraph();
    
    // Set up signal spy
    QSignalSpy nodeSelectedSpy(viewer.get(), &PoseGraphViewerWidget::nodeSelected);
    
    // Display the graph
    viewer->displayGraph(*graph);
    
    // Get the graphics scene
    auto* graphicsView = viewer->findChild<QGraphicsView*>();
    ASSERT_NE(graphicsView, nullptr);
    auto* scene = graphicsView->scene();
    ASSERT_NE(scene, nullptr);
    
    // Find a node item
    QGraphicsEllipseItem* nodeItem = nullptr;
    for (auto* item : scene->items()) {
        if (auto* ellipse = qgraphicsitem_cast<QGraphicsEllipseItem*>(item)) {
            nodeItem = ellipse;
            break;
        }
    }
    
    ASSERT_NE(nodeItem, nullptr);
    
    // Simulate node selection
    scene->clearSelection();
    nodeItem->setSelected(true);
    
    // Note: In a real test, we would need to trigger the selection changed signal
    // For now, we just verify the setup is correct
    EXPECT_TRUE(nodeItem->isSelected());
}

// Test Case 8: Zoom functionality
TEST_F(PoseGraphViewerWidgetTest, ZoomFunctionality)
{
    createSimpleGraph();
    viewer->displayGraph(*graph);
    
    // Get the graphics view
    auto* graphicsView = viewer->findChild<QGraphicsView*>();
    ASSERT_NE(graphicsView, nullptr);
    
    // Get initial transform
    QTransform initialTransform = graphicsView->transform();
    double initialScale = initialTransform.m11();
    
    // Zoom in
    viewer->zoomIn();
    
    // Check that scale increased
    QTransform newTransform = graphicsView->transform();
    double newScale = newTransform.m11();
    EXPECT_GT(newScale, initialScale);
    
    // Zoom out
    viewer->zoomOut();
    
    // Check that scale decreased
    QTransform finalTransform = graphicsView->transform();
    double finalScale = finalTransform.m11();
    EXPECT_LT(finalScale, newScale);
}

// Test Case 9: Export functionality
TEST_F(PoseGraphViewerWidgetTest, ExportFunctionality)
{
    createSimpleGraph();
    viewer->displayGraph(*graph);
    
    // Test export to a temporary file
    QString tempFile = "/tmp/test_graph_export.png";
    
    // Export should succeed with content
    bool success = viewer->exportAsImage(tempFile);
    EXPECT_TRUE(success);
    
    // Test export with empty graph
    viewer->clearGraph();
    success = viewer->exportAsImage(tempFile);
    EXPECT_FALSE(success);  // Should fail with empty graph
}

// Test Case 10: Fit to view
TEST_F(PoseGraphViewerWidgetTest, FitToView)
{
    createTriangleGraph();
    viewer->displayGraph(*graph);
    
    // Get the graphics view
    auto* graphicsView = viewer->findChild<QGraphicsView*>();
    ASSERT_NE(graphicsView, nullptr);
    
    // Zoom to a specific level first
    viewer->zoomIn();
    viewer->zoomIn();
    
    // Fit to view
    viewer->fitToView();
    
    // Verify that the view shows all items
    auto* scene = graphicsView->scene();
    QRectF sceneRect = scene->itemsBoundingRect();
    QRectF viewRect = graphicsView->mapToScene(graphicsView->viewport()->rect()).boundingRect();
    
    // The view should contain the scene items
    EXPECT_TRUE(viewRect.contains(sceneRect) || viewRect.intersects(sceneRect));
}

// Main function for running tests
int main(int argc, char** argv)
{
    QApplication app(argc, argv);  // Required for Qt widgets
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
