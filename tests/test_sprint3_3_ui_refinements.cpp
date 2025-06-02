#include <gtest/gtest.h>
#include <QApplication>
#include <QTest>
#include <QSignalSpy>
#include <QTimer>

#include "../src/iconmanager.h"
#include "../src/progressmanager.h"
#include "../src/projecttreemodel.h"

class Sprint33UIRefinementsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create QApplication if it doesn't exist
        if (!QApplication::instance()) {
            int argc = 1;
            char* argv[] = {const_cast<char*>("test")};
            app = new QApplication(argc, argv);
        }
    }

    void TearDown() override {
        // Don't delete QApplication here as it might be used by other tests
    }

    QApplication* app = nullptr;
};

// Test IconManager functionality
TEST_F(Sprint33UIRefinementsTest, IconManagerSingleton) {
    IconManager& manager1 = IconManager::instance();
    IconManager& manager2 = IconManager::instance();
    
    EXPECT_EQ(&manager1, &manager2);
}

TEST_F(Sprint33UIRefinementsTest, IconManagerBasicIcons) {
    IconManager& manager = IconManager::instance();
    
    // Test basic icon retrieval
    QIcon scanIcon = manager.getIcon(ItemType::Scan, ItemState::Unloaded);
    EXPECT_FALSE(scanIcon.isNull());
    
    QIcon clusterIcon = manager.getIcon(ItemType::Cluster, ItemState::Unloaded);
    EXPECT_FALSE(clusterIcon.isNull());
    
    QIcon projectIcon = manager.getIcon(ItemType::Project, ItemState::Unloaded);
    EXPECT_FALSE(projectIcon.isNull());
}

TEST_F(Sprint33UIRefinementsTest, IconManagerCompositeIcons) {
    IconManager& manager = IconManager::instance();
    
    // Test composite icons with different states and import types
    QIcon loadedScanIcon = manager.getCompositeIcon(ItemType::Scan, ItemState::Loaded, ImportType::Copy);
    EXPECT_FALSE(loadedScanIcon.isNull());
    
    QIcon lockedClusterIcon = manager.getCompositeIcon(ItemType::Cluster, ItemState::Locked, ImportType::None);
    EXPECT_FALSE(lockedClusterIcon.isNull());
    
    QIcon missingScanIcon = manager.getCompositeIcon(ItemType::Scan, ItemState::Missing, ImportType::Link);
    EXPECT_FALSE(missingScanIcon.isNull());
}

// Test ProgressManager functionality
TEST_F(Sprint33UIRefinementsTest, ProgressManagerSingleton) {
    ProgressManager& manager1 = ProgressManager::instance();
    ProgressManager& manager2 = ProgressManager::instance();
    
    EXPECT_EQ(&manager1, &manager2);
}

TEST_F(Sprint33UIRefinementsTest, ProgressManagerBasicOperations) {
    ProgressManager& manager = ProgressManager::instance();
    
    // Test starting an operation
    QString operationId = manager.startOperation(OperationType::ScanImport, "Test Import", 100, true);
    EXPECT_FALSE(operationId.isEmpty());
    
    // Test operation info
    ProgressInfo info = manager.getProgressInfo(operationId);
    EXPECT_EQ(info.operationName, "Test Import");
    EXPECT_EQ(info.type, OperationType::ScanImport);
    EXPECT_EQ(info.maxValue, 100);
    EXPECT_TRUE(info.isActive);
    EXPECT_TRUE(info.isCancellable);
    
    // Test progress update
    manager.updateProgress(operationId, 50, "Processing files");
    info = manager.getProgressInfo(operationId);
    EXPECT_EQ(info.currentValue, 50);
    EXPECT_EQ(info.currentStep, "Processing files");
    
    // Test finishing operation
    manager.finishOperation(operationId, "Import completed");
    
    // Wait a bit for cleanup
    QTest::qWait(1100);
    
    // Operation should be removed after cleanup
    info = manager.getProgressInfo(operationId);
    EXPECT_FALSE(info.isActive);
}

TEST_F(Sprint33UIRefinementsTest, ProgressManagerSignals) {
    ProgressManager& manager = ProgressManager::instance();
    
    // Set up signal spies
    QSignalSpy startedSpy(&manager, &ProgressManager::operationStarted);
    QSignalSpy updatedSpy(&manager, &ProgressManager::progressUpdated);
    QSignalSpy finishedSpy(&manager, &ProgressManager::operationFinished);
    
    // Start operation
    QString operationId = manager.startOperation(OperationType::ClusterLoad, "Test Load", 50);
    
    // Check started signal
    EXPECT_EQ(startedSpy.count(), 1);
    QList<QVariant> startedArgs = startedSpy.takeFirst();
    EXPECT_EQ(startedArgs.at(0).toString(), operationId);
    EXPECT_EQ(startedArgs.at(1).toString(), "Test Load");
    
    // Update progress
    manager.updateProgress(operationId, 25, "Loading data");
    
    // Check updated signal
    EXPECT_EQ(updatedSpy.count(), 1);
    QList<QVariant> updatedArgs = updatedSpy.takeFirst();
    EXPECT_EQ(updatedArgs.at(0).toString(), operationId);
    EXPECT_EQ(updatedArgs.at(1).toInt(), 25);
    EXPECT_EQ(updatedArgs.at(2).toInt(), 50);
    
    // Finish operation
    manager.finishOperation(operationId, "Load completed");
    
    // Check finished signal
    EXPECT_EQ(finishedSpy.count(), 1);
    QList<QVariant> finishedArgs = finishedSpy.takeFirst();
    EXPECT_EQ(finishedArgs.at(0).toString(), operationId);
    EXPECT_EQ(finishedArgs.at(1).toString(), "Load completed");
}

TEST_F(Sprint33UIRefinementsTest, ProgressManagerCancellation) {
    ProgressManager& manager = ProgressManager::instance();
    
    QSignalSpy cancelledSpy(&manager, &ProgressManager::operationCancelled);
    
    // Start cancellable operation
    QString operationId = manager.startOperation(OperationType::DataExport, "Test Export", 100, true);
    
    // Cancel operation
    manager.cancelOperation(operationId);
    
    // Check cancelled signal
    EXPECT_EQ(cancelledSpy.count(), 1);
    QList<QVariant> cancelledArgs = cancelledSpy.takeFirst();
    EXPECT_EQ(cancelledArgs.at(0).toString(), operationId);
    
    // Check operation is cancelled
    ProgressInfo info = manager.getProgressInfo(operationId);
    EXPECT_TRUE(info.isCancelled);
    EXPECT_FALSE(info.isActive);
}

TEST_F(Sprint33UIRefinementsTest, ProgressManagerTimeEstimation) {
    ProgressManager& manager = ProgressManager::instance();
    
    QString operationId = manager.startOperation(OperationType::ScanImport, "Test Time Estimation", 100);
    
    // Update progress to trigger time estimation
    manager.updateProgress(operationId, 10);
    QTest::qWait(100);
    manager.updateProgress(operationId, 20);
    
    // Get progress percentage
    int percentage = manager.getProgressPercentage(operationId);
    EXPECT_EQ(percentage, 20);
    
    // Test time formatting (should not crash)
    QString timeRemaining = manager.formatTimeRemaining(operationId);
    // Time remaining might be empty or "Calculating..." initially
    
    manager.finishOperation(operationId);
}

// Test ProjectTreeModel enhancements
TEST_F(Sprint33UIRefinementsTest, ProjectTreeModelCustomRoles) {
    ProjectTreeModel model;
    
    // Test that the model can be created without crashing
    EXPECT_EQ(model.rowCount(), 0);
    
    // Test header
    EXPECT_EQ(model.headerData(0, Qt::Horizontal, Qt::DisplayRole).toString(), "Project Structure");
}

// Integration test for icon and progress systems
TEST_F(Sprint33UIRefinementsTest, IntegrationTest) {
    IconManager& iconManager = IconManager::instance();
    ProgressManager& progressManager = ProgressManager::instance();
    
    // Test that both managers work together
    QIcon icon = iconManager.getIcon(ItemType::Scan, ItemState::Loading);
    EXPECT_FALSE(icon.isNull());
    
    QString opId = progressManager.startOperation(OperationType::ScanImport, "Integration Test", 10);
    EXPECT_FALSE(opId.isEmpty());
    
    progressManager.updateProgress(opId, 5);
    progressManager.finishOperation(opId, "Integration test completed");
    
    // Both systems should work independently
    QIcon anotherIcon = iconManager.getIcon(ItemType::Cluster, ItemState::Loaded);
    EXPECT_FALSE(anotherIcon.isNull());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
