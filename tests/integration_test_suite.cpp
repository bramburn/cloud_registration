#include "integration_test_suite.h"
#include "../src/e57parser.h"
#include "../src/lasparser.h"
#include "../src/loadingsettings.h"
#include <QStandardPaths>
#include <QCoreApplication>

void IntegrationTestSuite::SetUp() {
    setupTestDataDirectories();
    setupTestApplication();
    compileComprehensiveTestScenarios();
}

void IntegrationTestSuite::TearDown() {
    // Cleanup test artifacts
    m_comprehensiveTestScenarios.clear();
}

void IntegrationTestSuite::setupTestDataDirectories() {
    // Set up test data directory
    m_testDataDir = QDir::currentPath() + "/test_data";
    QDir testDir(m_testDataDir);
    if (!testDir.exists()) {
        qWarning() << "Test data directory does not exist:" << m_testDataDir;
        qWarning() << "Some integration tests may be skipped";
    }
    
    qDebug() << "Test data directory:" << m_testDataDir;
}

void IntegrationTestSuite::setupTestApplication() {
    // Initialize test application if needed
    if (!QCoreApplication::instance()) {
        static int argc = 1;
        static char* argv[] = {const_cast<char*>("test"), nullptr};
        m_testApp = std::make_unique<QApplication>(argc, argv);
    }
}

void IntegrationTestSuite::compileComprehensiveTestScenarios() {
    // Task 1.4.1.1.A: Valid E57 files (simple uncompressed, CompressedVector with uncompressed data)
    m_comprehensiveTestScenarios.push_back({
        "test_data/simple_uncompressed.e57",
        "valid", "e57", "success",
        "Basic E57 with uncompressed XYZ data from Sprint 1.1",
        {"sprint_1_1", "basic_functionality"},
        "1.4.1.A.1",
        5.0,  // Expected load time
        -1    // Unknown point count
    });
    
    m_comprehensiveTestScenarios.push_back({
        "test_data/compressedvector_uncompressed_data.e57", 
        "valid", "e57", "success",
        "E57 with CompressedVector containing uncompressed data from Sprint 1.2",
        {"sprint_1_2", "compressed_vector"},
        "1.4.1.A.2",
        10.0,
        -1
    });
    
    // Task 1.4.1.1.B: Valid LAS files (versions 1.2, 1.3, 1.4; PDRFs 0-3)
    for (int version = 2; version <= 4; version++) {
        for (int pdrf = 0; pdrf <= 3; pdrf++) {
            QString fileName = QString("test_data/las_v1_%1_pdrf_%2.las").arg(version).arg(pdrf);
            m_comprehensiveTestScenarios.push_back({
                fileName,
                "valid", "las", "success",
                QString("LAS v1.%1 with PDRF %2 from Sprint 1.3").arg(version).arg(pdrf),
                {"sprint_1_3", "las_enhanced"},
                QString("1.4.1.B.%1.%2").arg(version).arg(pdrf),
                3.0,
                -1
            });
        }
    }
    
    // Task 1.4.1.1.C: Edge cases
    m_comprehensiveTestScenarios.push_back({
        "test_data/test_large_coords.e57",
        "edge_case", "e57", "success",
        "E57 with large coordinate values",
        {"sprint_1_1", "edge_cases"},
        "1.4.1.C.1",
        8.0,
        -1
    });
    
    m_comprehensiveTestScenarios.push_back({
        "test_data/test_3_points_line.e57",
        "edge_case", "e57", "success", 
        "E57 with minimal point count (3 points)",
        {"sprint_1_1", "edge_cases"},
        "1.4.1.C.2",
        2.0,
        3
    });
    
    // Task 1.4.1.1.D: Error cases
    m_comprehensiveTestScenarios.push_back({
        "test_data/malformed_compressedvector.e57",
        "error", "e57", "graceful_failure",
        "Malformed E57 CompressedVector structure",
        {"sprint_1_2", "error_handling"},
        "1.4.1.D.1",
        5.0,
        0
    });
    
    m_comprehensiveTestScenarios.push_back({
        "test_data/nonexistent_file.e57",
        "error", "e57", "file_not_found",
        "Non-existent E57 file",
        {"sprint_1_1", "error_handling"},
        "1.4.1.D.2",
        1.0,
        0
    });
    
    // Real-world test files
    if (QFile::exists("sample/bunnyDouble.e57")) {
        m_comprehensiveTestScenarios.push_back({
            "sample/bunnyDouble.e57",
            "real_world", "e57", "success",
            "Real-world E57 file - bunnyDouble",
            {"real_world", "sprint_1_1"},
            "1.4.1.R.1",
            15.0,
            -1
        });
    }
    
    if (QFile::exists("sample/S2max-Power line202503.las")) {
        m_comprehensiveTestScenarios.push_back({
            "sample/S2max-Power line202503.las",
            "real_world", "las", "success",
            "Real-world LAS file - S2max Power line",
            {"real_world", "sprint_1_3"},
            "1.4.1.R.2",
            20.0,
            -1
        });
    }
    
    qDebug() << "Compiled" << m_comprehensiveTestScenarios.size() << "test scenarios";
}

std::vector<IntegrationTestSuite::TestScenario> IntegrationTestSuite::getScenariosByTag(const QString& tag) {
    std::vector<TestScenario> filtered;
    for (const auto& scenario : m_comprehensiveTestScenarios) {
        if (scenario.sprintTags.contains(tag)) {
            filtered.push_back(scenario);
        }
    }
    return filtered;
}

std::vector<IntegrationTestSuite::TestScenario> IntegrationTestSuite::getScenariosByCategory(const QString& category) {
    std::vector<TestScenario> filtered;
    for (const auto& scenario : m_comprehensiveTestScenarios) {
        if (scenario.category == category) {
            filtered.push_back(scenario);
        }
    }
    return filtered;
}

IntegrationTestSuite::DetailedTestResult IntegrationTestSuite::executeTestScenario(const TestScenario& scenario) {
    DetailedTestResult result;
    result.startTime = QDateTime::currentDateTime();
    result.testFile = scenario.filePath;
    result.testCaseId = scenario.testCaseId;
    result.passed = false;
    result.timedOut = false;
    
    qDebug() << "Executing test scenario:" << scenario.testCaseId << "-" << scenario.description;
    
    // Check if test file exists
    if (!QFile::exists(scenario.filePath)) {
        if (scenario.expectedOutcome == "file_not_found") {
            result.passed = true;
            result.errorMessage = "File not found (expected)";
        } else {
            result.passed = false;
            result.errorMessage = QString("Test file not found: %1").arg(scenario.filePath);
        }
        result.endTime = QDateTime::currentDateTime();
        result.duration = result.startTime.msecsTo(result.endTime);
        return result;
    }
    
    // Attempt to parse the file
    QFileInfo fileInfo(scenario.filePath);
    QString extension = fileInfo.suffix().toLower();
    
    try {
        if (extension == "e57") {
            E57Parser parser;
            std::vector<float> points = parser.parse(scenario.filePath);
            
            result.fileLoaded = !parser.getLastError().isEmpty() ? false : true;
            result.pointCount = static_cast<int>(points.size() / 3);
            result.errorMessage = parser.getLastError();
            result.viewerHasData = !points.empty();

        } else if (extension == "las") {
            LasParser parser;
            LoadingSettings settings;
            settings.method = LoadingMethod::FullLoad;

            std::vector<float> points = parser.parse(scenario.filePath, settings);

            result.fileLoaded = !parser.getLastError().isEmpty() ? false : true;
            result.pointCount = static_cast<int>(points.size() / 3);
            result.errorMessage = parser.getLastError();
            result.viewerHasData = !points.empty();
        }
        
        // Evaluate test success based on expected outcome
        if (scenario.expectedOutcome == "success") {
            result.passed = result.fileLoaded && result.viewerHasData;
        } else if (scenario.expectedOutcome == "graceful_failure") {
            result.passed = !result.fileLoaded && result.errorMessage.isEmpty() == false;
        } else {
            result.passed = !result.fileLoaded;
        }
        
    } catch (const std::exception& e) {
        result.passed = false;
        result.errorMessage = QString("Exception during parsing: %1").arg(e.what());
    }
    
    result.endTime = QDateTime::currentDateTime();
    result.duration = result.startTime.msecsTo(result.endTime);
    result.loadingTime = result.duration / 1000.0;

    return result;
}

IntegrationTestSuite::DetailedTestResult IntegrationTestSuite::executeTestScenarioWithTimeout(const TestScenario& scenario, int timeoutMs) {
    // For now, just call the regular executeTestScenario method
    // In a full implementation, this would use QTimer and QEventLoop for timeout handling
    DetailedTestResult result = executeTestScenario(scenario);

    // Check if execution took longer than timeout
    if (result.duration > timeoutMs) {
        result.timedOut = true;
        result.passed = false;
        result.errorMessage = QString("Test execution timed out after %1ms").arg(timeoutMs);
    }

    return result;
}
