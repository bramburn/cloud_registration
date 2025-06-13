#include <QCoreApplication>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTimer>

#include <memory>

#include "../src/IE57Parser.h"
#include "../src/MainPresenter.h"
#include "../src/mainwindow.h"
#include "../src/pointcloudviewerwidget.h"
#include "../src/projectmanager.h"
#include "mocks/MockE57Parser.h"
#include "mocks/MockE57Writer.h"
#include "mocks/MockMainView.h"
#include "mocks/MockPointCloudViewer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;

/**
 * @brief Integration tests for Sprint 5 - Core Component Decoupling
 *
 * These tests verify the correct interaction between the major components
 * of the application after the MVP refactoring. They simulate user workflows
 * and ensure that the presenter, view, and model components work together
 * as expected.
 *
 * Sprint 5 Requirements:
 * - End-to-end integration testing of all major user flows
 * - Validation of component interactions in MVP architecture
 * - Performance validation of integrated system
 * - Stability testing under various conditions
 */
class IntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize Qt application if not already done
        if (!QCoreApplication::instance())
        {
            int argc = 0;
            char** argv = nullptr;
            m_app = std::make_unique<QCoreApplication>(argc, argv);
        }

        // Create mock components for controlled testing
        m_mockView = std::make_unique<NiceMock<MockMainView>>();
        m_mockParser = std::make_unique<NiceMock<MockE57Parser>>();
        m_mockWriter = std::make_unique<NiceMock<MockE57Writer>>();

        // Get raw pointers for convenience
        m_view = m_mockView.get();
        m_parser = m_mockParser.get();
        m_writer = m_mockWriter.get();
        m_viewer = m_view->getMockViewer();

        // Set up default mock behaviors
        setupDefaultMockBehaviors();
    }

    void TearDown() override
    {
        m_presenter.reset();
        m_mockWriter.reset();
        m_mockParser.reset();
        m_mockView.reset();
    }

    void setupDefaultMockBehaviors()
    {
        // Default viewer behaviors
        ON_CALL(*m_viewer, hasData()).WillByDefault(Return(false));
        ON_CALL(*m_viewer, getViewerState()).WillByDefault(Return(ViewerState::Empty));

        // Default parser behaviors
        ON_CALL(*m_parser, isFileOpen()).WillByDefault(Return(false));
        ON_CALL(*m_parser, getLastError()).WillByDefault(Return("No error"));

        // Default view behaviors
        ON_CALL(*m_view, getViewer()).WillByDefault(Return(m_viewer));
    }

    void createPresenter()
    {
        m_presenter = std::make_unique<MainPresenter>(m_view, m_parser, m_writer);
        m_presenter->initialize();
    }

protected:
    std::unique_ptr<QCoreApplication> m_app;

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
// Test Case 1: Full End-to-End Manual Test Simulation
// ============================================================================

TEST_F(IntegrationTest, FullWorkflowProjectCreationToFileLoading)
{
    // This test simulates the complete user workflow:
    // 1. Create new project
    // 2. Open E57 file
    // 3. Verify point cloud loading
    // 4. Check UI state updates

    createPresenter();

    // Phase 1: Project Creation
    {
        InSequence seq;

        EXPECT_CALL(*m_view, showProjectHub()).Times(1);
        EXPECT_CALL(*m_view, setWindowTitle(testing::HasSubstr("New Project"))).Times(1);
        EXPECT_CALL(*m_view, updateStatusBar(testing::HasSubstr("project created"))).Times(1);
    }

    // Simulate new project creation
    m_presenter->handleNewProject();

    // Phase 2: File Opening
    std::vector<float> testPoints = MockE57Parser::createTestPointData(1000);
    m_parser->setupSuccessfulParsing(testPoints);

    {
        InSequence seq;

        EXPECT_CALL(*m_parser, openFile(m_testFilePath.toStdString())).Times(1).WillOnce(Return(true));

        EXPECT_CALL(*m_parser, extractPointData()).Times(1).WillOnce(Return(testPoints));

        EXPECT_CALL(*m_viewer, loadPointCloud(testPoints)).Times(1);

        EXPECT_CALL(*m_viewer, resetCamera()).Times(1);

        EXPECT_CALL(*m_view, updateStatusBar(testing::HasSubstr("loaded successfully"))).Times(1);
    }

    // Simulate file opening
    m_presenter->handleOpenFile(m_testFilePath);

    // Phase 3: Verify final state
    EXPECT_TRUE(m_presenter->isFileOpen());
    EXPECT_EQ(m_presenter->getCurrentFilePath(), m_testFilePath);
}

// ============================================================================
// Test Case 2: Error Handling Integration
// ============================================================================

TEST_F(IntegrationTest, ErrorHandlingWorkflow)
{
    // Test error propagation through the MVP architecture

    createPresenter();

    // Setup parser to fail
    ON_CALL(*m_parser, openFile(_)).WillByDefault(Return(false));
    ON_CALL(*m_parser, getLastError()).WillByDefault(Return("File not found"));

    // Expect error to be displayed to user
    EXPECT_CALL(*m_view, displayErrorMessage(testing::HasSubstr("File Opening"), testing::HasSubstr("File not found")))
        .Times(1);

    EXPECT_CALL(*m_view, updateStatusBar(testing::HasSubstr("Failed"))).Times(1);

    // Attempt to open invalid file
    m_presenter->handleOpenFile("invalid_file.e57");

    // Verify presenter state remains consistent
    EXPECT_FALSE(m_presenter->isFileOpen());
    EXPECT_TRUE(m_presenter->getCurrentFilePath().isEmpty());
}

// ============================================================================
// Test Case 3: Component Interaction Validation
// ============================================================================

TEST_F(IntegrationTest, ComponentInteractionValidation)
{
    // Test that all components interact correctly through interfaces

    createPresenter();

    // Test view operations
    EXPECT_CALL(*m_view, getViewer()).Times(AtLeast(1));

    // Test camera controls
    EXPECT_CALL(*m_viewer, setTopView()).Times(1);
    EXPECT_CALL(*m_view, setStatusViewChanged("Top")).Times(1);

    m_presenter->handleTopViewClicked();

    // Test memory usage updates
    size_t testMemoryUsage = 1024 * 1024;  // 1MB
    EXPECT_CALL(*m_view, updateMemoryUsage(testMemoryUsage)).Times(1);

    m_presenter->onMemoryUsageChanged(testMemoryUsage);

    // Test rendering stats updates
    float testFPS = 60.0f;
    int testVisiblePoints = 50000;
    EXPECT_CALL(*m_view, updateRenderingStats(testFPS, testVisiblePoints)).Times(1);

    m_presenter->onRenderingStatsUpdated(testFPS, testVisiblePoints);
}

// ============================================================================
// Test Case 4: Performance Integration Test
// ============================================================================

TEST_F(IntegrationTest, PerformanceIntegrationTest)
{
    // Test performance characteristics of integrated system

    createPresenter();

    // Create large test dataset
    std::vector<float> largePointCloud = MockE57Parser::createTestPointData(100000);
    m_parser->setupSuccessfulParsing(largePointCloud);

    // Measure operation time
    QElapsedTimer timer;
    timer.start();

    // Setup expectations for large data handling
    EXPECT_CALL(*m_parser, openFile(_)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*m_parser, extractPointData()).Times(1).WillOnce(Return(largePointCloud));
    EXPECT_CALL(*m_viewer, loadPointCloud(_)).Times(1);

    // Perform operation
    m_presenter->handleOpenFile(m_testFilePath);

    qint64 elapsedMs = timer.elapsed();

    // Performance assertion - should complete within reasonable time
    EXPECT_LT(elapsedMs, 5000) << "Large point cloud loading took too long: " << elapsedMs << "ms";

    // Verify data integrity
    EXPECT_TRUE(m_presenter->isFileOpen());
}

// ============================================================================
// Test Case 5: State Consistency Validation
// ============================================================================

TEST_F(IntegrationTest, StateConsistencyValidation)
{
    // Test that application state remains consistent across operations

    createPresenter();

    // Initial state verification
    EXPECT_FALSE(m_presenter->isFileOpen());
    EXPECT_FALSE(m_presenter->isProjectOpen());
    EXPECT_TRUE(m_presenter->getCurrentFilePath().isEmpty());

    // Open project and verify state change
    m_presenter->handleNewProject();
    EXPECT_TRUE(m_presenter->isProjectOpen());

    // Open file and verify state change
    std::vector<float> testPoints = MockE57Parser::createTestPointData(100);
    m_parser->setupSuccessfulParsing(testPoints);

    ON_CALL(*m_parser, openFile(_)).WillByDefault(Return(true));
    ON_CALL(*m_parser, extractPointData()).WillByDefault(Return(testPoints));

    m_presenter->handleOpenFile(m_testFilePath);

    EXPECT_TRUE(m_presenter->isFileOpen());
    EXPECT_EQ(m_presenter->getCurrentFilePath(), m_testFilePath);

    // Close file and verify state reset
    m_presenter->handleCloseFile();

    EXPECT_FALSE(m_presenter->isFileOpen());
    EXPECT_TRUE(m_presenter->getCurrentFilePath().isEmpty());
}
