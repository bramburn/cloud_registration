#include <QtTest/QtTest>
#include <QTemporaryFile>
#include <QTextStream>
#include <QThread>

#include "core/profiling_macros.h"

/**
 * @brief Unit tests for profiling macros (Sprint 7.3)
 * 
 * Tests the convenience macros for performance profiling including:
 * - PROFILE_FUNCTION() macro
 * - PROFILE_SECTION() macro
 * - PROFILE_SECTION_DETAILED() macro
 * - PROFILE_SECTION_CONDITIONAL() macro
 * - Proper RAII behavior
 * - Integration with PerformanceProfiler
 */
class TestProfilingMacros : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic macro tests
    void testProfileFunction();
    void testProfileSection();
    void testProfileSectionDetailed();
    void testProfileSectionConditional();
    
    // RAII behavior tests
    void testMacroRAIIBehavior();
    void testNestedMacros();
    void testMacroWithException();
    
    // Integration tests
    void testMacroIntegrationWithProfiler();
    void testMacroPerformanceWhenDisabled();

private:
    void simulateWork(int milliseconds);
    QString generateTempFilePath();
    void testFunction();
    void nestedFunction();
};

void TestProfilingMacros::initTestCase()
{
    qDebug() << "Starting profiling macros tests";
}

void TestProfilingMacros::cleanupTestCase()
{
    qDebug() << "Profiling macros tests completed";
}

void TestProfilingMacros::init()
{
    // Reset profiler state before each test
    PerformanceProfiler::instance().setEnabled(false);
    PerformanceProfiler::instance().clearData();
}

void TestProfilingMacros::cleanup()
{
    // Clean up after each test
    PerformanceProfiler::instance().setEnabled(false);
    PerformanceProfiler::instance().clearData();
}

void TestProfilingMacros::testProfileFunction()
{
    PerformanceProfiler::instance().setEnabled(true);
    
    // Call a function that uses PROFILE_FUNCTION()
    testFunction();
    
    // Generate report and verify the function was profiled
    QString tempFile = generateTempFilePath();
    PerformanceProfiler::instance().generateReport(tempFile);
    
    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();
    
    QVERIFY(content.contains("testFunction"));
    
    // Clean up
    QFile::remove(tempFile);
}

void TestProfilingMacros::testProfileSection()
{
    PerformanceProfiler::instance().setEnabled(true);
    
    {
        PROFILE_SECTION("TestSectionMacro");
        simulateWork(10);
    }
    
    // Generate report and verify the section was profiled
    QString tempFile = generateTempFilePath();
    PerformanceProfiler::instance().generateReport(tempFile);
    
    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();
    
    QVERIFY(content.contains("TestSectionMacro"));
    
    // Clean up
    QFile::remove(tempFile);
}

void TestProfilingMacros::testProfileSectionDetailed()
{
    PerformanceProfiler::instance().setEnabled(true);
    
    {
        PROFILE_SECTION_DETAILED("DetailedSection");
        simulateWork(5);
    }
    
    // Generate report and verify the detailed section was profiled
    QString tempFile = generateTempFilePath();
    PerformanceProfiler::instance().generateReport(tempFile);
    
    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();
    
    // Should contain function name and custom section name
    QVERIFY(content.contains("DetailedSection"));
    QVERIFY(content.contains("testProfileSectionDetailed") || content.contains("::"));
    
    // Clean up
    QFile::remove(tempFile);
}

void TestProfilingMacros::testProfileSectionConditional()
{
    // Test when profiling is disabled
    PerformanceProfiler::instance().setEnabled(false);
    
    {
        PROFILE_SECTION_CONDITIONAL("ConditionalSection");
        simulateWork(5);
    }
    
    // Generate report - should not contain the section
    QString tempFile = generateTempFilePath();
    PerformanceProfiler::instance().generateReport(tempFile);
    
    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();
    
    QVERIFY(!content.contains("ConditionalSection"));
    
    // Test when profiling is enabled
    PerformanceProfiler::instance().setEnabled(true);
    
    {
        PROFILE_SECTION_CONDITIONAL("ConditionalSectionEnabled");
        simulateWork(5);
    }
    
    // Generate new report - should contain the section
    QString tempFile2 = generateTempFilePath();
    PerformanceProfiler::instance().generateReport(tempFile2);
    
    QFile file2(tempFile2);
    QVERIFY(file2.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content2 = file2.readAll();
    file2.close();
    
    QVERIFY(content2.contains("ConditionalSectionEnabled"));
    
    // Clean up
    QFile::remove(tempFile);
    QFile::remove(tempFile2);
}

void TestProfilingMacros::testMacroRAIIBehavior()
{
    PerformanceProfiler::instance().setEnabled(true);
    
    // Test that macros properly start and end timing
    {
        PROFILE_SECTION("RAIITest");
        simulateWork(10);
        // Timer should automatically end when scope exits
    }
    
    // Start another section to ensure the previous one ended
    {
        PROFILE_SECTION("RAIITest2");
        simulateWork(5);
    }
    
    // Generate report
    QString tempFile = generateTempFilePath();
    PerformanceProfiler::instance().generateReport(tempFile);
    
    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();
    
    QVERIFY(content.contains("RAIITest"));
    QVERIFY(content.contains("RAIITest2"));
    
    // Clean up
    QFile::remove(tempFile);
}

void TestProfilingMacros::testNestedMacros()
{
    PerformanceProfiler::instance().setEnabled(true);
    
    {
        PROFILE_SECTION("OuterMacro");
        simulateWork(5);
        
        {
            PROFILE_SECTION("InnerMacro");
            simulateWork(5);
        }
        
        simulateWork(5);
    }
    
    // Generate report
    QString tempFile = generateTempFilePath();
    PerformanceProfiler::instance().generateReport(tempFile);
    
    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();
    
    QVERIFY(content.contains("OuterMacro"));
    QVERIFY(content.contains("InnerMacro"));
    
    // Clean up
    QFile::remove(tempFile);
}

void TestProfilingMacros::testMacroWithException()
{
    PerformanceProfiler::instance().setEnabled(true);
    
    // Test that macros work properly even when exceptions occur
    try {
        PROFILE_SECTION("ExceptionMacro");
        simulateWork(5);
        throw std::runtime_error("Test exception");
    } catch (const std::exception&) {
        // Exception caught, timer should have been destroyed
    }
    
    // Generate report
    QString tempFile = generateTempFilePath();
    PerformanceProfiler::instance().generateReport(tempFile);
    
    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();
    
    QVERIFY(content.contains("ExceptionMacro"));
    
    // Clean up
    QFile::remove(tempFile);
}

void TestProfilingMacros::testMacroIntegrationWithProfiler()
{
    PerformanceProfiler::instance().setEnabled(true);
    
    // Mix macro usage with direct profiler calls
    PerformanceProfiler::instance().startSection("DirectCall");
    simulateWork(5);
    
    {
        PROFILE_SECTION("MacroCall");
        simulateWork(5);
    }
    
    PerformanceProfiler::instance().endSection("DirectCall");
    
    // Generate report
    QString tempFile = generateTempFilePath();
    PerformanceProfiler::instance().generateReport(tempFile);
    
    QFile file(tempFile);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = file.readAll();
    file.close();
    
    QVERIFY(content.contains("DirectCall"));
    QVERIFY(content.contains("MacroCall"));
    
    // Clean up
    QFile::remove(tempFile);
}

void TestProfilingMacros::testMacroPerformanceWhenDisabled()
{
    PerformanceProfiler::instance().setEnabled(false);
    
    // Test that macros have minimal overhead when profiling is disabled
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        PROFILE_SECTION("DisabledMacro");
        // Minimal work
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Should complete very quickly when disabled (less than 1ms for 1000 operations)
    QVERIFY(duration.count() < 1000);
}

void TestProfilingMacros::simulateWork(int milliseconds)
{
    QThread::msleep(milliseconds);
}

QString TestProfilingMacros::generateTempFilePath()
{
    QTemporaryFile tempFile;
    tempFile.open();
    QString path = tempFile.fileName();
    tempFile.close();
    return path;
}

void TestProfilingMacros::testFunction()
{
    PROFILE_FUNCTION();
    simulateWork(10);
}

void TestProfilingMacros::nestedFunction()
{
    PROFILE_FUNCTION();
    simulateWork(5);
    testFunction();
}

QTEST_MAIN(TestProfilingMacros)
#include "test_profiling_macros.moc"
