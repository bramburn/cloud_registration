#include "ICPProgressWidget.h"

#include <QApplication>
#include <QStyle>

#include "../algorithms/ICPRegistration.h"

ICPProgressWidget::ICPProgressWidget(QWidget* parent)
    : QWidget(parent),
      m_mainLayout(nullptr),
      m_buttonLayout(nullptr),
      m_titleLabel(nullptr),
      m_statusLabel(nullptr),
      m_iterationLabel(nullptr),
      m_errorLabel(nullptr),
      m_timeLabel(nullptr),
      m_progressBar(nullptr),
      m_cancelButton(nullptr),
      m_closeButton(nullptr),
      m_isMonitoring(false),
      m_maxIterations(50),
      m_currentIteration(0),
      m_currentError(0.0f),
      m_initialError(0.0f),
      m_elapsedTimer(new QTimer(this)),
      m_elapsedSeconds(0),
      m_icpAlgorithm(nullptr)
{
    setupUI();

    // Setup elapsed time timer
    connect(m_elapsedTimer, &QTimer::timeout, this, &ICPProgressWidget::updateElapsedTime);

    // Initially hidden
    hide();
}

ICPProgressWidget::~ICPProgressWidget()
{
    stopMonitoring();
}

void ICPProgressWidget::setupUI()
{
    setWindowTitle("ICP Registration Progress");
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setModal(true);
    setFixedSize(400, 200);

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    m_titleLabel = new QLabel("ICP Registration in Progress");
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_titleLabel);

    // Status
    m_statusLabel = new QLabel("Initializing...");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_statusLabel);

    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_mainLayout->addWidget(m_progressBar);

    // Iteration info
    m_iterationLabel = new QLabel("Iteration: 0 / 50");
    m_mainLayout->addWidget(m_iterationLabel);

    // Error info
    m_errorLabel = new QLabel("RMS Error: --");
    m_mainLayout->addWidget(m_errorLabel);

    // Time info
    m_timeLabel = new QLabel("Elapsed Time: 00:00");
    m_mainLayout->addWidget(m_timeLabel);

    // Buttons
    m_buttonLayout = new QHBoxLayout();

    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setIcon(style()->standardIcon(QStyle::SP_DialogCancelButton));
    connect(m_cancelButton, &QPushButton::clicked, this, &ICPProgressWidget::onCancelClicked);

    m_closeButton = new QPushButton("Close");
    m_closeButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    m_closeButton->setVisible(false);
    connect(m_closeButton, &QPushButton::clicked, this, &QWidget::hide);

    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_cancelButton);
    m_buttonLayout->addWidget(m_closeButton);

    m_mainLayout->addLayout(m_buttonLayout);
}

void ICPProgressWidget::startMonitoring(ICPRegistration* icpAlgorithm, int maxIterations)
{
    if (!icpAlgorithm)
    {
        qWarning() << "ICPProgressWidget: Cannot monitor null ICP algorithm";
        return;
    }

    m_icpAlgorithm = icpAlgorithm;
    m_maxIterations = maxIterations;
    m_isMonitoring = true;

    // Connect to ICP signals
    connect(m_icpAlgorithm, &ICPRegistration::progressUpdated, this, &ICPProgressWidget::updateProgress);
    connect(m_icpAlgorithm, &ICPRegistration::computationFinished, this, &ICPProgressWidget::onComputationFinished);

    // Connect cancel signal
    connect(this, &ICPProgressWidget::cancelRequested, m_icpAlgorithm, &ICPRegistration::cancel);

    // Reset display
    resetDisplay();

    // Start timing
    m_elapsedSeconds = 0;
    m_elapsedTimer->start(1000);  // Update every second

    // Show the widget
    show();
    raise();
    activateWindow();

    qDebug() << "ICPProgressWidget: Started monitoring ICP with max iterations:" << maxIterations;
}

void ICPProgressWidget::stopMonitoring()
{
    if (!m_isMonitoring)
        return;

    m_isMonitoring = false;
    m_elapsedTimer->stop();

    // Disconnect from ICP algorithm
    if (m_icpAlgorithm)
    {
        disconnect(m_icpAlgorithm, nullptr, this, nullptr);
        disconnect(this, nullptr, m_icpAlgorithm, nullptr);
        m_icpAlgorithm = nullptr;
    }

    qDebug() << "ICPProgressWidget: Stopped monitoring";
}

void ICPProgressWidget::updateProgress(int iteration, float rmsError, const QMatrix4x4& /*transformation*/)
{
    if (!m_isMonitoring)
        return;

    m_currentIteration = iteration;
    m_currentError = rmsError;

    // Store initial error for comparison
    if (iteration == 0)
    {
        m_initialError = rmsError;
    }

    // Update error history
    m_errorHistory.append(rmsError);
    if (m_errorHistory.size() > MAX_ERROR_HISTORY)
    {
        m_errorHistory.removeFirst();
    }

    // Update UI
    int progressPercent = (m_maxIterations > 0) ? (iteration * 100) / m_maxIterations : 0;
    m_progressBar->setValue(progressPercent);

    m_iterationLabel->setText(QString("Iteration: %1 / %2").arg(iteration).arg(m_maxIterations));
    m_errorLabel->setText(QString("RMS Error: %1").arg(formatError(rmsError)));

    // Update status based on convergence
    if (iteration > 0 && m_errorHistory.size() > 1)
    {
        float errorChange = m_errorHistory.last() - m_errorHistory[m_errorHistory.size() - 2];
        if (errorChange < 0)
        {
            m_statusLabel->setText("Converging (error decreasing)");
        }
        else if (errorChange > 0)
        {
            m_statusLabel->setText("Error increasing");
        }
        else
        {
            m_statusLabel->setText("Error stable");
        }
    }
    else
    {
        m_statusLabel->setText("Computing correspondences...");
    }

    // Force UI update
    QApplication::processEvents();
}

void ICPProgressWidget::onComputationFinished(bool success,
                                              const QMatrix4x4& /*finalTransformation*/,
                                              float finalRMSError,
                                              int iterations)
{
    m_elapsedTimer->stop();

    // Update final status
    QString message;
    if (success)
    {
        float improvement = (m_initialError > 0) ? ((m_initialError - finalRMSError) / m_initialError) * 100.0f : 0.0f;

        message = QString("ICP completed successfully!\n"
                          "Final RMS Error: %1\n"
                          "Iterations: %2\n"
                          "Improvement: %3%")
                      .arg(formatError(finalRMSError))
                      .arg(iterations)
                      .arg(improvement, 0, 'f', 1);

        m_statusLabel->setText("Completed successfully");
        m_progressBar->setValue(100);
    }
    else
    {
        message = QString("ICP computation was cancelled or failed.\n"
                          "Last RMS Error: %1\n"
                          "Iterations completed: %2")
                      .arg(formatError(finalRMSError))
                      .arg(iterations);

        m_statusLabel->setText("Computation cancelled or failed");
    }

    // Update final iteration display
    m_iterationLabel->setText(QString("Final Iteration: %1").arg(iterations));
    m_errorLabel->setText(QString("Final RMS Error: %1").arg(formatError(finalRMSError)));

    // Switch buttons
    m_cancelButton->setVisible(false);
    m_closeButton->setVisible(true);

    // Stop monitoring
    stopMonitoring();

    // Emit completion signal
    emit computationCompleted(success, message);

    qDebug() << "ICPProgressWidget: Computation finished. Success:" << success;
}

void ICPProgressWidget::onCancelClicked()
{
    if (m_isMonitoring)
    {
        m_statusLabel->setText("Cancelling...");
        m_cancelButton->setEnabled(false);
        emit cancelRequested();
    }
}

void ICPProgressWidget::updateElapsedTime()
{
    m_elapsedSeconds++;
    m_timeLabel->setText(QString("Elapsed Time: %1").arg(formatTime(m_elapsedSeconds)));
}

void ICPProgressWidget::resetDisplay()
{
    m_currentIteration = 0;
    m_currentError = 0.0f;
    m_initialError = 0.0f;
    m_errorHistory.clear();

    m_statusLabel->setText("Initializing...");
    m_iterationLabel->setText(QString("Iteration: 0 / %1").arg(m_maxIterations));
    m_errorLabel->setText("RMS Error: --");
    m_timeLabel->setText("Elapsed Time: 00:00");

    m_progressBar->setValue(0);

    m_cancelButton->setVisible(true);
    m_cancelButton->setEnabled(true);
    m_closeButton->setVisible(false);
}

QString ICPProgressWidget::formatTime(int seconds) const
{
    int minutes = seconds / 60;
    int secs = seconds % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
}

QString ICPProgressWidget::formatError(float error) const
{
    if (error < 0.001f)
    {
        return QString("%1 μm").arg(error * 1000000.0f, 0, 'f', 1);
    }
    else if (error < 1.0f)
    {
        return QString("%1 mm").arg(error * 1000.0f, 0, 'f', 2);
    }
    else
    {
        return QString("%1 m").arg(error, 0, 'f', 3);
    }
}
