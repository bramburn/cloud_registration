#include <QCoreApplication>
#include <QSignalSpy>

#include <memory>

#include "../src/MainPresenter.h"
#include "mocks/MockE57Parser.h"
#include "mocks/MockE57Writer.h"
#include "mocks/MockMainView.h"
#include "mocks/MockPointCloudLoadManager.h"
#include "mocks/MockPointCloudViewer.h"
#include "mocks/MockProjectManager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;

/**
 * @brief Unit tests for MainPresenter class
 *
 * Tests Sprint 5 requirements:
 * - MainPresenter logic with mock dependencies
 * - File opening and error handling
 * - UI interaction patterns
 * - Component coordination
 *
 * These tests run without requiring access to the file system or a live OpenGL context.
 */
class MainPresenterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create mock objects
        m_mockView = std::make_unique<NiceMock<MockMainView>>();
        m_mockParser = std::make_unique<NiceMock<MockE57Parser>>();
        m_mockWriter = std::make_unique<NiceMock<MockE57Writer>>();

        // Get raw pointers for presenter construction (presenter doesn't own these)
        m_view = m_mockView.get();
        m_parser = m_mockParser.get();
        m_writer = m_mockWriter.get();
        m_viewer = m_mockView->getMockViewer();

        // Create presenter with mock dependencies
        m_presenter = std::make_unique<MainPresenter>(m_view, m_parser, m_writer);

        // Initialize presenter (sets up connections)
        m_presenter->initialize();
    }

    void TearDown() override
    {
        m_presenter.reset();
        m_mockWriter.reset();
        m_mockParser.reset();
        m_mockView.reset();
    }

protected:
    // Mock objects (owned by test)
    std::unique_ptr<MockMainView> m_mockView;
    std::unique_ptr<MockE57Parser> m_mockParser;
    std::unique_ptr<MockE57Writer> m_mockWriter;

    // Raw pointers for convenience (not owned)
    MockMainView* m_view;
    MockE57Parser* m_parser;
    MockE57Writer* m_writer;
    MockPointCloudViewer* m_viewer;

    // Presenter under test
    std::unique_ptr<MainPresenter> m_presenter;

    // Test data
    const QString m_testFilePath = "test_file.e57";
    const QString m_testProjectPath = "test_project.crp";
};

// ============================================================================
// Test Case 1: MainPresenter Logic Test with Mocks (from Sprint 5 documentation)
// ============================================================================

TEST_F(MainPresenterTest, HandleOpenFileSuccess)
{
    // Arrange: Set up successful parsing scenario
    std::vector<float> testPoints = MockE57Parser::createTestPointData(100);
    m_parser->setupSuccessfulParsing(testPoints);
    m_viewer->setupEmptyViewer();

    // Set expectations
    EXPECT_CALL(*m_parser, openFile(m_testFilePath.toStdString())).Times(1).WillOnce(Return(true));

    EXPECT_CALL(*m_parser, extractPointData(_)).Times(1).WillOnce(Return(testPoints));

    EXPECT_CALL(*m_viewer, loadPointCloud(testPoints)).Times(1);

    EXPECT_CALL(*m_viewer, resetCamera()).Times(1);

    m_view->verifyProgressDialogShown(true);
    m_view->verifyActionsEnabled(false);  // Should disable during processing
    m_view->verifyStatusUpdated();
    m_view->verifyInfoDisplayed();

    // Act: Handle file opening
    m_presenter->handleOpenFile(m_testFilePath);

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleOpenFileFailure)
{
    // Arrange: Set up failed parsing scenario
    const QString errorMessage = "Failed to open E57 file";
    m_parser->setupFailedParsing(errorMessage);

    // Set expectations
    EXPECT_CALL(*m_parser, openFile(m_testFilePath.toStdString())).Times(1).WillOnce(Return(false));

    EXPECT_CALL(*m_parser, getLastError()).Times(AtLeast(1)).WillRepeatedly(Return(errorMessage));

    // Viewer should NOT be called for failed parsing
    EXPECT_CALL(*m_viewer, loadPointCloud(_)).Times(0);

    m_view->verifyErrorDisplayed("File Opening Failed", errorMessage);
    m_view->verifyStatusUpdated("Failed to open file");

    // Act: Handle file opening
    m_presenter->handleOpenFile(m_testFilePath);

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleOpenFileInvalidPath)
{
    // Arrange: Test with invalid file path
    const QString invalidPath = "";

    m_view->verifyErrorDisplayed("Invalid File", "File path is empty.");

    // Parser should NOT be called for invalid path
    EXPECT_CALL(*m_parser, openFile(_)).Times(0);

    // Act: Handle invalid file path
    m_presenter->handleOpenFile(invalidPath);

    // Assert: Verify expectations are met (handled by Google Mock)
}

// ============================================================================
// Test Case 2: Project Management Tests
// ============================================================================

TEST_F(MainPresenterTest, HandleNewProject)
{
    // Arrange: Set up confirmation dialog
    m_view->setupConfirmationDialog(true);

    m_view->verifyInfoDisplayed();
    m_view->verifyWindowTitleSet();

    // Act: Handle new project creation
    m_presenter->handleNewProject();

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleOpenProject)
{
    // Arrange: Set up successful file dialog
    m_view->setupSuccessfulFileDialog(m_testProjectPath);

    EXPECT_CALL(*m_view, setProjectTitle(_)).Times(1);

    m_view->verifyProjectViewShown();
    m_view->verifyInfoDisplayed();
    m_view->verifyWindowTitleSet();

    // Act: Handle project opening
    m_presenter->handleOpenProject();

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleOpenProjectCancelled)
{
    // Arrange: Set up cancelled file dialog
    m_view->setupCancelledFileDialog();

    // No other methods should be called if dialog is cancelled
    EXPECT_CALL(*m_view, setProjectTitle(_)).Times(0);

    EXPECT_CALL(*m_view, showProjectView()).Times(0);

    // Act: Handle cancelled project opening
    m_presenter->handleOpenProject();

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleCloseProject)
{
    // Arrange: Simulate open project state
    // First open a project
    m_view->setupSuccessfulFileDialog(m_testProjectPath);
    m_presenter->handleOpenProject();

    // Now test closing
    m_view->verifyProjectHubShown();
    m_view->verifyStatusUpdated("Project closed");

    // Act: Handle project closing
    m_presenter->handleCloseProject();

    // Assert: Verify expectations are met (handled by Google Mock)
}

// ============================================================================
// Test Case 3: Import Scans Tests
// ============================================================================

TEST_F(MainPresenterTest, HandleImportScansWithoutProject)
{
    // Arrange: No project is open
    m_view->verifyErrorDisplayed("Import Scans", "Please open or create a project first.");

    // File dialog should NOT be shown
    EXPECT_CALL(*m_view, askForOpenFilePath(_, _)).Times(0);

    // Act: Handle import scans without project
    m_presenter->handleImportScans();

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleImportScansWithProject)
{
    // Arrange: Open a project first
    m_view->setupSuccessfulFileDialog(m_testProjectPath);
    m_presenter->handleOpenProject();

    // Set up file dialog for scan import
    m_view->setupSuccessfulFileDialog(m_testFilePath);

    // Set up successful parsing
    std::vector<float> testPoints = MockE57Parser::createTestPointData(50);
    m_parser->setupSuccessfulParsing(testPoints);

    m_view->verifyFileDialogCalled("Import E57 Scan", "E57 Files (*.e57)");

    // Act: Handle import scans with project
    m_presenter->handleImportScans();

    // Assert: Verify expectations are met (handled by Google Mock)
}

// ============================================================================
// Test Case 4: Scan Activation Tests
// ============================================================================

TEST_F(MainPresenterTest, HandleScanActivation)
{
    // Arrange: Set up file with scans
    std::vector<float> testPoints = MockE57Parser::createTestPointData(100);
    m_parser->setupSuccessfulParsing(testPoints);
    m_presenter->handleOpenFile(m_testFilePath);

    const QString scanId = "Scan_001";

    EXPECT_CALL(*m_view, highlightScan(scanId)).Times(1);

    m_view->verifyStatusUpdated("Activated scan: " + scanId);

    // Act: Handle scan activation
    m_presenter->handleScanActivation(scanId);

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleScanActivationWithoutFile)
{
    // Arrange: No file is open
    const QString scanId = "Scan_001";

    m_view->verifyErrorDisplayed("Scan Activation", "No file is currently open.");

    // Highlight should NOT be called
    EXPECT_CALL(*m_view, highlightScan(_)).Times(0);

    // Act: Handle scan activation without file
    m_presenter->handleScanActivation(scanId);

    // Assert: Verify expectations are met (handled by Google Mock)
}

// ============================================================================
// Test Case 5: Signal Handling Tests
// ============================================================================

TEST_F(MainPresenterTest, OnParsingProgressSignal)
{
    // Arrange: Set up progress tracking
    const int percentage = 50;
    const QString stage = "Reading point data";

    EXPECT_CALL(*m_view, updateProgress(percentage, stage)).Times(1);

    // Act: Simulate progress signal from parser
    m_parser->emitProgressUpdated(percentage, stage);

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, OnScanMetadataAvailableSignal)
{
    // Arrange: Set up scan metadata
    const int scanCount = 3;
    const QStringList scanNames = {"Scan_001", "Scan_002", "Scan_003"};

    EXPECT_CALL(*m_view, updateScanList(scanNames)).Times(1);

    m_view->verifyStatusUpdated();

    // Act: Simulate metadata signal from parser
    m_parser->emitScanMetadataAvailable(scanCount, scanNames);

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, OnViewerStatsUpdatedSignal)
{
    // Arrange: Set up rendering stats
    const float fps = 60.0f;
    const int visiblePoints = 1500;

    EXPECT_CALL(*m_view, updateRenderingStats(fps, visiblePoints)).Times(1);

    // Act: Simulate stats signal from viewer
    m_viewer->emitStatsUpdated(fps, visiblePoints);

    // Assert: Verify expectations are met (handled by Google Mock)
}

// ============================================================================
// Test Case 6: Exit Handling Tests
// ============================================================================

TEST_F(MainPresenterTest, HandleExitWithConfirmation)
{
    // Arrange: Open a file first
    std::vector<float> testPoints = MockE57Parser::createTestPointData(100);
    m_parser->setupSuccessfulParsing(testPoints);
    m_presenter->handleOpenFile(m_testFilePath);

    // Set up confirmation dialog
    m_view->setupConfirmationDialog(true);

    m_view->verifyConfirmationAsked("Exit Application",
                                    "Are you sure you want to exit? Any unsaved changes will be lost.");

    EXPECT_CALL(*m_parser, closeFile()).Times(1);

    EXPECT_CALL(*m_viewer, clearPointCloud()).Times(1);

    // Act: Handle exit request
    m_presenter->handleExit();

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleExitCancelled)
{
    // Arrange: Open a file first
    std::vector<float> testPoints = MockE57Parser::createTestPointData(100);
    m_parser->setupSuccessfulParsing(testPoints);
    m_presenter->handleOpenFile(m_testFilePath);

    // Set up cancelled confirmation dialog
    m_view->setupConfirmationDialog(false);

    // Cleanup should NOT happen if user cancels
    EXPECT_CALL(*m_parser, closeFile()).Times(0);

    EXPECT_CALL(*m_viewer, clearPointCloud()).Times(0);

    // Act: Handle cancelled exit request
    m_presenter->handleExit();

    // Assert: Verify expectations are met (handled by Google Mock)
}

// ============================================================================
// Test Case 7: Sprint 4 - Sidebar Integration Tests
// ============================================================================

TEST_F(MainPresenterTest, HandleClusterCreation)
{
    // Arrange: Set up project manager mock
    auto mockProjectManager = std::make_unique<MockProjectManager>();
    auto* projectManager = mockProjectManager.get();
    m_presenter->setProjectManager(projectManager);

    const QString clusterName = "Test Cluster";
    const QString parentClusterId = "parent-123";
    const QString newClusterId = "cluster-456";

    EXPECT_CALL(*projectManager, createCluster(clusterName, parentClusterId)).Times(1).WillOnce(Return(newClusterId));

    m_view->verifyInfoDisplayed("Cluster Creation", QString("Cluster '%1' created successfully.").arg(clusterName));
    m_view->verifyStatusUpdated(QString("Created cluster: %1").arg(clusterName));

    // Act: Handle cluster creation
    m_presenter->handleClusterCreation(clusterName, parentClusterId);

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleClusterCreationFailure)
{
    // Arrange: Set up project manager mock to return empty string (failure)
    auto mockProjectManager = std::make_unique<MockProjectManager>();
    auto* projectManager = mockProjectManager.get();
    m_presenter->setProjectManager(projectManager);

    const QString clusterName = "Test Cluster";
    const QString parentClusterId = "parent-123";

    EXPECT_CALL(*projectManager, createCluster(clusterName, parentClusterId))
        .Times(1)
        .WillOnce(Return(QString()));  // Return empty string to indicate failure

    m_view->verifyErrorDisplayed("Cluster Creation", "Failed to create cluster. Please try again.");

    // Act: Handle cluster creation
    m_presenter->handleClusterCreation(clusterName, parentClusterId);

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleClusterRename)
{
    // Arrange: Set up project manager mock
    auto mockProjectManager = std::make_unique<MockProjectManager>();
    auto* projectManager = mockProjectManager.get();
    m_presenter->setProjectManager(projectManager);

    const QString clusterId = "cluster-123";
    const QString newName = "Renamed Cluster";

    EXPECT_CALL(*projectManager, renameCluster(clusterId, newName)).Times(1).WillOnce(Return(true));

    m_view->verifyInfoDisplayed("Cluster Rename", QString("Cluster renamed to '%1' successfully.").arg(newName));
    m_view->verifyStatusUpdated(QString("Renamed cluster to: %1").arg(newName));

    // Act: Handle cluster rename
    m_presenter->handleClusterRename(clusterId, newName);

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleClusterDeletion)
{
    // Arrange: Set up project manager mock
    auto mockProjectManager = std::make_unique<MockProjectManager>();
    auto* projectManager = mockProjectManager.get();
    m_presenter->setProjectManager(projectManager);

    const QString clusterId = "cluster-123";
    const bool deletePhysicalFiles = false;

    // Set up confirmation dialog
    m_view->setupConfirmationDialog(true);

    EXPECT_CALL(*projectManager, deleteCluster(clusterId, deletePhysicalFiles)).Times(1).WillOnce(Return(true));

    m_view->verifyConfirmationAsked(
        "Delete Cluster", "Are you sure you want to delete this cluster? The physical files will be preserved.");
    m_view->verifyInfoDisplayed("Cluster Deletion", "Cluster deleted successfully.");
    m_view->verifyStatusUpdated("Cluster deleted");

    // Act: Handle cluster deletion
    m_presenter->handleClusterDeletion(clusterId, deletePhysicalFiles);

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleScanLoad)
{
    // Arrange: Set up load manager mock
    auto mockLoadManager = std::make_unique<MockPointCloudLoadManager>();
    auto* loadManager = mockLoadManager.get();
    m_presenter->setPointCloudLoadManager(loadManager);

    const QString scanId = "scan-123";

    EXPECT_CALL(*loadManager, loadScan(scanId)).Times(1).WillOnce(Return(true));

    m_view->verifyInfoDisplayed("Scan Load", "Scan loaded successfully.");
    m_view->verifyStatusUpdated(QString("Loaded scan: %1").arg(scanId));

    // Act: Handle scan load
    m_presenter->handleScanLoad(scanId);

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleScanLoadFailure)
{
    // Arrange: Set up load manager mock to fail
    auto mockLoadManager = std::make_unique<MockPointCloudLoadManager>();
    auto* loadManager = mockLoadManager.get();
    m_presenter->setPointCloudLoadManager(loadManager);

    const QString scanId = "scan-123";

    EXPECT_CALL(*loadManager, loadScan(scanId)).Times(1).WillOnce(Return(false));

    m_view->verifyErrorDisplayed("Scan Load", "Failed to load scan. Please try again.");

    // Act: Handle scan load
    m_presenter->handleScanLoad(scanId);

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleClusterLoad)
{
    // Arrange: Set up both managers
    auto mockProjectManager = std::make_unique<MockProjectManager>();
    auto mockLoadManager = std::make_unique<MockPointCloudLoadManager>();
    auto* projectManager = mockProjectManager.get();
    auto* loadManager = mockLoadManager.get();

    m_presenter->setProjectManager(projectManager);
    m_presenter->setPointCloudLoadManager(loadManager);

    const QString clusterId = "cluster-123";
    const QStringList scanIds = {"scan-1", "scan-2", "scan-3"};

    EXPECT_CALL(*projectManager, getScansInCluster(clusterId)).Times(1).WillOnce(Return(scanIds));

    // Expect load calls for each scan
    for (const QString& scanId : scanIds)
    {
        EXPECT_CALL(*loadManager, loadScan(scanId)).Times(1).WillOnce(Return(true));
    }

    m_view->verifyInfoDisplayed("Cluster Load", QString("Loaded %1 scans from cluster.").arg(scanIds.size()));
    m_view->verifyStatusUpdated(QString("Loaded %1 scans from cluster").arg(scanIds.size()));

    // Act: Handle cluster load
    m_presenter->handleClusterLoad(clusterId);

    // Assert: Verify expectations are met (handled by Google Mock)
}

TEST_F(MainPresenterTest, HandleDragDropOperation)
{
    // Arrange: Set up project manager mock
    auto mockProjectManager = std::make_unique<MockProjectManager>();
    auto* projectManager = mockProjectManager.get();
    m_presenter->setProjectManager(projectManager);

    const QStringList draggedItems = {"scan-1", "scan-2"};
    const QString draggedType = "scan";
    const QString targetItemId = "cluster-123";
    const QString targetType = "cluster";

    // Expect move calls for each scan
    for (const QString& scanId : draggedItems)
    {
        EXPECT_CALL(*projectManager, moveScanToCluster(scanId, targetItemId)).Times(1).WillOnce(Return(true));
    }

    m_view->verifyInfoDisplayed("Drag and Drop", QString("Moved %1 scan(s) successfully.").arg(draggedItems.size()));
    m_view->verifyStatusUpdated(QString("Moved %1 scan(s)").arg(draggedItems.size()));

    // Act: Handle drag and drop
    m_presenter->handleDragDropOperation(draggedItems, draggedType, targetItemId, targetType);

    // Assert: Verify expectations are met (handled by Google Mock)
}
