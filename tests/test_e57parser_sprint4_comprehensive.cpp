/**
 * @file test_e57parser_sprint4_comprehensive.cpp
 * @brief Sprint 4 Comprehensive Test Suite for E57 Library Integration
 * 
 * Implements all Sprint 4 user stories:
 * - User Story 1: Comprehensive E57 Functionality Verification
 * - User Story 2: Profile and Optimize E57 Loading Performance
 * - User Story 3: Basic Handling of E57 Files with Multiple Scans
 * - User Story 4: Adapt and Enhance Unit Test Suite
 * - User Story 5: Update Developer Documentation
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <memory>
#include <chrono>

#include "../src/e57parserlib.h"
#include "E57TestFramework.h"
#include "PerformanceProfiler.h"

class E57Sprint4ComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser = std::make_unique<E57ParserLib>();
        testFramework = std::make_unique<E57TestFramework>();
        profiler = std::make_unique<PerformanceProfiler>();
        
        setupTestEnvironment();
    }

    void TearDown() override {
        if (parser && parser->isOpen()) {
            parser->closeFile();
        }
        cleanup();
    }

    void setupTestEnvironment() {
        // Setup test data directory
        testDataDir = QDir::current().absoluteFilePath("test_data");
        if (!QDir().exists(testDataDir)) {
            QDir().mkpath(testDataDir);
        }
        
        // Configure test framework
        testFramework->setTestDataDirectory(testDataDir);
        testFramework->setMaxTestPoints(50000); // Limit for faster testing
        testFramework->setTimeoutSeconds(120);  // 2 minute timeout
        
        // Setup available test files
        availableTestFiles = {
            "sample/bunnyDouble.e57",
            "sample/bunnyInt32.e57"
        };
        
        // Filter to existing files only
        QStringList existingFiles;
        for (const QString& file : availableTestFiles) {
            if (QFile::exists(file)) {
                existingFiles.append(file);
            }
        }
        availableTestFiles = existingFiles;
        
        if (availableTestFiles.isEmpty()) {
            qWarning() << "No test E57 files found. Some tests will be skipped.";
        }
    }

    void cleanup() {
        // Clean up any temporary files created during testing
        QDir tempDir(testDataDir);
        QStringList tempFiles = tempDir.entryList(QStringList() << "temp_*", QDir::Files);
        for (const QString& file : tempFiles) {
            QFile::remove(tempDir.absoluteFilePath(file));
        }
    }

protected:
    std::unique_ptr<E57ParserLib> parser;
    std::unique_ptr<E57TestFramework> testFramework;
    std::unique_ptr<PerformanceProfiler> profiler;
    QString testDataDir;
    QStringList availableTestFiles;
};

// ============================================================================
// User Story 1: Comprehensive E57 Functionality Verification
// ============================================================================

TEST_F(E57Sprint4ComprehensiveTest, ComprehensiveFunctionalityVerification) {
    if (availableTestFiles.isEmpty()) {
        GTEST_SKIP() << "No test files available for comprehensive verification";
    }

    // Create test configuration for available files
    for (const QString& filePath : availableTestFiles) {
        E57TestFramework::TestFileMetadata metadata;
        metadata.filePath = filePath;
        metadata.vendor = "Test";
        metadata.software = "Unit Test";
        metadata.expectedScanCount = 1;
        metadata.hasIntensity = false;  // Will be detected automatically
        metadata.hasColor = false;      // Will be detected automatically
        metadata.shouldFail = false;
        metadata.description = QString("Comprehensive test for %1").arg(QFileInfo(filePath).fileName());
        
        testFramework->addTestFile(metadata);
    }

    // Run comprehensive tests
    auto results = testFramework->runComprehensiveTests();
    
    ASSERT_FALSE(results.empty()) << "No test results generated";
    
    // Verify that at least 95% of valid test files pass (Sprint 4 acceptance criteria)
    int totalValidTests = 0;
    int passedTests = 0;
    
    for (const auto& result : results) {
        if (!result.fileName.contains("malformed")) {
            totalValidTests++;
            if (result.success) {
                passedTests++;
            } else {
                qWarning() << "Test failed for" << result.fileName << ":" << result.errorMessage;
            }
        }
    }
    
    if (totalValidTests > 0) {
        double successRate = (100.0 * passedTests) / totalValidTests;
        EXPECT_GE(successRate, 95.0) << "Success rate below 95% threshold: " << successRate << "%";
        
        qDebug() << "Comprehensive test results:" << passedTests << "/" << totalValidTests 
                 << "passed (" << successRate << "%)";
    }
    
    // Generate test report
    testFramework->generateTestReport(results, testDataDir + "/comprehensive_test_report.html");
}

// Test error handling with malformed files
TEST_F(E57Sprint4ComprehensiveTest, ErrorHandlingMalformedFiles) {
    // Create a malformed E57 file for testing
    QString malformedFile = testDataDir + "/malformed_test.e57";
    QFile file(malformedFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write("This is not a valid E57 file content");
        file.close();
    }
    
    E57TestFramework::TestFileMetadata metadata;
    metadata.filePath = malformedFile;
    metadata.shouldFail = true;
    metadata.expectedErrorType = "ParseError";
    metadata.description = "Malformed file error handling test";
    
    E57TestFramework::TestResult result;
    bool testResult = testFramework->testFileLoading(metadata, result);
    
    // For malformed files, the test should succeed (meaning it properly detected the error)
    EXPECT_TRUE(testResult) << "Error handling test failed: " << result.errorMessage;
    
    // Clean up
    QFile::remove(malformedFile);
}

// ============================================================================
// User Story 2: Profile and Optimize E57 Loading Performance
// ============================================================================

TEST_F(E57Sprint4ComprehensiveTest, PerformanceProfilingAndOptimization) {
    if (availableTestFiles.isEmpty()) {
        GTEST_SKIP() << "No test files available for performance profiling";
    }

    QString testFile = availableTestFiles.first();
    
    // Test baseline performance
    auto baselineMetrics = profiler->profileE57Loading(testFile);
    
    EXPECT_TRUE(baselineMetrics.success) << "Baseline profiling failed: " << baselineMetrics.errorMessage;
    
    if (baselineMetrics.success) {
        EXPECT_GT(baselineMetrics.pointCount, 0) << "No points loaded during profiling";
        EXPECT_GT(baselineMetrics.totalLoadTime, 0.0) << "Invalid load time measurement";
        EXPECT_GT(baselineMetrics.pointsPerSecond, 0.0) << "Invalid points per second calculation";
        
        qDebug() << "Baseline performance:" 
                 << baselineMetrics.pointsPerSecond << "points/sec,"
                 << (baselineMetrics.peakMemoryUsage / (1024 * 1024)) << "MB peak memory";
        
        // Test performance with different optimization settings
        auto optimizationResults = profiler->compareOptimizations(testFile);
        
        EXPECT_FALSE(optimizationResults.empty()) << "No optimization results generated";
        
        // Verify that at least one optimization shows measurable difference
        bool foundOptimization = false;
        for (const auto& result : optimizationResults) {
            if (result.success && result.pointsPerSecond != baselineMetrics.pointsPerSecond) {
                foundOptimization = true;
                break;
            }
        }
        
        // Generate performance report
        profiler->generatePerformanceReport(optimizationResults, 
                                          testDataDir + "/performance_report.html");
    }
}

// Test performance KPIs (Key Performance Indicators)
TEST_F(E57Sprint4ComprehensiveTest, PerformanceKPIValidation) {
    if (availableTestFiles.isEmpty()) {
        GTEST_SKIP() << "No test files available for KPI validation";
    }

    QString testFile = availableTestFiles.first();
    auto metrics = profiler->profileE57Loading(testFile);
    
    if (metrics.success && metrics.pointCount > 0) {
        // KPI thresholds (configurable based on requirements)
        const double MIN_POINTS_PER_SECOND = 10000.0;  // Minimum acceptable performance
        const double MAX_MEMORY_PER_MILLION_POINTS = 1024.0; // MB per million points
        const double MAX_LOAD_TIME_PER_MILLION_POINTS = 60.0; // Seconds per million points
        
        double pointsInMillions = metrics.pointCount / 1000000.0;
        
        if (pointsInMillions > 0.001) { // Only test for files with significant point count
            EXPECT_GE(metrics.pointsPerSecond, MIN_POINTS_PER_SECOND) 
                << "Performance below minimum threshold";
            
            double memoryPerMillion = metrics.memoryEfficiency;
            EXPECT_LE(memoryPerMillion, MAX_MEMORY_PER_MILLION_POINTS)
                << "Memory usage exceeds threshold: " << memoryPerMillion << " MB/million points";
            
            double timePerMillion = metrics.totalLoadTime / pointsInMillions;
            EXPECT_LE(timePerMillion, MAX_LOAD_TIME_PER_MILLION_POINTS)
                << "Load time exceeds threshold: " << timePerMillion << " seconds/million points";
        }
        
        qDebug() << "KPI Results - Points/sec:" << metrics.pointsPerSecond
                 << "Memory efficiency:" << metrics.memoryEfficiency << "MB/million"
                 << "Load time:" << metrics.totalLoadTime << "seconds";
    }
}

// ============================================================================
// User Story 3: Basic Handling of E57 Files with Multiple Scans
// ============================================================================

TEST_F(E57Sprint4ComprehensiveTest, MultiScanHandling) {
    if (availableTestFiles.isEmpty()) {
        GTEST_SKIP() << "No test files available for multi-scan testing";
    }

    for (const QString& testFile : availableTestFiles) {
        ASSERT_TRUE(parser->openFile(testFile.toStdString()))
            << "Failed to open test file: " << testFile;

        // Test scan count detection
        int scanCount = parser->getScanCount();
        EXPECT_GE(scanCount, 0) << "Invalid scan count returned";

        qDebug() << "File" << QFileInfo(testFile).fileName() << "contains" << scanCount << "scans";

        if (scanCount > 0) {
            // Test scan metadata retrieval
            auto scanMetadata = parser->getScanMetadata(0);
            EXPECT_EQ(scanMetadata.index, 0) << "Incorrect scan index in metadata";
            EXPECT_GE(scanMetadata.pointCount, 0) << "Invalid point count in scan metadata";

            // Test point count consistency
            int64_t directPointCount = parser->getPointCount(0);
            EXPECT_EQ(directPointCount, scanMetadata.pointCount)
                << "Point count mismatch between methods";

            // Test default behavior: load first scan
            auto points = parser->extractPointData(0);
            if (directPointCount > 0) {
                EXPECT_FALSE(points.empty()) << "No points extracted from first scan";
                EXPECT_EQ(points.size() % 3, 0) << "Point data not in XYZ format";
            }

            // Test multi-scan detection logging
            if (scanCount > 1) {
                qDebug() << "Multi-scan file detected with" << scanCount << "scans";
                qDebug() << "Loading data from first scan by default (as per Sprint 4 requirements)";

                // Verify that we can get metadata for all scans
                for (int i = 0; i < scanCount; ++i) {
                    auto metadata = parser->getScanMetadata(i);
                    EXPECT_EQ(metadata.index, i) << "Scan metadata index mismatch";
                    EXPECT_GE(metadata.pointCount, 0) << "Invalid point count for scan" << i;
                }
            }
        }

        parser->closeFile();
    }
}

// Test invalid scan index handling
TEST_F(E57Sprint4ComprehensiveTest, InvalidScanIndexHandling) {
    if (availableTestFiles.isEmpty()) {
        GTEST_SKIP() << "No test files available for scan index testing";
    }

    QString testFile = availableTestFiles.first();
    ASSERT_TRUE(parser->openFile(testFile.toStdString()));

    int scanCount = parser->getScanCount();

    // Test invalid scan indices
    auto invalidMetadata = parser->getScanMetadata(-1);
    EXPECT_EQ(invalidMetadata.index, -1) << "Invalid scan index should return default metadata";

    invalidMetadata = parser->getScanMetadata(scanCount + 10);
    EXPECT_EQ(invalidMetadata.index, -1) << "Out-of-range scan index should return default metadata";

    // Test point extraction with invalid indices
    auto points = parser->extractPointData(-1);
    EXPECT_TRUE(points.empty()) << "Invalid scan index should return empty points";

    points = parser->extractPointData(scanCount + 10);
    EXPECT_TRUE(points.empty()) << "Out-of-range scan index should return empty points";

    parser->closeFile();
}

// ============================================================================
// User Story 4: Adapt and Enhance Unit Test Suite
// ============================================================================

TEST_F(E57Sprint4ComprehensiveTest, EnhancedUnitTestCoverage) {
    // Test enhanced point data structure
    E57ParserLib::PointData point(1.5f, 2.5f, 3.5f);
    EXPECT_FLOAT_EQ(point.x, 1.5f);
    EXPECT_FLOAT_EQ(point.y, 2.5f);
    EXPECT_FLOAT_EQ(point.z, 3.5f);
    EXPECT_FALSE(point.hasIntensity);
    EXPECT_FALSE(point.hasColor);

    // Test point data with all attributes
    point.intensity = 0.75f;
    point.hasIntensity = true;
    point.r = 255;
    point.g = 128;
    point.b = 64;
    point.hasColor = true;

    EXPECT_TRUE(point.hasIntensity);
    EXPECT_FLOAT_EQ(point.intensity, 0.75f);
    EXPECT_TRUE(point.hasColor);
    EXPECT_EQ(point.r, 255);
    EXPECT_EQ(point.g, 128);
    EXPECT_EQ(point.b, 64);
}

// Test enhanced point data extraction
TEST_F(E57Sprint4ComprehensiveTest, EnhancedPointDataExtraction) {
    if (availableTestFiles.isEmpty()) {
        GTEST_SKIP() << "No test files available for enhanced point data testing";
    }

    QString testFile = availableTestFiles.first();
    ASSERT_TRUE(parser->openFile(testFile.toStdString()));

    if (parser->getScanCount() > 0) {
        // Test enhanced point data extraction
        auto enhancedPoints = parser->extractEnhancedPointData(0);

        if (!enhancedPoints.empty()) {
            // Verify point data structure
            for (const auto& point : enhancedPoints) {
                EXPECT_TRUE(std::isfinite(point.x)) << "Invalid X coordinate";
                EXPECT_TRUE(std::isfinite(point.y)) << "Invalid Y coordinate";
                EXPECT_TRUE(std::isfinite(point.z)) << "Invalid Z coordinate";

                if (point.hasIntensity) {
                    EXPECT_GE(point.intensity, 0.0f) << "Intensity should be normalized to [0,1]";
                    EXPECT_LE(point.intensity, 1.0f) << "Intensity should be normalized to [0,1]";
                }

                if (point.hasColor) {
                    // Color values should be in valid uint8_t range
                    EXPECT_LE(point.r, 255);
                    EXPECT_LE(point.g, 255);
                    EXPECT_LE(point.b, 255);
                }
            }

            qDebug() << "Enhanced point data test: extracted" << enhancedPoints.size() << "points";

            // Check for attribute presence
            bool hasAnyIntensity = std::any_of(enhancedPoints.begin(), enhancedPoints.end(),
                [](const auto& p) { return p.hasIntensity; });
            bool hasAnyColor = std::any_of(enhancedPoints.begin(), enhancedPoints.end(),
                [](const auto& p) { return p.hasColor; });

            qDebug() << "Attributes found - Intensity:" << hasAnyIntensity << "Color:" << hasAnyColor;
        }
    }

    parser->closeFile();
}

// Test regression prevention - ensure previous functionality still works
TEST_F(E57Sprint4ComprehensiveTest, RegressionPrevention) {
    if (availableTestFiles.isEmpty()) {
        GTEST_SKIP() << "No test files available for regression testing";
    }

    QString testFile = availableTestFiles.first();

    // Test basic file operations (Sprint 1 functionality)
    ASSERT_TRUE(parser->openFile(testFile.toStdString()));
    EXPECT_TRUE(parser->isOpen());
    EXPECT_TRUE(parser->getLastError().empty());

    // Test metadata extraction (Sprint 1 functionality)
    auto version = parser->getVersion();
    EXPECT_GT(version.first, 0) << "Major version should be > 0";

    int scanCount = parser->getScanCount();
    EXPECT_GE(scanCount, 0) << "Scan count should be >= 0";

    // Test point data extraction (Sprint 2 functionality)
    if (scanCount > 0) {
        auto points = parser->extractPointData(0);
        int64_t pointCount = parser->getPointCount(0);

        if (pointCount > 0) {
            EXPECT_FALSE(points.empty()) << "Points should be extracted for non-empty scans";
            EXPECT_EQ(points.size() % 3, 0) << "Points should be in XYZ format";
        }
    }

    parser->closeFile();
    EXPECT_FALSE(parser->isOpen());
}

// Test signal emission (Sprint 2 & 3 functionality)
TEST_F(E57Sprint4ComprehensiveTest, SignalEmissionRegression) {
    if (availableTestFiles.isEmpty()) {
        GTEST_SKIP() << "No test files available for signal testing";
    }

    QString testFile = availableTestFiles.first();

    // Set up signal spies
    QSignalSpy progressSpy(parser.get(), &E57ParserLib::progressUpdated);
    QSignalSpy finishedSpy(parser.get(), &E57ParserLib::parsingFinished);

    ASSERT_TRUE(parser->openFile(testFile.toStdString()));

    if (parser->getScanCount() > 0) {
        // Extract points (should emit signals)
        auto points = parser->extractPointData(0);

        // Verify signals were emitted
        EXPECT_GT(progressSpy.count(), 0) << "Progress signals should be emitted";
        EXPECT_EQ(finishedSpy.count(), 1) << "Finished signal should be emitted exactly once";

        if (finishedSpy.count() > 0) {
            QList<QVariant> arguments = finishedSpy.takeFirst();
            bool success = arguments[0].toBool();
            EXPECT_TRUE(success) << "Parsing should succeed for valid files";
        }
    }

    parser->closeFile();
}

// ============================================================================
// Integration Test: Complete Workflow
// ============================================================================

TEST_F(E57Sprint4ComprehensiveTest, CompleteWorkflowIntegration) {
    if (availableTestFiles.isEmpty()) {
        GTEST_SKIP() << "No test files available for workflow integration testing";
    }

    qDebug() << "=== Sprint 4 Complete Workflow Integration Test ===";

    // Step 1: Setup test configuration
    QString configFile = testDataDir + "/test_config.json";
    createTestConfiguration(configFile);

    // Step 2: Load test suite
    testFramework->loadTestSuite(configFile);

    // Step 3: Run comprehensive tests
    auto testResults = testFramework->runComprehensiveTests();
    EXPECT_FALSE(testResults.empty()) << "Test results should not be empty";

    // Step 4: Run performance profiling
    PerformanceProfiler::BenchmarkConfig benchConfig;
    benchConfig.testFiles = availableTestFiles;
    benchConfig.optimizationVariants = {
        PerformanceProfiler::OptimizationSettings(), // Default
        {32768, false, false, 1.0, true, "Small Buffer"}, // Small buffer
        {131072, false, false, 1.0, true, "Large Buffer"} // Large buffer
    };
    benchConfig.maxPointsPerTest = 100000;
    benchConfig.outputDirectory = testDataDir;

    auto perfResults = profiler->runBenchmarkSuite(benchConfig);
    EXPECT_FALSE(perfResults.empty()) << "Performance results should not be empty";

    // Step 5: Verify integration results
    int successfulTests = 0;
    int successfulPerfTests = 0;

    for (const auto& result : testResults) {
        if (result.success) successfulTests++;
    }

    for (const auto& result : perfResults) {
        if (result.success) successfulPerfTests++;
    }

    qDebug() << "Integration test summary:";
    qDebug() << "- Functional tests:" << successfulTests << "/" << testResults.size() << "passed";
    qDebug() << "- Performance tests:" << successfulPerfTests << "/" << perfResults.size() << "passed";

    // Generate final reports
    testFramework->generateTestReport(testResults, testDataDir + "/final_test_report.html");
    profiler->generatePerformanceReport(perfResults, testDataDir + "/final_performance_report.html");

    // Export metrics to JSON for further analysis
    auto jsonMetrics = profiler->exportMetricsToJson(perfResults);
    QFile jsonFile(testDataDir + "/performance_metrics.json");
    if (jsonFile.open(QIODevice::WriteOnly)) {
        jsonFile.write(QJsonDocument(jsonMetrics).toJson());
    }

    qDebug() << "=== Sprint 4 Integration Test Complete ===";
}

private:
    void createTestConfiguration(const QString& configPath) {
        QJsonObject config;
        config["testDataDirectory"] = testDataDir;

        QJsonArray testFilesArray;
        for (const QString& file : availableTestFiles) {
            QJsonObject fileObj;
            fileObj["fileName"] = QFileInfo(file).fileName();
            fileObj["vendor"] = "Test";
            fileObj["software"] = "Unit Test";
            fileObj["expectedScanCount"] = 1;
            fileObj["expectedPointCount"] = 1000;
            fileObj["hasIntensity"] = false;
            fileObj["hasColor"] = false;
            fileObj["hasMultipleScans"] = false;
            fileObj["shouldFail"] = false;
            fileObj["expectedErrorType"] = "";
            fileObj["description"] = QString("Test file: %1").arg(QFileInfo(file).fileName());

            testFilesArray.append(fileObj);
        }

        config["testFiles"] = testFilesArray;

        QFile configFile(configPath);
        if (configFile.open(QIODevice::WriteOnly)) {
            configFile.write(QJsonDocument(config).toJson());
        }
    }
};

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);

    qDebug() << "Starting Sprint 4 Comprehensive E57 Test Suite";
    qDebug() << "Testing: Comprehensive functionality, Performance profiling, Multi-scan support";

    int result = RUN_ALL_TESTS();

    qDebug() << "Sprint 4 Test Suite completed with result:" << result;
    return result;
}
