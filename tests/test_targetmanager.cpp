#include <gtest/gtest.h>
#include <QVector3D>
#include <QTemporaryFile>
#include <memory>
#include "registration/TargetManager.h"
#include "registration/Target.h"
#include "registration/TargetCorrespondence.h"

class TargetManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        targetManager = std::make_unique<TargetManager>();
        
        // Create test targets
        sphereTarget1 = std::make_shared<SphereTarget>("sphere_001", QVector3D(1.0f, 2.0f, 3.0f), 0.15f);
        sphereTarget2 = std::make_shared<SphereTarget>("sphere_002", QVector3D(4.0f, 5.0f, 6.0f), 0.20f);
        naturalTarget1 = std::make_shared<NaturalPointTarget>("natural_001", QVector3D(7.0f, 8.0f, 9.0f), "Corner point");
        
        scanId1 = "scan_001";
        scanId2 = "scan_002";
    }

    std::unique_ptr<TargetManager> targetManager;
    std::shared_ptr<SphereTarget> sphereTarget1;
    std::shared_ptr<SphereTarget> sphereTarget2;
    std::shared_ptr<NaturalPointTarget> naturalTarget1;
    QString scanId1;
    QString scanId2;
};

// Test adding targets to scans
TEST_F(TargetManagerTest, AddTargetsToScans) {
    // Add targets to different scans
    EXPECT_TRUE(targetManager->addTarget(scanId1, sphereTarget1));
    EXPECT_TRUE(targetManager->addTarget(scanId1, naturalTarget1));
    EXPECT_TRUE(targetManager->addTarget(scanId2, sphereTarget2));
    
    // Check targets were added
    auto scan1Targets = targetManager->getTargetsForScan(scanId1);
    auto scan2Targets = targetManager->getTargetsForScan(scanId2);
    
    EXPECT_EQ(scan1Targets.size(), 2);
    EXPECT_EQ(scan2Targets.size(), 1);
    
    // Check specific targets
    EXPECT_EQ(scan1Targets[0]->targetId(), sphereTarget1->targetId());
    EXPECT_EQ(scan2Targets[0]->targetId(), sphereTarget2->targetId());
}

// Test retrieving targets by ID
TEST_F(TargetManagerTest, GetTargetById) {
    targetManager->addTarget(scanId1, sphereTarget1);
    
    auto retrievedTarget = targetManager->getTarget(sphereTarget1->targetId());
    EXPECT_NE(retrievedTarget, nullptr);
    EXPECT_EQ(retrievedTarget->targetId(), sphereTarget1->targetId());
    EXPECT_EQ(retrievedTarget->position(), sphereTarget1->position());
    
    // Test non-existent target
    auto nonExistentTarget = targetManager->getTarget("non_existent_id");
    EXPECT_EQ(nonExistentTarget, nullptr);
}

// Test retrieving targets by type
TEST_F(TargetManagerTest, GetTargetsByType) {
    targetManager->addTarget(scanId1, sphereTarget1);
    targetManager->addTarget(scanId1, sphereTarget2);
    targetManager->addTarget(scanId1, naturalTarget1);
    
    auto sphereTargets = targetManager->getTargetsByType("Sphere");
    auto naturalTargets = targetManager->getTargetsByType("Natural Point");
    auto checkerboardTargets = targetManager->getTargetsByType("Checkerboard");
    
    EXPECT_EQ(sphereTargets.size(), 2);
    EXPECT_EQ(naturalTargets.size(), 1);
    EXPECT_EQ(checkerboardTargets.size(), 0);
}

// Test removing targets
TEST_F(TargetManagerTest, RemoveTargets) {
    targetManager->addTarget(scanId1, sphereTarget1);
    targetManager->addTarget(scanId1, naturalTarget1);
    
    // Remove one target
    EXPECT_TRUE(targetManager->removeTarget(sphereTarget1->targetId()));

    auto remainingTargets = targetManager->getTargetsForScan(scanId1);
    EXPECT_EQ(remainingTargets.size(), 1);
    EXPECT_EQ(remainingTargets[0]->targetId(), naturalTarget1->targetId());
    
    // Try to remove non-existent target
    EXPECT_FALSE(targetManager->removeTarget("non_existent_id"));
}

// Test clearing targets for scan
TEST_F(TargetManagerTest, ClearTargetsForScan) {
    targetManager->addTarget(scanId1, sphereTarget1);
    targetManager->addTarget(scanId1, naturalTarget1);
    targetManager->addTarget(scanId2, sphereTarget2);
    
    targetManager->clearTargetsForScan(scanId1);
    
    auto scan1Targets = targetManager->getTargetsForScan(scanId1);
    auto scan2Targets = targetManager->getTargetsForScan(scanId2);
    
    EXPECT_EQ(scan1Targets.size(), 0);
    EXPECT_EQ(scan2Targets.size(), 1);  // scan2 targets should remain
}

// Test adding correspondences
TEST_F(TargetManagerTest, AddCorrespondences) {
    targetManager->addTarget(scanId1, sphereTarget1);
    targetManager->addTarget(scanId2, sphereTarget2);
    
    TargetCorrespondence correspondence(
        sphereTarget1->targetId(),
        sphereTarget2->targetId(),
        scanId1,
        scanId2
    );
    correspondence.setConfidence(0.85f);
    correspondence.setDistance(0.5f);

    EXPECT_TRUE(targetManager->addCorrespondence(correspondence));

    auto correspondences = targetManager->getAllCorrespondences();
    EXPECT_EQ(correspondences.size(), 1);
    EXPECT_EQ(correspondences[0].targetId1(), sphereTarget1->targetId());
    EXPECT_EQ(correspondences[0].targetId2(), sphereTarget2->targetId());
    EXPECT_FLOAT_EQ(correspondences[0].confidence(), 0.85f);
}

// Test correspondence validation
TEST_F(TargetManagerTest, CorrespondenceValidation) {
    targetManager->addTarget(scanId1, sphereTarget1);
    
    // Try to add correspondence with non-existent target
    TargetCorrespondence invalidCorr(
        sphereTarget1->targetId(),
        "non_existent_target",
        scanId1,
        scanId2
    );
    
    EXPECT_FALSE(targetManager->addCorrespondence(invalidCorr));
    
    // Try to add correspondence between targets in same scan
    targetManager->addTarget(scanId1, sphereTarget2);
    TargetCorrespondence sameScanCorr(
        sphereTarget1->targetId(),
        sphereTarget2->targetId(),
        scanId1,
        scanId1  // Same scan
    );
    
    EXPECT_FALSE(targetManager->addCorrespondence(sameScanCorr));
}

// Test getting correspondences for target
TEST_F(TargetManagerTest, GetCorrespondencesForTarget) {
    targetManager->addTarget(scanId1, sphereTarget1);
    targetManager->addTarget(scanId2, sphereTarget2);
    targetManager->addTarget(scanId2, naturalTarget1);
    
    // Add correspondences
    TargetCorrespondence corr1(sphereTarget1->targetId(), sphereTarget2->targetId(), scanId1, scanId2);
    TargetCorrespondence corr2(sphereTarget1->targetId(), naturalTarget1->targetId(), scanId1, scanId2);

    targetManager->addCorrespondence(corr1);
    targetManager->addCorrespondence(corr2);

    auto correspondences = targetManager->getCorrespondencesForTarget(sphereTarget1->targetId());
    EXPECT_EQ(correspondences.size(), 2);
}

// Test getting correspondences between scans
TEST_F(TargetManagerTest, GetCorrespondencesBetweenScans) {
    targetManager->addTarget(scanId1, sphereTarget1);
    targetManager->addTarget(scanId2, sphereTarget2);
    
    TargetCorrespondence correspondence(
        sphereTarget1->targetId(),
        sphereTarget2->targetId(),
        scanId1,
        scanId2
    );
    
    targetManager->addCorrespondence(correspondence);
    
    auto correspondences = targetManager->getCorrespondencesBetweenScans(scanId1, scanId2);
    EXPECT_EQ(correspondences.size(), 1);
    
    // Test reverse order
    auto reverseCorrespondences = targetManager->getCorrespondencesBetweenScans(scanId2, scanId1);
    EXPECT_EQ(reverseCorrespondences.size(), 1);
}

// Test statistics calculation
TEST_F(TargetManagerTest, CalculateStatistics) {
    targetManager->addTarget(scanId1, sphereTarget1);
    targetManager->addTarget(scanId1, sphereTarget2);
    targetManager->addTarget(scanId2, naturalTarget1);
    
    sphereTarget1->setConfidence(0.8f);
    sphereTarget2->setConfidence(0.9f);
    naturalTarget1->setConfidence(0.7f);

    TargetCorrespondence correspondence(
        sphereTarget1->targetId(),
        naturalTarget1->targetId(),
        scanId1,
        scanId2
    );
    targetManager->addCorrespondence(correspondence);
    
    auto stats = targetManager->getStatistics();
    
    EXPECT_EQ(stats.totalTargets, 3);
    EXPECT_EQ(stats.sphereTargets, 2);
    EXPECT_EQ(stats.naturalPointTargets, 1);
    EXPECT_EQ(stats.checkerboardTargets, 0);
    EXPECT_EQ(stats.validTargets, 3);
    EXPECT_EQ(stats.correspondences, 1);
    EXPECT_FLOAT_EQ(stats.averageQuality, (0.8f + 0.9f + 0.7f) / 3.0f);
}

// Test finding potential correspondences
TEST_F(TargetManagerTest, FindPotentialCorrespondences) {
    // Create targets with similar positions
    auto closeTarget1 = std::make_shared<SphereTarget>("close_001", QVector3D(1.0f, 1.0f, 1.0f), 0.1f);
    auto closeTarget2 = std::make_shared<SphereTarget>("close_002", QVector3D(1.1f, 1.1f, 1.1f), 0.1f);
    
    targetManager->addTarget(scanId1, closeTarget1);
    targetManager->addTarget(scanId2, closeTarget2);
    
    auto potentialCorrespondences = targetManager->findPotentialCorrespondences(scanId1, scanId2, 0.5f);
    
    EXPECT_EQ(potentialCorrespondences.size(), 1);
    EXPECT_EQ(potentialCorrespondences[0].targetId1(), closeTarget1->targetId());
    EXPECT_EQ(potentialCorrespondences[0].targetId2(), closeTarget2->targetId());
    EXPECT_GT(potentialCorrespondences[0].confidence(), 0.5f);
}

// Test serialization and deserialization
TEST_F(TargetManagerTest, SerializationDeserialization) {
    // Add test data
    targetManager->addTarget(scanId1, sphereTarget1);
    targetManager->addTarget(scanId2, sphereTarget2);
    
    TargetCorrespondence correspondence(
        sphereTarget1->targetId(),
        sphereTarget2->targetId(),
        scanId1,
        scanId2
    );
    targetManager->addCorrespondence(correspondence);
    
    // Serialize
    QVariantMap data = targetManager->serialize();
    
    // Create new manager and deserialize
    auto newManager = std::make_unique<TargetManager>();
    EXPECT_TRUE(newManager->deserialize(data));
    
    // Verify data
    auto allTargets = newManager->getAllTargets();
    auto allCorrespondences = newManager->getAllCorrespondences();
    
    EXPECT_EQ(allTargets.size(), 2);
    EXPECT_EQ(allCorrespondences.size(), 1);
}

// Test file save and load
TEST_F(TargetManagerTest, FileSaveLoad) {
    // Add test data
    targetManager->addTarget(scanId1, sphereTarget1);
    targetManager->addTarget(scanId2, naturalTarget1);
    
    // Create temporary file
    QTemporaryFile tempFile;
    EXPECT_TRUE(tempFile.open());
    QString fileName = tempFile.fileName();
    tempFile.close();
    
    // Save to file
    EXPECT_TRUE(targetManager->saveToFile(fileName));
    
    // Create new manager and load
    auto newManager = std::make_unique<TargetManager>();
    EXPECT_TRUE(newManager->loadFromFile(fileName));
    
    // Verify loaded data
    auto loadedTargets = newManager->getAllTargets();
    EXPECT_EQ(loadedTargets.size(), 2);
    
    auto scan1Targets = newManager->getTargetsForScan(scanId1);
    auto scan2Targets = newManager->getTargetsForScan(scanId2);
    EXPECT_EQ(scan1Targets.size(), 1);
    EXPECT_EQ(scan2Targets.size(), 1);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
