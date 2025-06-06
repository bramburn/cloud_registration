#include "PerformanceProfiler.h"
#include "../src/e57parserlib.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <algorithm>
#include <cmath>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#elif defined(Q_OS_LINUX)
#include <unistd.h>
#include <fstream>
#include <sstream>
#endif

PerformanceProfiler::PerformanceProfiler(QObject* parent) 
    : QObject(parent), m_baselineMemory(0), m_peakMemory(0), m_monitoringActive(false)
{
    m_parser = std::make_unique<E57ParserLib>(this);
    m_baselineMemory = getCurrentMemoryUsage();
    
    // Connect parser signals for detailed monitoring
    connect(m_parser.get(), &E57ParserLib::progressUpdated,
            this, [this](int percentage, const QString& stage) {
        if (m_verboseLogging) {
            qDebug() << "Parser progress:" << percentage << "%" << stage;
        }
        emit profilingProgress(stage, percentage);
    });
}

PerformanceProfiler::~PerformanceProfiler() = default;

PerformanceProfiler::PerformanceMetrics PerformanceProfiler::profileE57Loading(
    const QString& filePath, const OptimizationSettings& settings) {
    
    PerformanceMetrics metrics;
    metrics.fileName = QFileInfo(filePath).fileName();
    metrics.fileSize = QFileInfo(filePath).size();
    metrics.optimizationSettings = settings.toString();
    
    emit profilingStarted(metrics.fileName);
    emit profilingProgress("Starting profiling", 0);
    
    try {
        MemoryMonitor memMonitor(this);
        
        // Phase 1: XML parsing time measurement
        emit profilingProgress("Measuring XML parsing", 10);
        m_timer.start();
        
        bool openResult = m_parser->openFile(filePath.toStdString());
        metrics.xmlParseTime = m_timer.elapsed() / 1000.0; // Convert to seconds
        
        if (!openResult) {
            metrics.success = false;
            metrics.errorMessage = QString::fromStdString(m_parser->getLastError());
            emit profilingFinished(metrics);
            return metrics;
        }
        
        // Get scan information
        int scanCount = m_parser->getScanCount();
        if (scanCount == 0) {
            metrics.success = false;
            metrics.errorMessage = "No scans found in E57 file";
            m_parser->closeFile();
            emit profilingFinished(metrics);
            return metrics;
        }
        
        emit profilingProgress("Analyzing scan metadata", 20);
        
        // Count total points across all scans
        metrics.pointCount = 0;
        for (int i = 0; i < scanCount; ++i) {
            int64_t scanPoints = m_parser->getPointCount(i);
            metrics.pointCount += scanPoints;
        }
        
        // Phase 2: Binary data reading time measurement
        emit profilingProgress("Measuring binary data reading", 30);
        
        m_timer.restart();
        size_t memoryBefore = getCurrentMemoryUsage();
        
        std::vector<float> allPoints;
        
        // Apply subsampling if configured
        int maxPointsToRead = (settings.subsamplingRatio < 1.0) ? 
            static_cast<int>(metrics.pointCount * settings.subsamplingRatio) : -1;
        
        for (int i = 0; i < scanCount; ++i) {
            emit profilingProgress(QString("Reading scan %1/%2").arg(i+1).arg(scanCount), 
                                 30 + (40 * i / scanCount));
            
            int scanMaxPoints = maxPointsToRead > 0 ? 
                std::min(maxPointsToRead, static_cast<int>(m_parser->getPointCount(i))) : -1;
            
            auto points = m_parser->extractPointData(i);
            
            if (scanMaxPoints > 0 && points.size() > static_cast<size_t>(scanMaxPoints * 3)) {
                points.resize(scanMaxPoints * 3); // Limit to XYZ coordinates
            }
            
            allPoints.insert(allPoints.end(), points.begin(), points.end());
            
            if (maxPointsToRead > 0) {
                maxPointsToRead -= static_cast<int>(points.size() / 3);
                if (maxPointsToRead <= 0) break;
            }
        }
        
        metrics.binaryReadTime = m_timer.elapsed() / 1000.0;
        size_t memoryAfter = getCurrentMemoryUsage();
        
        emit profilingProgress("Measuring data conversion", 80);
        
        // Phase 3: Data conversion time (if any additional processing)
        m_timer.restart();
        
        // Simulate additional data processing that might be needed
        if (settings.enableParallelProcessing) {
            // Example: parallel coordinate validation
            #pragma omp parallel for
            for (size_t i = 0; i < allPoints.size(); i += 3) {
                if (i + 2 < allPoints.size()) {
                    // Simple validation/processing
                    volatile float sum = allPoints[i] + allPoints[i+1] + allPoints[i+2];
                    (void)sum; // Suppress unused variable warning
                }
            }
        }
        
        metrics.dataConversionTime = m_timer.elapsed() / 1000.0;
        
        // Calculate final metrics
        metrics.totalLoadTime = metrics.xmlParseTime + metrics.binaryReadTime + metrics.dataConversionTime;
        metrics.peakMemoryUsage = memMonitor.getPeakUsage();
        metrics.finalMemoryUsage = memoryAfter - memoryBefore;
        
        if (metrics.totalLoadTime > 0) {
            metrics.pointsPerSecond = (allPoints.size() / 3) / metrics.totalLoadTime;
        }
        
        if (allPoints.size() > 0) {
            metrics.memoryEfficiency = calculateMemoryEfficiency(metrics.finalMemoryUsage, allPoints.size() / 3);
        }
        
        metrics.success = true;
        
        m_parser->closeFile();
        
        emit profilingProgress("Profiling complete", 100);
        
    } catch (const std::exception& ex) {
        metrics.success = false;
        metrics.errorMessage = QString("Exception during profiling: %1").arg(ex.what());
    }
    
    emit profilingFinished(metrics);
    return metrics;
}

std::vector<PerformanceProfiler::PerformanceMetrics> PerformanceProfiler::runBenchmarkSuite(
    const BenchmarkConfig& config) {
    
    std::vector<PerformanceMetrics> allResults;
    
    qDebug() << "Starting benchmark suite with" << config.testFiles.size() << "files and" 
             << config.optimizationVariants.size() << "optimization variants";
    
    int totalTests = config.testFiles.size() * static_cast<int>(config.optimizationVariants.size());
    int currentTest = 0;
    
    for (const QString& filePath : config.testFiles) {
        if (!QFile::exists(filePath)) {
            qWarning() << "Benchmark file not found:" << filePath;
            continue;
        }
        
        for (const auto& settings : config.optimizationVariants) {
            currentTest++;
            
            qDebug() << QString("Benchmark %1/%2: %3 with %4")
                        .arg(currentTest).arg(totalTests)
                        .arg(QFileInfo(filePath).fileName())
                        .arg(settings.description);
            
            auto metrics = profileE57Loading(filePath, settings);
            allResults.push_back(metrics);
            
            emit benchmarkCompleted(metrics);
            
            // Brief pause between tests to allow system to stabilize
            QThread::msleep(100);
        }
    }
    
    if (config.generateDetailedReport) {
        QString reportPath = QDir(config.outputDirectory).absoluteFilePath(
            QString("Benchmark_Report_%1.html").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")));
        generatePerformanceReport(allResults, reportPath);
    }
    
    return allResults;
}

std::vector<PerformanceProfiler::PerformanceMetrics> PerformanceProfiler::compareOptimizations(
    const QString& filePath) {
    
    std::vector<PerformanceMetrics> results;
    auto optimizationVariants = generateOptimizationVariants();
    
    qDebug() << "Comparing" << optimizationVariants.size() << "optimization settings for" 
             << QFileInfo(filePath).fileName();
    
    for (const auto& settings : optimizationVariants) {
        auto metrics = profileE57Loading(filePath, settings);
        results.push_back(metrics);
        
        qDebug() << QString("Optimization '%1': %2 points/sec, %3 MB peak memory")
                    .arg(settings.description)
                    .arg(metrics.pointsPerSecond, 0, 'f', 0)
                    .arg(metrics.peakMemoryUsage / (1024 * 1024));
    }
    
    // Sort by performance (points per second)
    std::sort(results.begin(), results.end(), 
              [](const PerformanceMetrics& a, const PerformanceMetrics& b) {
                  return a.pointsPerSecond > b.pointsPerSecond;
              });
    
    return results;
}

size_t PerformanceProfiler::getCurrentMemoryUsage() {
#ifdef Q_OS_WIN
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
#elif defined(Q_OS_LINUX)
    std::ifstream statusFile("/proc/self/status");
    std::string line;
    while (std::getline(statusFile, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            std::istringstream iss(line);
            std::string label, value, unit;
            iss >> label >> value >> unit;
            if (unit == "kB") {
                return std::stoull(value) * 1024; // Convert KB to bytes
            }
        }
    }
#endif
    return 0; // Fallback
}

double PerformanceProfiler::calculatePointsPerSecond(int64_t pointCount, double timeSeconds) {
    return timeSeconds > 0.0 ? pointCount / timeSeconds : 0.0;
}

double PerformanceProfiler::calculateMemoryEfficiency(size_t memoryBytes, int64_t pointCount) {
    if (pointCount == 0) return 0.0;
    double pointsInMillions = pointCount / 1000000.0;
    double memoryInMB = memoryBytes / (1024.0 * 1024.0);
    return pointsInMillions > 0 ? memoryInMB / pointsInMillions : memoryInMB;
}

std::vector<PerformanceProfiler::OptimizationSettings> PerformanceProfiler::generateOptimizationVariants() {
    std::vector<OptimizationSettings> variants;

    // Baseline configuration
    OptimizationSettings baseline;
    baseline.description = "Baseline (64K buffer)";
    variants.push_back(baseline);

    // Buffer size variants
    OptimizationSettings smallBuffer = baseline;
    smallBuffer.bufferSize = 32768;
    smallBuffer.description = "Small buffer (32K)";
    variants.push_back(smallBuffer);

    OptimizationSettings largeBuffer = baseline;
    largeBuffer.bufferSize = 131072;
    largeBuffer.description = "Large buffer (128K)";
    variants.push_back(largeBuffer);

    // Parallel processing variant
    OptimizationSettings parallel = baseline;
    parallel.enableParallelProcessing = true;
    parallel.description = "Parallel processing";
    variants.push_back(parallel);

    // Subsampling variants
    OptimizationSettings subsample50 = baseline;
    subsample50.subsamplingRatio = 0.5;
    subsample50.description = "50% subsampling";
    variants.push_back(subsample50);

    OptimizationSettings subsample10 = baseline;
    subsample10.subsamplingRatio = 0.1;
    subsample10.description = "10% subsampling";
    variants.push_back(subsample10);

    return variants;
}

void PerformanceProfiler::generatePerformanceReport(const std::vector<PerformanceMetrics>& metrics,
                                                   const QString& outputPath) {
    QString reportPath = outputPath.isEmpty() ?
        QString("Performance_Report_%1.html").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")) :
        outputPath;

    // Ensure output directory exists
    QDir().mkpath(QFileInfo(reportPath).absolutePath());

    QFile reportFile(reportPath);
    if (!reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to create performance report file:" << reportPath;
        return;
    }

    QTextStream out(&reportFile);

    // Generate HTML report
    out << "<!DOCTYPE html>\n<html>\n<head>\n";
    out << "<title>E57 Performance Profiling Report</title>\n";
    out << "<style>\n";
    out << "body { font-family: Arial, sans-serif; margin: 20px; }\n";
    out << "table { border-collapse: collapse; width: 100%; margin: 20px 0; }\n";
    out << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
    out << "th { background-color: #f2f2f2; }\n";
    out << ".metric { background-color: #f9f9f9; padding: 10px; margin: 10px 0; }\n";
    out << ".good { color: green; font-weight: bold; }\n";
    out << ".warning { color: orange; font-weight: bold; }\n";
    out << ".poor { color: red; font-weight: bold; }\n";
    out << "</style>\n</head>\n<body>\n";

    out << "<h1>E57 Performance Profiling Report</h1>\n";
    out << "<p>Generated: " << QDateTime::currentDateTime().toString() << "</p>\n";

    // Summary statistics
    if (!metrics.empty()) {
        double avgLoadTime = 0.0;
        double avgPointsPerSec = 0.0;
        size_t avgMemoryUsage = 0;
        int successfulTests = 0;

        for (const auto& metric : metrics) {
            if (metric.success) {
                avgLoadTime += metric.totalLoadTime;
                avgPointsPerSec += metric.pointsPerSecond;
                avgMemoryUsage += metric.peakMemoryUsage;
                successfulTests++;
            }
        }

        if (successfulTests > 0) {
            avgLoadTime /= successfulTests;
            avgPointsPerSec /= successfulTests;
            avgMemoryUsage /= successfulTests;
        }

        out << "<div class='metric'>\n";
        out << "<h2>Summary Statistics</h2>\n";
        out << "<p>Total Tests: " << metrics.size() << "</p>\n";
        out << "<p>Successful Tests: " << successfulTests << "</p>\n";
        out << "<p>Average Load Time: " << QString::number(avgLoadTime, 'f', 3) << " seconds</p>\n";
        out << "<p>Average Points/Second: " << QString::number(avgPointsPerSec, 'f', 0) << "</p>\n";
        out << "<p>Average Peak Memory: " << (avgMemoryUsage / (1024 * 1024)) << " MB</p>\n";
        out << "</div>\n";
    }

    qDebug() << "Performance report generated:" << reportPath;
}

// MemoryMonitor implementation
MemoryMonitor::MemoryMonitor(PerformanceProfiler* profiler)
    : m_profiler(profiler), m_startMemory(0), m_peakMemory(0) {
    if (m_profiler) {
        m_startMemory = m_profiler->getCurrentMemoryUsage();
        m_peakMemory = m_startMemory;
        m_profiler->startMemoryMonitoring();
    }
}

MemoryMonitor::~MemoryMonitor() {
    if (m_profiler) {
        m_profiler->stopMemoryMonitoring();
    }
}

size_t MemoryMonitor::getCurrentUsage() const {
    return m_profiler ? m_profiler->getCurrentMemoryUsage() : 0;
}

size_t MemoryMonitor::getPeakUsage() const {
    return m_profiler ? m_profiler->getPeakMemoryUsage() : m_peakMemory;
}

void PerformanceProfiler::startMemoryMonitoring() {
    m_monitoringActive = true;
    m_peakMemory = getCurrentMemoryUsage();
    m_memorySnapshots.clear();
    m_memorySnapshots.push_back(m_peakMemory);
}

void PerformanceProfiler::stopMemoryMonitoring() {
    m_monitoringActive = false;
}

size_t PerformanceProfiler::getPeakMemoryUsage() {
    if (m_monitoringActive) {
        size_t current = getCurrentMemoryUsage();
        m_peakMemory = std::max(m_peakMemory, current);
        m_memorySnapshots.push_back(current);
    }
    return m_peakMemory;
}

QJsonObject PerformanceProfiler::exportMetricsToJson(const std::vector<PerformanceMetrics>& metrics) {
    QJsonObject root;
    root["reportType"] = "E57PerformanceMetrics";
    root["generatedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["totalTests"] = static_cast<int>(metrics.size());

    QJsonArray metricsArray;
    for (const auto& metric : metrics) {
        QJsonObject metricObj;
        metricObj["fileName"] = metric.fileName;
        metricObj["fileSize"] = static_cast<qint64>(metric.fileSize);
        metricObj["pointCount"] = static_cast<qint64>(metric.pointCount);
        metricObj["totalLoadTime"] = metric.totalLoadTime;
        metricObj["xmlParseTime"] = metric.xmlParseTime;
        metricObj["binaryReadTime"] = metric.binaryReadTime;
        metricObj["dataConversionTime"] = metric.dataConversionTime;
        metricObj["peakMemoryUsage"] = static_cast<qint64>(metric.peakMemoryUsage);
        metricObj["finalMemoryUsage"] = static_cast<qint64>(metric.finalMemoryUsage);
        metricObj["pointsPerSecond"] = metric.pointsPerSecond;
        metricObj["memoryEfficiency"] = metric.memoryEfficiency;
        metricObj["optimizationSettings"] = metric.optimizationSettings;
        metricObj["success"] = metric.success;
        metricObj["errorMessage"] = metric.errorMessage;

        metricsArray.append(metricObj);
    }

    root["metrics"] = metricsArray;
    return root;
}
