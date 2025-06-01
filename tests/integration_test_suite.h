#ifndef INTEGRATION_TEST_SUITE_H
#define INTEGRATION_TEST_SUITE_H

#include <gtest/gtest.h>
#include <QApplication>
#include <QDateTime>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QEventLoop>
#include <vector>
#include <memory>

/**
 * @brief Comprehensive test scenario framework for Sprint 1.4
 * 
 * This class implements the integration testing framework as specified
 * in Sprint 1.4 User Story 1, providing systematic testing of E57 and
 * LAS parsing functionality across diverse datasets.
 */
class IntegrationTestSuite : public ::testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
    
    /**
     * @brief Test scenario structure for comprehensive testing
     * 
     * Defines all parameters needed for systematic testing as per
     * Sprint 1.4 Task 1.4.1.1 requirements.
     */
    struct TestScenario {
        QString filePath;           ///< Path to test file
        QString category;           ///< "valid", "edge_case", "error"
        QString fileType;          ///< "e57", "las"
        QString expectedOutcome;   ///< "success", "specific_error"
        QString description;       ///< Human-readable description
        QStringList sprintTags;    ///< Track which sprint features are tested
        QString testCaseId;        ///< Reference to backlog test cases
        double expectedLoadTime;   ///< Expected loading time in seconds
        int expectedPointCount;    ///< Expected number of points (if known)
    };
    
    /**
     * @brief Detailed test result structure
     * 
     * Captures comprehensive test execution results for bug reporting
     * and documentation as per Sprint 1.4 Task 1.4.1.3-1.4.1.4.
     */
    struct DetailedTestResult {
        bool passed;
        bool fileLoaded;
        int pointCount;
        QString statusMessage;
        QString errorMessage;
        bool viewerHasData;
        QString metadataDisplayed;
        double loadingTime;
        bool timedOut;
        QDateTime startTime;
        QDateTime endTime;
        qint64 duration;
        QString testFile;
        QString testCaseId;
    };
    
    // Test scenario management
    void compileComprehensiveTestScenarios();
    std::vector<TestScenario> getScenariosByTag(const QString& tag);
    std::vector<TestScenario> getScenariosByCategory(const QString& category);
    
    // Test execution
    DetailedTestResult executeTestScenario(const TestScenario& scenario);
    DetailedTestResult executeTestScenarioWithTimeout(const TestScenario& scenario, int timeoutMs = 30000);
    bool executeTestScenarioSync(const TestScenario& scenario);
    
    // Test data management
    void setupTestDataDirectories();
    void setupTestApplication();
    QStringList findRealWorldTestFiles();
    QString detectFileType(const QString& filePath);
    
    // State capture and validation
    DetailedTestResult captureApplicationState();
    void validateTestResult(const TestScenario& scenario, 
                          const DetailedTestResult& initialState,
                          const DetailedTestResult& finalState, 
                          bool loadResult);
    
    // Test data and state
    std::vector<TestScenario> m_comprehensiveTestScenarios;
    QString m_testDataDir;
    std::unique_ptr<QApplication> m_testApp;
    
    // Test file paths for different categories
    QStringList m_validE57Files;
    QStringList m_validLasFiles;
    QStringList m_edgeCaseFiles;
    QStringList m_errorFiles;
    QStringList m_realWorldFiles;
};

/**
 * @brief Test fixture for E57 and LAS integration testing
 * 
 * Specialized test fixture for comprehensive file format testing
 * as specified in Sprint 1.4 requirements.
 */
class E57LasIntegrationTest : public IntegrationTestSuite {
protected:
    void SetUp() override {
        IntegrationTestSuite::SetUp();
        // Additional setup specific to E57/LAS testing
    }
    
    // File format specific helpers
    bool attemptFileLoad(const QString& filePath);
    void clearPointCloudViewer();
    bool validateE57FileStructure(const QString& filePath);
    bool validateLasFileStructure(const QString& filePath);
};

#endif // INTEGRATION_TEST_SUITE_H
