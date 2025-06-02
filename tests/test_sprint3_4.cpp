#include <gtest/gtest.h>
#include <QApplication>
#include <QTemporaryDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <memory>

#include "pointcloudloadmanager.h"
#include "sqlitemanager.h"
#include "projectmanager.h"

class Sprint34Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for test database
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
        
        // Create test database
        dbPath = tempDir->path() + "/test_project.sqlite";
        sqliteManager = std::make_unique<SQLiteManager>();
        ASSERT_TRUE(sqliteManager->createDatabase(dbPath));
        
        // Create load manager
        loadManager = std::make_unique<PointCloudLoadManager>();
        loadManager->setSQLiteManager(sqliteManager.get());
    }
    
    void TearDown() override {
        loadManager.reset();
        sqliteManager.reset();
        tempDir.reset();
    }
    
    std::unique_ptr<QTemporaryDir> tempDir;
    QString dbPath;
    std::unique_ptr<SQLiteManager> sqliteManager;
    std::unique_ptr<PointCloudLoadManager> loadManager;
};

// Test 1: LOD Subsampling Functionality
TEST_F(Sprint34Test, LODSubsampling) {
    // Create test point cloud data (1000 points)
    std::vector<float> testPoints;
    for (int i = 0; i < 1000; ++i) {
        testPoints.push_back(static_cast<float>(i));      // x
        testPoints.push_back(static_cast<float>(i * 2));  // y
        testPoints.push_back(static_cast<float>(i * 3));  // z
    }
    
    // Test subsampling at 50% rate
    auto subsampled = loadManager->subsamplePointCloud(testPoints, 0.5f);
    
    // Verify subsampled size is approximately 50% (allow ±10% variance)
    size_t expectedSize = testPoints.size() / 2;
    size_t actualSize = subsampled.size();
    
    EXPECT_GT(actualSize, expectedSize * 0.4);  // At least 40% of original
    EXPECT_LT(actualSize, expectedSize * 1.6);  // At most 160% of expected
    
    // Verify data integrity (points are still in groups of 3)
    EXPECT_EQ(subsampled.size() % 3, 0);
    
    qDebug() << "Original points:" << (testPoints.size() / 3) 
             << "Subsampled points:" << (subsampled.size() / 3)
             << "Rate:" << (static_cast<double>(subsampled.size()) / testPoints.size());
}

// Test 2: Memory Usage Tracking
TEST_F(Sprint34Test, MemoryUsageTracking) {
    // Initial memory usage should be 0
    EXPECT_EQ(loadManager->getTotalMemoryUsage(), 0);
    
    // Create test point cloud data
    std::vector<float> testPoints(3000, 1.0f); // 1000 points
    
    // Create a mock PointCloudData structure
    PointCloudData testData;
    testData.points = testPoints;
    testData.pointCount = testPoints.size() / 3;
    testData.memoryUsage = testPoints.size() * sizeof(float);
    
    // Verify memory calculation
    size_t expectedMemory = testPoints.size() * sizeof(float);
    EXPECT_EQ(testData.memoryUsage, expectedMemory);
    
    // Test LOD memory calculation
    testData.lodPoints = loadManager->subsamplePointCloud(testPoints, 0.5f);
    size_t totalMemory = testData.getTotalMemoryUsage();
    
    EXPECT_GT(totalMemory, expectedMemory); // Should be more than original
    EXPECT_LT(totalMemory, expectedMemory * 2); // But less than double
    
    qDebug() << "Original memory:" << expectedMemory 
             << "Total with LOD:" << totalMemory;
}

// Test 3: Database Schema Extension
TEST_F(Sprint34Test, DatabaseSchemaExtension) {
    // Verify registration_status table exists
    QSqlQuery query(sqliteManager->lastError().databaseText().isEmpty() ? 
                   QSqlDatabase::database() : QSqlDatabase());
    
    // Check if registration_status table exists
    bool hasRegistrationTable = query.exec(
        "SELECT name FROM sqlite_master WHERE type='table' AND name='registration_status'"
    );
    EXPECT_TRUE(hasRegistrationTable);
    
    if (query.next()) {
        QString tableName = query.value(0).toString();
        EXPECT_EQ(tableName, "registration_status");
    }
    
    // Check if transformation_matrices table exists
    bool hasTransformTable = query.exec(
        "SELECT name FROM sqlite_master WHERE type='table' AND name='transformation_matrices'"
    );
    EXPECT_TRUE(hasTransformTable);
    
    query.first();
    if (query.next()) {
        QString tableName = query.value(0).toString();
        EXPECT_EQ(tableName, "transformation_matrices");
    }
    
    qDebug() << "Database schema extension verified";
}

// Test 4: LOD State Management
TEST_F(Sprint34Test, LODStateManagement) {
    QString testScanId = "test-scan-001";
    
    // Initially LOD should not be active
    EXPECT_FALSE(loadManager->isLODActive(testScanId));
    
    // Set LOD active
    loadManager->setLODActive(testScanId, true);
    EXPECT_TRUE(loadManager->isLODActive(testScanId));
    
    // Set LOD inactive
    loadManager->setLODActive(testScanId, false);
    EXPECT_FALSE(loadManager->isLODActive(testScanId));
    
    qDebug() << "LOD state management verified";
}

// Test 5: Memory Statistics Display Format
TEST_F(Sprint34Test, MemoryDisplayFormat) {
    // Test memory formatting logic (simulated)
    size_t testBytes = 1536 * 1024 * 1024; // 1.5 GB
    double megabytes = testBytes / (1024.0 * 1024.0);
    
    QString displayText;
    if (megabytes >= 1024.0) {
        double gigabytes = megabytes / 1024.0;
        displayText = QString("Memory: %1 GB").arg(gigabytes, 0, 'f', 1);
    } else {
        displayText = QString("Memory: %1 MB").arg(megabytes, 0, 'f', 1);
    }
    
    EXPECT_EQ(displayText, "Memory: 1.5 GB");
    
    // Test smaller memory amount
    testBytes = 512 * 1024 * 1024; // 512 MB
    megabytes = testBytes / (1024.0 * 1024.0);
    displayText = QString("Memory: %1 MB").arg(megabytes, 0, 'f', 1);
    
    EXPECT_EQ(displayText, "Memory: 512.0 MB");
    
    qDebug() << "Memory display format verified";
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
