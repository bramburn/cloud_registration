#include "test_reporter.h"
#include <QJsonArray>
#include <QTextStream>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QSysInfo>
#include <QDebug>

TestReporter::TestReporter() {
    m_currentTimestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    m_reportOutputDir = QDir::currentPath() + "/test_reports";
    ensureOutputDirectory();
}

TestReporter::~TestReporter() {
    // Auto-generate final report on destruction
    generateComprehensiveReport();
}

void TestReporter::ensureOutputDirectory() {
    QDir dir(m_reportOutputDir);
    if (!dir.exists()) {
        dir.mkpath(".");
        qDebug() << "Created test reports directory:" << m_reportOutputDir;
    }
}

void TestReporter::documentTestResult(const QString& testCaseId,
                                    const QString& description,
                                    bool passed,
                                    const QString& details) {
    TestDocumentation doc;
    doc.testCaseId = testCaseId;
    doc.description = description;
    doc.passed = passed;
    doc.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    doc.errorDetails = details;
    
    addTestDocumentation(doc);
}

void TestReporter::addTestDocumentation(const TestDocumentation& doc) {
    m_testDocumentation.push_back(doc);
    
    // Auto-generate bug report for failed tests
    if (!doc.passed) {
        BugReport bug = createBugReport(doc.testCaseId, doc.testFile, 
                                      doc.errorDetails, doc.expectedOutcome);
        addBugReport(bug);
    }
}

TestReporter::BugReport TestReporter::createBugReport(const QString& testCaseId,
                                                    const QString& testFile,
                                                    const QString& errorMessage,
                                                    const QString& expectedBehavior) {
    BugReport bug;
    bug.testCaseId = testCaseId;
    bug.title = QString("Sprint 1.4 Test Failure: %1").arg(testCaseId);
    bug.testFile = testFile;
    bug.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    bug.sprintContext = "Sprint 1.4 - Integration, Testing & Refinement";
    bug.environmentInfo = captureEnvironmentInfo();
    
    // Determine severity based on test context
    QFileInfo fileInfo(testFile);
    QString category = "unknown";
    if (testCaseId.contains("valid")) category = "valid";
    else if (testCaseId.contains("edge")) category = "edge_case";
    else if (testCaseId.contains("error")) category = "error";
    
    bug.severity = determineSeverity(category, expectedBehavior, "failure");
    
    // Generate comprehensive description
    bug.description = QString(
        "Test case %1 failed during Sprint 1.4 integration testing.\n\n"
        "**Test Context:**\n"
        "- File: %2\n"
        "- Expected: %3\n"
        "- Error: %4\n\n"
        "**Sprint 1.4 Impact:**\n"
        "This failure affects Phase 1 integration testing objectives."
    ).arg(testCaseId, testFile, expectedBehavior, errorMessage);
    
    // Generate steps to reproduce
    bug.stepsToReproduce = generateStepsToReproduce(testFile);
    
    bug.expectedResult = expectedBehavior.isEmpty() ? 
        "File should load successfully without errors" : expectedBehavior;
    
    bug.actualResult = errorMessage.isEmpty() ? 
        "Unknown failure occurred" : errorMessage;
    
    // Capture relevant log snippets
    bug.logSnippets = captureRelevantLogs();
    
    // Determine affected components
    if (testFile.endsWith(".e57")) {
        bug.affectedComponents << "E57Parser" << "XML Parsing" << "Binary Section Reading";
    } else if (testFile.endsWith(".las")) {
        bug.affectedComponents << "LasParser" << "Header Parsing" << "Point Data Reading";
    }
    bug.affectedComponents << "MainWindow" << "File Loading" << "Error Handling";
    
    bug.reproductionRate = "Always"; // Integration tests should be deterministic
    
    return bug;
}

void TestReporter::addBugReport(const BugReport& bug) {
    m_bugReports.push_back(bug);
}

QString TestReporter::determineSeverity(const QString& category,
                                      const QString& expectedOutcome,
                                      const QString& actualOutcome,
                                      bool hasStaleData) {
    // Task 1.4.1.4: Severity determination logic
    if (category == "valid" && actualOutcome.contains("failure")) {
        return "Critical";  // Valid files should always load
    }
    if (category == "error" && expectedOutcome.contains("success")) {
        return "High";      // Invalid files shouldn't load successfully
    }
    if (hasStaleData) {
        return "High";      // Stale data not cleared on error
    }
    if (category == "edge_case") {
        return "Medium";    // Edge cases are important but not critical
    }
    return "Low";
}

QString TestReporter::captureEnvironmentInfo() {
    QString info;
    QTextStream stream(&info);
    
    stream << "**Environment Information:**\n";
    stream << "- OS: " << QSysInfo::prettyProductName() << "\n";
    stream << "- Qt Version: " << QT_VERSION_STR << "\n";
    stream << "- Build Type: " << 
#ifdef QT_DEBUG
        "Debug" << 
#else
        "Release" <<
#endif
        "\n";
    stream << "- Architecture: " << QSysInfo::currentCpuArchitecture() << "\n";
    stream << "- Timestamp: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    
    return info;
}

QStringList TestReporter::captureRelevantLogs() {
    QStringList logs;
    
    // In a real implementation, this would capture actual log output
    // For now, we'll provide placeholder log entries
    logs << "=== Test Execution Log ===";
    logs << QString("Timestamp: %1").arg(QDateTime::currentDateTime().toString());
    logs << "Application started successfully";
    logs << "Parsers initialized";
    logs << "Test file loading attempted";
    logs << "=== End Log ===";
    
    return logs;
}

QString TestReporter::generateStepsToReproduce(const QString& testFile) {
    return QString(
        "1. Launch FARO Scene Registration application\n"
        "2. Click 'Open Point Cloud File' or use File > Open menu\n"
        "3. Navigate to and select file: %1\n"
        "4. Observe loading behavior and any error messages\n"
        "5. Check status bar and viewer state\n"
        "6. Verify error handling and data clearing behavior"
    ).arg(testFile);
}

void TestReporter::generateTestReport() {
    QString reportPath = m_reportOutputDir + QString("/test_report_%1.txt").arg(m_currentTimestamp);
    QFile file(reportPath);
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to create test report:" << reportPath;
        return;
    }
    
    QTextStream out(&file);
    out << "Sprint 1.4 Integration Test Report\n";
    out << "==================================\n\n";
    out << "Generated: " << QDateTime::currentDateTime().toString() << "\n\n";
    
    out << "Test Summary:\n";
    out << "- Total Tests: " << getTotalTests() << "\n";
    out << "- Passed: " << getPassedTests() << "\n";
    out << "- Failed: " << getFailedTests() << "\n";
    out << "- Success Rate: " << QString::number((double)getPassedTests() / getTotalTests() * 100, 'f', 1) << "%\n\n";
    
    out << "Bug Summary:\n";
    out << "- Total Bugs: " << m_bugReports.size() << "\n";
    out << "- Critical: " << getCriticalBugs() << "\n";
    out << "- High Priority: " << getHighPriorityBugs() << "\n\n";
    
    // Detailed test results
    out << "Detailed Test Results:\n";
    out << "=====================\n\n";
    
    for (const auto& doc : m_testDocumentation) {
        out << "Test Case: " << doc.testCaseId << "\n";
        out << "Description: " << doc.description << "\n";
        out << "Result: " << (doc.passed ? "PASSED" : "FAILED") << "\n";
        out << "Execution Time: " << formatDuration(doc.executionTime) << "\n";
        if (!doc.passed) {
            out << "Error: " << doc.errorDetails << "\n";
        }
        out << "---\n\n";
    }
    
    qDebug() << "Test report generated:" << reportPath;
}

void TestReporter::generateComprehensiveReport() {
    generateTestReport();
    generateBugReports();
    exportBugReportsToJson();
    generateMarkdownReport();
}

int TestReporter::getPassedTests() const {
    int count = 0;
    for (const auto& doc : m_testDocumentation) {
        if (doc.passed) count++;
    }
    return count;
}

int TestReporter::getFailedTests() const {
    return getTotalTests() - getPassedTests();
}

int TestReporter::getCriticalBugs() const {
    int count = 0;
    for (const auto& bug : m_bugReports) {
        if (bug.severity == "Critical") count++;
    }
    return count;
}

int TestReporter::getHighPriorityBugs() const {
    int count = 0;
    for (const auto& bug : m_bugReports) {
        if (bug.severity == "High") count++;
    }
    return count;
}

QString TestReporter::formatDuration(double seconds) {
    if (seconds < 1.0) {
        return QString("%1 ms").arg(qRound(seconds * 1000));
    } else {
        return QString("%1 s").arg(QString::number(seconds, 'f', 2));
    }
}

void TestReporter::generateBugReports() {
    QString reportPath = m_reportOutputDir + QString("/bug_reports_%1.txt").arg(m_currentTimestamp);
    QFile file(reportPath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to create bug report:" << reportPath;
        return;
    }

    QTextStream out(&file);
    out << "Sprint 1.4 Bug Reports\n";
    out << "======================\n\n";
    out << "Generated: " << QDateTime::currentDateTime().toString() << "\n\n";

    for (const auto& bug : m_bugReports) {
        out << "Bug ID: " << bug.testCaseId << "\n";
        out << "Title: " << bug.title << "\n";
        out << "Severity: " << bug.severity << "\n";
        out << "Test File: " << bug.testFile << "\n";
        out << "Timestamp: " << bug.timestamp << "\n\n";

        out << "Description:\n" << bug.description << "\n\n";

        out << "Steps to Reproduce:\n" << bug.stepsToReproduce << "\n\n";

        out << "Expected Result:\n" << bug.expectedResult << "\n\n";

        out << "Actual Result:\n" << bug.actualResult << "\n\n";

        out << "Environment:\n" << bug.environmentInfo << "\n\n";

        if (!bug.logSnippets.isEmpty()) {
            out << "Log Snippets:\n";
            for (const QString& log : bug.logSnippets) {
                out << log << "\n";
            }
            out << "\n";
        }

        out << "Affected Components: " << bug.affectedComponents.join(", ") << "\n";
        out << "Reproduction Rate: " << bug.reproductionRate << "\n";
        out << "========================================\n\n";
    }

    qDebug() << "Bug reports generated:" << reportPath;
}

void TestReporter::exportBugReportsToJson() {
    QString jsonPath = m_reportOutputDir + QString("/bug_reports_%1.json").arg(m_currentTimestamp);
    QFile file(jsonPath);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create JSON bug report:" << jsonPath;
        return;
    }

    QJsonArray bugsArray;
    for (const auto& bug : m_bugReports) {
        bugsArray.append(bugReportToJson(bug));
    }

    QJsonObject root;
    root["sprint"] = "1.4";
    root["title"] = "Integration Testing Bug Reports";
    root["generated"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["total_bugs"] = static_cast<qint64>(m_bugReports.size());
    root["critical_bugs"] = getCriticalBugs();
    root["high_priority_bugs"] = getHighPriorityBugs();
    root["bugs"] = bugsArray;

    QJsonDocument doc(root);
    file.write(doc.toJson());

    qDebug() << "JSON bug reports exported:" << jsonPath;
}

QJsonObject TestReporter::bugReportToJson(const BugReport& bug) {
    QJsonObject obj;
    obj["test_case_id"] = bug.testCaseId;
    obj["title"] = bug.title;
    obj["severity"] = bug.severity;
    obj["description"] = bug.description;
    obj["steps_to_reproduce"] = bug.stepsToReproduce;
    obj["expected_result"] = bug.expectedResult;
    obj["actual_result"] = bug.actualResult;
    obj["test_file"] = bug.testFile;
    obj["timestamp"] = bug.timestamp;
    obj["environment_info"] = bug.environmentInfo;
    obj["reproduction_rate"] = bug.reproductionRate;

    QJsonArray componentsArray;
    for (const QString& component : bug.affectedComponents) {
        componentsArray.append(component);
    }
    obj["affected_components"] = componentsArray;

    QJsonArray logsArray;
    for (const QString& log : bug.logSnippets) {
        logsArray.append(log);
    }
    obj["log_snippets"] = logsArray;

    return obj;
}

void TestReporter::generateMarkdownReport() {
    QString mdPath = m_reportOutputDir + QString("/sprint_1_4_report_%1.md").arg(m_currentTimestamp);
    QFile file(mdPath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to create markdown report:" << mdPath;
        return;
    }

    QTextStream out(&file);
    out << "# Sprint 1.4 Integration Testing Report\n\n";
    out << "**Generated:** " << QDateTime::currentDateTime().toString() << "\n\n";

    out << "## Executive Summary\n\n";
    out << "| Metric | Value |\n";
    out << "|--------|-------|\n";
    out << "| Total Tests | " << getTotalTests() << " |\n";
    out << "| Passed Tests | " << getPassedTests() << " |\n";
    out << "| Failed Tests | " << getFailedTests() << " |\n";
    out << "| Success Rate | " << QString::number((double)getPassedTests() / getTotalTests() * 100, 'f', 1) << "% |\n";
    out << "| Total Bugs | " << m_bugReports.size() << " |\n";
    out << "| Critical Bugs | " << getCriticalBugs() << " |\n";
    out << "| High Priority Bugs | " << getHighPriorityBugs() << " |\n\n";

    out << "## Test Results by Category\n\n";
    // Add category breakdown here

    out << "## Critical Issues\n\n";
    for (const auto& bug : m_bugReports) {
        if (bug.severity == "Critical") {
            out << "### " << bug.title << "\n\n";
            out << "**Test Case:** " << bug.testCaseId << "\n\n";
            out << "**File:** `" << bug.testFile << "`\n\n";
            out << "**Description:** " << bug.description << "\n\n";
            out << "---\n\n";
        }
    }

    qDebug() << "Markdown report generated:" << mdPath;
}
