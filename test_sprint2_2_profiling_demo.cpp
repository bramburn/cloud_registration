/**
 * @file test_sprint2_2_profiling_demo.cpp
 * @brief Sprint 2.2 Performance Profiling Demonstration
 * 
 * This test demonstrates the performance profiling functionality implemented
 * in Sprint 2.2, using the existing sample files to generate detailed
 * performance reports and timing breakdowns.
 */

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QTimer>
#include <QEventLoop>
#include <iostream>

#include "src/performance_profiler.h"
#include "src/performance_benchmark.h"
#include "src/e57parser.h"
#include "src/lasparser.h"

class Sprint22ProfilingDemo : public QObject
{
    Q_OBJECT

public:
    explicit Sprint22ProfilingDemo(QObject *parent = nullptr) : QObject(parent) {}

    void runDemo()
    {
        qDebug() << "\n=== SPRINT 2.2 PERFORMANCE PROFILING DEMONSTRATION ===";
        qDebug() << "Testing profiling functionality with sample files\n";

        // Test 1: Basic profiler functionality
        testBasicProfiler();

        // Test 2: E57 file profiling
        testE57Profiling();

        // Test 3: LAS file profiling
        testLASProfiling();

        // Test 4: Comprehensive benchmark suite
        testBenchmarkSuite();

        qDebug() << "\n=== SPRINT 2.2 PROFILING DEMO COMPLETED ===";
    }

private slots:
    void onE57ParsingFinished(bool success, const QString &message, const std::vector<float> &points)
    {
        qDebug() << "E57 parsing finished:" << success << "Message:" << message;
        qDebug() << "Points loaded:" << (points.size() / 3);
        
        // Generate profiling report
        PerformanceProfiler::instance().generateReport("e57_profile_report");
        
        m_parsingComplete = true;
    }

    void onLASParsingFinished(bool success, const QString &message, const std::vector<float> &points)
    {
        qDebug() << "LAS parsing finished:" << success << "Message:" << message;
        qDebug() << "Points loaded:" << (points.size() / 3);
        
        // Generate profiling report
        PerformanceProfiler::instance().generateReport("las_profile_report");
        
        m_parsingComplete = true;
    }

private:
    void testBasicProfiler()
    {
        qDebug() << "\n--- Test 1: Basic Profiler Functionality ---";
        
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
        
        // Generate and display report
        PerformanceProfiler::instance().generateReport("basic_profiler_test");
        
        qDebug() << "✓ Basic profiler test completed";
    }

    void testE57Profiling()
    {
        qDebug() << "\n--- Test 2: E57 File Profiling ---";

        // Get project root
        QString projectRoot = "C:/dev/cloud_registration";

        // Test with both E57 sample files
        QStringList e57Files = {
            projectRoot + "/sample/bunnyDouble.e57",
            projectRoot + "/sample/bunnyInt32.e57"
        };
        
        for (const QString &filePath : e57Files) {
            if (!QFileInfo::exists(filePath)) {
                qWarning() << "Sample file not found:" << filePath;
                continue;
            }
            
            qDebug() << "Profiling E57 file:" << filePath;
            
            // Reset profiler for each file
            PerformanceProfiler::instance().reset();
            
            // Create parser and connect signals
            E57Parser parser;
            connect(&parser, &E57Parser::parsingFinished,
                    this, &Sprint22ProfilingDemo::onE57ParsingFinished);
            
            m_parsingComplete = false;
            
            // Start parsing with profiling
            parser.startParsing(filePath);
            
            // Wait for completion
            waitForCompletion();
            
            qDebug() << "✓ E57 profiling completed for" << QFileInfo(filePath).fileName();
        }
    }

    void testLASProfiling()
    {
        qDebug() << "\n--- Test 3: LAS File Profiling ---";

        // Get project root
        QString projectRoot = "C:/dev/cloud_registration";
        QString lasFile = projectRoot + "/sample/S2max-Power line202503.las";
        
        if (!QFileInfo::exists(lasFile)) {
            qWarning() << "LAS sample file not found:" << lasFile;
            return;
        }
        
        qDebug() << "Profiling LAS file:" << lasFile;
        
        // Reset profiler
        PerformanceProfiler::instance().reset();
        
        // Create parser and connect signals
        LasParser parser;
        connect(&parser, &LasParser::parsingFinished,
                this, &Sprint22ProfilingDemo::onLASParsingFinished);
        
        m_parsingComplete = false;
        
        // Start parsing with profiling
        parser.startParsing(lasFile);
        
        // Wait for completion
        waitForCompletion();
        
        qDebug() << "✓ LAS profiling completed";
    }

    void testBenchmarkSuite()
    {
        qDebug() << "\n--- Test 4: Comprehensive Benchmark Suite ---";
        
        // Collect all available sample files
        QStringList testFiles;

        // Get project root
        QString projectRoot = "C:/dev/cloud_registration";

        QStringList candidates = {
            projectRoot + "/sample/bunnyDouble.e57",
            projectRoot + "/sample/bunnyInt32.e57",
            projectRoot + "/sample/S2max-Power line202503.las"
        };
        
        for (const QString &file : candidates) {
            if (QFileInfo::exists(file)) {
                testFiles << file;
            }
        }
        
        if (testFiles.isEmpty()) {
            qWarning() << "No sample files found for benchmarking";
            return;
        }
        
        qDebug() << "Running benchmark suite on" << testFiles.size() << "files";
        
        // Create benchmark instance
        PerformanceBenchmark benchmark;
        benchmark.setMemoryMonitoringEnabled(true);
        
        // Run comparison suite
        benchmark.runComparisonSuite(testFiles);
        
        // Generate comprehensive report
        QString reportPath = "sprint2_2_benchmark_results";
        benchmark.generateBenchmarkReport(reportPath);
        
        // Display summary
        const auto &results = benchmark.getResults();
        qDebug() << "\nBenchmark Summary:";
        qDebug() << "Total tests run:" << results.size();
        
        for (const auto &result : results) {
            qDebug() << QString("  %1: %2ms (%3 points, %4 pts/sec)")
                        .arg(QFileInfo(result.filePath).fileName())
                        .arg(result.loadTimeMs)
                        .arg(result.pointCount)
                        .arg(result.pointsPerSecond, 0, 'f', 0);
        }
        
        qDebug() << "✓ Benchmark suite completed";
        qDebug() << "Reports saved to:" << reportPath + "_benchmark_report.txt";
        qDebug() << "JSON data saved to:" << reportPath + "_benchmark_data.json";
    }

    void waitForCompletion()
    {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        timer.start(30000); // 30 second timeout
        
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        
        // Check periodically if parsing is complete
        QTimer checkTimer;
        connect(&checkTimer, &QTimer::timeout, [&]() {
            if (m_parsingComplete) {
                loop.quit();
            }
        });
        checkTimer.start(100); // Check every 100ms
        
        loop.exec();
        
        if (!m_parsingComplete) {
            qWarning() << "Parsing timed out";
        }
    }

    bool m_parsingComplete = false;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Determine project root directory - use hardcoded path for now
    QString projectRoot = "C:/dev/cloud_registration";

    qDebug() << "Application directory:" << QCoreApplication::applicationDirPath();
    qDebug() << "Project root:" << projectRoot;

    // Check if sample files exist with absolute paths
    QStringList sampleFiles = {
        projectRoot + "/sample/bunnyDouble.e57",
        projectRoot + "/sample/bunnyInt32.e57",
        projectRoot + "/sample/S2max-Power line202503.las"
    };
    for (const QString &file : sampleFiles) {
        qDebug() << "Sample file" << file << "exists:" << QFileInfo::exists(file);
    }
    
    // Create and run demo
    Sprint22ProfilingDemo demo;
    demo.runDemo();
    
    return 0;
}

#include "test_sprint2_2_profiling_demo.moc"
