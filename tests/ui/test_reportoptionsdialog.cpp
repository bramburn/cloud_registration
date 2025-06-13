#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <QTest>

#include "ui/ReportOptionsDialog.h"
#include "quality/PDFReportGenerator.h"
#include "tests/mocks/MockMainView.h"

class ReportOptionsDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create mock main view
        m_mockView = std::make_unique<MockMainView>();
        
        // Create dialog with mock view
        m_dialog = std::make_unique<ReportOptionsDialog>(m_mockView.get());
    }

    void TearDown() override
    {
        m_dialog.reset();
        m_mockView.reset();
    }

    std::unique_ptr<MockMainView> m_mockView;
    std::unique_ptr<ReportOptionsDialog> m_dialog;
};

// Test Case 1: UI element presence and default values
TEST_F(ReportOptionsDialogTest, UIElementsPresent)
{
    ASSERT_NE(m_dialog, nullptr);
    
    // Check that dialog has proper title and is modal
    EXPECT_EQ(m_dialog->windowTitle(), "PDF Report Options");
    EXPECT_TRUE(m_dialog->isModal());
    
    // Check minimum size is set
    EXPECT_GE(m_dialog->minimumWidth(), 500);
    EXPECT_GE(m_dialog->minimumHeight(), 600);
    
    // Verify default checkbox states
    auto options = m_dialog->getReportOptions();
    EXPECT_FALSE(options.includeCharts);
    EXPECT_FALSE(options.includeScreenshots);
    EXPECT_FALSE(options.includeRecommendations);
    EXPECT_TRUE(options.includeDetailedMetrics); // Should be checked by default
}

// Test Case 2: Parameter get/set round-trip
TEST_F(ReportOptionsDialogTest, ParameterRoundTrip)
{
    // Create test options
    PDFReportGenerator::ReportOptions testOptions;
    testOptions.reportTitle = "Test Report Title";
    testOptions.companyName = "Test Company";
    testOptions.operatorName = "Test Operator";
    testOptions.logoPath = "/path/to/logo.png";
    testOptions.outputPath = "/path/to/output.pdf";
    testOptions.includeCharts = true;
    testOptions.includeScreenshots = true;
    testOptions.includeRecommendations = false;
    testOptions.includeDetailedMetrics = true;
    
    // Set options in dialog
    m_dialog->setReportOptions(testOptions);
    
    // Get options back from dialog
    auto retrievedOptions = m_dialog->getReportOptions();
    
    // Verify all fields match
    EXPECT_EQ(retrievedOptions.reportTitle, testOptions.reportTitle);
    EXPECT_EQ(retrievedOptions.companyName, testOptions.companyName);
    EXPECT_EQ(retrievedOptions.operatorName, testOptions.operatorName);
    EXPECT_EQ(retrievedOptions.logoPath, testOptions.logoPath);
    EXPECT_EQ(retrievedOptions.outputPath, testOptions.outputPath);
    EXPECT_EQ(retrievedOptions.includeCharts, testOptions.includeCharts);
    EXPECT_EQ(retrievedOptions.includeScreenshots, testOptions.includeScreenshots);
    EXPECT_EQ(retrievedOptions.includeRecommendations, testOptions.includeRecommendations);
    EXPECT_EQ(retrievedOptions.includeDetailedMetrics, testOptions.includeDetailedMetrics);
}

// Test Case 3: Progress display
TEST_F(ReportOptionsDialogTest, ProgressDisplay)
{
    // Initially progress should be hidden
    // Note: We can't directly access private members, but we can test behavior
    
    // Simulate progress updates
    m_dialog->onReportProgress(25, "Processing data...");
    m_dialog->onReportProgress(50, "Generating charts...");
    m_dialog->onReportProgress(75, "Writing PDF...");
    m_dialog->onReportProgress(100, "Completed");
    
    // Test successful completion
    m_dialog->onReportFinished(true, "Report generated successfully at /path/to/report.pdf");
    
    // Test error handling
    m_dialog->onReportFinished(false, "Failed to write to output file");
}

// Test Case 4: generateReportRequested signal
TEST_F(ReportOptionsDialogTest, GenerateReportSignal)
{
    // Set up signal spy
    QSignalSpy spy(m_dialog.get(), &ReportOptionsDialog::generateReportRequested);
    
    // Set valid options
    PDFReportGenerator::ReportOptions testOptions;
    testOptions.reportTitle = "Test Report";
    testOptions.outputPath = "/valid/path/report.pdf";
    m_dialog->setReportOptions(testOptions);
    
    // Note: We can't directly trigger the private slot onGenerateButtonClicked()
    // In a real test, we would need to access the button and click it
    // For now, we'll test that the signal can be emitted
    
    // Verify signal spy is set up correctly
    EXPECT_TRUE(spy.isValid());
    EXPECT_EQ(spy.count(), 0);
}

// Test Case 5: Validation logic
TEST_F(ReportOptionsDialogTest, ValidationLogic)
{
    // Test with empty title - should fail validation
    PDFReportGenerator::ReportOptions emptyTitleOptions;
    emptyTitleOptions.reportTitle = "";
    emptyTitleOptions.outputPath = "/valid/path/report.pdf";
    m_dialog->setReportOptions(emptyTitleOptions);
    
    // Test with empty output path - should fail validation
    PDFReportGenerator::ReportOptions emptyPathOptions;
    emptyPathOptions.reportTitle = "Valid Title";
    emptyPathOptions.outputPath = "";
    m_dialog->setReportOptions(emptyPathOptions);
    
    // Test with valid options - should pass validation
    PDFReportGenerator::ReportOptions validOptions;
    validOptions.reportTitle = "Valid Title";
    validOptions.outputPath = "/valid/path/report.pdf";
    m_dialog->setReportOptions(validOptions);
    
    // Note: Actual validation testing would require access to private methods
    // or triggering the validation through button clicks
}

// Test Case 6: File browsing integration
TEST_F(ReportOptionsDialogTest, FileBrowsingIntegration)
{
    // Set up mock expectations for file dialogs
    EXPECT_CALL(*m_mockView, askForSaveFilePath(
        QString("Save Quality Report"),
        QString("PDF files (*.pdf)"),
        testing::_))
        .WillOnce(testing::Return("/selected/path/report.pdf"));
    
    EXPECT_CALL(*m_mockView, askForOpenFilePath(
        QString("Select Company Logo"),
        QString("Image files (*.png *.jpg *.jpeg *.bmp *.gif)")))
        .WillOnce(testing::Return("/selected/logo.png"));
    
    // Note: In a real test, we would trigger the browse button clicks
    // and verify that the paths are set correctly in the dialog
}

// Test Case 7: Default options factory method
TEST_F(ReportOptionsDialogTest, DefaultOptionsFactory)
{
    // Test createDefault with project name
    auto defaultOptions = PDFReportGenerator::ReportOptions::createDefault("Test Project");
    
    EXPECT_EQ(defaultOptions.projectName, "Test Project");
    EXPECT_EQ(defaultOptions.reportTitle, "Quality Report - Test Project");
    EXPECT_EQ(defaultOptions.operatorName, "Default User");
    EXPECT_EQ(defaultOptions.companyName, "CloudRegistration");
    EXPECT_FALSE(defaultOptions.includeCharts);
    EXPECT_FALSE(defaultOptions.includeScreenshots);
    EXPECT_TRUE(defaultOptions.includeRecommendations);
    EXPECT_TRUE(defaultOptions.includeDetailedMetrics);
    EXPECT_TRUE(defaultOptions.logoPath.isEmpty());
    EXPECT_TRUE(defaultOptions.outputPath.isEmpty());
    
    // Test createDefault without project name
    auto defaultOptionsNoProject = PDFReportGenerator::ReportOptions::createDefault();
    
    EXPECT_EQ(defaultOptionsNoProject.projectName, "Untitled Project");
    EXPECT_EQ(defaultOptionsNoProject.reportTitle, "Point Cloud Registration Quality Report");
}

// Integration test helper
class ReportOptionsDialogIntegrationTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Ensure QApplication exists for widget tests
        if (!QApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            m_app = std::make_unique<QApplication>(argc, argv);
        }
        
        m_mockView = std::make_unique<MockMainView>();
        m_dialog = std::make_unique<ReportOptionsDialog>(m_mockView.get());
    }

    void TearDown() override
    {
        m_dialog.reset();
        m_mockView.reset();
    }

    std::unique_ptr<QApplication> m_app;
    std::unique_ptr<MockMainView> m_mockView;
    std::unique_ptr<ReportOptionsDialog> m_dialog;
};

// Integration Test: Full dialog workflow
TEST_F(ReportOptionsDialogIntegrationTest, FullWorkflow)
{
    // Set up default options
    auto defaultOptions = PDFReportGenerator::ReportOptions::createDefault("Integration Test Project");
    m_dialog->setReportOptions(defaultOptions);
    
    // Verify options are set correctly
    auto retrievedOptions = m_dialog->getReportOptions();
    EXPECT_EQ(retrievedOptions.projectName, "Integration Test Project");
    EXPECT_EQ(retrievedOptions.reportTitle, "Quality Report - Integration Test Project");
    
    // Test progress simulation
    m_dialog->onReportProgress(0, "Starting...");
    m_dialog->onReportProgress(50, "Halfway...");
    m_dialog->onReportProgress(100, "Complete");
    
    // Test successful completion
    m_dialog->onReportFinished(true, "Report generated successfully");
    
    // Dialog should be in completed state
    // Note: In a real integration test, we would verify UI state changes
}
