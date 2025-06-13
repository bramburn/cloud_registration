#pragma once

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QElapsedTimer>
#include <QTimer>

// Forward declarations
namespace Optimization {
    class BundleAdjustment;
    struct BundleAdjustmentResult;
}

/**
 * @brief Modal dialog for displaying Bundle Adjustment progress
 * 
 * This dialog provides real-time feedback during Bundle Adjustment optimization,
 * including iteration count, current error, elapsed time, and a progress bar.
 * Users can cancel the optimization at any time.
 * 
 * Features:
 * - Real-time progress updates
 * - Iteration and error display
 * - Elapsed time counter
 * - Cancellation support
 * - Final status display
 */
class BundleAdjustmentProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BundleAdjustmentProgressDialog(QWidget* parent = nullptr);
    virtual ~BundleAdjustmentProgressDialog() = default;

    /**
     * @brief Start monitoring Bundle Adjustment progress
     * @param baAlgorithm Pointer to the Bundle Adjustment algorithm
     * @param maxIterations Maximum number of iterations expected
     */
    void startMonitoring(Optimization::BundleAdjustment* baAlgorithm, int maxIterations);

    /**
     * @brief Update progress display
     * @param iteration Current iteration number
     * @param currentError Current optimization error
     */
    void updateProgress(int iteration, double currentError);

    /**
     * @brief Handle optimization completion
     * @param success True if optimization completed successfully
     * @param result Bundle adjustment result information
     */
    void onComputationFinished(bool success, const QString& statusMessage);

signals:
    /**
     * @brief Emitted when user requests cancellation
     */
    void cancelRequested();

protected:
    /**
     * @brief Handle close event (prevent closing during optimization)
     * @param event Close event
     */
    void closeEvent(QCloseEvent* event) override;

private slots:
    /**
     * @brief Handle cancel button click
     */
    void onCancelClicked();

    /**
     * @brief Update elapsed time display
     */
    void updateElapsedTime();

private:
    // UI setup
    void setupUI();
    void createProgressSection();
    void createButtonSection();
    void setupConnections();

    // Utility methods
    QString formatTime(qint64 milliseconds) const;
    QString formatError(double error) const;

    // UI components
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;

    // Progress display
    QProgressBar* m_progressBar;
    QLabel* m_iterationLabel;
    QLabel* m_errorLabel;
    QLabel* m_initialErrorLabel;
    QLabel* m_elapsedTimeLabel;
    QLabel* m_statusLabel;

    // Controls
    QPushButton* m_cancelButton;
    QPushButton* m_closeButton;

    // State tracking
    QElapsedTimer m_elapsedTimer;
    QTimer* m_updateTimer;
    int m_maxIterations;
    double m_initialError;
    bool m_isOptimizationRunning;
    bool m_optimizationCompleted;

    // Constants
    static constexpr int UPDATE_INTERVAL_MS = 100;
};
