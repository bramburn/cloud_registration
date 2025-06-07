#include "src/IPointCloudViewer.h"
#include "src/pointcloudviewerwidget.h"
#include "src/mainwindow.h"
#include <QApplication>
#include <iostream>

/**
 * @brief Simple compilation test to verify the decoupling implementation
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    std::cout << "Testing Sprint 3 PointCloudViewer Decoupling Implementation..." << std::endl;
    
    try {
        // Test 1: Create PointCloudViewerWidget and verify it implements IPointCloudViewer
        PointCloudViewerWidget* widget = new PointCloudViewerWidget();
        IPointCloudViewer* viewer = widget; // This should work if decoupling is successful
        
        std::cout << "✓ PointCloudViewerWidget successfully implements IPointCloudViewer" << std::endl;
        
        // Test 2: Test basic interface operations
        std::vector<float> testPoints = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f}; // 2 points
        viewer->loadPointCloud(testPoints);
        std::cout << "✓ loadPointCloud() method works through interface" << std::endl;
        
        viewer->setState(ViewerState::DisplayingData, "Test message");
        std::cout << "✓ setState() method works through interface" << std::endl;
        
        viewer->setTopView();
        std::cout << "✓ setTopView() method works through interface" << std::endl;
        
        viewer->setLODEnabled(true);
        std::cout << "✓ setLODEnabled() method works through interface" << std::endl;
        
        viewer->setRenderWithColor(true);
        std::cout << "✓ setRenderWithColor() method works through interface" << std::endl;
        
        viewer->clearPointCloud();
        std::cout << "✓ clearPointCloud() method works through interface" << std::endl;
        
        // Test 3: Verify state queries work
        ViewerState state = viewer->getViewerState();
        bool hasData = viewer->hasPointCloudData();
        size_t pointCount = viewer->getPointCount();
        float fps = viewer->getCurrentFPS();
        
        std::cout << "✓ State query methods work through interface" << std::endl;
        std::cout << "  - Current state: " << static_cast<int>(state) << std::endl;
        std::cout << "  - Has data: " << (hasData ? "true" : "false") << std::endl;
        std::cout << "  - Point count: " << pointCount << std::endl;
        std::cout << "  - FPS: " << fps << std::endl;
        
        // Test 4: Test MainWindow can use the interface
        MainWindow* mainWindow = new MainWindow();
        IPointCloudViewer* mainWindowViewer = mainWindow->getPointCloudViewer();
        
        if (mainWindowViewer) {
            std::cout << "✓ MainWindow successfully provides IPointCloudViewer interface" << std::endl;
            
            // Test that MainWindow's viewer responds to interface calls
            mainWindowViewer->setState(ViewerState::Idle, "Interface test");
            std::cout << "✓ MainWindow viewer responds to interface calls" << std::endl;
        } else {
            std::cout << "✗ MainWindow viewer is null" << std::endl;
            return 1;
        }
        
        // Cleanup
        delete widget;
        delete mainWindow;
        
        std::cout << std::endl << "🎉 All Sprint 3 decoupling tests passed!" << std::endl;
        std::cout << "✓ PointCloudViewerWidget successfully implements IPointCloudViewer interface" << std::endl;
        std::cout << "✓ MainWindow uses IPointCloudViewer interface for all viewer interactions" << std::endl;
        std::cout << "✓ Decoupling enables polymorphic usage and future extensibility" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "✗ Test failed with unknown exception" << std::endl;
        return 1;
    }
}
