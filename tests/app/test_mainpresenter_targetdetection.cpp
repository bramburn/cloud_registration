#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QApplication>
#include <QSignalSpy>

#include "app/MainPresenter.h"
#include "interfaces/IMainView.h"
#include "interfaces/IE57Parser.h"
#include "interfaces/IPointCloudViewer.h"

// Mock classes for testing
class MockMainView : public IMainView
{
public:
    MOCK_METHOD(void, displayErrorMessage, (const QString& title, const QString& message), (override));
    MOCK_METHOD(void, displayInfoMessage, (const QString& title, const QString& message), (override));
    MOCK_METHOD(void, displayWarningMessage, (const QString& title, const QString& message), (override));
    MOCK_METHOD(void, updateStatusBar, (const QString& text), (override));
    MOCK_METHOD(void, setWindowTitle, (const QString& title), (override));
    MOCK_METHOD(IPointCloudViewer*, getViewer, (), (override));
    MOCK_METHOD(SidebarWidget*, getSidebar, (), (override));
    MOCK_METHOD(void, showProgressDialog, (bool show, const QString& title, const QString& message), (override));
    MOCK_METHOD(void, updateProgress, (int percentage, const QString& message), (override));
    MOCK_METHOD(void, setActionsEnabled, (bool enabled), (override));
    MOCK_METHOD(void, setProjectTitle, (const QString& projectName), (override));
    MOCK_METHOD(void, showProjectView, (), (override));
    MOCK_METHOD(void, showProjectHub, (), (override));
    MOCK_METHOD(void, updateScanList, (const QStringList& scanNames), (override));
    MOCK_METHOD(void, highlightScan, (const QString& scanId), (override));
    MOCK_METHOD(void, updateRenderingStats, (float fps, int visiblePoints), (override));
    MOCK_METHOD(void, updateMemoryUsage, (size_t totalBytes), (override));
    MOCK_METHOD(QString, askForOpenFilePath, (const QString& title, const QString& filter), (override));
    MOCK_METHOD(QString, askForSaveFilePath, (const QString& title, const QString& filter), (override));
    MOCK_METHOD(bool, askForConfirmation, (const QString& title, const QString& message), (override));
};

class MockE57Parser : public IE57Parser
{
public:
    MOCK_METHOD(bool, openFile, (const std::string& filePath), (override));
    MOCK_METHOD(void, closeFile, (), (override));
    MOCK_METHOD(std::vector<float>, extractPointData, (), (override));
    MOCK_METHOD(std::vector<float>, extractIntensityData, (), (override));
    MOCK_METHOD(std::vector<uint8_t>, extractColorData, (), (override));
    MOCK_METHOD(int, getScanCount, (), (const, override));
    MOCK_METHOD(std::vector<std::string>, getScanNames, (), (const, override));
    MOCK_METHOD(std::string, getLastError, (), (const, override));
    MOCK_METHOD(bool, isFileOpen, (), (const, override));
};

class MockPointCloudViewer : public IPointCloudViewer
{
public:
    MOCK_METHOD(void, loadPointCloud, (const std::vector<float>& points), (override));
    MOCK_METHOD(void, clearPointCloud, (), (override));
    MOCK_METHOD(void, resetCamera, (), (override));
    MOCK_METHOD(bool, hasData, (), (const, override));
    MOCK_METHOD(void, setPointSize, (float size), (override));
    MOCK_METHOD(void, setBackgroundColor, (const QColor& color), (override));
    MOCK_METHOD(void, focusOnScan, (const QString& scanId), (override));
    MOCK_METHOD(void, focusOnCluster, (const QString& clusterId), (override));
};

class MainPresenterTargetDetectionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Ensure QApplication exists for Qt widgets
        if (!QApplication::instance())
        {
            int argc = 0;
            char** argv = nullptr;
            app = new QApplication(argc, argv);
        }
        
        mockView = new MockMainView();
        mockParser = new MockE57Parser();
        mockViewer = new MockPointCloudViewer();
        
        // Setup default expectations
        EXPECT_CALL(*mockView, getViewer())
            .WillRepeatedly(::testing::Return(mockViewer));
        EXPECT_CALL(*mockView, getSidebar())
            .WillRepeatedly(::testing::Return(nullptr));
        
        presenter = new MainPresenter(mockView, mockParser);
        presenter->initialize();
    }

    void TearDown() override
    {
        delete presenter;
        delete mockView;
        delete mockParser;
        // mockViewer is owned by mockView, don't delete separately
    }

    QApplication* app = nullptr;
    MockMainView* mockView = nullptr;
    MockE57Parser* mockParser = nullptr;
    MockPointCloudViewer* mockViewer = nullptr;
    MainPresenter* presenter = nullptr;
};

TEST_F(MainPresenterTargetDetectionTest, HandleTargetDetectionClickedNoScans)
{
    // Test behavior when no scans are loaded
    EXPECT_CALL(*mockView, displayErrorMessage(
        QString("Target Detection"), 
        QString("Please load point cloud scans first.")))
        .Times(1);
    
    presenter->handleTargetDetectionClicked();
}

TEST_F(MainPresenterTargetDetectionTest, HandleTargetDetectionClickedWithScans)
{
    // Simulate having loaded scans
    QStringList testScans = {"scan1", "scan2"};
    
    // First, simulate opening a file to have scans available
    EXPECT_CALL(*mockParser, openFile(::testing::_))
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(*mockParser, extractPointData())
        .WillOnce(::testing::Return(std::vector<float>{1.0f, 2.0f, 3.0f}));
    EXPECT_CALL(*mockViewer, loadPointCloud(::testing::_))
        .Times(1);
    EXPECT_CALL(*mockViewer, resetCamera())
        .Times(1);
    EXPECT_CALL(*mockView, showProgressDialog(true, ::testing::_, ::testing::_))
        .Times(1);
    EXPECT_CALL(*mockView, showProgressDialog(false))
        .Times(1);
    EXPECT_CALL(*mockView, setActionsEnabled(false))
        .Times(1);
    EXPECT_CALL(*mockView, setActionsEnabled(true))
        .Times(1);
    EXPECT_CALL(*mockView, updateProgress(::testing::_, ::testing::_))
        .Times(::testing::AtLeast(0));
    EXPECT_CALL(*mockView, updateStatusBar(::testing::_))
        .Times(::testing::AtLeast(1));
    EXPECT_CALL(*mockView, displayInfoMessage(::testing::_, ::testing::_))
        .Times(1);
    EXPECT_CALL(*mockView, setWindowTitle(::testing::_))
        .Times(::testing::AtLeast(1));
    
    // Open a file to have scans available
    presenter->handleOpenFile("test.e57");
    
    // Now test target detection - this should not show an error
    // Instead it should attempt to create and show the dialog
    // Since we can't easily mock the dialog creation, we'll just verify
    // that no error is displayed for missing scans
    EXPECT_CALL(*mockView, displayErrorMessage(
        QString("Target Detection"), 
        QString("Please load point cloud scans first.")))
        .Times(0);
    
    // The actual dialog creation might fail in test environment,
    // but we're mainly testing the logic flow
    presenter->handleTargetDetectionClicked();
}

TEST_F(MainPresenterTargetDetectionTest, TargetDetectionWorkflow)
{
    // Test the complete workflow of target detection
    
    // 1. First load some scans
    EXPECT_CALL(*mockParser, openFile(::testing::_))
        .WillOnce(::testing::Return(true));
    EXPECT_CALL(*mockParser, extractPointData())
        .WillOnce(::testing::Return(std::vector<float>{1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f}));
    EXPECT_CALL(*mockViewer, loadPointCloud(::testing::_))
        .Times(1);
    EXPECT_CALL(*mockViewer, resetCamera())
        .Times(1);
    EXPECT_CALL(*mockView, showProgressDialog(::testing::_, ::testing::_, ::testing::_))
        .Times(::testing::AtLeast(1));
    EXPECT_CALL(*mockView, setActionsEnabled(::testing::_))
        .Times(::testing::AtLeast(2));
    EXPECT_CALL(*mockView, updateStatusBar(::testing::_))
        .Times(::testing::AtLeast(1));
    EXPECT_CALL(*mockView, displayInfoMessage(::testing::_, ::testing::_))
        .Times(::testing::AtLeast(1));
    EXPECT_CALL(*mockView, setWindowTitle(::testing::_))
        .Times(::testing::AtLeast(1));
    
    presenter->handleOpenFile("test.e57");
    
    // 2. Now test target detection
    // The dialog creation might fail in test environment, but we test the logic
    presenter->handleTargetDetectionClicked();
    
    // If we get here without exceptions, the basic workflow is working
    SUCCEED();
}

TEST_F(MainPresenterTargetDetectionTest, InitializationState)
{
    // Test that presenter initializes correctly for target detection
    EXPECT_NE(presenter, nullptr);
    
    // The presenter should be ready to handle target detection requests
    // even if no scans are loaded (it should show appropriate error)
    EXPECT_CALL(*mockView, displayErrorMessage(::testing::_, ::testing::_))
        .Times(1);
    
    presenter->handleTargetDetectionClicked();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
