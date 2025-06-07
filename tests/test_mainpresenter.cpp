#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QApplication>
#include <QTest>

#include "MainPresenter.h"
#include "IMainView.h"
#include "IPointCloudViewer.h"

// Mock implementation of IMainView for testing
class MockMainView : public IMainView
{
public:
    MOCK_METHOD(void, setWindowTitle, (const QString& title), (override));
    MOCK_METHOD(void, updateWindowTitle, (), (override));
    MOCK_METHOD(void, updateStatusBar, (const QString& text), (override));
    MOCK_METHOD(void, setStatusReady, (), (override));
    MOCK_METHOD(void, setStatusLoading, (const QString& fileName), (override));
    MOCK_METHOD(void, setStatusLoadSuccess, (const QString& fileName, int pointCount), (override));
    MOCK_METHOD(void, setStatusLoadFailed, (const QString& fileName, const QString& message), (override));
    MOCK_METHOD(void, setStatusViewChanged, (const QString& viewName), (override));
    MOCK_METHOD(void, displayErrorMessage, (const QString& title, const QString& message), (override));
    MOCK_METHOD(void, displayWarningMessage, (const QString& title, const QString& message), (override));
    MOCK_METHOD(void, displayInfoMessage, (const QString& title, const QString& message), (override));
    MOCK_METHOD(void, showProjectHub, (), (override));
    MOCK_METHOD(void, transitionToProjectView, (const QString& projectPath), (override));
    MOCK_METHOD(void, enableProjectActions, (bool enabled), (override));
    MOCK_METHOD(void, showImportGuidance, (bool show), (override));
    MOCK_METHOD(IPointCloudViewer*, getViewer, (), (override));
    MOCK_METHOD(void, showProgressDialog, (const QString& title, const QString& message), (override));
    MOCK_METHOD(void, updateProgressDialog, (int percentage, const QString& stage), (override));
    MOCK_METHOD(void, hideProgressDialog, (), (override));
    MOCK_METHOD(void, updateMemoryDisplay, (size_t totalBytes), (override));
    MOCK_METHOD(void, updatePerformanceStats, (float fps, int visiblePoints), (override));
    MOCK_METHOD(void, setLoadingState, (bool isLoading), (override));
    MOCK_METHOD(void, updateLoadingProgress, (int percentage, const QString& stage), (override));
    MOCK_METHOD(QString, showOpenFileDialog, (const QString& title, const QString& filter), (override));
    MOCK_METHOD(QString, showOpenProjectDialog, (), (override));
    MOCK_METHOD(QString, showSaveFileDialog, (const QString& title, const QString& filter), (override));
    MOCK_METHOD(bool, showLoadingSettingsDialog, (), (override));
    MOCK_METHOD(bool, showCreateProjectDialog, (QString& projectName, QString& projectPath), (override));
    MOCK_METHOD(bool, showScanImportDialog, (), (override));
    MOCK_METHOD(void, refreshScanList, (), (override));
    MOCK_METHOD(void, enableViewControls, (bool enabled), (override));
    MOCK_METHOD(void, updateViewControlsState, (), (override));
    MOCK_METHOD(bool, isProjectOpen, (), (const, override));
    MOCK_METHOD(QString, getCurrentProjectPath, (), (const, override));
    MOCK_METHOD(Project*, getCurrentProject, (), (const, override));
    MOCK_METHOD(void, prepareForShutdown, (), (override));
    MOCK_METHOD(void, cleanupResources, (), (override));
};

// Mock implementation of IPointCloudViewer for testing
class MockPointCloudViewer : public IPointCloudViewer
{
public:
    MOCK_METHOD(void, loadPointCloud, (const std::vector<float>& points), (override));
    MOCK_METHOD(void, clearPointCloud, (), (override));
    MOCK_METHOD(void, addPointCloudData, (const std::vector<float>& additionalPoints), (override));
    MOCK_METHOD(void, setState, (ViewerState state, const QString& message), (override));
    MOCK_METHOD(ViewerState, getState, (), (const, override));
    MOCK_METHOD(void, setPointSize, (float size), (override));
    MOCK_METHOD(void, setBackgroundColor, (const QColor& color), (override));
    MOCK_METHOD(void, setShowGrid, (bool show), (override));
    MOCK_METHOD(void, setShowAxes, (bool show), (override));
    MOCK_METHOD(void, setLODEnabled, (bool enabled), (override));
    MOCK_METHOD(bool, isLODEnabled, (), (const, override));
    MOCK_METHOD(void, setRenderWithColor, (bool enabled), (override));
    MOCK_METHOD(void, setRenderWithIntensity, (bool enabled), (override));
    MOCK_METHOD(bool, isRenderingWithColor, (), (const, override));
    MOCK_METHOD(bool, isRenderingWithIntensity, (), (const, override));
    MOCK_METHOD(void, setTopView, (), (override));
    MOCK_METHOD(void, setLeftView, (), (override));
    MOCK_METHOD(void, setRightView, (), (override));
    MOCK_METHOD(void, setBottomView, (), (override));
    MOCK_METHOD(void, setFrontView, (), (override));
    MOCK_METHOD(void, setBackView, (), (override));
    MOCK_METHOD(void, setIsometricView, (), (override));
    MOCK_METHOD(bool, hasData, (), (const, override));
    MOCK_METHOD(size_t, pointCount, (), (const, override));
    MOCK_METHOD(void, setMinPointSize, (float size), (override));
    MOCK_METHOD(void, setMaxPointSize, (float size), (override));
    MOCK_METHOD(void, setAttenuationEnabled, (bool enabled), (override));
    MOCK_METHOD(void, setAttenuationFactor, (float factor), (override));
    MOCK_METHOD(void, setSplattingEnabled, (bool enabled), (override));
    MOCK_METHOD(void, setLightingEnabled, (bool enabled), (override));
    MOCK_METHOD(void, setLightDirection, (const QVector3D& direction), (override));
    MOCK_METHOD(void, setLightColor, (const QColor& color), (override));
    MOCK_METHOD(void, setAmbientIntensity, (float intensity), (override));
    MOCK_METHOD(void, onLoadingStarted, (), (override));
    MOCK_METHOD(void, onLoadingProgress, (int percentage, const QString& stage), (override));
    MOCK_METHOD(void, onLoadingFinished, (bool success, const QString& message), (override));
    MOCK_METHOD(size_t, getMemoryUsage, (), (const, override));
    MOCK_METHOD(void, optimizeMemory, (), (override));
};

class MainPresenterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create QApplication if it doesn't exist
        if (!QApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = new QApplication(argc, argv);
        }
        
        mockView = std::make_unique<MockMainView>();
        mockViewer = std::make_unique<MockPointCloudViewer>();
        
        // Create presenter with mock view
        presenter = std::make_unique<MainPresenter>(mockView.get());
    }
    
    void TearDown() override
    {
        presenter.reset();
        mockViewer.reset();
        mockView.reset();
    }
    
    QApplication* app = nullptr;
    std::unique_ptr<MockMainView> mockView;
    std::unique_ptr<MockPointCloudViewer> mockViewer;
    std::unique_ptr<MainPresenter> presenter;
};

TEST_F(MainPresenterTest, ConstructorInitializesCorrectly)
{
    EXPECT_NE(presenter.get(), nullptr);
}

TEST_F(MainPresenterTest, InitializeCallsViewMethods)
{
    EXPECT_CALL(*mockView, setStatusReady()).Times(1);
    EXPECT_CALL(*mockView, updateWindowTitle()).Times(1);
    EXPECT_CALL(*mockView, enableProjectActions(false)).Times(1);
    EXPECT_CALL(*mockView, showProjectHub()).Times(1);
    
    presenter->initialize();
}

TEST_F(MainPresenterTest, HandleNewProjectShowsDialog)
{
    QString projectName = "Test Project";
    QString projectPath = "/test/path";
    
    EXPECT_CALL(*mockView, showCreateProjectDialog(testing::_, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgReferee<0>(projectName),
            testing::SetArgReferee<1>(projectPath),
            testing::Return(true)
        ));
    
    presenter->handleNewProject();
}

TEST_F(MainPresenterTest, HandleOpenProjectShowsDialog)
{
    EXPECT_CALL(*mockView, showOpenProjectDialog())
        .WillOnce(testing::Return("/test/project/path"));
    
    presenter->handleOpenProject();
}

TEST_F(MainPresenterTest, HandleViewControlsCallsViewer)
{
    EXPECT_CALL(*mockView, getViewer())
        .WillRepeatedly(testing::Return(mockViewer.get()));
    EXPECT_CALL(*mockViewer, setTopView()).Times(1);
    EXPECT_CALL(*mockView, setStatusViewChanged("Top")).Times(1);
    
    presenter->handleTopViewClicked();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
