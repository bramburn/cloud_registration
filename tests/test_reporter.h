#ifndef TEST_REPORTER_H
#define TEST_REPORTER_H

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <vector>

/**
 * @brief Automated test documentation and bug reporting for Sprint 1.4
 * 
 * This class implements comprehensive test result documentation and
 * automated bug report generation as specified in Sprint 1.4 
 * Tasks 1.4.1.3 and 1.4.1.4.
 */
class TestReporter {
public:
    /**
     * @brief Detailed bug report structure
     * 
     * Contains all information required for comprehensive bug reporting
     * as per Sprint 1.4 Task 1.4.1.4 requirements.
     */
    struct BugReport {
        QString title;
        QString severity;          ///< "Critical", "High", "Medium", "Low"
        QString description;
        QString stepsToReproduce;
        QString expectedResult;
        QString actualResult;
        QString testFile;
        QStringList logSnippets;
        QString timestamp;
        QString testCaseId;
        QString sprintContext;
        QString environmentInfo;
        QString reproductionRate;  ///< "Always", "Sometimes", "Rare"
        QStringList affectedComponents;
        QString workaround;
    };
    
    /**
     * @brief Test documentation structure
     * 
     * Comprehensive test execution documentation for Sprint 1.4
     * Task 1.4.1.3 requirements.
     */
    struct TestDocumentation {
        QString testCaseId;
        QString description;
        QString category;
        QString fileType;
        QString expectedOutcome;
        QString actualOutcome;
        bool passed;
        double executionTime;
        QString timestamp;
        QString testFile;
        QStringList logEntries;
        QString errorDetails;
        int pointCount;
        QString performanceMetrics;
    };
    
    TestReporter();
    ~TestReporter();
    
    // Test documentation methods (Task 1.4.1.3)
    void documentTestResult(const QString& testCaseId,
                          const QString& description,
                          bool passed,
                          const QString& details);
    
    void addTestDocumentation(const TestDocumentation& doc);
    void generateTestReport();
    void generateTestSummary();
    
    // Bug reporting methods (Task 1.4.1.4)
    BugReport createBugReport(const QString& testCaseId,
                            const QString& testFile,
                            const QString& errorMessage,
                            const QString& expectedBehavior);
    
    void addBugReport(const BugReport& bug);
    void generateBugReports();
    void exportBugReportsToJson();
    
    // Severity assessment
    QString determineSeverity(const QString& category,
                            const QString& expectedOutcome,
                            const QString& actualOutcome,
                            bool hasStaleData = false);
    
    // Environment and context
    QString captureEnvironmentInfo();
    QStringList captureRelevantLogs();
    QString generateStepsToReproduce(const QString& testFile);
    
    // Report generation
    void generateComprehensiveReport();
    void generateMarkdownReport();
    void generateHtmlReport();
    
    // Getters
    const std::vector<BugReport>& getBugReports() const { return m_bugReports; }
    const std::vector<TestDocumentation>& getTestDocumentation() const { return m_testDocumentation; }

    // Statistics
    int getTotalTests() const { return static_cast<int>(m_testDocumentation.size()); }
    int getPassedTests() const;
    int getFailedTests() const;
    int getCriticalBugs() const;
    int getHighPriorityBugs() const;
    
private:
    std::vector<BugReport> m_bugReports;
    std::vector<TestDocumentation> m_testDocumentation;
    QString m_reportOutputDir;
    QString m_currentTimestamp;
    
    // Helper methods
    void ensureOutputDirectory();
    QString formatTimestamp(const QDateTime& dateTime);
    QString escapeHtml(const QString& text);
    QString formatDuration(double seconds);
    QJsonObject bugReportToJson(const BugReport& bug);
    QJsonObject testDocumentationToJson(const TestDocumentation& doc);
};

#endif // TEST_REPORTER_H
