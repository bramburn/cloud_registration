#include <QCoreApplication>
#include <QEventLoop>
#include <QSignalSpy>
#include <QTimer>
#include <QVector3D>
#include <QMatrix4x4>

#include <memory>
#include <vector>

// Core components (concrete implementations)
#include "app/MainPresenter.h"
#include "registration/RegistrationProject.h"
#include "registration/AlignmentEngine.h"
#include "registration/TargetManager.h"
#include "registration/NaturalPointSelector.h"
#include "registration/SphereDetector.h"
#include "app/pointcloudloadmanager.h"
#include "core/projectmanager.h"
#include "core/octree.h"
#include "interfaces/IPointCloudViewer.h"

// Mock interfaces
#include "../mocks/MockMainView.h"
#include "../mocks/MockE57Parser.h"
#include "../mocks/MockE57Writer.h"
#include "../mocks/MockPointCloudViewer.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::StrictMock;
using ::testing::DoAll;
using ::testing::Invoke;

/**
 * @brief End-to-End Integration Test for Manual Alignment MVP Workflow
 *
 * This test simulates the complete user journey for manual point cloud registration:
 * 1. Project creation
 * 2. Loading two scans
 * 3. Manual point selection for correspondences
 * 4. Alignment computation
 * 5. Accepting the alignment
 * 6. Exporting the final result
 *
 * Sprint 3.4 Requirements:
 * - Validates integration of all MVP components
 * - Uses mocks for external dependencies (UI, file I/O)
 * - Uses concrete implementations for core business logic
 * - Verifies correct data flow and state management
 */
class ManualAlignmentE2ETest : public ::testing::Test
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

        // Create mock components for external dependencies
        m_mockView = std::make_unique<NiceMock<MockMainView>>();
        m_mockParser = std::make_unique<NiceMock<MockE57Parser>>();
        m_mockWriter = std::make_unique<NiceMock<MockE57Writer>>();

        // Get raw pointers for convenience
        m_view = m_mockView.get();
        m_parser = m_mockParser.get();
        m_writer = m_mockWriter.get();
        m_viewer = m_view->getMockViewer();

        // Create concrete core components
        m_registrationProject = std::make_unique<RegistrationProject>();
        m_alignmentEngine = std::make_unique<AlignmentEngine>();
        m_targetManager = std::make_unique<TargetManager>();
        m_naturalPointSelector = std::make_unique<NaturalPointSelector>();
        m_sphereDetector = std::make_unique<SphereDetector>();
        m_loadManager = std::make_unique<PointCloudLoadManager>();
        m_projectManager = std::make_unique<ProjectManager>();

        // Set up default mock behaviors
        setupDefaultMockBehaviors();

        // Create presenter with dependency injection
        createPresenter();
    }

    void TearDown() override
    {
        m_presenter.reset();
        m_projectManager.reset();
        m_loadManager.reset();
        m_sphereDetector.reset();
        m_naturalPointSelector.reset();
        m_targetManager.reset();
        m_alignmentEngine.reset();
        m_registrationProject.reset();
        m_mockWriter.reset();
        m_mockParser.reset();
        m_mockView.reset();
    }

    void setupDefaultMockBehaviors()
    {
        // Default viewer behaviors
        ON_CALL(*m_viewer, hasData()).WillByDefault(Return(false));
        ON_CALL(*m_viewer, loadPointCloud(_)).WillByDefault(Return());
        ON_CALL(*m_viewer, clearPointCloud()).WillByDefault(Return());
        ON_CALL(*m_viewer, resetCamera()).WillByDefault(Return());
        ON_CALL(*m_viewer, getViewerState()).WillByDefault(Return(ViewerState::Empty));

        // Default parser behaviors for successful loading
        ON_CALL(*m_parser, isValidE57File(_)).WillByDefault(Return(true));
        ON_CALL(*m_parser, openFile(_)).WillByDefault(Return(true));
        ON_CALL(*m_parser, getPointCount(_)).WillByDefault(Return(1000));
        ON_CALL(*m_parser, getLastError()).WillByDefault(Return("No error"));

        // Default writer behaviors for successful export
        ON_CALL(*m_writer, createFile(_)).WillByDefault(Return(true));
        ON_CALL(*m_writer, writePoints(_, _)).WillByDefault(Return(true));
        ON_CALL(*m_writer, closeFile()).WillByDefault(Return(true));

        // Default view behaviors
        ON_CALL(*m_view, getViewer()).WillByDefault(Return(m_viewer));
        ON_CALL(*m_view, askForOpenFilePath(_, _)).WillByDefault(Return(QString()));
        ON_CALL(*m_view, askForSaveFilePath(_, _, _)).WillByDefault(Return(QString()));
        ON_CALL(*m_view, askForConfirmation(_, _)).WillByDefault(Return(true));
    }

    void createPresenter()
    {
        m_presenter = std::make_unique<MainPresenter>(
            m_view, m_parser, m_writer, m_projectManager.get(), m_loadManager.get());

        // Set up dependencies for alignment functionality
        m_presenter->setTargetManager(m_targetManager.get());
        m_presenter->setAlignmentEngine(m_alignmentEngine.get());

        m_presenter->initialize();
    }

    // Helper method to create test point cloud data
    std::vector<float> createTestPointData(const QString& scanName, size_t numPoints = 1000)
    {
        std::vector<float> points;
        points.reserve(numPoints * 3);

        // Create a simple point cloud with some offset based on scan name
        float offset = (scanName == "scan_A.e57") ? 0.0f : 1.0f;

        for (size_t i = 0; i < numPoints; ++i)
        {
            points.push_back(static_cast<float>(i % 10) + offset);      // x
            points.push_back(static_cast<float>((i / 10) % 10) + offset); // y
            points.push_back(static_cast<float>(i / 100) + offset);     // z
        }

        return points;
    }

    // Helper method to create test correspondences
    QList<QPair<QVector3D, QVector3D>> createTestCorrespondences()
    {
        QList<QPair<QVector3D, QVector3D>> correspondences;
        
        // Create 4 correspondence pairs for a robust transformation
        correspondences.append(qMakePair(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0f, 1.0f, 1.0f)));
        correspondences.append(qMakePair(QVector3D(1.0f, 0.0f, 0.0f), QVector3D(2.0f, 1.0f, 1.0f)));
        correspondences.append(qMakePair(QVector3D(0.0f, 1.0f, 0.0f), QVector3D(1.0f, 2.0f, 1.0f)));
        correspondences.append(qMakePair(QVector3D(0.0f, 0.0f, 1.0f), QVector3D(1.0f, 1.0f, 2.0f)));

        return correspondences;
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

    // Core components (concrete implementations)
    std::unique_ptr<MainPresenter> m_presenter;
    std::unique_ptr<RegistrationProject> m_registrationProject;
    std::unique_ptr<AlignmentEngine> m_alignmentEngine;
    std::unique_ptr<TargetManager> m_targetManager;
    std::unique_ptr<NaturalPointSelector> m_naturalPointSelector;
    std::unique_ptr<SphereDetector> m_sphereDetector;
    std::unique_ptr<PointCloudLoadManager> m_loadManager;
    std::unique_ptr<ProjectManager> m_projectManager;

    // Test data
    const QString m_testProjectPath = "test_project.crp";
    const QString m_testScanA = "scan_A.e57";
    const QString m_testScanB = "scan_B.e57";
    const QString m_testExportPath = "aligned_result.e57";
};

// ============================================================================
// Test Case: Full Manual Alignment Workflow Simulation
// ============================================================================

TEST_F(ManualAlignmentE2ETest, FullManualAlignmentWorkflow)
{
    // This test simulates the complete manual alignment workflow:
    // 1. Create new project
    // 2. Load scan A
    // 3. Load scan B
    // 4. Add manual correspondences
    // 5. Compute alignment
    // 6. Accept alignment
    // 7. Export result

    // Phase 1: Project Creation
    {
        EXPECT_CALL(*m_view, displayInfoMessage(testing::HasSubstr("New Project"), _)).Times(AtLeast(1));
        EXPECT_CALL(*m_view, setWindowTitle(_)).Times(AtLeast(1));
    }

    m_presenter->handleNewProject();
    // Note: handleNewProject() currently just shows info message, doesn't actually create project

    // Phase 2: Load Scan A
    std::vector<float> scanAData = createTestPointData(m_testScanA, 1000);

    {
        InSequence seq;

        EXPECT_CALL(*m_view, askForOpenFilePath(_, _)).Times(1).WillOnce(Return(m_testScanA));
        EXPECT_CALL(*m_parser, openFile(m_testScanA.toStdString())).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*m_parser, extractPointData()).Times(1).WillOnce(Return(scanAData));
        EXPECT_CALL(*m_viewer, loadPointCloud(scanAData)).Times(1);
        EXPECT_CALL(*m_viewer, resetCamera()).Times(1);
        EXPECT_CALL(*m_view, updateStatusBar(testing::HasSubstr("loaded"))).Times(1);
    }

    m_presenter->handleImportScans();
    EXPECT_TRUE(m_presenter->isFileOpen());

    // Phase 3: Load Scan B
    std::vector<float> scanBData = createTestPointData(m_testScanB, 1000);
    
    {
        InSequence seq;
        
        EXPECT_CALL(*m_view, askForOpenFilePath(_, _)).Times(1).WillOnce(Return(m_testScanB));
        EXPECT_CALL(*m_parser, openFile(m_testScanB.toStdString())).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*m_parser, extractPointData()).Times(1).WillOnce(Return(scanBData));
        EXPECT_CALL(*m_viewer, loadPointCloud(scanBData)).Times(1);
    }

    m_presenter->handleImportScans();

    // Phase 4: Manual Alignment (Point Selection)
    // Simulate adding correspondence points directly to alignment engine
    auto correspondences = createTestCorrespondences();
    
    for (const auto& correspondence : correspondences)
    {
        m_alignmentEngine->addCorrespondence(correspondence.first, correspondence.second);
    }

    EXPECT_EQ(m_alignmentEngine->getCorrespondences().size(), 4);

    // Phase 5: Compute Alignment
    {
        // Expect transformation update signal
        QSignalSpy transformationSpy(m_alignmentEngine.get(), &AlignmentEngine::transformationUpdated);
        QSignalSpy qualityMetricsSpy(m_alignmentEngine.get(), &AlignmentEngine::qualityMetricsUpdated);
    }

    m_alignmentEngine->recomputeAlignment();

    // Wait for async computation to complete
    QEventLoop loop;
    QTimer::singleShot(100, &loop, &QEventLoop::quit);
    loop.exec();

    const auto& result = m_alignmentEngine->getCurrentResult();
    EXPECT_TRUE(result.isValid());
    EXPECT_GT(result.errorStats.rmsError, 0.0f); // Should have computed some error

    // Phase 6: Accept Alignment
    {
        EXPECT_CALL(*m_view, updateStatusBar(testing::HasSubstr("accepted"))).Times(AtLeast(1));
    }

    m_presenter->handleAcceptAlignment();

    // Verify registration project state (if available)
    // Note: Registration project integration may not be fully implemented yet

    // Phase 7: Export
    {
        InSequence seq;

        EXPECT_CALL(*m_view, askForSaveFilePath(_, _, _)).Times(1).WillOnce(Return(m_testExportPath));
        EXPECT_CALL(*m_writer, createFile(m_testExportPath)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*m_writer, writePoints(_, _)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*m_writer, closeFile()).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*m_view, updateStatusBar(testing::HasSubstr("exported"))).Times(1);
    }

    m_presenter->handleExportPointCloud();

    // Final verification
    EXPECT_TRUE(m_presenter->isFileOpen());
    EXPECT_EQ(m_alignmentEngine->getCorrespondences().size(), 4);
    EXPECT_TRUE(m_alignmentEngine->getCurrentResult().isValid());
}
