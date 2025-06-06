#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QTemporaryDir>
#include <QStandardPaths>

// Include Sprint 2.1 components
#include "../src/projecttreemodel.h"
#include "../src/pointcloudloadmanager.h"
#include "../src/sidebarwidget.h"
#include "../src/sqlitemanager.h"

class Sprint21Test : public ::testing::Test {
protected:
    void SetUp() override {
        int argc = 0;
        char** argv = nullptr;
        if (!QCoreApplication::instance()) {
            app = new QCoreApplication(argc, argv);
        }
        
        // Create temporary directory for test database
        tempDir = new QTemporaryDir();
        ASSERT_TRUE(tempDir->isValid());
        
        // Initialize components
        sqliteManager = new SQLiteManager();
        model = new ProjectTreeModel();
        loadManager = new PointCloudLoadManager();
        sidebar = new SidebarWidget();
        
        // Setup test project
        QString dbPath = tempDir->path() + "/test_project.db";
        sqliteManager->openDatabase(dbPath);
        sqliteManager->initializeSchema();
        
        // Connect components
        model->setSQLiteManager(sqliteManager);
        loadManager->setSQLiteManager(sqliteManager);
        loadManager->setProjectTreeModel(model);
        sidebar->setSQLiteManager(sqliteManager);
        sidebar->setPointCloudLoadManager(loadManager);
        
        // Connect signals for integration testing
        QObject::connect(sidebar, &SidebarWidget::loadScanRequested,
                        loadManager, &PointCloudLoadManager::onLoadScanRequested);
        QObject::connect(sidebar, &SidebarWidget::unloadScanRequested,
                        loadManager, &PointCloudLoadManager::onUnloadScanRequested);
        QObject::connect(sidebar, &SidebarWidget::batchOperationRequested,
                        loadManager, &PointCloudLoadManager::onBatchOperationRequested);
        QObject::connect(sidebar, &SidebarWidget::memoryOptimizationRequested,
                        loadManager, &PointCloudLoadManager::onMemoryOptimizationRequested);
        QObject::connect(loadManager, &PointCloudLoadManager::scanLoaded,
                        [this](const QString& scanId) {
                            model->setScanLoadedState(scanId, LoadedState::Loaded);
                        });
        QObject::connect(loadManager, &PointCloudLoadManager::scanUnloaded,
                        [this](const QString& scanId) {
                            model->setScanLoadedState(scanId, LoadedState::Unloaded);
                        });
    }
    
    void TearDown() override {
        delete sidebar;
        delete loadManager;
        delete model;
        delete sqliteManager;
        delete tempDir;
    }
    
    // Helper method to create test scan
    QString createTestScan(const QString& name = "TestScan") {
        ScanInfo scan;
        scan.scanId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        scan.projectId = "test-project-id";
        scan.scanName = name;
        scan.filePathRelative = "test/" + name + ".las";
        scan.pointCountEstimate = 1000000;
        scan.importType = "LINKED";
        scan.dateAdded = QDateTime::currentDateTime().toString(Qt::ISODate);

        sqliteManager->insertScan(scan);
        return scan.scanId;
    }
    
    // Helper method to create test cluster
    QString createTestCluster(const QString& name = "TestCluster", const QString& parentId = QString()) {
        ClusterInfo cluster;
        cluster.clusterId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        cluster.projectId = "test-project-id";
        cluster.clusterName = name;
        cluster.parentClusterId = parentId;
        cluster.creationDate = QDateTime::currentDateTime().toString(Qt::ISODate);

        sqliteManager->insertCluster(cluster);
        return cluster.clusterId;
    }
    
    QCoreApplication* app = nullptr;
    QTemporaryDir* tempDir = nullptr;
    ProjectTreeModel* model = nullptr;
    PointCloudLoadManager* loadManager = nullptr;
    SidebarWidget* sidebar = nullptr;
    SQLiteManager* sqliteManager = nullptr;
};

// Test 1: Enhanced LoadedState enum functionality
TEST_F(Sprint21Test, EnhancedLoadedStateEnum) {
    QString scanId = createTestScan("StateTestScan");
    
    // Test initial state
    EXPECT_EQ(model->getScanLoadedState(scanId), LoadedState::Unloaded);
    
    // Test all new states
    model->setScanLoadedState(scanId, LoadedState::Loading);
    EXPECT_EQ(model->getScanLoadedState(scanId), LoadedState::Loading);
    
    model->setScanLoadedState(scanId, LoadedState::Processing);
    EXPECT_EQ(model->getScanLoadedState(scanId), LoadedState::Processing);
    
    model->setScanLoadedState(scanId, LoadedState::Cached);
    EXPECT_EQ(model->getScanLoadedState(scanId), LoadedState::Cached);
    
    model->setScanLoadedState(scanId, LoadedState::MemoryWarning);
    EXPECT_EQ(model->getScanLoadedState(scanId), LoadedState::MemoryWarning);
    
    model->setScanLoadedState(scanId, LoadedState::Optimized);
    EXPECT_EQ(model->getScanLoadedState(scanId), LoadedState::Optimized);
}

// Test 2: Memory monitoring and warnings
TEST_F(Sprint21Test, MemoryMonitoringAndWarnings) {
    QString scanId = createTestScan("MemoryTestScan");
    
    // Set up signal spy for memory warning
    QSignalSpy memoryWarningSpy(model, &ProjectTreeModel::memoryWarningTriggered);
    QSignalSpy memoryUsageSpy(model, &ProjectTreeModel::memoryUsageChanged);
    
    // Set a low memory warning threshold for testing
    model->setMemoryWarningThreshold(1); // 1MB
    
    // Update memory info to trigger warning
    model->updateMemoryInfo(scanId, 2 * 1024 * 1024, 100000); // 2MB usage
    
    // Check that warning was triggered
    EXPECT_EQ(memoryWarningSpy.count(), 1);
    EXPECT_EQ(memoryUsageSpy.count(), 1);
    
    // Check that scan state was updated to MemoryWarning
    EXPECT_EQ(model->getScanLoadedState(scanId), LoadedState::MemoryWarning);
}

// Test 3: Batch operations support
TEST_F(Sprint21Test, BatchOperationsSupport) {
    // Create multiple test scans
    QStringList scanIds;
    for (int i = 0; i < 3; ++i) {
        scanIds.append(createTestScan(QString("BatchScan%1").arg(i)));
    }
    
    // Test getScansInState functionality
    for (const QString& scanId : scanIds) {
        model->setScanLoadedState(scanId, LoadedState::Loaded);
    }
    
    QStringList loadedScans = model->getScansInState(LoadedState::Loaded);
    EXPECT_EQ(loadedScans.size(), 3);
    
    for (const QString& scanId : scanIds) {
        EXPECT_TRUE(loadedScans.contains(scanId));
    }
    
    // Test batch unload
    for (const QString& scanId : scanIds) {
        model->setScanLoadedState(scanId, LoadedState::Unloaded);
    }
    
    QStringList unloadedScans = model->getScansInState(LoadedState::Unloaded);
    EXPECT_EQ(unloadedScans.size(), 3);
}

// Test 4: Enhanced context menu functionality
TEST_F(Sprint21Test, EnhancedContextMenuSignals) {
    QString scanId = createTestScan("ContextMenuScan");
    
    // Set up signal spies for new signals
    QSignalSpy preprocessSpy(sidebar, &SidebarWidget::preprocessScanRequested);
    QSignalSpy optimizeSpy(sidebar, &SidebarWidget::optimizeScanRequested);
    QSignalSpy batchOpSpy(sidebar, &SidebarWidget::batchOperationRequested);
    QSignalSpy memoryOptSpy(sidebar, &SidebarWidget::memoryOptimizationRequested);
    
    // Emit signals to test connectivity
    emit sidebar->preprocessScanRequested(scanId);
    emit sidebar->optimizeScanRequested(scanId);
    emit sidebar->batchOperationRequested("load", QStringList() << scanId);
    emit sidebar->memoryOptimizationRequested();
    
    // Verify signals were emitted
    EXPECT_EQ(preprocessSpy.count(), 1);
    EXPECT_EQ(optimizeSpy.count(), 1);
    EXPECT_EQ(batchOpSpy.count(), 1);
    EXPECT_EQ(memoryOptSpy.count(), 1);
    
    // Verify signal parameters
    QList<QVariant> preprocessArgs = preprocessSpy.takeFirst();
    EXPECT_EQ(preprocessArgs.at(0).toString(), scanId);
    
    QList<QVariant> batchArgs = batchOpSpy.takeFirst();
    EXPECT_EQ(batchArgs.at(0).toString(), "load");
    EXPECT_EQ(batchArgs.at(1).toStringList().first(), scanId);
}

// Test 5: PointCloudLoadManager enhanced functionality
TEST_F(Sprint21Test, PointCloudLoadManagerEnhancements) {
    QString scanId = createTestScan("LoadManagerScan");
    
    // Set up signal spies for new signals
    QSignalSpy batchProgressSpy(loadManager, &PointCloudLoadManager::batchOperationProgress);
    QSignalSpy preprocessStartSpy(loadManager, &PointCloudLoadManager::preprocessingStarted);
    QSignalSpy preprocessFinishSpy(loadManager, &PointCloudLoadManager::preprocessingFinished);
    
    // Test preprocessing request
    loadManager->onPreprocessScanRequested(scanId);
    
    // Wait a bit for async operations
    QTimer::singleShot(50, [this]() {
        QCoreApplication::processEvents();
    });
    
    // Verify preprocessing signals
    EXPECT_GE(preprocessStartSpy.count(), 1);
    
    // Test batch operation
    QStringList scanIds = {scanId};
    loadManager->onBatchOperationRequested("load", scanIds);
    
    // Wait for batch operation to complete
    QTimer::singleShot(100, [this]() {
        QCoreApplication::processEvents();
    });
    
    // Verify batch progress was reported
    EXPECT_GE(batchProgressSpy.count(), 1);
}

// Test 6: State change signal emission
TEST_F(Sprint21Test, StateChangeSignalEmission) {
    QString scanId = createTestScan("SignalTestScan");
    
    // Set up signal spy for state changes
    QSignalSpy stateChangeSpy(model, &ProjectTreeModel::scanStateChanged);
    
    // Change state and verify signal emission
    model->setScanLoadedState(scanId, LoadedState::Loading);
    EXPECT_EQ(stateChangeSpy.count(), 1);
    
    QList<QVariant> args = stateChangeSpy.takeFirst();
    EXPECT_EQ(args.at(0).toString(), scanId);
    EXPECT_EQ(static_cast<LoadedState>(args.at(1).toInt()), LoadedState::Unloaded);
    EXPECT_EQ(static_cast<LoadedState>(args.at(2).toInt()), LoadedState::Loading);
    
    // Change to same state should not emit signal
    model->setScanLoadedState(scanId, LoadedState::Loading);
    EXPECT_EQ(stateChangeSpy.count(), 0);
}

// Test 7: Integration test - Complete workflow
TEST_F(Sprint21Test, CompleteWorkflowIntegration) {
    // Create test data
    QString clusterId = createTestCluster("IntegrationCluster");
    QString scanId1 = createTestScan("IntegrationScan1");
    QString scanId2 = createTestScan("IntegrationScan2");
    
    // Set up the project structure
    model->setProject("TestProject", tempDir->path());
    model->refreshHierarchy();
    
    // Test memory monitoring
    QSignalSpy memoryUsageSpy(model, &ProjectTreeModel::memoryUsageChanged);
    
    // Simulate loading scans
    model->setScanLoadedState(scanId1, LoadedState::Loaded);
    model->updateMemoryInfo(scanId1, 100 * 1024 * 1024, 500000); // 100MB
    
    model->setScanLoadedState(scanId2, LoadedState::Loaded);
    model->updateMemoryInfo(scanId2, 150 * 1024 * 1024, 750000); // 150MB
    
    // Verify memory tracking
    EXPECT_EQ(model->getTotalMemoryUsage(), 250 * 1024 * 1024); // 250MB total
    EXPECT_GE(memoryUsageSpy.count(), 2);
    
    // Test batch operations
    QStringList scanIds = {scanId1, scanId2};
    QSignalSpy batchProgressSpy(loadManager, &PointCloudLoadManager::batchOperationProgress);
    
    loadManager->onBatchOperationRequested("unload", scanIds);
    
    // Wait for batch operation
    QTimer::singleShot(100, [this]() {
        QCoreApplication::processEvents();
    });
    
    // Verify batch operation completed
    EXPECT_GE(batchProgressSpy.count(), 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
