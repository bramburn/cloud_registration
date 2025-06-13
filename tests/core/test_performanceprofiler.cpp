#include <QtTest/QtTest>
#include <QTemporaryFile>
#include <QTextStream>
#include <QThread>
#include <QSignalSpy>

#include "core/performance_profiler.h"

/**
 * @brief Unit tests for PerformanceProfiler class (Sprint 7.3)
 * 
 * Tests the core functionality of the performance profiling system including:
 * - Enable/disable functionality
 * - Section timing
 * - Report generation
 * - Thread safety
 * - RAII timer functionality
 */
class TestPerformanceProfiler : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testSingletonAccess();
    void testEnableDisable();
    void testBasicSectionTiming();
    void testNestedSections();
    void testMultipleSections();
    
    // RAII timer tests
    void testSectionTimer();
    void testSectionTimerException();
    
    // Report generation tests
    void testReportGeneration();
    void testReportGenerationDisabled();
    void testReportGenerationEmpty();
    
    // Thread safety tests
    void testThreadSafety();
    
    // Performance tests
    void testOverheadWhenDisabled();
    void testLargeNumberOfSections();

private:
    void simulateWork(int milliseconds);
    QString generateTempFilePath();
};

void TestPerformanceProfiler::initTestCase()
{
    qDebug() << "Starting PerformanceProfiler tests";
}

void TestPerformanceProfiler::cleanupTestCase()
{
    qDebug() << "PerformanceProfiler tests completed";
}

void TestPerformanceProfiler::init()
{
    // Reset profiler state before each test
    PerformanceProfiler::instance().setEnabled(false);
    PerformanceProfiler::instance().clearData();
}

void TestPerformanceProfiler::cleanup()
{
    // Clean up after each test
    PerformanceProfiler::instance().setEnabled(false);
    PerformanceProfiler::instance().clearData();
}

void TestPerformanceProfiler::testSingletonAccess()
{
    // Test that we get the same instance
    PerformanceProfiler& profiler1 = PerformanceProfiler::instance();
    PerformanceProfiler& profiler2 = PerformanceProfiler::instance();
    
    QVERIFY(&profiler1 == &profiler2);
}

void TestPerformanceProfiler::testEnableDisable()
{
    PerformanceProfiler& profiler = PerformanceProfiler::instance();
    
    // Test initial state
    QVERIFY(!profiler.isEnabled());
    
    // Test enabling
    profiler.setEnabled(true);
    QVERIFY(profiler.isEnabled());
    
    // Test disabling
    profiler.setEnabled(false);
    QVERIFY(!profiler.isEnabled());
}

void TestPerformanceProfiler::testBasicSectionTiming()
{
    PerformanceProfiler& profiler = PerformanceProfiler::instance();
    profiler.setEnabled(true);
    
    // Time a section
    profiler.startSection("TestSection");
    simulateWork(10);
    profiler.endSection("TestSection");
    
    // Generate report to verify data was captured
    QString tempFile = generateTempFilePath();
    profiler.generateReport(tempFile);
    
    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();
    
    QVERIFY(content.contains("TestSection"));
    QVERIFY(content.contains("ms"));
    
    // Clean up
    QFile::remove(tempFile);
}

void TestPerformanceProfiler::testNestedSections()
{
    PerformanceProfiler& profiler = PerformanceProfiler::instance();
    profiler.setEnabled(true);
    
    // Test nested sections
    profiler.startSection("OuterSection");
    simulateWork(5);
    
    profiler.startSection("InnerSection");
    simulateWork(5);
    profiler.endSection("InnerSection");
    
    simulateWork(5);
    profiler.endSection("OuterSection");
    
    // Generate report
    QString tempFile = generateTempFilePath();
    profiler.generateReport(tempFile);
    
    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();
    
    QVERIFY(content.contains("OuterSection"));
    QVERIFY(content.contains("InnerSection"));
    
    // Clean up
    QFile::remove(tempFile);
}

void TestPerformanceProfiler::testSectionTimer()
{
    PerformanceProfiler& profiler = PerformanceProfiler::instance();
    profiler.setEnabled(true);
    
    // Test RAII timer
    {
        PerformanceProfiler::SectionTimer timer("RAIISection");
        simulateWork(10);
    } // Timer should automatically end here
    
    // Generate report
    QString tempFile = generateTempFilePath();
    profiler.generateReport(tempFile);
    
    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();
    
    QVERIFY(content.contains("RAIISection"));
    
    // Clean up
    QFile::remove(tempFile);
}

void TestPerformanceProfiler::testReportGeneration()
{
    PerformanceProfiler& profiler = PerformanceProfiler::instance();
    profiler.setEnabled(true);
    
    // Add some timing data
    profiler.startSection("Section1");
    simulateWork(5);
    profiler.endSection("Section1");
    
    profiler.startSection("Section2");
    simulateWork(10);
    profiler.endSection("Section2");
    
    // Generate report
    QString tempFile = generateTempFilePath();
    profiler.generateReport(tempFile);
    
    // Verify file was created and contains expected content
    QFile file(tempFile);
    QVERIFY(file.exists());
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    
    QString content = file.readAll();
    file.close();
    
    QVERIFY(!content.isEmpty());
    QVERIFY(content.contains("Performance Report"));
    QVERIFY(content.contains("Section1"));
    QVERIFY(content.contains("Section2"));
    
    // Clean up
    QFile::remove(tempFile);
}

void TestPerformanceProfiler::testReportGenerationDisabled()
{
    PerformanceProfiler& profiler = PerformanceProfiler::instance();
    profiler.setEnabled(false);
    
    // Try to time sections when disabled
    profiler.startSection("DisabledSection");
    simulateWork(10);
    profiler.endSection("DisabledSection");
    
    // Generate report
    QString tempFile = generateTempFilePath();
    profiler.generateReport(tempFile);
    
    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();
    
    // Should contain report header but no timing data
    QVERIFY(content.contains("Performance Report"));
    QVERIFY(!content.contains("DisabledSection"));
    
    // Clean up
    QFile::remove(tempFile);
}

void TestPerformanceProfiler::testOverheadWhenDisabled()
{
    PerformanceProfiler& profiler = PerformanceProfiler::instance();
    profiler.setEnabled(false);
    
    // Time operations when profiler is disabled - should have minimal overhead
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        profiler.startSection("TestSection");
        profiler.endSection("TestSection");
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should complete very quickly when disabled (less than 1ms for 1000 operations)
    QVERIFY(duration.count() < 1000);
}

void TestPerformanceProfiler::simulateWork(int milliseconds)
{
    QThread::msleep(milliseconds);
}

QString TestPerformanceProfiler::generateTempFilePath()
{
    QTemporaryFile tempFile;
    tempFile.open();
    QString path = tempFile.fileName();
    tempFile.close();
    return path;
}

void TestPerformanceProfiler::testMultipleSections()
{
    PerformanceProfiler& profiler = PerformanceProfiler::instance();
    profiler.setEnabled(true);

    // Time multiple different sections
    for (int i = 0; i < 5; ++i) {
        QString sectionName = QString("Section%1").arg(i);
        profiler.startSection(sectionName);
        simulateWork(2);
        profiler.endSection(sectionName);
    }

    // Generate report
    QString tempFile = generateTempFilePath();
    profiler.generateReport(tempFile);

    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();

    // Verify all sections are in the report
    for (int i = 0; i < 5; ++i) {
        QString sectionName = QString("Section%1").arg(i);
        QVERIFY(content.contains(sectionName));
    }

    // Clean up
    QFile::remove(tempFile);
}

void TestPerformanceProfiler::testSectionTimerException()
{
    PerformanceProfiler& profiler = PerformanceProfiler::instance();
    profiler.setEnabled(true);

    // Test that RAII timer works even when exceptions occur
    try {
        PerformanceProfiler::SectionTimer timer("ExceptionSection");
        simulateWork(5);
        throw std::runtime_error("Test exception");
    } catch (const std::exception&) {
        // Exception caught, timer should have been destroyed and section ended
    }

    // Generate report
    QString tempFile = generateTempFilePath();
    profiler.generateReport(tempFile);

    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();

    QVERIFY(content.contains("ExceptionSection"));

    // Clean up
    QFile::remove(tempFile);
}

void TestPerformanceProfiler::testReportGenerationEmpty()
{
    PerformanceProfiler& profiler = PerformanceProfiler::instance();
    profiler.setEnabled(true);

    // Generate report without any timing data
    QString tempFile = generateTempFilePath();
    profiler.generateReport(tempFile);

    QFile file(tempFile);
    QVERIFY(file.exists());
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QString content = file.readAll();
    file.close();

    // Should contain header but indicate no data
    QVERIFY(content.contains("Performance Report"));
    QVERIFY(content.contains("No profiling data") || content.contains("0 sections"));

    // Clean up
    QFile::remove(tempFile);
}

void TestPerformanceProfiler::testThreadSafety()
{
    PerformanceProfiler& profiler = PerformanceProfiler::instance();
    profiler.setEnabled(true);

    // Test concurrent access from multiple threads
    QList<QThread*> threads;
    QAtomicInt completedThreads(0);

    for (int i = 0; i < 4; ++i) {
        QThread* thread = QThread::create([&profiler, &completedThreads, i]() {
            for (int j = 0; j < 10; ++j) {
                QString sectionName = QString("Thread%1_Section%2").arg(i).arg(j);
                PerformanceProfiler::SectionTimer timer(sectionName);
                QThread::msleep(1);
            }
            completedThreads.fetchAndAddOrdered(1);
        });
        threads.append(thread);
        thread->start();
    }

    // Wait for all threads to complete
    for (QThread* thread : threads) {
        thread->wait(5000); // 5 second timeout
        thread->deleteLater();
    }

    QCOMPARE(completedThreads.loadAcquire(), 4);

    // Generate report and verify data from all threads
    QString tempFile = generateTempFilePath();
    profiler.generateReport(tempFile);

    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();

    // Should contain sections from all threads
    for (int i = 0; i < 4; ++i) {
        QString threadPrefix = QString("Thread%1").arg(i);
        QVERIFY(content.contains(threadPrefix));
    }

    // Clean up
    QFile::remove(tempFile);
}

void TestPerformanceProfiler::testLargeNumberOfSections()
{
    PerformanceProfiler& profiler = PerformanceProfiler::instance();
    profiler.setEnabled(true);

    // Test with a large number of sections
    const int numSections = 1000;

    for (int i = 0; i < numSections; ++i) {
        QString sectionName = QString("LargeTest_Section%1").arg(i);
        PerformanceProfiler::SectionTimer timer(sectionName);
        // Very brief work to keep test fast
        QThread::usleep(10); // 10 microseconds
    }

    // Generate report
    QString tempFile = generateTempFilePath();
    profiler.generateReport(tempFile);

    QFile file(tempFile);
    QVERIFY(file.exists());
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QString content = file.readAll();
    file.close();

    // Should contain report header and some of the sections
    QVERIFY(content.contains("Performance Report"));
    QVERIFY(content.contains("LargeTest_Section"));

    // Clean up
    QFile::remove(tempFile);
}

QTEST_MAIN(TestPerformanceProfiler)
#include "test_performanceprofiler.moc"
