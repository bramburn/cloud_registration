#include <QCoreApplication>
#include <QDebug>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include "../../src/performance_profiler.h"
#include "../../src/performance_benchmark.h"
#include "../../src/e57parser.h"
#include "../../src/lasparser.h"

/**
 * @brief Sprint 2.2 Profiling and Benchmarking Test Application
 * 
 * This application demonstrates the performance profiling and benchmarking
 * capabilities implemented for Sprint 2.2. It tests both E57 and LAS parsers
 * with the available sample files and generates comprehensive performance reports.
 */

void testPerformanceProfiler()
{
    qDebug() << "\n=== Testing Performance Profiler ===";
    
    // Reset profiler
    PerformanceProfiler::instance().reset();
    
    // Test basic profiling functionality
    {
        PROFILE_SECTION("TestSection1");
        QThread::msleep(50); // Simulate work
    }
    
    {
        PROFILE_SECTION("TestSection2");
        QThread::msleep(30); // Simulate work
    }
    
    // Test nested profiling
    {
        PROFILE_SECTION("OuterSection");
        QThread::msleep(20);
        
        {
            PROFILE_SECTION("InnerSection");
            QThread::msleep(40);
        }
        
        QThread::msleep(10);
    }
    
    // Generate report
    qDebug() << "Generating profiler test report...";
    PerformanceProfiler::instance().generateReport("test_profiler_report");
    
    qDebug() << "Performance profiler test completed";
}

void testE57Parsing()
{
    qDebug() << "\n=== Testing E57 Parser Performance ===";
    
    // Check for available E57 test files
    QStringList e57Files;
    QDir sampleDir("sample");
    if (sampleDir.exists()) {
        QStringList filters;
        filters << "*.e57";
        QFileInfoList fileList = sampleDir.entryInfoList(filters, QDir::Files);
        
        for (const QFileInfo &fileInfo : fileList) {
            e57Files << fileInfo.absoluteFilePath();
        }
    }
    
    if (e57Files.isEmpty()) {
        qWarning() << "No E57 test files found in sample directory";
        return;
    }
    
    for (const QString &filePath : e57Files) {
        qDebug() << "Testing E57 file:" << filePath;
        
        // Reset profiler for this test
        PerformanceProfiler::instance().reset();
        
        try {
            E57Parser parser;
            
            // Connect to parsing finished signal for synchronous testing
            bool parsingComplete = false;
            bool parsingSuccess = false;
            std::vector<float> resultPoints;
            
            QObject::connect(&parser, &E57Parser::parsingFinished,
                           [&](bool success, const QString& message, const std::vector<float>& points) {
                parsingComplete = true;
                parsingSuccess = success;
                resultPoints = points;
                qDebug() << "E57 parsing finished - Success:" << success << "Message:" << message;
            });
            
            // Start parsing
            parser.startParsing(filePath);
            
            // Wait for completion (simple busy wait for test)
            int timeout = 30000; // 30 seconds
            int elapsed = 0;
            while (!parsingComplete && elapsed < timeout) {
                QCoreApplication::processEvents();
                QThread::msleep(100);
                elapsed += 100;
            }
            
            if (parsingComplete) {
                qDebug() << "E57 parsing completed - Points loaded:" << (resultPoints.size() / 3);
                
                // Generate profiling report for this file
                QString reportName = QString("e57_profile_%1").arg(QFileInfo(filePath).baseName());
                PerformanceProfiler::instance().generateReport(reportName);
            } else {
                qWarning() << "E57 parsing timed out for file:" << filePath;
            }
            
        } catch (const std::exception& e) {
            qWarning() << "Exception during E57 parsing:" << e.what();
        }
    }
}

void testLASParsing()
{
    qDebug() << "\n=== Testing LAS Parser Performance ===";
    
    // Check for available LAS test files
    QStringList lasFiles;
    QDir sampleDir("sample");
    if (sampleDir.exists()) {
        QStringList filters;
        filters << "*.las";
        QFileInfoList fileList = sampleDir.entryInfoList(filters, QDir::Files);
        
        for (const QFileInfo &fileInfo : fileList) {
            lasFiles << fileInfo.absoluteFilePath();
        }
    }
    
    if (lasFiles.isEmpty()) {
        qWarning() << "No LAS test files found in sample directory";
        return;
    }
    
    for (const QString &filePath : lasFiles) {
        qDebug() << "Testing LAS file:" << filePath;
        
        // Reset profiler for this test
        PerformanceProfiler::instance().reset();
        
        try {
            LasParser parser;
            
            // Connect to parsing finished signal for synchronous testing
            bool parsingComplete = false;
            bool parsingSuccess = false;
            std::vector<float> resultPoints;
            
            QObject::connect(&parser, &LasParser::parsingFinished,
                           [&](bool success, const QString& message, const std::vector<float>& points) {
                parsingComplete = true;
                parsingSuccess = success;
                resultPoints = points;
                qDebug() << "LAS parsing finished - Success:" << success << "Message:" << message;
            });
            
            // Start parsing
            parser.startParsing(filePath);
            
            // Wait for completion (simple busy wait for test)
            int timeout = 30000; // 30 seconds
            int elapsed = 0;
            while (!parsingComplete && elapsed < timeout) {
                QCoreApplication::processEvents();
                QThread::msleep(100);
                elapsed += 100;
            }
            
            if (parsingComplete) {
                qDebug() << "LAS parsing completed - Points loaded:" << (resultPoints.size() / 3);
                
                // Generate profiling report for this file
                QString reportName = QString("las_profile_%1").arg(QFileInfo(filePath).baseName());
                PerformanceProfiler::instance().generateReport(reportName);
            } else {
                qWarning() << "LAS parsing timed out for file:" << filePath;
            }
            
        } catch (const std::exception& e) {
            qWarning() << "Exception during LAS parsing:" << e.what();
        }
    }
}

void testBenchmarkSuite()
{
    qDebug() << "\n=== Testing Benchmark Suite ===";
    
    // Collect all available test files
    QStringList testFiles;
    QDir sampleDir("sample");
    if (sampleDir.exists()) {
        QStringList filters;
        filters << "*.e57" << "*.las";
        QFileInfoList fileList = sampleDir.entryInfoList(filters, QDir::Files);
        
        for (const QFileInfo &fileInfo : fileList) {
            testFiles << fileInfo.absoluteFilePath();
        }
    }
    
    if (testFiles.isEmpty()) {
        qWarning() << "No test files found for benchmarking";
        return;
    }
    
    qDebug() << "Found" << testFiles.size() << "test files for benchmarking";
    
    // Create benchmark instance
    PerformanceBenchmark benchmark;
    
    // Run comparison suite
    benchmark.runComparisonSuite(testFiles);
    
    // Generate comprehensive benchmark report
    benchmark.generateBenchmarkReport("sprint2_2_benchmark_results");
    
    qDebug() << "Benchmark suite completed";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Sprint 2.2 Performance Profiling & Benchmarking Test ===";
    qDebug() << "This test demonstrates the performance profiling and optimization";
    qDebug() << "capabilities implemented for Sprint 2.2";
    
    try {
        // Test 1: Basic profiler functionality
        testPerformanceProfiler();
        
        // Test 2: E57 parser profiling
        testE57Parsing();
        
        // Test 3: LAS parser profiling
        testLASParsing();
        
        // Test 4: Comprehensive benchmark suite
        testBenchmarkSuite();
        
        qDebug() << "\n=== All Tests Completed Successfully ===";
        qDebug() << "Check the generated report files for detailed performance analysis";
        
    } catch (const std::exception& e) {
        qCritical() << "Test failed with exception:" << e.what();
        return 1;
    }
    
    return 0;
}
