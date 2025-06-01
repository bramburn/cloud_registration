#include "E57TestFramework.h"
#include "../src/e57parserlib.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QTextStream>
#include <QDateTime>
#include <algorithm>
#include <cmath>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#elif defined(Q_OS_LINUX)
#include <unistd.h>
#include <fstream>
#endif

E57TestFramework::E57TestFramework(QObject* parent) 
    : QObject(parent)
{
    m_parser = std::make_unique<E57ParserLib>(this);
    
    // Connect parser signals for monitoring
    connect(m_parser.get(), &E57ParserLib::progressUpdated,
            this, [this](int percentage, const QString& stage) {
        qDebug() << "Parser progress:" << percentage << "%" << stage;
    });
}

E57TestFramework::~E57TestFramework() = default;

void E57TestFramework::loadTestSuite(const QString& testConfigPath) {
    QFile configFile(testConfigPath);
    if (!configFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open test configuration file:" << testConfigPath;
        return;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return;
    }

    QJsonObject config = doc.object();
    
    // Set test data directory
    if (config.contains("testDataDirectory")) {
        m_testDataDirectory = config["testDataDirectory"].toString();
    }
    
    // Load test files
    if (config.contains("testFiles")) {
        QJsonArray testFilesArray = config["testFiles"].toArray();
        
        for (const QJsonValue& value : testFilesArray) {
            QJsonObject fileObj = value.toObject();
            
            TestFileMetadata metadata;
            metadata.filePath = QDir(m_testDataDirectory).absoluteFilePath(fileObj["fileName"].toString());
            metadata.vendor = fileObj["vendor"].toString();
            metadata.software = fileObj["software"].toString();
            metadata.expectedScanCount = fileObj["expectedScanCount"].toInt(1);
            metadata.expectedPointCount = fileObj["expectedPointCount"].toInt(0);
            metadata.hasIntensity = fileObj["hasIntensity"].toBool(false);
            metadata.hasColor = fileObj["hasColor"].toBool(false);
            metadata.hasMultipleScans = fileObj["hasMultipleScans"].toBool(false);
            metadata.shouldFail = fileObj["shouldFail"].toBool(false);
            metadata.expectedErrorType = fileObj["expectedErrorType"].toString();
            metadata.description = fileObj["description"].toString();
            
            m_testFiles.push_back(metadata);
        }
    }
    
    qDebug() << "Loaded" << m_testFiles.size() << "test files from configuration";
}

void E57TestFramework::addTestFile(const TestFileMetadata& metadata) {
    m_testFiles.push_back(metadata);
}

std::vector<E57TestFramework::TestResult> E57TestFramework::runComprehensiveTests() {
    std::vector<TestResult> results;
    results.reserve(m_testFiles.size());
    
    emit testSuiteStarted(static_cast<int>(m_testFiles.size()));
    
    for (size_t i = 0; i < m_testFiles.size(); ++i) {
        const auto& metadata = m_testFiles[i];
        
        qDebug() << "Running test" << (i + 1) << "of" << m_testFiles.size() 
                 << ":" << metadata.filePath;
        
        TestResult result;
        result.fileName = QFileInfo(metadata.filePath).fileName();
        result.testCategory = determineTestCategory(metadata);
        
        m_timer.start();
        
        try {
            // Check if file exists
            if (!QFile::exists(metadata.filePath)) {
                if (metadata.shouldFail) {
                    result.success = true;
                    result.errorMessage = "File not found (expected for negative test)";
                } else {
                    result.success = false;
                    result.errorMessage = "Test file not found: " + metadata.filePath;
                }
            } else {
                // Run comprehensive test
                bool loadSuccess = testFileLoading(metadata, result);
                bool integritySuccess = loadSuccess ? testDataIntegrity(metadata, result) : false;
                bool attributeSuccess = loadSuccess ? testAttributeExtraction(metadata, result) : false;
                bool performanceSuccess = loadSuccess ? testPerformance(metadata, result) : false;
                
                result.success = loadSuccess && integritySuccess && attributeSuccess && performanceSuccess;
                result.dataIntegrityPassed = integritySuccess;
                result.attributeValidationPassed = attributeSuccess;
            }
            
        } catch (const std::exception& ex) {
            result.success = false;
            result.errorMessage = QString("Exception during test: %1").arg(ex.what());
        }
        
        result.loadTime = m_timer.elapsed() / 1000.0; // Convert to seconds
        result.memoryUsage = getCurrentMemoryUsage();
        
        results.push_back(result);
        emit testCompleted(result);
        emit testProgress(static_cast<int>(i + 1), static_cast<int>(m_testFiles.size()));
    }
    
    updateStatistics(results);
    emit testSuiteCompleted(m_lastStats);
    
    return results;
}

bool E57TestFramework::testFileLoading(const TestFileMetadata& metadata, TestResult& result) {
    try {
        bool openResult = m_parser->openFile(metadata.filePath.toStdString());
        
        if (metadata.shouldFail) {
            // For negative tests, failure is expected
            if (!openResult) {
                result.errorMessage = "File failed to open as expected";
                return true;
            } else {
                result.errorMessage = "File opened but was expected to fail";
                return false;
            }
        }
        
        if (!openResult) {
            result.errorMessage = "Failed to open E57 file: " + QString::fromStdString(m_parser->getLastError());
            return false;
        }
        
        // Validate scan count
        result.actualScanCount = m_parser->getScanCount();
        if (metadata.expectedScanCount > 0 && result.actualScanCount != metadata.expectedScanCount) {
            result.errorMessage = QString("Scan count mismatch: expected %1, got %2")
                                 .arg(metadata.expectedScanCount)
                                 .arg(result.actualScanCount);
            m_parser->closeFile();
            return false;
        }
        
        // Count total points across all scans
        result.actualPointCount = 0;
        for (int i = 0; i < result.actualScanCount; ++i) {
            int64_t scanPoints = m_parser->getPointCount(i);
            result.actualPointCount += scanPoints;
        }
        
        m_parser->closeFile();
        return true;
        
    } catch (const std::exception& ex) {
        result.errorMessage = QString("Exception during file loading: %1").arg(ex.what());
        return false;
    }
}

bool E57TestFramework::testDataIntegrity(const TestFileMetadata& metadata, TestResult& result) {
    try {
        if (!m_parser->openFile(metadata.filePath.toStdString())) {
            result.errorMessage = "Failed to reopen file for integrity test";
            return false;
        }
        
        // Test first scan only for performance
        if (m_parser->getScanCount() > 0) {
            // Extract sample points for validation
            int maxPoints = std::min(m_maxTestPoints, static_cast<int>(m_parser->getPointCount(0)));
            
            auto points = m_parser->extractPointData(0);
            
            if (points.empty() && maxPoints > 0) {
                result.errorMessage = "No points extracted despite non-zero point count";
                m_parser->closeFile();
                return false;
            }
            
            // Validate coordinate integrity
            if (!validateCoordinates(points)) {
                result.errorMessage = "Coordinate validation failed";
                m_parser->closeFile();
                return false;
            }
        }
        
        m_parser->closeFile();
        return true;
        
    } catch (const std::exception& ex) {
        result.errorMessage = QString("Exception during integrity test: %1").arg(ex.what());
        return false;
    }
}

QString E57TestFramework::determineTestCategory(const TestFileMetadata& metadata) {
    if (metadata.shouldFail) return "Error Handling";
    if (metadata.hasMultipleScans) return "Multi-Scan";
    if (metadata.hasIntensity && metadata.hasColor) return "Full Attributes";
    if (metadata.hasIntensity) return "Intensity";
    if (metadata.hasColor) return "Color";
    if (!metadata.vendor.isEmpty()) return "Vendor: " + metadata.vendor;
    return "Basic";
}

bool E57TestFramework::testAttributeExtraction(const TestFileMetadata& metadata, TestResult& result) {
    try {
        if (!m_parser->openFile(metadata.filePath.toStdString())) {
            result.errorMessage = "Failed to open file for attribute test";
            return false;
        }

        if (m_parser->getScanCount() > 0) {
            // Test enhanced point data extraction
            auto enhancedPoints = m_parser->extractEnhancedPointData(0);

            if (!enhancedPoints.empty()) {
                // Validate intensity attributes
                if (metadata.hasIntensity) {
                    bool hasIntensityData = std::any_of(enhancedPoints.begin(), enhancedPoints.end(),
                        [](const auto& p) { return p.hasIntensity; });

                    if (!hasIntensityData) {
                        result.errorMessage = "Expected intensity data but none found";
                        m_parser->closeFile();
                        return false;
                    }
                }

                // Validate color attributes
                if (metadata.hasColor) {
                    bool hasColorData = std::any_of(enhancedPoints.begin(), enhancedPoints.end(),
                        [](const auto& p) { return p.hasColor; });

                    if (!hasColorData) {
                        result.errorMessage = "Expected color data but none found";
                        m_parser->closeFile();
                        return false;
                    }
                }
            }
        }

        m_parser->closeFile();
        return true;

    } catch (const std::exception& ex) {
        result.errorMessage = QString("Exception during attribute test: %1").arg(ex.what());
        return false;
    }
}

bool E57TestFramework::testPerformance(const TestFileMetadata& metadata, TestResult& result) {
    try {
        QElapsedTimer perfTimer;
        perfTimer.start();

        if (!m_parser->openFile(metadata.filePath.toStdString())) {
            result.errorMessage = "Failed to open file for performance test";
            return false;
        }

        size_t memoryBefore = getCurrentMemoryUsage();

        // Extract points and measure time
        if (m_parser->getScanCount() > 0) {
            auto points = m_parser->extractPointData(0);

            // Basic performance validation
            double loadTime = perfTimer.elapsed() / 1000.0;
            size_t memoryAfter = getCurrentMemoryUsage();

            // Store performance metrics
            result.loadTime = loadTime;
            result.memoryUsage = memoryAfter - memoryBefore;

            // Performance thresholds (configurable)
            const double MAX_LOAD_TIME_PER_MILLION_POINTS = 60.0; // 60 seconds per million points
            const size_t MAX_MEMORY_PER_MILLION_POINTS = 1024 * 1024 * 1024; // 1GB per million points

            if (!points.empty()) {
                int64_t pointCount = points.size() / 3; // XYZ coordinates
                double pointsInMillions = pointCount / 1000000.0;

                if (pointsInMillions > 0) {
                    double timePerMillion = loadTime / pointsInMillions;
                    size_t memoryPerMillion = static_cast<size_t>(result.memoryUsage / pointsInMillions);

                    if (timePerMillion > MAX_LOAD_TIME_PER_MILLION_POINTS) {
                        result.errorMessage = QString("Performance warning: %1 seconds per million points (threshold: %2)")
                                             .arg(timePerMillion, 0, 'f', 2)
                                             .arg(MAX_LOAD_TIME_PER_MILLION_POINTS);
                        // Don't fail the test, just warn
                    }

                    if (memoryPerMillion > MAX_MEMORY_PER_MILLION_POINTS) {
                        result.errorMessage += QString(" Memory usage: %1 MB per million points")
                                              .arg(memoryPerMillion / (1024 * 1024));
                    }
                }
            }
        }

        m_parser->closeFile();
        return true;

    } catch (const std::exception& ex) {
        result.errorMessage = QString("Exception during performance test: %1").arg(ex.what());
        return false;
    }
}

bool E57TestFramework::validateCoordinates(const std::vector<float>& points) {
    if (points.empty()) return true;
    if (points.size() % 3 != 0) return false;

    // Check for reasonable coordinate ranges and valid values
    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    double minZ = std::numeric_limits<double>::max();
    double maxZ = std::numeric_limits<double>::lowest();

    int validPoints = 0;

    for (size_t i = 0; i < points.size(); i += 3) {
        float x = points[i];
        float y = points[i + 1];
        float z = points[i + 2];

        // Check for NaN or infinite values
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
            qWarning() << "Invalid coordinate values detected at point" << (i / 3);
            return false;
        }

        minX = std::min(minX, static_cast<double>(x));
        maxX = std::max(maxX, static_cast<double>(x));
        minY = std::min(minY, static_cast<double>(y));
        maxY = std::max(maxY, static_cast<double>(y));
        minZ = std::min(minZ, static_cast<double>(z));
        maxZ = std::max(maxZ, static_cast<double>(z));

        validPoints++;
    }

    // Check for reasonable coordinate ranges (not too large)
    double maxRange = std::max({maxX - minX, maxY - minY, maxZ - minZ});
    if (maxRange > 1e6) { // 1 million units seems excessive for most point clouds
        qWarning() << "Coordinate range seems excessive:" << maxRange;
        // Don't fail, just warn
    }

    return validPoints > 0;
}

size_t E57TestFramework::getCurrentMemoryUsage() {
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

void E57TestFramework::updateStatistics(const std::vector<TestResult>& results) {
    m_lastStats = TestSuiteStats();
    m_lastStats.totalTests = static_cast<int>(results.size());

    double totalTime = 0.0;
    size_t maxMemory = 0;

    for (const auto& result : results) {
        if (result.success) {
            m_lastStats.passedTests++;
        } else {
            m_lastStats.failedTests++;
        }

        totalTime += result.loadTime;
        maxMemory = std::max(maxMemory, result.memoryUsage);
    }

    m_lastStats.totalTime = totalTime;
    m_lastStats.averageLoadTime = m_lastStats.totalTests > 0 ? totalTime / m_lastStats.totalTests : 0.0;
    m_lastStats.peakMemoryUsage = maxMemory;
}

void E57TestFramework::generateTestReport(const std::vector<TestResult>& results, const QString& outputPath) {
    QString reportPath = outputPath.isEmpty() ?
        QString("E57_Test_Report_%1.html").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")) :
        outputPath;

    QFile reportFile(reportPath);
    if (!reportFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to create test report file:" << reportPath;
        return;
    }

    QTextStream out(&reportFile);

    // Generate HTML report
    out << "<!DOCTYPE html>\n<html>\n<head>\n";
    out << "<title>E57 Library Integration Test Report</title>\n";
    out << "<style>\n";
    out << "body { font-family: Arial, sans-serif; margin: 20px; }\n";
    out << "table { border-collapse: collapse; width: 100%; }\n";
    out << "th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }\n";
    out << "th { background-color: #f2f2f2; }\n";
    out << ".pass { color: green; font-weight: bold; }\n";
    out << ".fail { color: red; font-weight: bold; }\n";
    out << ".summary { background-color: #f9f9f9; padding: 15px; margin: 20px 0; }\n";
    out << "</style>\n</head>\n<body>\n";

    out << "<h1>E57 Library Integration Test Report</h1>\n";
    out << "<p>Generated: " << QDateTime::currentDateTime().toString() << "</p>\n";

    // Summary section
    out << "<div class='summary'>\n";
    out << "<h2>Test Summary</h2>\n";
    out << "<p>Total Tests: " << m_lastStats.totalTests << "</p>\n";
    out << "<p>Passed: <span class='pass'>" << m_lastStats.passedTests << "</span></p>\n";
    out << "<p>Failed: <span class='fail'>" << m_lastStats.failedTests << "</span></p>\n";
    out << "<p>Success Rate: " << (m_lastStats.totalTests > 0 ?
        (100.0 * m_lastStats.passedTests / m_lastStats.totalTests) : 0.0) << "%</p>\n";
    out << "<p>Total Time: " << m_lastStats.totalTime << " seconds</p>\n";
    out << "<p>Average Load Time: " << m_lastStats.averageLoadTime << " seconds</p>\n";
    out << "<p>Peak Memory Usage: " << (m_lastStats.peakMemoryUsage / (1024 * 1024)) << " MB</p>\n";
    out << "</div>\n";

    // Detailed results table
    out << "<h2>Detailed Results</h2>\n";
    out << "<table>\n";
    out << "<tr><th>File</th><th>Category</th><th>Status</th><th>Load Time (s)</th>";
    out << "<th>Memory (MB)</th><th>Scans</th><th>Points</th><th>Error Message</th></tr>\n";

    for (const auto& result : results) {
        out << "<tr>\n";
        out << "<td>" << result.fileName << "</td>\n";
        out << "<td>" << result.testCategory << "</td>\n";
        out << "<td class='" << (result.success ? "pass" : "fail") << "'>"
            << (result.success ? "PASS" : "FAIL") << "</td>\n";
        out << "<td>" << QString::number(result.loadTime, 'f', 3) << "</td>\n";
        out << "<td>" << (result.memoryUsage / (1024 * 1024)) << "</td>\n";
        out << "<td>" << result.actualScanCount << "</td>\n";
        out << "<td>" << result.actualPointCount << "</td>\n";
        out << "<td>" << result.errorMessage << "</td>\n";
        out << "</tr>\n";
    }

    out << "</table>\n";
    out << "</body>\n</html>\n";

    qDebug() << "Test report generated:" << reportPath;
}
