#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QSignalSpy>
#include <QEventLoop>
#include <QTimer>
#include "E57DataManager.h"

class E57DataManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize Qt application if not already done
        if (!QCoreApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = new QCoreApplication(argc, argv);
        }
        
        manager = std::make_unique<E57DataManager>();
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
    }
    
    void TearDown() override {
        manager.reset();
        tempDir.reset();
    }
    
    // Helper method to create test point data
    QVector<PointData> createTestPoints(size_t count, bool withColor = true, bool withIntensity = true) {
        QVector<PointData> points;
        points.reserve(count);
        
        for (size_t i = 0; i < count; ++i) {
            double x = static_cast<double>(i) * 0.1;
            double y = static_cast<double>(i) * 0.2;
            double z = static_cast<double>(i) * 0.3;
            
            PointData point(x, y, z);
            
            if (withColor) {
                point.r = static_cast<uint8_t>(i % 256);
                point.g = static_cast<uint8_t>((i * 2) % 256);
                point.b = static_cast<uint8_t>((i * 3) % 256);
                point.hasColor = true;
            }
            
            if (withIntensity) {
                point.intensity = static_cast<float>(i % 100) / 100.0f;
                point.hasIntensity = true;
            }
            
            points.append(point);
        }
        
        return points;
    }
    
    // Helper method to wait for signals with timeout
    bool waitForSignal(QObject* sender, const char* signal, int timeoutMs = 5000) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        timer.setInterval(timeoutMs);
        
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        QObject::connect(sender, signal, &loop, SLOT(quit()));
        
        timer.start();
        loop.exec();
        
        return timer.isActive(); // Returns true if signal was received before timeout
    }

protected:
    std::unique_ptr<E57DataManager> manager;
    std::unique_ptr<QTemporaryDir> tempDir;
    QCoreApplication* app = nullptr;
};

TEST_F(E57DataManagerTest, ConstructorDestructor) {
    // Test that the manager can be created and destroyed without issues
    EXPECT_NE(manager.get(), nullptr);
    EXPECT_TRUE(manager->getLastError().isEmpty());
}

TEST_F(E57DataManagerTest, PointDataStructure) {
    // Test PointData structure functionality
    PointData point1;
    EXPECT_EQ(point1.x, 0.0);
    EXPECT_EQ(point1.y, 0.0);
    EXPECT_EQ(point1.z, 0.0);
    EXPECT_FALSE(point1.hasColor);
    EXPECT_FALSE(point1.hasIntensity);
    
    PointData point2(1.0, 2.0, 3.0);
    EXPECT_EQ(point2.x, 1.0);
    EXPECT_EQ(point2.y, 2.0);
    EXPECT_EQ(point2.z, 3.0);
    EXPECT_FALSE(point2.hasColor);
    EXPECT_FALSE(point2.hasIntensity);
    
    PointData point3(1.0, 2.0, 3.0, 255, 128, 64);
    EXPECT_EQ(point3.x, 1.0);
    EXPECT_EQ(point3.y, 2.0);
    EXPECT_EQ(point3.z, 3.0);
    EXPECT_EQ(point3.r, 255);
    EXPECT_EQ(point3.g, 128);
    EXPECT_EQ(point3.b, 64);
    EXPECT_TRUE(point3.hasColor);
    EXPECT_FALSE(point3.hasIntensity);
    
    PointData point4(1.0, 2.0, 3.0, 0.75f);
    EXPECT_EQ(point4.x, 1.0);
    EXPECT_EQ(point4.y, 2.0);
    EXPECT_EQ(point4.z, 3.0);
    EXPECT_FLOAT_EQ(point4.intensity, 0.75f);
    EXPECT_FALSE(point4.hasColor);
    EXPECT_TRUE(point4.hasIntensity);
    
    PointData point5(1.0, 2.0, 3.0, 255, 128, 64, 0.5f);
    EXPECT_EQ(point5.x, 1.0);
    EXPECT_EQ(point5.y, 2.0);
    EXPECT_EQ(point5.z, 3.0);
    EXPECT_EQ(point5.r, 255);
    EXPECT_EQ(point5.g, 128);
    EXPECT_EQ(point5.b, 64);
    EXPECT_FLOAT_EQ(point5.intensity, 0.5f);
    EXPECT_TRUE(point5.hasColor);
    EXPECT_TRUE(point5.hasIntensity);
}

TEST_F(E57DataManagerTest, InvalidFileHandling) {
    // Test handling of non-existent files
    QString nonExistentFile = tempDir->filePath("nonexistent.e57");
    
    EXPECT_FALSE(manager->isValidE57File(nonExistentFile));
    
    // Test importing non-existent file
    EXPECT_THROW({
        manager->importE57File(nonExistentFile);
    }, E57Exception);
    
    // Test getting metadata from non-existent file
    EXPECT_THROW({
        manager->getScanMetadata(nonExistentFile);
    }, E57Exception);
}

TEST_F(E57DataManagerTest, ExportImportRoundTrip) {
    // Create test data
    QVector<QVector<PointData>> originalScans;
    
    // Scan 1: Points with color and intensity
    originalScans.append(createTestPoints(100, true, true));
    
    // Scan 2: Points with color only
    originalScans.append(createTestPoints(50, true, false));
    
    // Scan 3: Points with intensity only
    originalScans.append(createTestPoints(75, false, true));
    
    QString testFile = tempDir->filePath("test_roundtrip.e57");
    
    // Set up signal spies
    QSignalSpy progressSpy(manager.get(), &E57DataManager::progress);
    QSignalSpy operationStartedSpy(manager.get(), &E57DataManager::operationStarted);
    QSignalSpy operationCompletedSpy(manager.get(), &E57DataManager::operationCompleted);
    QSignalSpy errorSpy(manager.get(), &E57DataManager::errorOccurred);
    
    // Export the data
    EXPECT_NO_THROW({
        manager->exportE57File(testFile, originalScans);
    });
    
    // Check that export signals were emitted
    EXPECT_GT(operationStartedSpy.count(), 0);
    EXPECT_GT(operationCompletedSpy.count(), 0);
    EXPECT_GT(progressSpy.count(), 0);
    EXPECT_EQ(errorSpy.count(), 0);
    
    // Verify file was created
    EXPECT_TRUE(QFileInfo::exists(testFile));
    EXPECT_TRUE(manager->isValidE57File(testFile));
    
    // Clear signal spies
    progressSpy.clear();
    operationStartedSpy.clear();
    operationCompletedSpy.clear();
    errorSpy.clear();
    
    // Import the data back
    QVector<QVector<PointData>> importedScans;
    EXPECT_NO_THROW({
        importedScans = manager->importE57File(testFile);
    });
    
    // Check that import signals were emitted
    EXPECT_GT(operationStartedSpy.count(), 0);
    EXPECT_GT(operationCompletedSpy.count(), 0);
    EXPECT_GT(progressSpy.count(), 0);
    EXPECT_EQ(errorSpy.count(), 0);
    
    // Verify the imported data
    EXPECT_EQ(importedScans.size(), originalScans.size());
    
    for (int scanIndex = 0; scanIndex < originalScans.size(); ++scanIndex) {
        const auto& originalScan = originalScans[scanIndex];
        const auto& importedScan = importedScans[scanIndex];
        
        EXPECT_EQ(importedScan.size(), originalScan.size()) 
            << "Scan " << scanIndex << " point count mismatch";
        
        // Check a few sample points for accuracy
        for (int i = 0; i < std::min(10, static_cast<int>(originalScan.size())); ++i) {
            const auto& originalPoint = originalScan[i];
            const auto& importedPoint = importedScan[i];
            
            EXPECT_NEAR(importedPoint.x, originalPoint.x, 1e-6) 
                << "X coordinate mismatch at scan " << scanIndex << " point " << i;
            EXPECT_NEAR(importedPoint.y, originalPoint.y, 1e-6) 
                << "Y coordinate mismatch at scan " << scanIndex << " point " << i;
            EXPECT_NEAR(importedPoint.z, originalPoint.z, 1e-6) 
                << "Z coordinate mismatch at scan " << scanIndex << " point " << i;
            
            if (originalPoint.hasColor) {
                EXPECT_TRUE(importedPoint.hasColor) 
                    << "Color flag mismatch at scan " << scanIndex << " point " << i;
                EXPECT_EQ(importedPoint.r, originalPoint.r) 
                    << "Red component mismatch at scan " << scanIndex << " point " << i;
                EXPECT_EQ(importedPoint.g, originalPoint.g) 
                    << "Green component mismatch at scan " << scanIndex << " point " << i;
                EXPECT_EQ(importedPoint.b, originalPoint.b) 
                    << "Blue component mismatch at scan " << scanIndex << " point " << i;
            }
            
            if (originalPoint.hasIntensity) {
                EXPECT_TRUE(importedPoint.hasIntensity) 
                    << "Intensity flag mismatch at scan " << scanIndex << " point " << i;
                EXPECT_NEAR(importedPoint.intensity, originalPoint.intensity, 1e-6) 
                    << "Intensity mismatch at scan " << scanIndex << " point " << i;
            }
        }
    }
}

TEST_F(E57DataManagerTest, ScanMetadataExtraction) {
    // Create and export test data
    QVector<QVector<PointData>> scans;
    scans.append(createTestPoints(50, true, true));
    scans.append(createTestPoints(30, true, false));
    
    QString testFile = tempDir->filePath("test_metadata.e57");
    
    EXPECT_NO_THROW({
        manager->exportE57File(testFile, scans);
    });
    
    // Get metadata
    QVector<ScanMetadata> metadata;
    EXPECT_NO_THROW({
        metadata = manager->getScanMetadata(testFile);
    });
    
    // Verify metadata
    EXPECT_EQ(metadata.size(), scans.size());
    
    for (int i = 0; i < metadata.size(); ++i) {
        const auto& meta = metadata[i];
        EXPECT_EQ(meta.pointCount, static_cast<size_t>(scans[i].size()));
        EXPECT_FALSE(meta.name.isEmpty());
        EXPECT_FALSE(meta.guid.isEmpty());
    }
}

TEST_F(E57DataManagerTest, EmptyScansHandling) {
    // Test handling of empty scans
    QVector<QVector<PointData>> scans;
    scans.append(QVector<PointData>()); // Empty scan
    scans.append(createTestPoints(10, true, true)); // Non-empty scan
    scans.append(QVector<PointData>()); // Another empty scan
    
    QString testFile = tempDir->filePath("test_empty.e57");
    
    // Export should handle empty scans gracefully
    EXPECT_NO_THROW({
        manager->exportE57File(testFile, scans);
    });
    
    // Import should return only non-empty scans
    QVector<QVector<PointData>> importedScans;
    EXPECT_NO_THROW({
        importedScans = manager->importE57File(testFile);
    });
    
    // Should have only the non-empty scan
    EXPECT_EQ(importedScans.size(), 1);
    EXPECT_EQ(importedScans[0].size(), 10);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
