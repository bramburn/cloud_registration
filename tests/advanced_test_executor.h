#ifndef ADVANCED_TEST_EXECUTOR_H
#define ADVANCED_TEST_EXECUTOR_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QTimer>
#include <QProcess>
#include <vector>

/**
 * @brief Test result structure for advanced testing
 */
struct TestResult {
    QString testName;
    QString filePath;
    bool success;
    QString errorMessage;
    qint64 loadTimeMs;
    qint64 memoryUsageMB;
    int pointsLoaded;
    QStringList warnings;
    QJsonObject metadata;
    
    // Performance metrics
    double memoryEfficiency;
    double loadingSpeed; // points per second
    bool memoryLeakDetected;
    
    TestResult() : success(false), loadTimeMs(0), memoryUsageMB(0), 
                   pointsLoaded(0), memoryEfficiency(0.0), loadingSpeed(0.0), 
                   memoryLeakDetected(false) {}
};

/**
 * @brief Advanced Test Executor for Sprint 2.4
 * 
 * Implements comprehensive testing framework with:
 * - Memory monitoring and leak detection
 * - Performance benchmarking
 * - Stress testing capabilities
 * - Automated result analysis
 * 
 * Implements Tasks 2.4.1.2-2.4.1.4 from Sprint 2.4 requirements.
 */
class AdvancedTestExecutor : public QObject
{
    Q_OBJECT

public:
    explicit AdvancedTestExecutor(QObject *parent = nullptr);
    virtual ~AdvancedTestExecutor();
    
    // Main test execution methods
    void executeTestSuite(const QStringList &testFiles);
    void executeStressTest(const QString &testFile, int iterations = 10);
    void executeMemoryLeakTest(const QStringList &testFiles);
    void executePerformanceBenchmark(const QStringList &testFiles);
    
    // Individual test execution
    void executeIndividualTest(const QString &filePath);
    
    // Results access
    QList<TestResult> getResults() const { return m_results; }
    void clearResults() { m_results.clear(); }
    
    // Report generation
    void generateDetailedReport(const QString &outputPath);
    void generatePerformanceReport(const QString &outputPath);
    void generateMemoryReport(const QString &outputPath);
    
    // Configuration
    void setMemoryMonitoringEnabled(bool enabled) { m_memoryMonitoringEnabled = enabled; }
    void setPerformanceBenchmarkingEnabled(bool enabled) { m_performanceBenchmarkingEnabled = enabled; }
    void setTimeout(int timeoutMs) { m_timeoutMs = timeoutMs; }

signals:
    void testStarted(const QString &testName);
    void testCompleted(const TestResult &result);
    void testSuiteCompleted(int totalTests, int passed, int failed);
    void progressUpdated(int percentage, const QString &status);
    void memoryLeakDetected(const QString &testName, qint64 leakSize);
    void performanceIssueDetected(const QString &testName, const QString &issue);

private slots:
    void onParsingFinished(bool success, const QString &message, 
                          const std::vector<float> &points);
    void onMemoryMeasurementReady();
    void onTestTimeout();

private:
    // Memory monitoring
    void startMemoryMonitoring();
    void stopMemoryMonitoring();
    qint64 measureMemoryUsage();
    void detectMemoryLeaks();
    
    // Performance analysis
    void analyzePerformance(TestResult &result);
    void calculateMemoryEfficiency(TestResult &result);
    void detectPerformanceIssues(const TestResult &result);
    
    // Test execution helpers
    void setupTestEnvironment();
    void cleanupTestEnvironment();
    bool isTestFileValid(const QString &filePath);
    
    // Report generation helpers
    QJsonObject generateTestSummary();
    QJsonObject generatePerformanceMetrics();
    QJsonObject generateMemoryMetrics();
    
    // Member variables
    QList<TestResult> m_results;
    TestResult m_currentTest;
    QElapsedTimer m_testTimer;
    QTimer *m_memoryMonitor;
    QTimer *m_timeoutTimer;
    
    // Memory monitoring
    qint64 m_peakMemoryUsage;
    qint64 m_baselineMemoryUsage;
    QList<qint64> m_memorySnapshots;
    bool m_memoryMonitoringEnabled;
    
    // Performance monitoring
    bool m_performanceBenchmarkingEnabled;
    QList<double> m_loadingTimes;
    QList<double> m_memoryUsages;
    
    // Configuration
    int m_timeoutMs;
    QString m_currentTestFile;
    
    // External process for memory monitoring
    QProcess *m_memoryProcess;
    
    // Test statistics
    int m_totalTests;
    int m_passedTests;
    int m_failedTests;
    int m_currentTestIndex;
};

#endif // ADVANCED_TEST_EXECUTOR_H
