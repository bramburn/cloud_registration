#include "performance_benchmark.h"
#include "e57parserlib.h"
#include "lasparser.h"
#include "performance_profiler.h"
#include <QCoreApplication>
#include <QTimer>
#include <QEventLoop>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QThread>
#include <algorithm>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#elif defined(Q_OS_LINUX)
#include <unistd.h>
#include <fstream>
#elif defined(Q_OS_MAC)
#include <mach/mach.h>
#endif

QString BenchmarkResult::getFormattedFileSize() const
{
    double size = fileSize;
    QStringList units = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    
    while (size >= 1024.0 && unitIndex < units.size() - 1) {
        size /= 1024.0;
        unitIndex++;
    }
    
    return QString("%1 %2").arg(size, 0, 'f', 1).arg(units[unitIndex]);
}

QString BenchmarkResult::getFormattedPointCount() const
{
    if (pointCount >= 1000000) {
        return QString("%1M").arg(pointCount / 1000000.0, 0, 'f', 1);
    } else if (pointCount >= 1000) {
        return QString("%1K").arg(pointCount / 1000.0, 0, 'f', 1);
    } else {
        return QString::number(pointCount);
    }
}

PerformanceBenchmark::PerformanceBenchmark(QObject *parent) 
    : QObject(parent)
{
    m_baselineMemory = getBaselineMemoryUsage();
    qDebug() << "PerformanceBenchmark initialized. Baseline memory:" << m_baselineMemory << "bytes";
}

BenchmarkResult PerformanceBenchmark::runE57Benchmark(const QString &filePath)
{
    return runSingleBenchmark(filePath, "E57");
}

BenchmarkResult PerformanceBenchmark::runLASBenchmark(const QString &filePath)
{
    return runSingleBenchmark(filePath, "LAS");
}

BenchmarkResult PerformanceBenchmark::runSingleBenchmark(const QString &filePath, const QString &parserType)
{
    qDebug() << "Starting benchmark for" << parserType << "file:" << filePath;
    
    // Initialize result
    m_currentResult = BenchmarkResult();
    m_currentResult.filePath = filePath;
    m_currentResult.fileType = parserType;
    m_currentResult.testName = QString("%1 - %2").arg(parserType, QFileInfo(filePath).fileName());
    
    // Get file size
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        m_currentResult.success = false;
        m_currentResult.errorMessage = "File does not exist";
        return m_currentResult;
    }
    m_currentResult.fileSize = fileInfo.size();
    
    // Reset profiler for detailed timing
    PerformanceProfiler::instance().reset();
    
    // Start benchmark timing
    m_benchmarkTimer.start();
    m_detailedTimer.start();
    m_lastTimestamp = 0;
    m_benchmarkInProgress = true;
    
    // Create parser and connect signals
    QObject* parser = nullptr;
    if (parserType == "E57") {
        E57ParserLib* e57Parser = new E57ParserLib();
        parser = e57Parser;
        connect(e57Parser, &E57ParserLib::parsingFinished,
                this, &PerformanceBenchmark::onParsingFinished);
        
        // Start parsing in a separate thread
        QThread* thread = new QThread();
        e57Parser->moveToThread(thread);
        connect(thread, &QThread::started, e57Parser, [=]() {
            e57Parser->startParsing(filePath);
        });
        connect(thread, &QThread::finished, e57Parser, &QObject::deleteLater);
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        thread->start();
        
    } else if (parserType == "LAS") {
        LasParser* lasParser = new LasParser();
        parser = lasParser;
        connect(lasParser, &LasParser::parsingFinished, 
                this, &PerformanceBenchmark::onParsingFinished);
        
        // Start parsing in a separate thread
        QThread* thread = new QThread();
        lasParser->moveToThread(thread);
        connect(thread, &QThread::started, lasParser, [=]() {
            lasParser->startParsing(filePath);
        });
        connect(thread, &QThread::finished, lasParser, &QObject::deleteLater);
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        thread->start();
    }
    
    // Wait for parsing to complete
    if (!waitForParsingComplete()) {
        m_currentResult.success = false;
        m_currentResult.errorMessage = "Benchmark timed out";
        if (parser) {
            parser->deleteLater();
        }
    }
    
    // Add to results
    m_results.push_back(m_currentResult);
    
    qDebug() << "Benchmark completed:" << m_currentResult.testName 
             << "- Time:" << m_currentResult.loadTimeMs << "ms"
             << "- Points:" << m_currentResult.pointCount
             << "- Success:" << m_currentResult.success;
    
    return m_currentResult;
}

void PerformanceBenchmark::onParsingFinished(bool success, const QString &message, 
                                           const std::vector<float> &points)
{
    m_currentResult.loadTimeMs = m_benchmarkTimer.elapsed();
    m_currentResult.success = success;
    m_currentResult.errorMessage = success ? "" : message;
    m_currentResult.pointCount = static_cast<int>(points.size() / 3);
    
    if (m_currentResult.loadTimeMs > 0) {
        m_currentResult.pointsPerSecond = 
            (double)m_currentResult.pointCount * 1000.0 / m_currentResult.loadTimeMs;
    }
    
    // Get memory usage
    if (m_memoryMonitoringEnabled) {
        m_currentResult.memoryUsage = getMemoryUsage() - m_baselineMemory;
    }
    
    // Extract detailed timing from profiler
    auto fileOpenSection = PerformanceProfiler::instance().getSection("FileOpen");
    auto headerSection = PerformanceProfiler::instance().getSection("HeaderParse");
    auto dataSection = PerformanceProfiler::instance().getSection("DataParse");
    auto gpuSection = PerformanceProfiler::instance().getSection("GPUUpload");
    
    m_currentResult.fileOpenTime = fileOpenSection.totalTime;
    m_currentResult.headerParseTime = headerSection.totalTime;
    m_currentResult.dataParseTime = dataSection.totalTime;
    m_currentResult.gpuUploadTime = gpuSection.totalTime;
    
    m_benchmarkInProgress = false;
}

bool PerformanceBenchmark::waitForParsingComplete(int timeoutMs)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(timeoutMs);
    
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    
    // Check periodically if benchmark is complete
    QTimer checkTimer;
    connect(&checkTimer, &QTimer::timeout, [&]() {
        if (!m_benchmarkInProgress) {
            loop.quit();
        }
    });
    checkTimer.start(100); // Check every 100ms
    
    loop.exec();
    
    return !m_benchmarkInProgress;
}

qint64 PerformanceBenchmark::getMemoryUsage() const
{
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
#elif defined(Q_OS_LINUX)
    std::ifstream file("/proc/self/status");
    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            std::string memStr = line.substr(6);
            // Remove leading whitespace and "kB"
            size_t start = memStr.find_first_not_of(" \t");
            size_t end = memStr.find("kB");
            if (start != std::string::npos && end != std::string::npos) {
                long memKB = std::stol(memStr.substr(start, end - start));
                return memKB * 1024; // Convert to bytes
            }
        }
    }
#elif defined(Q_OS_MAC)
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
                  (task_info_t)&info, &infoCount) == KERN_SUCCESS) {
        return info.resident_size;
    }
#endif
    return 0; // Fallback for unsupported platforms
}

qint64 PerformanceBenchmark::getBaselineMemoryUsage() const
{
    return getMemoryUsage();
}

void PerformanceBenchmark::runComparisonSuite(const QStringList &files)
{
    qDebug() << "Starting comparison suite with" << files.size() << "files";
    
    clearResults();
    
    for (const QString &filePath : files) {
        QFileInfo fileInfo(filePath);
        QString extension = fileInfo.suffix().toLower();
        
        if (extension == "e57") {
            runE57Benchmark(filePath);
        } else if (extension == "las") {
            runLASBenchmark(filePath);
        } else {
            qWarning() << "Unsupported file type for benchmarking:" << filePath;
        }
        
        // Small delay between tests
        QThread::msleep(100);
    }
    
    qDebug() << "Comparison suite completed. Total results:" << m_results.size();
}

void PerformanceBenchmark::generateBenchmarkReport(const QString &outputPath)
{
    if (m_results.empty()) {
        qWarning() << "No benchmark results to report";
        return;
    }
    
    QString textReport = generateTextReport();
    QJsonObject jsonReport = generateJsonReport();
    
    // Ensure output directory exists
    QFileInfo pathInfo(outputPath);
    QDir().mkpath(pathInfo.absolutePath());
    
    // Save text report
    QString textFilePath = outputPath + "_benchmark_report.txt";
    QFile textFile(textFilePath);
    if (textFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&textFile);
        stream << textReport;
        qDebug() << "Benchmark report saved to:" << textFilePath;
    }
    
    // Save JSON report
    QString jsonFilePath = outputPath + "_benchmark_data.json";
    QFile jsonFile(jsonFilePath);
    if (jsonFile.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(jsonReport);
        jsonFile.write(doc.toJson());
        qDebug() << "Benchmark JSON data saved to:" << jsonFilePath;
    }
}

void PerformanceBenchmark::clearResults()
{
    m_results.clear();
    qDebug() << "Benchmark results cleared";
}

QString PerformanceBenchmark::generateTextReport() const
{
    QString report;
    QTextStream stream(&report);

    // Header
    stream << "=== PERFORMANCE BENCHMARK REPORT ===\n";
    stream << "Generated: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    stream << "Total Tests: " << m_results.size() << "\n\n";

    // Summary statistics
    if (!m_results.empty()) {
        double totalTime = 0;
        int totalPoints = 0;
        int successCount = 0;

        for (const auto &result : m_results) {
            if (result.success) {
                totalTime += result.loadTimeMs;
                totalPoints += result.pointCount;
                successCount++;
            }
        }

        stream << "SUMMARY:\n";
        stream << QString("  Successful Tests: %1/%2\n").arg(successCount).arg(m_results.size());
        if (successCount > 0) {
            stream << QString("  Average Load Time: %1 ms\n").arg(totalTime / successCount, 0, 'f', 1);
            stream << QString("  Total Points Loaded: %1\n").arg(totalPoints);
            stream << QString("  Average Points/Second: %1\n").arg(totalPoints * 1000.0 / totalTime, 0, 'f', 0);
        }
        stream << "\n";
    }

    // Detailed results table
    stream << generateComparisonTable();

    return report;
}

QString PerformanceBenchmark::generateComparisonTable() const
{
    QString table;
    QTextStream stream(&table);

    // Table header
    stream << "DETAILED RESULTS:\n";
    stream << QString("%-30s %-8s %-10s %-8s %-10s %-12s %-8s\n")
              .arg("Test Name")
              .arg("Type")
              .arg("File Size")
              .arg("Points")
              .arg("Time (ms)")
              .arg("Points/sec")
              .arg("Status");
    stream << QString("-").repeated(90) << "\n";

    // Sort results by load time for better comparison
    std::vector<BenchmarkResult> sortedResults = m_results;
    std::sort(sortedResults.begin(), sortedResults.end(),
              [](const BenchmarkResult &a, const BenchmarkResult &b) {
                  if (a.success != b.success) return a.success > b.success;
                  return a.loadTimeMs < b.loadTimeMs;
              });

    // Table rows
    for (const auto &result : sortedResults) {
        QString status = result.success ? "OK" : "FAIL";
        QString pointsPerSec = result.success ?
            QString::number(result.pointsPerSecond, 'f', 0) : "-";

        stream << QString("%-30s %-8s %-10s %-8s %-10lld %-12s %-8s\n")
                  .arg(QFileInfo(result.filePath).fileName().left(30))
                  .arg(result.fileType)
                  .arg(result.getFormattedFileSize())
                  .arg(result.getFormattedPointCount())
                  .arg(result.loadTimeMs)
                  .arg(pointsPerSec)
                  .arg(status);

        if (!result.success && !result.errorMessage.isEmpty()) {
            stream << QString("    Error: %1\n").arg(result.errorMessage);
        }
    }

    return table;
}

QJsonObject PerformanceBenchmark::generateJsonReport() const
{
    QJsonObject report;
    QJsonArray results;

    // Metadata
    report["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    report["totalTests"] = static_cast<qint64>(m_results.size());
    report["memoryMonitoringEnabled"] = m_memoryMonitoringEnabled;

    // Calculate summary statistics
    int successCount = 0;
    double totalTime = 0;
    int totalPoints = 0;

    for (const auto &result : m_results) {
        QJsonObject resultObj;
        resultObj["testName"] = result.testName;
        resultObj["filePath"] = result.filePath;
        resultObj["fileType"] = result.fileType;
        resultObj["loadTimeMs"] = result.loadTimeMs;
        resultObj["fileSize"] = result.fileSize;
        resultObj["pointCount"] = result.pointCount;
        resultObj["pointsPerSecond"] = result.pointsPerSecond;
        resultObj["memoryUsage"] = result.memoryUsage;
        resultObj["success"] = result.success;
        resultObj["errorMessage"] = result.errorMessage;
        resultObj["mbPerSecond"] = result.getMBPerSecond();

        // Detailed timing
        QJsonObject timing;
        timing["fileOpen"] = result.fileOpenTime;
        timing["headerParse"] = result.headerParseTime;
        timing["dataParse"] = result.dataParseTime;
        timing["gpuUpload"] = result.gpuUploadTime;
        resultObj["detailedTiming"] = timing;

        results.append(resultObj);

        if (result.success) {
            successCount++;
            totalTime += result.loadTimeMs;
            totalPoints += result.pointCount;
        }
    }

    // Summary
    QJsonObject summary;
    summary["successfulTests"] = successCount;
    summary["totalTests"] = static_cast<qint64>(m_results.size());
    if (successCount > 0) {
        summary["averageLoadTime"] = totalTime / successCount;
        summary["totalPointsLoaded"] = totalPoints;
        summary["averagePointsPerSecond"] = totalPoints * 1000.0 / totalTime;
    }

    report["summary"] = summary;
    report["results"] = results;

    return report;
}
