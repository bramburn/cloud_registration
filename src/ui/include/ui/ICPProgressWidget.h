#ifndef ICPPROGRESSWIDGET_H
#define ICPPROGRESSWIDGET_H

#include <QHBoxLayout>
#include <QLabel>
#include <QMatrix4x4>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

class ICPRegistration;

/**
 * @brief Progress monitoring widget for ICP computation
 *
 * Provides real-time feedback during ICP algorithm execution including:
 * - Iteration progress
 * - Current RMS error
 * - Convergence visualization
 * - Cancel functionality
 */
class ICPProgressWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ICPProgressWidget(QWidget* parent = nullptr);
    ~ICPProgressWidget();

    /**
     * @brief Start monitoring ICP progress
     * @param icpAlgorithm Pointer to ICP algorithm instance
     * @param maxIterations Maximum number of iterations expected
     */
    void startMonitoring(ICPRegistration* icpAlgorithm, int maxIterations);

    /**
     * @brief Stop monitoring and hide the widget
     */
    void stopMonitoring();

    /**
     * @brief Check if monitoring is currently active
     */
    bool isMonitoring() const
    {
        return m_isMonitoring;
    }

public slots:
    /**
     * @brief Update progress display
     * @param iteration Current iteration number
     * @param rmsError Current RMS error
     * @param transformation Current transformation estimate
     */
    void updateProgress(int iteration, float rmsError, const QMatrix4x4& transformation);

    /**
     * @brief Handle ICP computation completion
     * @param success True if ICP converged successfully
     * @param finalTransformation Final transformation matrix
     * @param finalRMSError Final RMS error
     * @param iterations Total number of iterations performed
     */
    void
    onComputationFinished(bool success, const QMatrix4x4& finalTransformation, float finalRMSError, int iterations);

signals:
    /**
     * @brief Emitted when user requests to cancel ICP computation
     */
    void cancelRequested();

    /**
     * @brief Emitted when ICP computation is completed (success or failure)
     * @param success True if computation was successful
     * @param message Status message
     */
    void computationCompleted(bool success, const QString& message);

private slots:
    void onCancelClicked();
    void updateElapsedTime();

private:
    void setupUI();
    void resetDisplay();
    QString formatTime(int seconds) const;
    QString formatError(float error) const;

private:
    // UI Components
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;

    QLabel* m_titleLabel;
    QLabel* m_statusLabel;
    QLabel* m_iterationLabel;
    QLabel* m_errorLabel;
    QLabel* m_timeLabel;

    QProgressBar* m_progressBar;
    QPushButton* m_cancelButton;
    QPushButton* m_closeButton;

    // State
    bool m_isMonitoring;
    int m_maxIterations;
    int m_currentIteration;
    float m_currentError;
    float m_initialError;

    // Timing
    QTimer* m_elapsedTimer;
    int m_elapsedSeconds;

    // Connected ICP algorithm
    ICPRegistration* m_icpAlgorithm;

    // Error history for convergence visualization
    QList<float> m_errorHistory;
    static const int MAX_ERROR_HISTORY = 50;
};

#endif  // ICPPROGRESSWIDGET_H
