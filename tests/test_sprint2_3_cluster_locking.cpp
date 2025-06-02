#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QUuid>
#include <QDateTime>
#include "sqlitemanager.h"
#include "projectmanager.h"
#include "confirmationdialog.h"

class Sprint23Test : public ::testing::Test {
protected:
    void SetUp() override {
        m_tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(m_tempDir->isValid());
        
        m_sqliteManager = std::make_unique<SQLiteManager>();
        m_projectManager = std::make_unique<ProjectManager>();
        
        // Create test database
        QString dbPath = m_tempDir->filePath("test.sqlite");
        ASSERT_TRUE(m_sqliteManager->createDatabase(dbPath));
        ASSERT_TRUE(m_sqliteManager->initializeSchema());
    }

    void TearDown() override {
        m_sqliteManager.reset();
        m_projectManager.reset();
        m_tempDir.reset();
    }

    QString createTestCluster(const QString& name = "TestCluster", const QString& parentId = QString()) {
        ClusterInfo cluster;
        cluster.clusterId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        cluster.projectId = QUuid::createUuid().toString(QUuid::WithoutBraces);
        cluster.clusterName = name;
        cluster.parentClusterId = parentId;
        cluster.creationDate = QDateTime::currentDateTime().toString(Qt::ISODate);
        cluster.isLocked = false;
        
        EXPECT_TRUE(m_sqliteManager->insertCluster(cluster));
        return cluster.clusterId;
    }

    std::unique_ptr<QTemporaryDir> m_tempDir;
    std::unique_ptr<SQLiteManager> m_sqliteManager;
    std::unique_ptr<ProjectManager> m_projectManager;
};

// Test Case 1.1: Lock an unlocked cluster
TEST_F(Sprint23Test, LockUnlockedCluster) {
    QString clusterId = createTestCluster();
    
    // Verify initially unlocked
    EXPECT_FALSE(m_sqliteManager->getClusterLockState(clusterId));
    
    // Lock the cluster
    EXPECT_TRUE(m_sqliteManager->setClusterLockState(clusterId, true));
    
    // Verify lock state in database
    EXPECT_TRUE(m_sqliteManager->getClusterLockState(clusterId));
}

// Test Case 1.2: Unlock a locked cluster
TEST_F(Sprint23Test, UnlockLockedCluster) {
    QString clusterId = createTestCluster();
    
    // Lock the cluster first
    EXPECT_TRUE(m_sqliteManager->setClusterLockState(clusterId, true));
    EXPECT_TRUE(m_sqliteManager->getClusterLockState(clusterId));
    
    // Unlock the cluster
    EXPECT_TRUE(m_sqliteManager->setClusterLockState(clusterId, false));
    
    // Verify unlock state in database
    EXPECT_FALSE(m_sqliteManager->getClusterLockState(clusterId));
}

// Test Case 1.3: Lock and unlock multiple different clusters
TEST_F(Sprint23Test, LockUnlockMultipleClusters) {
    QString cluster1Id = createTestCluster("Cluster1");
    QString cluster2Id = createTestCluster("Cluster2");
    QString cluster3Id = createTestCluster("Cluster3");
    
    // Lock cluster1 and cluster3
    EXPECT_TRUE(m_sqliteManager->setClusterLockState(cluster1Id, true));
    EXPECT_TRUE(m_sqliteManager->setClusterLockState(cluster3Id, true));
    
    // Verify states are managed independently
    EXPECT_TRUE(m_sqliteManager->getClusterLockState(cluster1Id));
    EXPECT_FALSE(m_sqliteManager->getClusterLockState(cluster2Id));
    EXPECT_TRUE(m_sqliteManager->getClusterLockState(cluster3Id));
    
    // Unlock cluster1
    EXPECT_TRUE(m_sqliteManager->setClusterLockState(cluster1Id, false));
    
    // Verify other clusters unchanged
    EXPECT_FALSE(m_sqliteManager->getClusterLockState(cluster1Id));
    EXPECT_FALSE(m_sqliteManager->getClusterLockState(cluster2Id));
    EXPECT_TRUE(m_sqliteManager->getClusterLockState(cluster3Id));
}

// Test recursive cluster deletion
TEST_F(Sprint23Test, DeleteClusterRecursive) {
    QString parentId = createTestCluster("Parent");
    QString childId = createTestCluster("Child", parentId);
    QString grandchildId = createTestCluster("Grandchild", childId);
    
    // Verify clusters exist
    EXPECT_TRUE(m_sqliteManager->getClusterById(parentId).isValid());
    EXPECT_TRUE(m_sqliteManager->getClusterById(childId).isValid());
    EXPECT_TRUE(m_sqliteManager->getClusterById(grandchildId).isValid());
    
    // Delete parent recursively
    EXPECT_TRUE(m_sqliteManager->deleteClusterRecursive(parentId));
    
    // Verify all clusters are deleted
    EXPECT_FALSE(m_sqliteManager->getClusterById(parentId).isValid());
    EXPECT_FALSE(m_sqliteManager->getClusterById(childId).isValid());
    EXPECT_FALSE(m_sqliteManager->getClusterById(grandchildId).isValid());
}

// Test schema migration
TEST_F(Sprint23Test, SchemaMigration) {
    // The schema should already be at version 3 after setup
    int version = m_sqliteManager->getCurrentSchemaVersion();
    EXPECT_GE(version, 3);
    
    // Test that is_locked column exists and works
    QString clusterId = createTestCluster();
    EXPECT_TRUE(m_sqliteManager->setClusterLockState(clusterId, true));
    EXPECT_TRUE(m_sqliteManager->getClusterLockState(clusterId));
}

// Test confirmation dialog creation
TEST_F(Sprint23Test, ConfirmationDialogCreation) {
    ConfirmationDialog dialog("Test Title", "Test message");
    EXPECT_EQ(dialog.windowTitle(), "Test Title");
    
    // Test static confirm method
    // Note: This would normally show a dialog, but in tests we just verify it compiles
    // In a real test environment, you'd mock the dialog or test with a headless setup
}
