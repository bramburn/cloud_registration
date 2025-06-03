/**
 * Sprint 3.2 Test Suite: 3D Point Cloud Viewer Component
 * 
 * This test suite validates the implementation of Sprint 3.2 requirements:
 * - User Story 1: 3D Point Cloud Viewer Component
 * - User Story 2: Point Cloud Data Rendering
 * - User Story 3: Camera Controls
 * 
 * Test Coverage:
 * - PointCloudViewerWidget functionality
 * - PointCloudLoadManager integration
 * - MainWindow integration
 * - Camera controls and rendering
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QTest>
#include <QSignalSpy>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <vector>

#include "../src/pointcloudviewerwidget.h"
#include "../src/pointcloudloadmanager.h"
#include "../src/mainwindow.h"
#include "../src/projectmanager.h"
#include "../src/sqlitemanager.h"

class Sprint32ViewerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Ensure QApplication exists for Qt widgets
        if (!QApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = new QApplication(argc, argv);
        }
        
        // Create test components
        viewer = new PointCloudViewerWidget();
        loadManager = new PointCloudLoadManager();
        mainWindow = new MainWindow();
        
        // Create test point cloud data
        createTestPointCloudData();
    }
    
    void TearDown() override
    {
        delete viewer;
        delete loadManager;
        delete mainWindow;
        
        if (app) {
            delete app;
            app = nullptr;
        }
    }
    
    void createTestPointCloudData()
    {
        // Create a simple cube point cloud for testing
        testPoints.clear();
        
        // 8 vertices of a cube
        std::vector<std::vector<float>> vertices = {
            {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f},
            {1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f},
            {-1.0f, -1.0f,  1.0f}, {1.0f, -1.0f,  1.0f},
            {1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f}
        };
        
        for (const auto& vertex : vertices) {
            testPoints.insert(testPoints.end(), vertex.begin(), vertex.end());
        }
    }
    
    QApplication* app = nullptr;
    PointCloudViewerWidget* viewer = nullptr;
    PointCloudLoadManager* loadManager = nullptr;
    MainWindow* mainWindow = nullptr;
    std::vector<float> testPoints;
};

// Test Case S3.2.1: 3D Point Cloud Viewer Component Creation
TEST_F(Sprint32ViewerTest, ViewerComponentCreation)
{
    ASSERT_NE(viewer, nullptr);
    EXPECT_EQ(viewer->getViewerState(), PointCloudViewerWidget::ViewerState::Idle);

    // Test initial state
    EXPECT_FALSE(viewer->hasPointCloudData());
    EXPECT_EQ(viewer->getPointCount(), 0);
}

// Test Case S3.2.2: Point Cloud Data Loading
TEST_F(Sprint32ViewerTest, PointCloudDataLoading)
{
    ASSERT_FALSE(testPoints.empty());
    
    // Load test data
    viewer->loadPointCloud(testPoints);
    
    // Verify state change
    EXPECT_EQ(viewer->getViewerState(), PointCloudViewerWidget::ViewerState::DisplayingData);
    EXPECT_TRUE(viewer->hasPointCloudData());
    
    // Verify point count
    EXPECT_EQ(viewer->getPointCount(), testPoints.size() / 3);
}

// Test Case S3.2.3: Point Cloud Data Clearing
TEST_F(Sprint32ViewerTest, PointCloudDataClearing)
{
    // Load data first
    viewer->loadPointCloud(testPoints);
    EXPECT_TRUE(viewer->hasPointCloudData());
    
    // Clear data
    viewer->clearPointCloud();
    
    // Verify cleared state
    EXPECT_EQ(viewer->getViewerState(), PointCloudViewerWidget::ViewerState::Idle);
    EXPECT_FALSE(viewer->hasPointCloudData());
    EXPECT_EQ(viewer->getPointCount(), 0);
}

// Test Case S3.2.4: Camera Controls - Orbit
TEST_F(Sprint32ViewerTest, CameraOrbitControls)
{
    // Load test data
    viewer->loadPointCloud(testPoints);
    
    // Get initial camera position
    auto initialYaw = viewer->getCameraYaw();
    auto initialPitch = viewer->getCameraPitch();
    
    // Simulate mouse orbit (left-click drag)
    QPoint startPos(100, 100);
    QPoint endPos(150, 120);
    
    viewer->simulateOrbitCamera(startPos, endPos);
    
    // Verify camera position changed
    EXPECT_NE(viewer->getCameraYaw(), initialYaw);
    EXPECT_NE(viewer->getCameraPitch(), initialPitch);
}

// Test Case S3.2.5: Camera Controls - Pan
TEST_F(Sprint32ViewerTest, CameraPanControls)
{
    // Load test data
    viewer->loadPointCloud(testPoints);
    
    // Get initial camera target
    auto initialTarget = viewer->getCameraTarget();
    
    // Simulate mouse pan (right-click drag)
    QPoint startPos(100, 100);
    QPoint endPos(120, 110);
    
    viewer->simulatePanCamera(startPos, endPos);
    
    // Verify camera target changed
    EXPECT_NE(viewer->getCameraTarget(), initialTarget);
}

// Test Case S3.2.6: Camera Controls - Zoom
TEST_F(Sprint32ViewerTest, CameraZoomControls)
{
    // Load test data
    viewer->loadPointCloud(testPoints);
    
    // Get initial camera distance
    auto initialDistance = viewer->getCameraDistance();
    
    // Simulate mouse wheel zoom
    viewer->simulateZoomCamera(1.2f); // Zoom in
    
    // Verify camera distance changed
    EXPECT_LT(viewer->getCameraDistance(), initialDistance);
    
    // Zoom out
    viewer->simulateZoomCamera(0.8f);
    EXPECT_GT(viewer->getCameraDistance(), viewer->getCameraDistance());
}

// Test Case S3.2.7: Load Manager Integration
TEST_F(Sprint32ViewerTest, LoadManagerIntegration)
{
    // Set up signal spy
    QSignalSpy dataSpy(loadManager, &PointCloudLoadManager::pointCloudDataReady);
    QSignalSpy errorSpy(loadManager, &PointCloudLoadManager::pointCloudViewFailed);
    
    // Test successful data emission
    QString sourceInfo = "Test Scan (8 points)";
    emit loadManager->pointCloudDataReady(testPoints, sourceInfo);
    
    // Verify signal was emitted
    EXPECT_EQ(dataSpy.count(), 1);
    EXPECT_EQ(errorSpy.count(), 0);
    
    // Verify signal parameters
    QList<QVariant> arguments = dataSpy.takeFirst();
    auto emittedPoints = arguments.at(0).value<std::vector<float>>();
    QString emittedInfo = arguments.at(1).toString();
    
    EXPECT_EQ(emittedPoints.size(), testPoints.size());
    EXPECT_EQ(emittedInfo, sourceInfo);
}

// Test Case S3.2.8: Error Handling
TEST_F(Sprint32ViewerTest, ErrorHandling)
{
    // Test empty point cloud
    std::vector<float> emptyPoints;
    viewer->loadPointCloud(emptyPoints);
    
    EXPECT_EQ(viewer->getViewerState(), PointCloudViewerWidget::ViewerState::Idle);
    EXPECT_FALSE(viewer->hasPointCloudData());
    
    // Test load failure
    QSignalSpy errorSpy(loadManager, &PointCloudLoadManager::pointCloudViewFailed);
    
    QString errorMessage = "Test error message";
    emit loadManager->pointCloudViewFailed(errorMessage);
    
    EXPECT_EQ(errorSpy.count(), 1);
    QList<QVariant> arguments = errorSpy.takeFirst();
    EXPECT_EQ(arguments.at(0).toString(), errorMessage);
}

// Test Case S3.2.9: MainWindow Integration
TEST_F(Sprint32ViewerTest, MainWindowIntegration)
{
    // Verify MainWindow has viewer component
    EXPECT_NE(mainWindow->getPointCloudViewer(), nullptr);

    // Verify MainWindow has load manager
    EXPECT_NE(mainWindow->getPointCloudLoadManager(), nullptr);

    // Test that components are properly initialized
    auto viewerWidget = mainWindow->getPointCloudViewer();
    auto loadManagerWidget = mainWindow->getPointCloudLoadManager();

    EXPECT_EQ(viewerWidget->getViewerState(), PointCloudViewerWidget::ViewerState::Idle);
    EXPECT_FALSE(viewerWidget->hasPointCloudData());
}

// Test Case S3.2.10: Performance with Large Point Clouds
TEST_F(Sprint32ViewerTest, LargePointCloudPerformance)
{
    // Create larger test dataset (1000 points)
    std::vector<float> largePoints;
    largePoints.reserve(3000); // 1000 points * 3 coordinates
    
    for (int i = 0; i < 1000; ++i) {
        largePoints.push_back(static_cast<float>(i % 100) / 10.0f);
        largePoints.push_back(static_cast<float>((i / 100) % 100) / 10.0f);
        largePoints.push_back(static_cast<float>(i / 10000) / 10.0f);
    }
    
    // Measure loading time
    auto startTime = std::chrono::high_resolution_clock::now();
    viewer->loadPointCloud(largePoints);
    auto endTime = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Verify successful loading
    EXPECT_TRUE(viewer->hasPointCloudData());
    EXPECT_EQ(viewer->getPointCount(), 1000);
    
    // Performance should be reasonable (less than 1 second for 1000 points)
    EXPECT_LT(duration.count(), 1000);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
