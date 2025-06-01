#include "advanced_test_executor.h"
#include "../src/e57parser.h"
#include "../src/lasparser.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QDir>
#include <QThread>
#include <QCoreApplication>

AdvancedTestExecutor::AdvancedTestExecutor(QObject *parent) 
    : QObject(parent)
    , m_peakMemoryUsage(0)
    , m_baselineMemoryUsage(0)
    , m_memoryMonitoringEnabled(true)
    , m_performanceBenchmarkingEnabled(true)
    , m_timeoutMs(300000) // 5 minutes default timeout
    , m_totalTests(0)
    , m_passedTests(0)
    , m_failedTests(0)
    , m_currentTestIndex(0)
{
    // Setup memory monitoring timer
    m_memoryMonitor = new QTimer(this);
    m_memoryMonitor->setInterval(500); // Monitor every 500ms
    connect(m_memoryMonitor, &QTimer::timeout, 
            this, &AdvancedTestExecutor::onMemoryMeasurementReady);
    
    // Setup timeout timer
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout,
            this, &AdvancedTestExecutor::onTestTimeout);
    
    // Setup external memory monitoring process
    m_memoryProcess = new QProcess(this);
    
    // Get baseline memory usage
    m_baselineMemoryUsage = measureMemoryUsage();
    
    qDebug() << "AdvancedTestExecutor initialized with baseline memory:" 
             << m_baselineMemoryUsage << "bytes";
}

AdvancedTestExecutor::~AdvancedTestExecutor()
{
    stopMemoryMonitoring();
    if (m_memoryProcess && m_memoryProcess->state() != QProcess::NotRunning) {
        m_memoryProcess->kill();
        m_memoryProcess->waitForFinished(3000);
    }
}

void AdvancedTestExecutor::executeTestSuite(const QStringList &testFiles)
{
    qDebug() << "Starting advanced test suite with" << testFiles.size() << "files";
    
    m_results.clear();
    m_totalTests = testFiles.size();
    m_passedTests = 0;
    m_failedTests = 0;
    m_currentTestIndex = 0;
    
    setupTestEnvironment();
    
    emit progressUpdated(0, "Initializing test suite...");
    
    for (const QString &filePath : testFiles) {
        if (!isTestFileValid(filePath)) {
            qWarning() << "Skipping invalid test file:" << filePath;
            continue;
        }
        
        m_currentTestIndex++;
        int progress = (m_currentTestIndex * 90) / m_totalTests;
        emit progressUpdated(progress, QString("Testing file %1/%2").arg(m_currentTestIndex).arg(m_totalTests));
        
        executeIndividualTest(filePath);
        
        // Process events to keep UI responsive
        QCoreApplication::processEvents();
        
        // Small delay between tests to allow system recovery
        QThread::msleep(100);
    }
    
    emit progressUpdated(95, "Analyzing results...");
    
    // Generate summary statistics
    for (const TestResult &result : m_results) {
        if (result.success) {
            m_passedTests++;
        } else {
            m_failedTests++;
        }
    }
    
    emit progressUpdated(100, "Test suite completed");
    emit testSuiteCompleted(m_totalTests, m_passedTests, m_failedTests);
    
    cleanupTestEnvironment();
    
    qDebug() << "Test suite completed:" << m_passedTests << "passed," << m_failedTests << "failed";
}

void AdvancedTestExecutor::executeIndividualTest(const QString &filePath)
{
    qDebug() << "Executing individual test for:" << filePath;
    
    // Initialize test result
    m_currentTest = TestResult();
    m_currentTest.testName = QFileInfo(filePath).baseName();
    m_currentTest.filePath = filePath;
    
    emit testStarted(m_currentTest.testName);
    
    // Start monitoring
    if (m_memoryMonitoringEnabled) {
        startMemoryMonitoring();
    }
    
    // Start timeout timer
    m_timeoutTimer->start(m_timeoutMs);
    
    // Start timing
    m_testTimer.start();
    
    // Determine file type and create appropriate parser
    QString extension = QFileInfo(filePath).suffix().toLower();
    
    if (extension == "e57") {
        E57Parser *parser = new E57Parser(this);
        connect(parser, &E57Parser::parsingFinished,
                this, &AdvancedTestExecutor::onParsingFinished);
        
        // Start parsing in a separate thread would be ideal, but for simplicity:
        try {
            std::vector<float> points = parser->parse(filePath);
            onParsingFinished(true, "Parsing completed successfully", points);
        } catch (const std::exception &e) {
            onParsingFinished(false, QString("Exception: %1").arg(e.what()), std::vector<float>());
        }
        
        parser->deleteLater();
        
    } else if (extension == "las") {
        LasParser *parser = new LasParser(this);
        connect(parser, &LasParser::parsingFinished,
                this, &AdvancedTestExecutor::onParsingFinished);
        
        try {
            std::vector<float> points = parser->parse(filePath);
            onParsingFinished(true, "Parsing completed successfully", points);
        } catch (const std::exception &e) {
            onParsingFinished(false, QString("Exception: %1").arg(e.what()), std::vector<float>());
        }
        
        parser->deleteLater();
        
    } else {
        m_currentTest.errorMessage = "Unsupported file format";
        onParsingFinished(false, m_currentTest.errorMessage, std::vector<float>());
    }
}

void AdvancedTestExecutor::onParsingFinished(bool success, const QString &message, 
                                           const std::vector<float> &points)
{
    // Stop monitoring
    m_timeoutTimer->stop();
    if (m_memoryMonitoringEnabled) {
        stopMemoryMonitoring();
    }
    
    // Record results
    m_currentTest.loadTimeMs = m_testTimer.elapsed();
    m_currentTest.success = success;
    m_currentTest.errorMessage = message;
    m_currentTest.pointsLoaded = points.size() / 3;
    m_currentTest.memoryUsageMB = (m_peakMemoryUsage - m_baselineMemoryUsage) / (1024 * 1024);
    
    // Analyze performance
    if (m_performanceBenchmarkingEnabled) {
        analyzePerformance(m_currentTest);
    }
    
    // Detect memory leaks
    if (m_memoryMonitoringEnabled) {
        detectMemoryLeaks();
    }
    
    // Generate warnings
    if (success) {
        if (m_currentTest.loadTimeMs > 30000) { // > 30 seconds
            m_currentTest.warnings.append("Loading time exceeds 30 seconds");
        }
        
        if (m_currentTest.memoryUsageMB > 4000) { // > 4GB
            m_currentTest.warnings.append("High memory usage detected");
            emit performanceIssueDetected(m_currentTest.testName, "High memory usage");
        }
        
        if (m_currentTest.pointsLoaded == 0) {
            m_currentTest.warnings.append("No points loaded despite success status");
        }
        
        if (m_currentTest.loadTimeMs > 0 && m_currentTest.pointsLoaded > 0) {
            m_currentTest.loadingSpeed = (double)m_currentTest.pointsLoaded / (m_currentTest.loadTimeMs / 1000.0);
        }
    }
    
    // Store metadata
    QJsonObject metadata;
    metadata["loadTimeMs"] = m_currentTest.loadTimeMs;
    metadata["memoryUsageMB"] = m_currentTest.memoryUsageMB;
    metadata["pointsLoaded"] = m_currentTest.pointsLoaded;
    metadata["fileSize"] = QFileInfo(m_currentTest.filePath).size();
    metadata["loadingSpeed"] = m_currentTest.loadingSpeed;
    metadata["memoryEfficiency"] = m_currentTest.memoryEfficiency;
    m_currentTest.metadata = metadata;
    
    // Add to results
    m_results.append(m_currentTest);
    emit testCompleted(m_currentTest);
    
    qDebug() << "Test completed:" << m_currentTest.testName 
             << "Success:" << m_currentTest.success
             << "Time:" << m_currentTest.loadTimeMs << "ms"
             << "Memory:" << m_currentTest.memoryUsageMB << "MB"
             << "Points:" << m_currentTest.pointsLoaded;
}

void AdvancedTestExecutor::executeStressTest(const QString &testFile, int iterations)
{
    qDebug() << "Starting stress test with" << iterations << "iterations on" << testFile;
    
    QList<TestResult> stressResults;
    QList<double> loadTimes;
    QList<qint64> memoryUsages;
    
    for (int i = 0; i < iterations; ++i) {
        qDebug() << "Stress test iteration" << (i + 1) << "/" << iterations;
        
        executeIndividualTest(testFile);
        
        if (!m_results.isEmpty()) {
            TestResult result = m_results.last();
            stressResults.append(result);
            loadTimes.append(result.loadTimeMs);
            memoryUsages.append(result.memoryUsageMB);
            
            // Check for performance degradation
            if (i > 0) {
                double avgLoadTime = std::accumulate(loadTimes.begin(), loadTimes.end(), 0.0) / loadTimes.size();
                if (result.loadTimeMs > avgLoadTime * 1.5) {
                    emit performanceIssueDetected(result.testName, 
                        QString("Performance degradation detected in iteration %1").arg(i + 1));
                }
            }
        }
        
        // Force garbage collection between iterations
        QCoreApplication::processEvents();
        QThread::msleep(1000);
    }
    
    // Analyze stress test results
    if (!loadTimes.isEmpty()) {
        double avgLoadTime = std::accumulate(loadTimes.begin(), loadTimes.end(), 0.0) / loadTimes.size();
        double maxLoadTime = *std::max_element(loadTimes.begin(), loadTimes.end());
        double minLoadTime = *std::min_element(loadTimes.begin(), loadTimes.end());
        
        qDebug() << "Stress test completed:";
        qDebug() << "  Average load time:" << avgLoadTime << "ms";
        qDebug() << "  Min load time:" << minLoadTime << "ms";
        qDebug() << "  Max load time:" << maxLoadTime << "ms";
        qDebug() << "  Performance variation:" << ((maxLoadTime - minLoadTime) / avgLoadTime * 100) << "%";
    }
}

void AdvancedTestExecutor::executeMemoryLeakTest(const QStringList &testFiles)
{
    qDebug() << "Starting memory leak test with" << testFiles.size() << "files";

    qint64 initialMemory = measureMemoryUsage();
    QList<qint64> memorySnapshots;

    for (int iteration = 0; iteration < 3; ++iteration) {
        qDebug() << "Memory leak test iteration" << (iteration + 1);

        for (const QString &filePath : testFiles) {
            executeIndividualTest(filePath);

            qint64 currentMemory = measureMemoryUsage();
            memorySnapshots.append(currentMemory);

            // Force cleanup
            QCoreApplication::processEvents();
            QThread::msleep(500);
        }
    }

    // Analyze memory usage trend
    if (memorySnapshots.size() >= 6) { // At least 2 iterations
        qint64 firstIterationAvg = 0;
        qint64 lastIterationAvg = 0;
        int filesPerIteration = testFiles.size();

        // Calculate average memory for first iteration
        for (int i = 0; i < filesPerIteration && i < memorySnapshots.size(); ++i) {
            firstIterationAvg += memorySnapshots[i];
        }
        firstIterationAvg /= filesPerIteration;

        // Calculate average memory for last iteration
        int lastIterationStart = memorySnapshots.size() - filesPerIteration;
        for (int i = lastIterationStart; i < memorySnapshots.size(); ++i) {
            lastIterationAvg += memorySnapshots[i];
        }
        lastIterationAvg /= filesPerIteration;

        qint64 memoryIncrease = lastIterationAvg - firstIterationAvg;
        double increasePercentage = (double)memoryIncrease / firstIterationAvg * 100.0;

        qDebug() << "Memory leak analysis:";
        qDebug() << "  Initial memory:" << initialMemory / (1024 * 1024) << "MB";
        qDebug() << "  First iteration avg:" << firstIterationAvg / (1024 * 1024) << "MB";
        qDebug() << "  Last iteration avg:" << lastIterationAvg / (1024 * 1024) << "MB";
        qDebug() << "  Memory increase:" << memoryIncrease / (1024 * 1024) << "MB (" << increasePercentage << "%)";

        if (increasePercentage > 10.0) { // More than 10% increase
            emit memoryLeakDetected("MemoryLeakTest", memoryIncrease);
            qWarning() << "Potential memory leak detected!";
        }
    }
}

void AdvancedTestExecutor::startMemoryMonitoring()
{
    m_peakMemoryUsage = m_baselineMemoryUsage;
    m_memorySnapshots.clear();
    m_memoryMonitor->start();
}

void AdvancedTestExecutor::stopMemoryMonitoring()
{
    m_memoryMonitor->stop();
}

void AdvancedTestExecutor::onMemoryMeasurementReady()
{
    qint64 currentMemory = measureMemoryUsage();
    if (currentMemory > m_peakMemoryUsage) {
        m_peakMemoryUsage = currentMemory;
    }
    m_memorySnapshots.append(currentMemory);
}

void AdvancedTestExecutor::onTestTimeout()
{
    qWarning() << "Test timeout reached for:" << m_currentTest.testName;
    m_currentTest.errorMessage = "Test timeout - execution exceeded maximum allowed time";
    onParsingFinished(false, m_currentTest.errorMessage, std::vector<float>());
}

qint64 AdvancedTestExecutor::measureMemoryUsage()
{
#ifdef Q_OS_LINUX
    // Linux implementation using /proc/self/status
    QFile statusFile("/proc/self/status");
    if (statusFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&statusFile);
        QString line;
        while (stream.readLineInto(&line)) {
            if (line.startsWith("VmRSS:")) {
                QStringList parts = line.split(QRegularExpression("\\s+"));
                if (parts.size() >= 2) {
                    return parts[1].toLongLong() * 1024; // Convert from KB to bytes
                }
            }
        }
    }
#elif defined(Q_OS_WIN)
    // Windows implementation would use GetProcessMemoryInfo
    // For now, return a placeholder
    return 0;
#endif
    return 0;
}

void AdvancedTestExecutor::detectMemoryLeaks()
{
    if (m_memorySnapshots.size() < 2) {
        return;
    }

    // Simple leak detection: check if memory consistently increases
    qint64 startMemory = m_memorySnapshots.first();
    qint64 endMemory = m_memorySnapshots.last();
    qint64 memoryIncrease = endMemory - startMemory;

    // Consider it a leak if memory increased by more than 100MB during a single test
    if (memoryIncrease > 100 * 1024 * 1024) {
        m_currentTest.memoryLeakDetected = true;
        emit memoryLeakDetected(m_currentTest.testName, memoryIncrease);
    }
}

void AdvancedTestExecutor::analyzePerformance(TestResult &result)
{
    // Calculate memory efficiency (points per MB)
    if (result.memoryUsageMB > 0) {
        result.memoryEfficiency = (double)result.pointsLoaded / result.memoryUsageMB;
    }

    // Store performance data for trend analysis
    if (result.success) {
        m_loadingTimes.append(result.loadTimeMs);
        m_memoryUsages.append(result.memoryUsageMB);
    }

    // Detect performance issues
    detectPerformanceIssues(result);
}

void AdvancedTestExecutor::detectPerformanceIssues(const TestResult &result)
{
    // Check for extremely slow loading (less than 1000 points per second)
    if (result.loadingSpeed > 0 && result.loadingSpeed < 1000) {
        emit performanceIssueDetected(result.testName, "Very slow loading speed detected");
    }

    // Check for poor memory efficiency (less than 1000 points per MB)
    if (result.memoryEfficiency > 0 && result.memoryEfficiency < 1000) {
        emit performanceIssueDetected(result.testName, "Poor memory efficiency detected");
    }

    // Check for excessive memory usage relative to file size
    qint64 fileSize = QFileInfo(result.filePath).size();
    if (fileSize > 0 && result.memoryUsageMB > 0) {
        double memoryToFileSizeRatio = (result.memoryUsageMB * 1024 * 1024) / (double)fileSize;
        if (memoryToFileSizeRatio > 10.0) { // Memory usage more than 10x file size
            emit performanceIssueDetected(result.testName, "Excessive memory usage relative to file size");
        }
    }
}

void AdvancedTestExecutor::setupTestEnvironment()
{
    // Ensure test data directory exists
    QDir().mkpath("tests/data/advanced");

    // Clear any previous results
    m_loadingTimes.clear();
    m_memoryUsages.clear();
}

void AdvancedTestExecutor::cleanupTestEnvironment()
{
    // Force garbage collection
    QCoreApplication::processEvents();
}

bool AdvancedTestExecutor::isTestFileValid(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return false;
    }

    QString extension = fileInfo.suffix().toLower();
    return (extension == "e57" || extension == "las");
}

void AdvancedTestExecutor::generateDetailedReport(const QString &outputPath)
{
    QJsonObject report;
    QJsonArray testResults;

    for (const TestResult &result : m_results) {
        QJsonObject testObj;
        testObj["testName"] = result.testName;
        testObj["filePath"] = result.filePath;
        testObj["success"] = result.success;
        testObj["errorMessage"] = result.errorMessage;
        testObj["loadTimeMs"] = result.loadTimeMs;
        testObj["memoryUsageMB"] = result.memoryUsageMB;
        testObj["pointsLoaded"] = result.pointsLoaded;
        testObj["loadingSpeed"] = result.loadingSpeed;
        testObj["memoryEfficiency"] = result.memoryEfficiency;
        testObj["memoryLeakDetected"] = result.memoryLeakDetected;
        testObj["warnings"] = QJsonArray::fromStringList(result.warnings);
        testObj["metadata"] = result.metadata;

        testResults.append(testObj);
    }

    report["testResults"] = testResults;
    report["summary"] = generateTestSummary();
    report["performanceMetrics"] = generatePerformanceMetrics();
    report["memoryMetrics"] = generateMemoryMetrics();
    report["executionDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    report["totalTests"] = m_results.size();
    report["passed"] = m_passedTests;
    report["failed"] = m_failedTests;

    QFile file(outputPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(report);
        file.write(doc.toJson());
        qDebug() << "Detailed report generated:" << outputPath;
    } else {
        qWarning() << "Failed to write report to:" << outputPath;
    }
}

QJsonObject AdvancedTestExecutor::generateTestSummary()
{
    QJsonObject summary;

    summary["totalTests"] = m_results.size();
    summary["passedTests"] = m_passedTests;
    summary["failedTests"] = m_failedTests;
    summary["successRate"] = m_results.isEmpty() ? 0.0 : (double)m_passedTests / m_results.size() * 100.0;

    // Calculate averages for successful tests
    if (m_passedTests > 0) {
        double avgLoadTime = 0.0;
        double avgMemoryUsage = 0.0;
        int totalPoints = 0;
        int testsWithWarnings = 0;

        for (const TestResult &result : m_results) {
            if (result.success) {
                avgLoadTime += result.loadTimeMs;
                avgMemoryUsage += result.memoryUsageMB;
                totalPoints += result.pointsLoaded;
                if (!result.warnings.isEmpty()) {
                    testsWithWarnings++;
                }
            }
        }

        summary["averageLoadTimeMs"] = avgLoadTime / m_passedTests;
        summary["averageMemoryUsageMB"] = avgMemoryUsage / m_passedTests;
        summary["totalPointsLoaded"] = totalPoints;
        summary["testsWithWarnings"] = testsWithWarnings;
    }

    return summary;
}

QJsonObject AdvancedTestExecutor::generatePerformanceMetrics()
{
    QJsonObject metrics;

    if (!m_loadingTimes.isEmpty()) {
        double avgLoadTime = std::accumulate(m_loadingTimes.begin(), m_loadingTimes.end(), 0.0) / m_loadingTimes.size();
        double maxLoadTime = *std::max_element(m_loadingTimes.begin(), m_loadingTimes.end());
        double minLoadTime = *std::min_element(m_loadingTimes.begin(), m_loadingTimes.end());

        metrics["averageLoadTimeMs"] = avgLoadTime;
        metrics["maxLoadTimeMs"] = maxLoadTime;
        metrics["minLoadTimeMs"] = minLoadTime;
        metrics["loadTimeVariation"] = ((maxLoadTime - minLoadTime) / avgLoadTime) * 100.0;
    }

    // Calculate loading speed statistics
    QList<double> loadingSpeeds;
    for (const TestResult &result : m_results) {
        if (result.success && result.loadingSpeed > 0) {
            loadingSpeeds.append(result.loadingSpeed);
        }
    }

    if (!loadingSpeeds.isEmpty()) {
        double avgSpeed = std::accumulate(loadingSpeeds.begin(), loadingSpeeds.end(), 0.0) / loadingSpeeds.size();
        double maxSpeed = *std::max_element(loadingSpeeds.begin(), loadingSpeeds.end());
        double minSpeed = *std::min_element(loadingSpeeds.begin(), loadingSpeeds.end());

        metrics["averageLoadingSpeed"] = avgSpeed;
        metrics["maxLoadingSpeed"] = maxSpeed;
        metrics["minLoadingSpeed"] = minSpeed;
    }

    return metrics;
}

QJsonObject AdvancedTestExecutor::generateMemoryMetrics()
{
    QJsonObject metrics;

    if (!m_memoryUsages.isEmpty()) {
        double avgMemory = std::accumulate(m_memoryUsages.begin(), m_memoryUsages.end(), 0.0) / m_memoryUsages.size();
        double maxMemory = *std::max_element(m_memoryUsages.begin(), m_memoryUsages.end());
        double minMemory = *std::min_element(m_memoryUsages.begin(), m_memoryUsages.end());

        metrics["averageMemoryUsageMB"] = avgMemory;
        metrics["maxMemoryUsageMB"] = maxMemory;
        metrics["minMemoryUsageMB"] = minMemory;
        metrics["memoryUsageVariation"] = ((maxMemory - minMemory) / avgMemory) * 100.0;
    }

    // Memory efficiency statistics
    QList<double> memoryEfficiencies;
    int memoryLeakCount = 0;

    for (const TestResult &result : m_results) {
        if (result.success && result.memoryEfficiency > 0) {
            memoryEfficiencies.append(result.memoryEfficiency);
        }
        if (result.memoryLeakDetected) {
            memoryLeakCount++;
        }
    }

    if (!memoryEfficiencies.isEmpty()) {
        double avgEfficiency = std::accumulate(memoryEfficiencies.begin(), memoryEfficiencies.end(), 0.0) / memoryEfficiencies.size();
        metrics["averageMemoryEfficiency"] = avgEfficiency;
    }

    metrics["memoryLeaksDetected"] = memoryLeakCount;
    metrics["baselineMemoryMB"] = m_baselineMemoryUsage / (1024 * 1024);

    return metrics;
}
