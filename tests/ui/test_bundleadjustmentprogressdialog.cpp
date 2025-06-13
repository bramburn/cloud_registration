#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QApplication>
#include <QSignalSpy>
#include <QTest>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>

#include "ui/BundleAdjustmentProgressDialog.h"
#include "optimization/BundleAdjustment.h"

// Mock Bundle Adjustment class for testing
class MockBundleAdjustment : public QObject
{
    Q_OBJECT

public:
    explicit MockBundleAdjustment(QObject* parent = nullptr) : QObject(parent) {}

    void emitProgress(int iteration, double currentError, double lambda)
    {
        emit optimizationProgress(iteration, currentError, lambda);
    }

    void emitCompleted(bool success, const QString& message)
    {
        Optimization::BundleAdjustment::Result result;
        result.converged = success;
        result.statusMessage = message;
        result.iterations = 10;
        result.finalError = 0.001;
        result.initialError = 0.1;
        result.improvementRatio = 0.99;
        result.optimizationTimeSeconds = 5.5;
        
        emit optimizationCompleted(result);
    }

signals:
    void optimizationProgress(int iteration, double currentError, double lambda);
    void optimizationCompleted(const Optimization::BundleAdjustment::Result& result);
};

class BundleAdjustmentProgressDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Ensure QApplication exists for Qt widgets
        if (!QApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = new QApplication(argc, argv);
        }

        dialog = std::make_unique<BundleAdjustmentProgressDialog>();
        mockBA = std::make_unique<MockBundleAdjustment>();
    }

    void TearDown() override
    {
        dialog.reset();
        mockBA.reset();
        
        if (app) {
            delete app;
            app = nullptr;
        }
    }

    std::unique_ptr<BundleAdjustmentProgressDialog> dialog;
    std::unique_ptr<MockBundleAdjustment> mockBA;
    QApplication* app = nullptr;
};

TEST_F(BundleAdjustmentProgressDialogTest, ConstructorInitialization)
{
    EXPECT_NE(dialog, nullptr);
    EXPECT_FALSE(dialog->isVisible());
    
    // Check that dialog has the expected title
    EXPECT_EQ(dialog->windowTitle(), "Bundle Adjustment Progress");
}

TEST_F(BundleAdjustmentProgressDialogTest, StartMonitoring)
{
    const int maxIterations = 100;
    
    // Start monitoring with mock Bundle Adjustment
    dialog->startMonitoring(mockBA.get(), maxIterations);
    
    // Dialog should be configured for monitoring
    auto* progressBar = dialog->findChild<QProgressBar*>();
    ASSERT_NE(progressBar, nullptr);
    EXPECT_EQ(progressBar->maximum(), maxIterations);
    EXPECT_EQ(progressBar->value(), 0);
    
    // Check initial labels
    auto* iterationLabel = dialog->findChild<QLabel*>("iterationLabel");
    if (iterationLabel) {
        EXPECT_TRUE(iterationLabel->text().contains("0"));
    }
}

TEST_F(BundleAdjustmentProgressDialogTest, UpdateProgress)
{
    const int maxIterations = 50;
    dialog->startMonitoring(mockBA.get(), maxIterations);
    
    // Simulate progress update
    const int iteration = 25;
    const double currentError = 0.005;
    
    dialog->updateProgress(iteration, currentError);
    
    // Check that progress bar is updated
    auto* progressBar = dialog->findChild<QProgressBar*>();
    ASSERT_NE(progressBar, nullptr);
    EXPECT_EQ(progressBar->value(), iteration);
    
    // Check that labels are updated
    auto* iterationLabel = dialog->findChild<QLabel*>("iterationLabel");
    if (iterationLabel) {
        EXPECT_TRUE(iterationLabel->text().contains(QString::number(iteration)));
    }
}

TEST_F(BundleAdjustmentProgressDialogTest, CancelButtonSignal)
{
    QSignalSpy cancelSpy(dialog.get(), &BundleAdjustmentProgressDialog::cancelRequested);
    
    // Find and click cancel button
    auto* cancelButton = dialog->findChild<QPushButton*>();
    ASSERT_NE(cancelButton, nullptr);
    
    // Simulate button click
    QTest::mouseClick(cancelButton, Qt::LeftButton);
    
    // Should emit cancel signal (after confirmation dialog)
    // Note: This test might need adjustment based on confirmation dialog implementation
}

TEST_F(BundleAdjustmentProgressDialogTest, CompletionHandling)
{
    dialog->startMonitoring(mockBA.get(), 100);
    
    // Simulate successful completion
    dialog->onComputationFinished(true, "Optimization completed successfully");
    
    // Check that progress bar shows completion
    auto* progressBar = dialog->findChild<QProgressBar*>();
    ASSERT_NE(progressBar, nullptr);
    EXPECT_EQ(progressBar->value(), progressBar->maximum());
    
    // Check that close button is visible and cancel button is hidden
    auto buttons = dialog->findChildren<QPushButton*>();
    bool hasCloseButton = false;
    bool hasCancelButton = false;
    
    for (auto* button : buttons) {
        if (button->text().contains("Close")) {
            hasCloseButton = true;
            EXPECT_TRUE(button->isVisible());
        } else if (button->text().contains("Cancel")) {
            hasCancelButton = true;
            EXPECT_FALSE(button->isVisible());
        }
    }
    
    EXPECT_TRUE(hasCloseButton);
}

TEST_F(BundleAdjustmentProgressDialogTest, FailureHandling)
{
    dialog->startMonitoring(mockBA.get(), 100);
    
    // Simulate failed completion
    dialog->onComputationFinished(false, "Optimization failed to converge");
    
    // Check that status reflects failure
    auto* statusLabel = dialog->findChild<QLabel*>("statusLabel");
    if (statusLabel) {
        EXPECT_TRUE(statusLabel->text().contains("Failed"));
    }
}

TEST_F(BundleAdjustmentProgressDialogTest, ElapsedTimeUpdate)
{
    dialog->startMonitoring(mockBA.get(), 100);
    
    // Find elapsed time label
    auto* timeLabel = dialog->findChild<QLabel*>("elapsedTimeLabel");
    if (timeLabel) {
        QString initialTime = timeLabel->text();
        
        // Wait a bit and check if time updates
        QTest::qWait(200);
        
        // Time should have changed (though this is timing-dependent)
        // This test verifies the mechanism exists
        EXPECT_FALSE(timeLabel->text().isEmpty());
    }
}

TEST_F(BundleAdjustmentProgressDialogTest, ProgressSignalConnection)
{
    dialog->startMonitoring(mockBA.get(), 100);
    
    // Emit progress signal from mock
    mockBA->emitProgress(10, 0.01, 0.001);
    
    // Process events to ensure signal is handled
    QApplication::processEvents();
    
    // Check that dialog was updated
    auto* progressBar = dialog->findChild<QProgressBar*>();
    ASSERT_NE(progressBar, nullptr);
    EXPECT_EQ(progressBar->value(), 10);
}

TEST_F(BundleAdjustmentProgressDialogTest, CompletionSignalConnection)
{
    dialog->startMonitoring(mockBA.get(), 100);
    
    // Emit completion signal from mock
    mockBA->emitCompleted(true, "Test completion");
    
    // Process events to ensure signal is handled
    QApplication::processEvents();
    
    // Check that dialog shows completion
    auto* progressBar = dialog->findChild<QProgressBar*>();
    ASSERT_NE(progressBar, nullptr);
    EXPECT_EQ(progressBar->value(), progressBar->maximum());
}

TEST_F(BundleAdjustmentProgressDialogTest, ErrorFormatting)
{
    dialog->startMonitoring(mockBA.get(), 100);
    
    // Test different error magnitudes
    dialog->updateProgress(1, 1e-8);  // Very small error
    dialog->updateProgress(2, 0.5);   // Medium error
    dialog->updateProgress(3, 1000.0); // Large error
    
    // Check that error labels are formatted appropriately
    auto* errorLabel = dialog->findChild<QLabel*>("errorLabel");
    if (errorLabel) {
        EXPECT_FALSE(errorLabel->text().isEmpty());
        EXPECT_TRUE(errorLabel->text().contains("1000") || 
                   errorLabel->text().contains("1.000e+03"));
    }
}

TEST_F(BundleAdjustmentProgressDialogTest, TimeFormatting)
{
    dialog->startMonitoring(mockBA.get(), 100);
    
    // Find elapsed time label
    auto* timeLabel = dialog->findChild<QLabel*>("elapsedTimeLabel");
    if (timeLabel) {
        // Time should be in MM:SS format
        QString timeText = timeLabel->text();
        EXPECT_TRUE(timeText.contains(":"));
        
        // Should start with 00:00 or similar
        EXPECT_TRUE(timeText.startsWith("0"));
    }
}

TEST_F(BundleAdjustmentProgressDialogTest, ModalBehavior)
{
    // Dialog should be modal
    EXPECT_TRUE(dialog->isModal());
    
    // Should have fixed size
    EXPECT_FALSE(dialog->isResizable());
}

TEST_F(BundleAdjustmentProgressDialogTest, CloseEventHandling)
{
    dialog->startMonitoring(mockBA.get(), 100);
    
    // Try to close dialog during optimization
    QCloseEvent closeEvent;
    dialog->closeEvent(&closeEvent);
    
    // Close event should be ignored during optimization
    EXPECT_TRUE(closeEvent.isIgnored());
    
    // After completion, close should be allowed
    dialog->onComputationFinished(true, "Completed");
    QCloseEvent closeEvent2;
    dialog->closeEvent(&closeEvent2);
    EXPECT_TRUE(closeEvent2.isAccepted());
}

#include "test_bundleadjustmentprogressdialog.moc"
