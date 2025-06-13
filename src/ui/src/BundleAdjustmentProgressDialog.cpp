#include "ui/BundleAdjustmentProgressDialog.h"
#include "optimization/BundleAdjustment.h"

#include <QCloseEvent>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QApplication>

BundleAdjustmentProgressDialog::BundleAdjustmentProgressDialog(QWidget* parent)
    : QDialog(parent),
      m_mainLayout(nullptr),
      m_buttonLayout(nullptr),
      m_progressBar(nullptr),
      m_iterationLabel(nullptr),
      m_errorLabel(nullptr),
      m_initialErrorLabel(nullptr),
      m_elapsedTimeLabel(nullptr),
      m_statusLabel(nullptr),
      m_cancelButton(nullptr),
      m_closeButton(nullptr),
      m_updateTimer(new QTimer(this)),
      m_maxIterations(100),
      m_initialError(0.0),
      m_isOptimizationRunning(false),
      m_optimizationCompleted(false)
{
    setupUI();
    setupConnections();
    
    // Configure dialog
    setWindowTitle("Bundle Adjustment Progress");
    setModal(true);
    setFixedSize(400, 250);
    
    // Start elapsed time updates
    m_updateTimer->setInterval(UPDATE_INTERVAL_MS);
    connect(m_updateTimer, &QTimer::timeout, this, &BundleAdjustmentProgressDialog::updateElapsedTime);
}

void BundleAdjustmentProgressDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    createProgressSection();
    createButtonSection();
    
    setLayout(m_mainLayout);
}

void BundleAdjustmentProgressDialog::createProgressSection()
{
    // Create progress group
    QGroupBox* progressGroup = new QGroupBox("Optimization Progress", this);
    QGridLayout* progressLayout = new QGridLayout(progressGroup);
    
    // Progress bar
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    progressLayout->addWidget(m_progressBar, 0, 0, 1, 2);
    
    // Status labels
    progressLayout->addWidget(new QLabel("Iteration:", this), 1, 0);
    m_iterationLabel = new QLabel("0 / 0", this);
    progressLayout->addWidget(m_iterationLabel, 1, 1);
    
    progressLayout->addWidget(new QLabel("Current Error:", this), 2, 0);
    m_errorLabel = new QLabel("N/A", this);
    progressLayout->addWidget(m_errorLabel, 2, 1);
    
    progressLayout->addWidget(new QLabel("Initial Error:", this), 3, 0);
    m_initialErrorLabel = new QLabel("N/A", this);
    progressLayout->addWidget(m_initialErrorLabel, 3, 1);
    
    progressLayout->addWidget(new QLabel("Elapsed Time:", this), 4, 0);
    m_elapsedTimeLabel = new QLabel("00:00", this);
    progressLayout->addWidget(m_elapsedTimeLabel, 4, 1);
    
    // Status message
    m_statusLabel = new QLabel("Initializing...", this);
    m_statusLabel->setWordWrap(true);
    progressLayout->addWidget(m_statusLabel, 5, 0, 1, 2);
    
    m_mainLayout->addWidget(progressGroup);
}

void BundleAdjustmentProgressDialog::createButtonSection()
{
    m_buttonLayout = new QHBoxLayout();
    
    // Cancel button
    m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setEnabled(true);
    m_buttonLayout->addWidget(m_cancelButton);
    
    // Close button (initially hidden)
    m_closeButton = new QPushButton("Close", this);
    m_closeButton->setVisible(false);
    m_buttonLayout->addWidget(m_closeButton);
    
    m_buttonLayout->addStretch();
    m_mainLayout->addLayout(m_buttonLayout);
}

void BundleAdjustmentProgressDialog::setupConnections()
{
    connect(m_cancelButton, &QPushButton::clicked, this, &BundleAdjustmentProgressDialog::onCancelClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
}

void BundleAdjustmentProgressDialog::startMonitoring(Optimization::BundleAdjustment* baAlgorithm, int maxIterations)
{
    if (!baAlgorithm) {
        return;
    }
    
    m_maxIterations = maxIterations;
    m_isOptimizationRunning = true;
    m_optimizationCompleted = false;
    m_initialError = 0.0;
    
    // Reset UI
    m_progressBar->setRange(0, maxIterations);
    m_progressBar->setValue(0);
    m_iterationLabel->setText(QString("0 / %1").arg(maxIterations));
    m_errorLabel->setText("N/A");
    m_initialErrorLabel->setText("N/A");
    m_elapsedTimeLabel->setText("00:00");
    m_statusLabel->setText("Starting optimization...");
    
    // Show cancel button, hide close button
    m_cancelButton->setVisible(true);
    m_cancelButton->setEnabled(true);
    m_closeButton->setVisible(false);
    
    // Start timers
    m_elapsedTimer.start();
    m_updateTimer->start();
    
    // Connect to Bundle Adjustment signals
    connect(baAlgorithm, &Optimization::BundleAdjustment::optimizationProgress,
            this, &BundleAdjustmentProgressDialog::updateProgress);
    connect(baAlgorithm, &Optimization::BundleAdjustment::optimizationCompleted,
            this, [this](const Optimization::BundleAdjustment::Result& result) {
                onComputationFinished(result.converged, result.statusMessage);
            });
}

void BundleAdjustmentProgressDialog::updateProgress(int iteration, double currentError)
{
    if (!m_isOptimizationRunning) {
        return;
    }
    
    // Store initial error on first iteration
    if (iteration == 0 && m_initialError == 0.0) {
        m_initialError = currentError;
        m_initialErrorLabel->setText(formatError(m_initialError));
    }
    
    // Update progress bar
    m_progressBar->setValue(iteration);
    
    // Update labels
    m_iterationLabel->setText(QString("%1 / %2").arg(iteration).arg(m_maxIterations));
    m_errorLabel->setText(formatError(currentError));
    
    // Update status
    if (m_initialError > 0.0) {
        double improvement = ((m_initialError - currentError) / m_initialError) * 100.0;
        m_statusLabel->setText(QString("Optimizing... (%.1f%% improvement)").arg(improvement));
    } else {
        m_statusLabel->setText("Optimizing...");
    }
    
    // Force UI update
    QApplication::processEvents();
}

void BundleAdjustmentProgressDialog::onComputationFinished(bool success, const QString& statusMessage)
{
    m_isOptimizationRunning = false;
    m_optimizationCompleted = true;
    
    // Stop timers
    m_updateTimer->stop();
    
    // Update progress bar to completion
    m_progressBar->setValue(m_maxIterations);
    
    // Update status
    if (success) {
        m_statusLabel->setText(QString("Completed Successfully! %1").arg(statusMessage));
    } else {
        m_statusLabel->setText(QString("Optimization Failed: %1").arg(statusMessage));
    }
    
    // Switch buttons
    m_cancelButton->setVisible(false);
    m_closeButton->setVisible(true);
    m_closeButton->setDefault(true);
    m_closeButton->setFocus();
}

void BundleAdjustmentProgressDialog::onCancelClicked()
{
    if (m_isOptimizationRunning) {
        int ret = QMessageBox::question(this, "Cancel Optimization",
                                       "Are you sure you want to cancel the Bundle Adjustment optimization?",
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            m_statusLabel->setText("Cancelling...");
            m_cancelButton->setEnabled(false);
            emit cancelRequested();
        }
    }
}

void BundleAdjustmentProgressDialog::updateElapsedTime()
{
    if (m_elapsedTimer.isValid()) {
        qint64 elapsed = m_elapsedTimer.elapsed();
        m_elapsedTimeLabel->setText(formatTime(elapsed));
    }
}

void BundleAdjustmentProgressDialog::closeEvent(QCloseEvent* event)
{
    if (m_isOptimizationRunning && !m_optimizationCompleted) {
        // Prevent closing during optimization
        event->ignore();
        
        QMessageBox::information(this, "Optimization Running",
                                "Please wait for the optimization to complete or click Cancel to stop it.");
    } else {
        event->accept();
    }
}

QString BundleAdjustmentProgressDialog::formatTime(qint64 milliseconds) const
{
    int seconds = static_cast<int>(milliseconds / 1000);
    int minutes = seconds / 60;
    seconds = seconds % 60;
    
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
}

QString BundleAdjustmentProgressDialog::formatError(double error) const
{
    if (error < 1e-6) {
        return QString::number(error, 'e', 2);
    } else if (error < 1.0) {
        return QString::number(error, 'f', 6);
    } else {
        return QString::number(error, 'f', 3);
    }
}
