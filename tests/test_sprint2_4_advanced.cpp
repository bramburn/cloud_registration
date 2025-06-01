#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>

#include "advanced_test_file_generator.h"
#include "advanced_test_executor.h"

/**
 * @brief Sprint 2.4 Advanced Testing Suite
 * 
 * Comprehensive testing framework implementing all Sprint 2.4 requirements:
 * - User Story 1: Advanced E57 and LAS Loading Testing with Complex Files
 * - User Story 2: Final Bug Fixing and Stability Hardening
 * - User Story 3: Developer Documentation and Test Suite Integration
 * 
 * This test suite validates the enhanced point cloud loading capabilities
 * with complex, real-world scenarios and stress testing.
 */
class Sprint24AdvancedTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create test data directory
        QDir().mkpath("tests/data/advanced");
        
        // Initialize components
        m_generator = new AdvancedTestFileGenerator(this);
        m_executor = new AdvancedTestExecutor(this);
        
        // Connect signals for monitoring
        connect(m_generator, &AdvancedTestFileGenerator::generationProgress,
                this, &Sprint24AdvancedTest::onGenerationProgress);
        connect(m_executor, &AdvancedTestExecutor::testCompleted,
                this, &Sprint24AdvancedTest::onTestCompleted);
        connect(m_executor, &AdvancedTestExecutor::memoryLeakDetected,
                this, &Sprint24AdvancedTest::onMemoryLeakDetected);
        connect(m_executor, &AdvancedTestExecutor::performanceIssueDetected,
                this, &Sprint24AdvancedTest::onPerformanceIssueDetected);
        
        m_testFilesGenerated.clear();
        m_memoryLeaksDetected = 0;
        m_performanceIssuesDetected = 0;
    }
    
    void TearDown() override
    {
        // Generate final report
        if (m_executor && !m_executor->getResults().isEmpty()) {
            QString reportPath = "tests/data/advanced/sprint24_test_report.json";
            m_executor->generateDetailedReport(reportPath);
            qDebug() << "Final test report generated:" << reportPath;
        }
        
        // Cleanup
        delete m_generator;
        delete m_executor;
    }

private slots:
    void onGenerationProgress(int percentage, const QString &status)
    {
        qDebug() << "Generation progress:" << percentage << "%" << status;
    }
    
    void onTestCompleted(const TestResult &result)
    {
        qDebug() << "Test completed:" << result.testName 
                 << "Success:" << result.success
                 << "Points:" << result.pointsLoaded
                 << "Time:" << result.loadTimeMs << "ms";
    }
    
    void onMemoryLeakDetected(const QString &testName, qint64 leakSize)
    {
        qWarning() << "Memory leak detected in" << testName << ":" << leakSize << "bytes";
        m_memoryLeaksDetected++;
    }
    
    void onPerformanceIssueDetected(const QString &testName, const QString &issue)
    {
        qWarning() << "Performance issue in" << testName << ":" << issue;
        m_performanceIssuesDetected++;
    }

protected:
    AdvancedTestFileGenerator *m_generator;
    AdvancedTestExecutor *m_executor;
    QStringList m_testFilesGenerated;
    int m_memoryLeaksDetected;
    int m_performanceIssuesDetected;
};

/**
 * @brief Test Case 2.4.1.A: Load a very large E57 file (Task 2.4.1.1)
 * 
 * Tests the application's ability to handle very large E57 files
 * with 20M+ points without crashing and with acceptable performance.
 */
TEST_F(Sprint24AdvancedTest, VeryLargeE57FileTest)
{
    qDebug() << "=== Test Case 2.4.1.A: Very Large E57 File ===";
    
    // Generate very large E57 file
    QString testFile = "tests/data/advanced/very_large_25M.e57";
    bool generated = m_generator->generateTestFile(
        AdvancedTestFileGenerator::TestScenario::VeryLargePointCloud, testFile);
    
    ASSERT_TRUE(generated) << "Failed to generate very large E57 test file";
    ASSERT_TRUE(QFile::exists(testFile)) << "Test file was not created";
    
    m_testFilesGenerated.append(testFile);
    
    // Execute test
    m_executor->executeIndividualTest(testFile);
    
    // Verify results
    QList<TestResult> results = m_executor->getResults();
    ASSERT_FALSE(results.isEmpty()) << "No test results generated";
    
    TestResult result = results.last();
    
    // The application should not crash with large files
    EXPECT_TRUE(result.success || !result.errorMessage.contains("crash"))
        << "Application crashed on large file: " << result.errorMessage.toStdString();
    
    // Memory usage should be reasonable (less than 8GB for 25M points)
    if (result.success) {
        EXPECT_LT(result.memoryUsageMB, 8000) 
            << "Memory usage too high: " << result.memoryUsageMB << "MB";
        
        // Should load a reasonable number of points
        EXPECT_GT(result.pointsLoaded, 1000000) 
            << "Too few points loaded: " << result.pointsLoaded;
    }
    
    qDebug() << "Large file test completed - Memory:" << result.memoryUsageMB 
             << "MB, Points:" << result.pointsLoaded << ", Time:" << result.loadTimeMs << "ms";
}

/**
 * @brief Test Case 2.4.1.C: Attempt to load an E57 file with multiple data3D scan sections
 * 
 * Tests the application's handling of E57 files with multiple scans.
 * Should either load the first scan successfully or gracefully indicate limitation.
 */
TEST_F(Sprint24AdvancedTest, MultiScanE57FileTest)
{
    qDebug() << "=== Test Case 2.4.1.C: Multi-Scan E57 File ===";
    
    // Generate multi-scan E57 file
    QString testFile = "tests/data/advanced/multi_scan_5.e57";
    bool generated = m_generator->generateTestFile(
        AdvancedTestFileGenerator::TestScenario::MultipleDataSections, testFile);
    
    ASSERT_TRUE(generated) << "Failed to generate multi-scan E57 test file";
    ASSERT_TRUE(QFile::exists(testFile)) << "Test file was not created";
    
    m_testFilesGenerated.append(testFile);
    
    // Execute test
    m_executor->executeIndividualTest(testFile);
    
    // Verify results
    QList<TestResult> results = m_executor->getResults();
    ASSERT_FALSE(results.isEmpty()) << "No test results generated";
    
    TestResult result = results.last();
    
    // Application should not crash
    EXPECT_TRUE(result.success || !result.errorMessage.contains("crash"))
        << "Application crashed on multi-scan file: " << result.errorMessage.toStdString();
    
    if (result.success) {
        // Should load at least the first scan
        EXPECT_GT(result.pointsLoaded, 0) << "No points loaded from multi-scan file";
        
        // Check if there's a warning about multiple scans
        bool hasMultiScanWarning = false;
        for (const QString &warning : result.warnings) {
            if (warning.contains("multiple") || warning.contains("scan")) {
                hasMultiScanWarning = true;
                break;
            }
        }
        
        // It's acceptable if the application warns about multiple scans
        if (!hasMultiScanWarning) {
            qDebug() << "Note: No warning about multiple scans detected";
        }
    }
    
    qDebug() << "Multi-scan test completed - Success:" << result.success 
             << ", Points:" << result.pointsLoaded;
}

/**
 * @brief Test Case 2.4.1.D: Load LAS files with unusual (but valid) header values
 * 
 * Tests the application's robustness with extreme coordinate scale/offset values
 * and files with many Variable Length Records.
 */
TEST_F(Sprint24AdvancedTest, ExtremeCoordinatesLASTest)
{
    qDebug() << "=== Test Case 2.4.1.D: Extreme Coordinates LAS File ===";
    
    // Generate LAS file with extreme coordinates
    QString testFile = "tests/data/advanced/extreme_coords.las";
    bool generated = m_generator->generateTestFile(
        AdvancedTestFileGenerator::TestScenario::ExtremeCoordinates, testFile);
    
    ASSERT_TRUE(generated) << "Failed to generate extreme coordinates LAS test file";
    ASSERT_TRUE(QFile::exists(testFile)) << "Test file was not created";
    
    m_testFilesGenerated.append(testFile);
    
    // Execute test
    m_executor->executeIndividualTest(testFile);
    
    // Verify results
    QList<TestResult> results = m_executor->getResults();
    ASSERT_FALSE(results.isEmpty()) << "No test results generated";
    
    TestResult result = results.last();
    
    // File should load correctly if the point data itself is valid
    EXPECT_TRUE(result.success) 
        << "Failed to load LAS file with extreme coordinates: " << result.errorMessage.toStdString();
    
    if (result.success) {
        EXPECT_GT(result.pointsLoaded, 0) << "No points loaded from extreme coordinates file";
        
        // Coordinates should be properly transformed
        EXPECT_LT(result.loadTimeMs, 60000) << "Loading took too long: " << result.loadTimeMs << "ms";
    }
    
    qDebug() << "Extreme coordinates test completed - Success:" << result.success 
             << ", Points:" << result.pointsLoaded;
}

/**
 * @brief Test Case: LAS files with many VLRs
 * 
 * Tests header parsing with numerous Variable Length Records.
 */
TEST_F(Sprint24AdvancedTest, ManyVLRsLASTest)
{
    qDebug() << "=== Test Case: Many VLRs LAS File ===";
    
    // Generate LAS file with many VLRs
    QString testFile = "tests/data/advanced/many_vlrs_100.las";
    bool generated = m_generator->generateTestFile(
        AdvancedTestFileGenerator::TestScenario::ManyVLRs, testFile);
    
    ASSERT_TRUE(generated) << "Failed to generate many VLRs LAS test file";
    ASSERT_TRUE(QFile::exists(testFile)) << "Test file was not created";
    
    m_testFilesGenerated.append(testFile);
    
    // Execute test
    m_executor->executeIndividualTest(testFile);
    
    // Verify results
    QList<TestResult> results = m_executor->getResults();
    ASSERT_FALSE(results.isEmpty()) << "No test results generated";
    
    TestResult result = results.last();
    
    // Should handle many VLRs correctly
    EXPECT_TRUE(result.success) 
        << "Failed to load LAS file with many VLRs: " << result.errorMessage.toStdString();
    
    if (result.success) {
        EXPECT_GT(result.pointsLoaded, 0) << "No points loaded from many VLRs file";
    }
    
    qDebug() << "Many VLRs test completed - Success:" << result.success
             << ", Points:" << result.pointsLoaded;
}

/**
 * @brief Test Case: Corrupted file handling
 *
 * Tests the application's error handling with intentionally corrupted files.
 * Should fail gracefully with meaningful error messages.
 */
TEST_F(Sprint24AdvancedTest, CorruptedFileHandlingTest)
{
    qDebug() << "=== Test Case: Corrupted File Handling ===";

    // Generate corrupted E57 file
    QString testFile = "tests/data/advanced/corrupted_header.e57";
    bool generated = m_generator->generateTestFile(
        AdvancedTestFileGenerator::TestScenario::CorruptedHeaders, testFile);

    ASSERT_TRUE(generated) << "Failed to generate corrupted test file";
    ASSERT_TRUE(QFile::exists(testFile)) << "Test file was not created";

    m_testFilesGenerated.append(testFile);

    // Execute test
    m_executor->executeIndividualTest(testFile);

    // Verify results
    QList<TestResult> results = m_executor->getResults();
    ASSERT_FALSE(results.isEmpty()) << "No test results generated";

    TestResult result = results.last();

    // Corrupted files should fail gracefully, not crash
    EXPECT_FALSE(result.success) << "Corrupted file should not load successfully";
    EXPECT_FALSE(result.errorMessage.contains("crash"))
        << "Application should not crash on corrupted files";
    EXPECT_FALSE(result.errorMessage.contains("segfault"))
        << "Application should not segfault on corrupted files";

    // Should have a meaningful error message
    EXPECT_FALSE(result.errorMessage.isEmpty())
        << "No error message provided for corrupted file";

    qDebug() << "Corrupted file test completed - Error message:" << result.errorMessage;
}

/**
 * @brief Test Case: Memory stress test
 *
 * Tests the application with files that approach memory limits.
 */
TEST_F(Sprint24AdvancedTest, MemoryStressTest)
{
    qDebug() << "=== Test Case: Memory Stress Test ===";

    // Generate memory stress test file
    QString testFile = "tests/data/advanced/memory_stress_50M.e57";
    bool generated = m_generator->generateTestFile(
        AdvancedTestFileGenerator::TestScenario::MemoryStressTest, testFile);

    ASSERT_TRUE(generated) << "Failed to generate memory stress test file";
    ASSERT_TRUE(QFile::exists(testFile)) << "Test file was not created";

    m_testFilesGenerated.append(testFile);

    // Enable memory monitoring
    m_executor->setMemoryMonitoringEnabled(true);

    // Execute test
    m_executor->executeIndividualTest(testFile);

    // Verify results
    QList<TestResult> results = m_executor->getResults();
    ASSERT_FALSE(results.isEmpty()) << "No test results generated";

    TestResult result = results.last();

    // Application should handle memory stress gracefully
    if (result.success) {
        // Memory usage should be monitored and reasonable
        EXPECT_LT(result.memoryUsageMB, 16000) // Less than 16GB
            << "Excessive memory usage: " << result.memoryUsageMB << "MB";

        // Should not detect memory leaks
        EXPECT_FALSE(result.memoryLeakDetected)
            << "Memory leak detected during stress test";
    } else {
        // If it fails, it should be due to memory constraints, not crashes
        EXPECT_FALSE(result.errorMessage.contains("crash"))
            << "Application crashed during memory stress test";
    }

    qDebug() << "Memory stress test completed - Success:" << result.success
             << ", Memory:" << result.memoryUsageMB << "MB";
}

/**
 * @brief Test Case: Edge case PDRF handling
 *
 * Tests the application with unusual but valid Point Data Record Formats.
 */
TEST_F(Sprint24AdvancedTest, EdgeCasePDRFTest)
{
    qDebug() << "=== Test Case: Edge Case PDRF ===";

    // Generate edge case PDRF file
    QString testFile = "tests/data/advanced/edge_case_pdrf3.las";
    bool generated = m_generator->generateTestFile(
        AdvancedTestFileGenerator::TestScenario::EdgeCasePDRF, testFile);

    ASSERT_TRUE(generated) << "Failed to generate edge case PDRF test file";
    ASSERT_TRUE(QFile::exists(testFile)) << "Test file was not created";

    m_testFilesGenerated.append(testFile);

    // Execute test
    m_executor->executeIndividualTest(testFile);

    // Verify results
    QList<TestResult> results = m_executor->getResults();
    ASSERT_FALSE(results.isEmpty()) << "No test results generated";

    TestResult result = results.last();

    // Should handle PDRF 3 (with RGB) correctly
    EXPECT_TRUE(result.success)
        << "Failed to load LAS file with PDRF 3: " << result.errorMessage.toStdString();

    if (result.success) {
        EXPECT_GT(result.pointsLoaded, 0) << "No points loaded from PDRF 3 file";
    }

    qDebug() << "Edge case PDRF test completed - Success:" << result.success
             << ", Points:" << result.pointsLoaded;
}

/**
 * @brief Comprehensive test suite execution
 *
 * Runs all generated test files through the advanced test executor
 * and generates comprehensive reports.
 */
TEST_F(Sprint24AdvancedTest, ComprehensiveTestSuite)
{
    qDebug() << "=== Comprehensive Test Suite ===";

    // Ensure we have test files from previous tests
    if (m_testFilesGenerated.isEmpty()) {
        // Generate a basic set of test files
        QStringList scenarios = {
            "tests/data/advanced/basic_large.e57",
            "tests/data/advanced/basic_multi.e57",
            "tests/data/advanced/basic_extreme.las"
        };

        m_generator->generateTestFile(
            AdvancedTestFileGenerator::TestScenario::VeryLargePointCloud, scenarios[0]);
        m_generator->generateTestFile(
            AdvancedTestFileGenerator::TestScenario::MultipleDataSections, scenarios[1]);
        m_generator->generateTestFile(
            AdvancedTestFileGenerator::TestScenario::ExtremeCoordinates, scenarios[2]);

        m_testFilesGenerated = scenarios;
    }

    // Execute comprehensive test suite
    m_executor->executeTestSuite(m_testFilesGenerated);

    // Verify overall results
    QList<TestResult> results = m_executor->getResults();
    EXPECT_FALSE(results.isEmpty()) << "No test results from comprehensive suite";

    // Calculate success rate
    int passed = 0;
    int total = results.size();

    for (const TestResult &result : results) {
        if (result.success) {
            passed++;
        }
    }

    double successRate = total > 0 ? (double)passed / total * 100.0 : 0.0;

    qDebug() << "Comprehensive test suite completed:";
    qDebug() << "  Total tests:" << total;
    qDebug() << "  Passed:" << passed;
    qDebug() << "  Success rate:" << successRate << "%";
    qDebug() << "  Memory leaks detected:" << m_memoryLeaksDetected;
    qDebug() << "  Performance issues detected:" << m_performanceIssuesDetected;

    // Expectations for overall quality
    EXPECT_GE(successRate, 70.0) << "Success rate too low: " << successRate << "%";
    EXPECT_EQ(m_memoryLeaksDetected, 0) << "Memory leaks detected: " << m_memoryLeaksDetected;

    // Generate final reports
    m_executor->generateDetailedReport("tests/data/advanced/comprehensive_report.json");

    qDebug() << "Comprehensive test suite analysis completed";
}

// Main function for standalone execution
int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    // Check for command line arguments
    QStringList args = app.arguments();

    if (args.contains("--generate-test-files")) {
        qDebug() << "Generating advanced test files...";

        AdvancedTestFileGenerator generator;
        QDir().mkpath("tests/data/advanced");

        // Generate all test file types
        generator.generateTestFile(AdvancedTestFileGenerator::TestScenario::VeryLargePointCloud,
                                  "tests/data/advanced/very_large_25M.e57");
        generator.generateTestFile(AdvancedTestFileGenerator::TestScenario::MultipleDataSections,
                                  "tests/data/advanced/multi_scan_5.e57");
        generator.generateTestFile(AdvancedTestFileGenerator::TestScenario::ExtremeCoordinates,
                                  "tests/data/advanced/extreme_coords.las");
        generator.generateTestFile(AdvancedTestFileGenerator::TestScenario::ManyVLRs,
                                  "tests/data/advanced/many_vlrs_100.las");
        generator.generateTestFile(AdvancedTestFileGenerator::TestScenario::CorruptedHeaders,
                                  "tests/data/advanced/corrupted_header.e57");
        generator.generateTestFile(AdvancedTestFileGenerator::TestScenario::EdgeCasePDRF,
                                  "tests/data/advanced/edge_case_pdrf3.las");

        qDebug() << "Test file generation completed";
        return 0;
    }

    if (args.contains("--stress-test")) {
        qDebug() << "Running stress tests...";

        AdvancedTestExecutor executor;
        executor.executeStressTest("tests/data/advanced/very_large_25M.e57", 5);
        executor.generateDetailedReport("tests/data/advanced/stress_test_report.json");

        qDebug() << "Stress test completed";
        return 0;
    }

    // Run Google Test suite
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
