#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QApplication>
#include <QSignalSpy>

#include "app/MainPresenter.h"
#include "quality/QualityAssessment.h"
#include "quality/PDFReportGenerator.h"
#include "interfaces/IMainView.h"

// Mock classes for testing
class MockMainView : public IMainView
{
public:
    MOCK_METHOD(void, displayErrorMessage, (const QString& title, const QString& message), (override));
    MOCK_METHOD(void, displayInfoMessage, (const QString& title, const QString& message), (override));
    MOCK_METHOD(QString, askForSaveFilePath, (const QString& title, const QString& filter, const QString& defaultName), (override));
    MOCK_METHOD(void, updateStatusBar, (const QString& text), (override));
    MOCK_METHOD(void, setWindowTitle, (const QString& title), (override));
    MOCK_METHOD(IPointCloudViewer*, getViewer, (), (override));
    MOCK_METHOD(class SidebarWidget*, getSidebar, (), (override));
    MOCK_METHOD(class AlignmentControlPanel*, getAlignmentControlPanel, (), (override));
    MOCK_METHOD(void, showProgressDialog, (bool show, const QString& title, const QString& message), (override));
    MOCK_METHOD(void, updateProgress, (int percentage, const QString& stage), (override));
    MOCK_METHOD(void, setActionsEnabled, (bool enabled), (override));
    MOCK_METHOD(void, setProjectTitle, (const QString& projectName), (override));
    MOCK_METHOD(void, updateScanList, (const QStringList& scanNames), (override));
    MOCK_METHOD(void, highlightScan, (const QString& scanName), (override));
    MOCK_METHOD(void, showProjectHub, (), (override));
    MOCK_METHOD(void, showProjectView, (), (override));
    MOCK_METHOD(void, updateMemoryUsage, (size_t totalBytes), (override));
    MOCK_METHOD(void, updateRenderingStats, (float fps, int visiblePoints), (override));
    MOCK_METHOD(QString, askForOpenFilePath, (const QString& title, const QString& filter), (override));
    MOCK_METHOD(bool, askForConfirmation, (const QString& title, const QString& message), (override));
    MOCK_METHOD(QString, promptForClusterName, (const QString& title, const QString& defaultName), (override));
    MOCK_METHOD(void, loadScan, (const QString& scanId), (override));
    MOCK_METHOD(void, unloadScan, (const QString& scanId), (override));
    MOCK_METHOD(void, loadCluster, (const QString& clusterId), (override));
    MOCK_METHOD(void, unloadCluster, (const QString& clusterId), (override));
    MOCK_METHOD(void, viewPointCloud, (const QString& itemId, const QString& itemType), (override));
    MOCK_METHOD(void, deleteScan, (const QString& scanId, bool deletePhysicalFile), (override));
    MOCK_METHOD(void, performBatchOperation, (const QString& operation, const QStringList& scanIds), (override));
};

class Sprint6_2Test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create QApplication if it doesn't exist
        if (!QApplication::instance())
        {
            int argc = 0;
            char** argv = nullptr;
            app = new QApplication(argc, argv);
        }

        mockView = std::make_unique<MockMainView>();
        qualityAssessment = std::make_unique<QualityAssessment>();
        reportGenerator = std::make_unique<PDFReportGenerator>();
        
        presenter = std::make_unique<MainPresenter>(mockView.get(), nullptr, nullptr, nullptr);
        presenter->setQualityAssessment(qualityAssessment.get());
        presenter->setPDFReportGenerator(reportGenerator.get());
    }

    void TearDown() override
    {
        presenter.reset();
        reportGenerator.reset();
        qualityAssessment.reset();
        mockView.reset();
    }

    QApplication* app = nullptr;
    std::unique_ptr<MockMainView> mockView;
    std::unique_ptr<QualityAssessment> qualityAssessment;
    std::unique_ptr<PDFReportGenerator> reportGenerator;
    std::unique_ptr<MainPresenter> presenter;
};

// Test Case 1: Generate Report action enablement
TEST_F(Sprint6_2Test, GenerateReportActionEnablement)
{
    // Create a valid quality report
    QualityReport report;
    report.projectName = "Test Project";
    report.timestamp = "2024-01-01 12:00:00";
    report.metrics.totalPoints = 1000;
    report.metrics.rmsError = 0.05f;

    // Simulate quality assessment completion
    presenter->onQualityAssessmentCompleted(report);

    // Verify that the presenter has stored the report
    // Note: This would require exposing the m_lastQualityReport for testing
    // or adding a getter method
}

// Test Case 2: Report generation initiation
TEST_F(Sprint6_2Test, ReportGenerationInitiation)
{
    // Setup expectations
    EXPECT_CALL(*mockView, askForSaveFilePath(
        "Save Quality Report", 
        "PDF files (*.pdf)", 
        "Test Project_QualityReport.pdf"))
        .WillOnce(::testing::Return("test_report.pdf"));

    EXPECT_CALL(*mockView, updateStatusBar("Generating quality report..."));

    // Create a valid quality report
    QualityReport report;
    report.projectName = "Test Project";
    report.timestamp = "2024-01-01 12:00:00";
    report.metrics.totalPoints = 1000;
    report.metrics.rmsError = 0.05f;

    // Set up the presenter with a valid report
    presenter->onQualityAssessmentCompleted(report);

    // Trigger report generation
    presenter->handleGenerateReportClicked();
}

// Test Case 3: Report generation with no quality data
TEST_F(Sprint6_2Test, ReportGenerationWithNoData)
{
    EXPECT_CALL(*mockView, displayErrorMessage(
        "Generate Quality Report",
        "No quality assessment data available. Please perform a quality assessment first."));

    // Try to generate report without quality assessment
    presenter->handleGenerateReportClicked();
}

// Test Case 4: QualityReport validation
TEST_F(Sprint6_2Test, QualityReportValidation)
{
    // Test invalid report (empty project name)
    QualityReport invalidReport;
    EXPECT_FALSE(invalidReport.isValid());

    // Test valid report
    QualityReport validReport;
    validReport.projectName = "Test Project";
    validReport.timestamp = "2024-01-01 12:00:00";
    validReport.metrics.totalPoints = 1000;
    EXPECT_TRUE(validReport.isValid());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
