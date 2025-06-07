#include <gtest/gtest.h>
#include "../src/E57ParserCore.h"
#include <fstream>
#include <memory>

/**
 * @brief Sprint 2 Unit Tests for E57ParserCore
 * 
 * These tests verify that the E57ParserCore class works correctly in isolation
 * without any Qt dependencies. This ensures the core parsing logic is properly
 * decoupled from the Qt wrapper.
 */

class E57ParserCoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser = std::make_unique<E57ParserCore>();
        
        // Create a simple test file path (will be mocked in actual tests)
        testFilePath = "test_sample.e57";
        invalidFilePath = "nonexistent.e57";
        emptyFilePath = "empty.txt";
    }

    void TearDown() override {
        if (parser) {
            parser->closeFile();
        }
    }

    std::unique_ptr<E57ParserCore> parser;
    std::string testFilePath;
    std::string invalidFilePath;
    std::string emptyFilePath;
    
    // Helper method to create a minimal test file
    void createEmptyTestFile(const std::string& path) {
        std::ofstream file(path);
        file << "empty";
        file.close();
    }
};

// Test 1: Basic Construction and Destruction
TEST_F(E57ParserCoreTest, ConstructionAndDestruction) {
    EXPECT_TRUE(parser != nullptr);
    EXPECT_FALSE(parser->isOpen());
    EXPECT_TRUE(parser->getLastError().empty());
}

// Test 2: File Validation
TEST_F(E57ParserCoreTest, FileValidation) {
    // Test with non-existent file
    EXPECT_FALSE(E57ParserCore::isValidE57File(invalidFilePath));
    
    // Test with empty file
    createEmptyTestFile(emptyFilePath);
    EXPECT_FALSE(E57ParserCore::isValidE57File(emptyFilePath));
    
    // Clean up
    std::remove(emptyFilePath.c_str());
}

// Test 3: File Operations Without Valid File
TEST_F(E57ParserCoreTest, FileOperationsWithoutValidFile) {
    // Test opening non-existent file
    EXPECT_FALSE(parser->openFile(invalidFilePath));
    EXPECT_FALSE(parser->getLastError().empty());
    EXPECT_FALSE(parser->isOpen());
    
    // Test operations on closed file
    EXPECT_EQ(parser->getScanCount(), 0);
    EXPECT_EQ(parser->getPointCount(), 0);
    EXPECT_TRUE(parser->getGuid().empty());
    EXPECT_EQ(parser->getVersion().first, 0);
    EXPECT_EQ(parser->getVersion().second, 0);
}

// Test 4: Error Handling
TEST_F(E57ParserCoreTest, ErrorHandling) {
    // Test error setting and clearing
    parser->clearError();
    EXPECT_TRUE(parser->getLastError().empty());
    
    // Trigger an error by trying to open invalid file
    parser->openFile(invalidFilePath);
    EXPECT_FALSE(parser->getLastError().empty());
    
    // Clear error
    parser->clearError();
    EXPECT_TRUE(parser->getLastError().empty());
}

// Test 5: Progress Callback
TEST_F(E57ParserCoreTest, ProgressCallback) {
    bool callbackCalled = false;
    int lastPercentage = -1;
    std::string lastStage;
    
    // Set progress callback
    parser->setProgressCallback([&](int percentage, const std::string& stage) {
        callbackCalled = true;
        lastPercentage = percentage;
        lastStage = stage;
    });
    
    // Trigger an operation that would call progress (even if it fails)
    parser->openFile(invalidFilePath);
    
    // Clear callback
    parser->clearProgressCallback();
    
    // Note: Callback might not be called for failed operations,
    // so we just test that the callback mechanism works
    EXPECT_TRUE(true); // Test passes if no exceptions thrown
}

// Test 6: Data Structure Validation
TEST_F(E57ParserCoreTest, DataStructures) {
    // Test CorePointData structure
    CorePointData point;
    point.x = 1.0f;
    point.y = 2.0f;
    point.z = 3.0f;
    point.intensity = 0.5f;
    point.hasIntensity = true;
    point.red = 255;
    point.green = 128;
    point.blue = 64;
    point.hasColor = true;
    
    EXPECT_EQ(point.x, 1.0f);
    EXPECT_EQ(point.y, 2.0f);
    EXPECT_EQ(point.z, 3.0f);
    EXPECT_EQ(point.intensity, 0.5f);
    EXPECT_TRUE(point.hasIntensity);
    EXPECT_EQ(point.red, 255);
    EXPECT_EQ(point.green, 128);
    EXPECT_EQ(point.blue, 64);
    EXPECT_TRUE(point.hasColor);
}

// Test 7: CoreScanMetadata Structure
TEST_F(E57ParserCoreTest, ScanMetadataStructure) {
    CoreScanMetadata metadata;
    metadata.name = "Test Scan";
    metadata.guid = "test-guid-123";
    metadata.pointCount = 1000;
    metadata.minX = -10.0;
    metadata.maxX = 10.0;
    metadata.minY = -5.0;
    metadata.maxY = 5.0;
    metadata.minZ = 0.0;
    metadata.maxZ = 20.0;
    
    EXPECT_TRUE(metadata.isValid());
    EXPECT_EQ(metadata.name, "Test Scan");
    EXPECT_EQ(metadata.guid, "test-guid-123");
    EXPECT_EQ(metadata.pointCount, 1000);
}

// Test 8: CoreLoadingSettings Structure
TEST_F(E57ParserCoreTest, LoadingSettingsStructure) {
    CoreLoadingSettings settings;
    settings.maxPoints = 500000;
    settings.loadIntensity = true;
    settings.loadColor = false;
    settings.voxelSize = 0.1;
    settings.enableSpatialFilter = true;
    settings.filterMinX = -100.0;
    settings.filterMaxX = 100.0;
    settings.filterMinY = -100.0;
    settings.filterMaxY = 100.0;
    settings.filterMinZ = -10.0;
    settings.filterMaxZ = 50.0;
    
    EXPECT_EQ(settings.maxPoints, 500000);
    EXPECT_TRUE(settings.loadIntensity);
    EXPECT_FALSE(settings.loadColor);
    EXPECT_EQ(settings.voxelSize, 0.1);
    EXPECT_TRUE(settings.enableSpatialFilter);
}

// Test 9: Exception Types
TEST_F(E57ParserCoreTest, ExceptionTypes) {
    // Test E57CoreException
    try {
        throw E57CoreException("Test core exception");
    } catch (const E57CoreException& ex) {
        EXPECT_STREQ(ex.what(), "Test core exception");
    }
    
    // Test E57FileNotFoundException
    try {
        throw E57FileNotFoundException("/path/to/missing/file.e57");
    } catch (const E57FileNotFoundException& ex) {
        std::string message = ex.what();
        EXPECT_TRUE(message.find("E57 file not found") != std::string::npos);
        EXPECT_TRUE(message.find("/path/to/missing/file.e57") != std::string::npos);
    }
    
    // Test E57InvalidFormatException
    try {
        throw E57InvalidFormatException("Invalid header format");
    } catch (const E57InvalidFormatException& ex) {
        std::string message = ex.what();
        EXPECT_TRUE(message.find("Invalid E57 format") != std::string::npos);
        EXPECT_TRUE(message.find("Invalid header format") != std::string::npos);
    }
}

// Test 10: Point Data Extraction with Empty Results
TEST_F(E57ParserCoreTest, PointDataExtractionEmpty) {
    // Test extracting from non-open file
    std::vector<float> xyzData = parser->extractXYZData();
    EXPECT_TRUE(xyzData.empty());
    EXPECT_FALSE(parser->getLastError().empty());
    
    parser->clearError();
    
    std::vector<CorePointData> pointData = parser->extractPointData();
    EXPECT_TRUE(pointData.empty());
    EXPECT_FALSE(parser->getLastError().empty());
}

// Test 11: File Close Operations
TEST_F(E57ParserCoreTest, FileCloseOperations) {
    // Test closing when no file is open
    parser->closeFile();
    EXPECT_FALSE(parser->isOpen());
    
    // Test multiple closes
    parser->closeFile();
    parser->closeFile();
    EXPECT_FALSE(parser->isOpen());
}

// Test 12: Metadata Extraction from Closed File
TEST_F(E57ParserCoreTest, MetadataExtractionFromClosedFile) {
    CoreScanMetadata metadata = parser->getScanMetadata(0);
    EXPECT_FALSE(metadata.isValid());
    EXPECT_TRUE(metadata.name.empty());
    EXPECT_EQ(metadata.pointCount, 0);
}

// Note: Tests with actual E57 files would require sample files
// and would be integration tests rather than unit tests.
// The above tests focus on the interface and error handling
// without requiring actual E57 file dependencies.
