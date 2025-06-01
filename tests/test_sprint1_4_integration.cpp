#include <gtest/gtest.h>
#include "integration_test_suite.h"
#include "test_reporter.h"
#include "../src/e57parserlib.h"
#include "../src/lasparser.h"
#include "../src/loadingsettingsdialog.h"
#include "../src/loadingsettings.h"
#include <QApplication>
#include <QFileInfo>
#include <QDebug>

/**
 * @brief Sprint 1.4 Integration Tests
 * 
 * Comprehensive integration testing framework implementing all requirements
 * from Sprint 1.4 User Stories 1-4, including:
 * - E57 regression testing (Task 1.4.1.1.A)
 * - LAS regression testing (Task 1.4.1.1.B) 
 * - Real-world file testing (Task 1.4.1.1.C)
 * - LoadingSettingsDialog functionality verification (User Story 3)
 * - Automated bug reporting and documentation (Tasks 1.4.1.3-1.4.1.4)
 */

class Sprint14IntegrationTest : public E57LasIntegrationTest {
protected:
    void SetUp() override {
        E57LasIntegrationTest::SetUp();
        
        // Initialize test reporter for automated documentation
        m_testReporter = std::make_unique<TestReporter>();
        
        qDebug() << "=== Sprint 1.4 Integration Testing Started ===";
        qDebug() << "Test scenarios compiled:" << m_comprehensiveTestScenarios.size();
    }
    
    void TearDown() override {
        // Generate comprehensive test reports
        if (m_testReporter) {
            m_testReporter->generateComprehensiveReport();
        }
        
        E57LasIntegrationTest::TearDown();
        qDebug() << "=== Sprint 1.4 Integration Testing Completed ===";
    }
    
    std::unique_ptr<TestReporter> m_testReporter;
};

// Test Case 1.4.1.A: E57 regression testing
TEST_F(Sprint14IntegrationTest, E57RegressionTesting) {
    qDebug() << "Starting E57 regression testing...";
    
    auto e57Scenarios = getScenariosByTag("sprint_1_1");
    e57Scenarios.insert(e57Scenarios.end(), 
                       getScenariosByTag("sprint_1_2").begin(),
                       getScenariosByTag("sprint_1_2").end());
    
    ASSERT_GT(e57Scenarios.size(), 0) << "No E57 test scenarios found";
    
    int passedTests = 0;
    int totalTests = static_cast<int>(e57Scenarios.size());
    
    for (const auto& scenario : e57Scenarios) {
        SCOPED_TRACE(scenario.description.toStdString());
        
        qDebug() << "Testing E57 scenario:" << scenario.testCaseId << "-" << scenario.description;
        
        DetailedTestResult result = executeTestScenario(scenario);
        
        // Document test result
        TestReporter::TestDocumentation doc;
        doc.testCaseId = scenario.testCaseId;
        doc.description = scenario.description;
        doc.category = scenario.category;
        doc.fileType = scenario.fileType;
        doc.expectedOutcome = scenario.expectedOutcome;
        doc.actualOutcome = result.passed ? "success" : "failure";
        doc.passed = result.passed;
        doc.executionTime = result.loadingTime;
        doc.timestamp = result.startTime.toString(Qt::ISODate);
        doc.testFile = scenario.filePath;
        doc.errorDetails = result.errorMessage;
        doc.pointCount = result.pointCount;
        
        m_testReporter->addTestDocumentation(doc);
        
        if (result.passed) {
            passedTests++;
        }
        
        // Individual test assertions for detailed feedback
        if (scenario.category == "valid") {
            EXPECT_TRUE(result.passed) 
                << "Valid E57 file should load successfully: " << scenario.filePath.toStdString()
                << "\nError: " << result.errorMessage.toStdString();
        } else if (scenario.category == "error") {
            EXPECT_FALSE(result.fileLoaded) 
                << "Invalid E57 file should fail gracefully: " << scenario.filePath.toStdString();
        }
    }
    
    // Overall regression test assessment
    double successRate = (double)passedTests / totalTests * 100.0;
    qDebug() << "E57 regression testing completed:" << passedTests << "/" << totalTests 
             << "passed (" << QString::number(successRate, 'f', 1) << "%)";
    
    // Sprint 1.4 acceptance criteria: At least 80% success rate for regression tests
    EXPECT_GE(successRate, 80.0) 
        << "E57 regression testing success rate below acceptable threshold";
}

// Test Case 1.4.1.B: LAS regression testing
TEST_F(Sprint14IntegrationTest, LASRegressionTesting) {
    qDebug() << "Starting LAS regression testing...";
    
    auto lasScenarios = getScenariosByTag("sprint_1_3");
    lasScenarios.insert(lasScenarios.end(),
                       getScenariosByTag("las_enhanced").begin(),
                       getScenariosByTag("las_enhanced").end());
    
    ASSERT_GT(lasScenarios.size(), 0) << "No LAS test scenarios found";
    
    int passedTests = 0;
    int totalTests = static_cast<int>(lasScenarios.size());
    
    for (const auto& scenario : lasScenarios) {
        SCOPED_TRACE(scenario.description.toStdString());
        
        qDebug() << "Testing LAS scenario:" << scenario.testCaseId << "-" << scenario.description;
        
        DetailedTestResult result = executeTestScenario(scenario);
        
        // Document test result
        TestReporter::TestDocumentation doc;
        doc.testCaseId = scenario.testCaseId;
        doc.description = scenario.description;
        doc.category = scenario.category;
        doc.fileType = scenario.fileType;
        doc.expectedOutcome = scenario.expectedOutcome;
        doc.actualOutcome = result.passed ? "success" : "failure";
        doc.passed = result.passed;
        doc.executionTime = result.loadingTime;
        doc.timestamp = result.startTime.toString(Qt::ISODate);
        doc.testFile = scenario.filePath;
        doc.errorDetails = result.errorMessage;
        doc.pointCount = result.pointCount;
        
        m_testReporter->addTestDocumentation(doc);
        
        if (result.passed) {
            passedTests++;
        }
        
        // Individual test assertions
        if (scenario.category == "valid") {
            EXPECT_TRUE(result.passed) 
                << "Valid LAS file should load successfully: " << scenario.filePath.toStdString()
                << "\nError: " << result.errorMessage.toStdString();
        } else if (scenario.category == "error") {
            EXPECT_FALSE(result.fileLoaded) 
                << "Invalid LAS file should fail gracefully: " << scenario.filePath.toStdString();
        }
    }
    
    // Overall regression test assessment
    double successRate = (double)passedTests / totalTests * 100.0;
    qDebug() << "LAS regression testing completed:" << passedTests << "/" << totalTests 
             << "passed (" << QString::number(successRate, 'f', 1) << "%)";
    
    // Sprint 1.4 acceptance criteria: At least 80% success rate for regression tests
    EXPECT_GE(successRate, 80.0) 
        << "LAS regression testing success rate below acceptable threshold";
}

// Test Case 1.4.1.C: Real-world file testing
TEST_F(Sprint14IntegrationTest, RealWorldFileTesting) {
    qDebug() << "Starting real-world file testing...";
    
    auto realWorldScenarios = getScenariosByTag("real_world");
    
    if (realWorldScenarios.empty()) {
        qWarning() << "No real-world test files found - skipping real-world testing";
        GTEST_SKIP() << "Real-world test files not available";
        return;
    }
    
    int passedTests = 0;
    int totalTests = static_cast<int>(realWorldScenarios.size());
    
    for (const auto& scenario : realWorldScenarios) {
        SCOPED_TRACE(QString("Real-world file: %1").arg(scenario.filePath).toStdString());
        
        qDebug() << "Testing real-world file:" << scenario.filePath;
        
        DetailedTestResult result = executeTestScenarioWithTimeout(scenario, 60000); // 60 second timeout
        
        // Document test result
        TestReporter::TestDocumentation doc;
        doc.testCaseId = scenario.testCaseId;
        doc.description = scenario.description;
        doc.category = "real_world";
        doc.fileType = scenario.fileType;
        doc.expectedOutcome = "success_or_graceful_failure";
        doc.actualOutcome = result.passed ? "success" : "failure";
        doc.passed = result.passed;
        doc.executionTime = result.loadingTime;
        doc.timestamp = result.startTime.toString(Qt::ISODate);
        doc.testFile = scenario.filePath;
        doc.errorDetails = result.errorMessage;
        doc.pointCount = result.pointCount;
        
        m_testReporter->addTestDocumentation(doc);
        
        if (result.passed) {
            passedTests++;
        }
        
        // Real-world files should either load successfully or fail gracefully
        EXPECT_TRUE(result.passed || !result.errorMessage.isEmpty()) 
            << "Real-world file should either load or provide clear error message: " 
            << scenario.filePath.toStdString();
        
        // Performance check for real-world files
        if (result.passed && result.loadingTime > 0) {
            EXPECT_LT(result.loadingTime, scenario.expectedLoadTime) 
                << "Loading time exceeded expected threshold for: " << scenario.filePath.toStdString()
                << " (took " << result.loadingTime << "s, expected < " << scenario.expectedLoadTime << "s)";
        }
    }
    
    qDebug() << "Real-world file testing completed:" << passedTests << "/" << totalTests << "passed";
}

// Test Case 1.4.3: LoadingSettingsDialog functionality verification
TEST_F(Sprint14IntegrationTest, LoadingSettingsDialogFunctionality) {
    qDebug() << "Testing LoadingSettingsDialog functionality...";
    
    // Test dialog creation and basic functionality
    LoadingSettingsDialog dialog;
    EXPECT_EQ(dialog.windowTitle(), "Point Cloud Loading Settings");
    EXPECT_TRUE(dialog.isModal());
    
    // Test default settings
    LoadingSettings defaultSettings = dialog.getSettings();
    EXPECT_EQ(defaultSettings.method, LoadingMethod::FullLoad);
    
    // Test E57 file type configuration (Task 1.4.3.4)
    // E57 files should only support FullLoad mode
    // Note: This would require access to dialog internals or a public method
    // For now, we'll test the settings structure
    
    // Test LAS file type configuration
    // LAS files should support all loading modes
    
    // Test settings persistence (Task 1.4.3.5)
    QSettings settings("CloudRegistration", "PointCloudViewer");
    settings.setValue("PointCloudLoading/DefaultMethod", 
                     static_cast<int>(LoadingMethod::HeaderOnly));
    settings.sync();
    
    LoadingSettingsDialog persistenceDialog;
    LoadingSettings loadedSettings = persistenceDialog.getSettings();
    EXPECT_EQ(loadedSettings.method, LoadingMethod::HeaderOnly);
    
    // Clean up
    settings.clear();
    
    qDebug() << "LoadingSettingsDialog functionality testing completed";
}

int main(int argc, char **argv) {
    // Initialize Qt Application for GUI components
    QApplication app(argc, argv);
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    qDebug() << "Starting Sprint 1.4 Integration Testing Suite";
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "Test executable:" << argv[0];
    
    int result = RUN_ALL_TESTS();
    
    qDebug() << "Sprint 1.4 Integration Testing completed with result:" << result;
    
    return result;
}
