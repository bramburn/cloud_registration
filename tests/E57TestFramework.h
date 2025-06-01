#ifndef E57TESTFRAMEWORK_H
#define E57TESTFRAMEWORK_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QElapsedTimer>
#include <vector>
#include <memory>

// Forward declaration
class E57ParserLib;

/**
 * @brief Comprehensive testing framework for E57 library integration
 * 
 * Implements Sprint 4 User Story 1: Comprehensive E57 Functionality Verification
 * Provides automated testing infrastructure for diverse E57 files with
 * validation of correctness, robustness, and error handling.
 */
class E57TestFramework : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Metadata for test E57 files
     */
    struct TestFileMetadata {
        QString filePath;
        QString vendor;           // "Leica", "FARO", "Trimble", etc.
        QString software;         // "ReCap", "Cyclone", "SCENE", etc.
        int expectedScanCount = 1;
        int64_t expectedPointCount = 0;
        bool hasIntensity = false;
        bool hasColor = false;
        bool hasMultipleScans = false;
        bool shouldFail = false;          // For testing malformed files
        QString expectedErrorType;
        QString description;
    };

    /**
     * @brief Results from individual test execution
     */
    struct TestResult {
        QString fileName;
        bool success = false;
        QString errorMessage;
        double loadTime = 0.0;
        size_t memoryUsage = 0;
        int actualScanCount = 0;
        int64_t actualPointCount = 0;
        bool dataIntegrityPassed = false;
        bool attributeValidationPassed = false;
        QString testCategory;
    };

    /**
     * @brief Test suite statistics
     */
    struct TestSuiteStats {
        int totalTests = 0;
        int passedTests = 0;
        int failedTests = 0;
        int skippedTests = 0;
        double totalTime = 0.0;
        double averageLoadTime = 0.0;
        size_t peakMemoryUsage = 0;
    };

public:
    explicit E57TestFramework(QObject* parent = nullptr);
    ~E57TestFramework();

    // Test suite management
    void loadTestSuite(const QString& testConfigPath);
    void addTestFile(const TestFileMetadata& metadata);
    std::vector<TestResult> runComprehensiveTests();
    void generateTestReport(const std::vector<TestResult>& results, const QString& outputPath = "");
    
    // Individual test functions
    bool testFileLoading(const TestFileMetadata& metadata, TestResult& result);
    bool testDataIntegrity(const TestFileMetadata& metadata, TestResult& result);
    bool testErrorHandling(const TestFileMetadata& metadata, TestResult& result);
    bool testPerformance(const TestFileMetadata& metadata, TestResult& result);
    bool testAttributeExtraction(const TestFileMetadata& metadata, TestResult& result);

    // Configuration
    void setTestDataDirectory(const QString& directory) { m_testDataDirectory = directory; }
    void setMaxTestPoints(int maxPoints) { m_maxTestPoints = maxPoints; }
    void setTimeoutSeconds(int timeout) { m_timeoutSeconds = timeout; }

    // Statistics
    TestSuiteStats getLastTestStats() const { return m_lastStats; }

signals:
    void testProgress(int completed, int total);
    void testCompleted(const TestResult& result);
    void testSuiteStarted(int totalTests);
    void testSuiteCompleted(const TestSuiteStats& stats);

private:
    std::vector<TestFileMetadata> m_testFiles;
    QString m_testDataDirectory;
    std::unique_ptr<E57ParserLib> m_parser;
    QElapsedTimer m_timer;
    TestSuiteStats m_lastStats;
    
    // Configuration
    int m_maxTestPoints = 10000;  // Maximum points to validate per test
    int m_timeoutSeconds = 300;   // 5 minute timeout per test
    
    // Helper methods
    void discoverTestFiles(const QString& directory);
    bool validateCoordinates(const std::vector<float>& points);
    bool validateIntensityData(const std::vector<float>& points);
    bool validateColorData(const std::vector<float>& points);
    size_t getCurrentMemoryUsage();
    void updateStatistics(const std::vector<TestResult>& results);
    QString generateDetailedReport(const std::vector<TestResult>& results);
    
    // Test categories
    bool isVendorSpecificTest(const TestFileMetadata& metadata);
    bool isAttributeTest(const TestFileMetadata& metadata);
    bool isPerformanceTest(const TestFileMetadata& metadata);
    bool isErrorHandlingTest(const TestFileMetadata& metadata);
};

#endif // E57TESTFRAMEWORK_H
