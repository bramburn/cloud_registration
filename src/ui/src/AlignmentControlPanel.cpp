#include "ui/AlignmentControlPanel.h"

#include <QApplication>
#include <QMessageBox>
#include <QStyle>

AlignmentControlPanel::AlignmentControlPanel(QWidget* parent) : QWidget(parent)
{
    setupUI();
    updateUIState(AlignmentEngine::AlignmentState::Idle);
}

void AlignmentControlPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);

    // Add groups
    m_mainLayout->addWidget(createCorrespondenceGroup());
    m_mainLayout->addWidget(createQualityGroup());
    m_mainLayout->addWidget(createControlsGroup());
    m_mainLayout->addWidget(createConfigurationGroup());

    // Status label
    m_statusLabel = new QLabel("Ready for alignment");
    m_statusLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");
    m_mainLayout->addWidget(m_statusLabel);

    // Details text (initially hidden)
    m_detailsText = new QTextEdit();
    m_detailsText->setMaximumHeight(100);
    m_detailsText->setVisible(false);
    m_mainLayout->addWidget(m_detailsText);

    m_mainLayout->addStretch();
}

QGroupBox* AlignmentControlPanel::createCorrespondenceGroup()
{
    QGroupBox* group = new QGroupBox("Correspondence Status");
    QGridLayout* layout = new QGridLayout(group);

    // Correspondence count
    layout->addWidget(new QLabel("Count:"), 0, 0);
    m_correspondenceCountLabel = new QLabel("0");
    m_correspondenceCountLabel->setStyleSheet("QLabel { font-weight: bold; }");
    layout->addWidget(m_correspondenceCountLabel, 0, 1);

    // Status
    layout->addWidget(new QLabel("Status:"), 1, 0);
    m_correspondenceStatusLabel = new QLabel("No correspondences");
    layout->addWidget(m_correspondenceStatusLabel, 1, 1);

    return group;
}

QGroupBox* AlignmentControlPanel::createQualityGroup()
{
    QGroupBox* group = new QGroupBox("Quality Metrics");
    QGridLayout* layout = new QGridLayout(group);

    // RMS Error
    layout->addWidget(new QLabel("RMS Error:"), 0, 0);
    m_rmsErrorLabel = new QLabel("- mm");
    m_rmsErrorLabel->setStyleSheet("QLabel { font-weight: bold; }");
    layout->addWidget(m_rmsErrorLabel, 0, 1);

    // Quality Level
    layout->addWidget(new QLabel("Quality:"), 1, 0);
    m_qualityLevelLabel = new QLabel("-");
    layout->addWidget(m_qualityLevelLabel, 1, 1);

    // Max Error
    layout->addWidget(new QLabel("Max Error:"), 2, 0);
    m_maxErrorLabel = new QLabel("- mm");
    layout->addWidget(m_maxErrorLabel, 2, 1);

    // Mean Error
    layout->addWidget(new QLabel("Mean Error:"), 3, 0);
    m_meanErrorLabel = new QLabel("- mm");
    layout->addWidget(m_meanErrorLabel, 3, 1);

    // Computation Time
    layout->addWidget(new QLabel("Compute Time:"), 4, 0);
    m_computationTimeLabel = new QLabel("- ms");
    layout->addWidget(m_computationTimeLabel, 4, 1);

    return group;
}

QGroupBox* AlignmentControlPanel::createControlsGroup()
{
    QGroupBox* group = new QGroupBox("Alignment Controls");
    QVBoxLayout* layout = new QVBoxLayout(group);

    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // Align button
    m_alignButton = new QPushButton("Compute Alignment");
    m_alignButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(m_alignButton, &QPushButton::clicked, this, &AlignmentControlPanel::onAlignmentButtonClicked);
    buttonLayout->addWidget(m_alignButton);

    // Clear button
    m_clearButton = new QPushButton("Clear All");
    m_clearButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    connect(m_clearButton, &QPushButton::clicked, this, &AlignmentControlPanel::onClearCorrespondencesClicked);
    buttonLayout->addWidget(m_clearButton);

    layout->addLayout(buttonLayout);

    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    layout->addWidget(m_progressBar);

    // Finalization buttons
    QHBoxLayout* finalizationLayout = new QHBoxLayout();

    m_acceptButton = new QPushButton("Accept Alignment");
    m_acceptButton->setEnabled(false);
    m_acceptButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    connect(m_acceptButton, &QPushButton::clicked, this, &AlignmentControlPanel::onAcceptAlignmentClicked);
    finalizationLayout->addWidget(m_acceptButton);

    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setEnabled(false);
    m_cancelButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; }");
    connect(m_cancelButton, &QPushButton::clicked, this, &AlignmentControlPanel::onCancelAlignmentClicked);
    finalizationLayout->addWidget(m_cancelButton);

    layout->addLayout(finalizationLayout);

    // Report button
    m_reportButton = new QPushButton("Show Detailed Report");
    m_reportButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    connect(m_reportButton, &QPushButton::clicked, this, &AlignmentControlPanel::onShowDetailedReport);
    layout->addWidget(m_reportButton);

    return group;
}

QGroupBox* AlignmentControlPanel::createConfigurationGroup()
{
    QGroupBox* group = new QGroupBox("Configuration");
    QGridLayout* layout = new QGridLayout(group);

    // RMS Threshold
    layout->addWidget(new QLabel("RMS Threshold:"), 0, 0);
    m_rmsThresholdSpin = new QDoubleSpinBox();
    m_rmsThresholdSpin->setRange(0.1, 100.0);
    m_rmsThresholdSpin->setValue(DEFAULT_RMS_THRESHOLD);
    m_rmsThresholdSpin->setSuffix(" mm");
    m_rmsThresholdSpin->setDecimals(1);
    connect(m_rmsThresholdSpin,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &AlignmentControlPanel::onQualityThresholdsChanged);
    layout->addWidget(m_rmsThresholdSpin, 0, 1);

    // Max Error Threshold
    layout->addWidget(new QLabel("Max Error Threshold:"), 1, 0);
    m_maxErrorThresholdSpin = new QDoubleSpinBox();
    m_maxErrorThresholdSpin->setRange(0.1, 200.0);
    m_maxErrorThresholdSpin->setValue(DEFAULT_MAX_ERROR_THRESHOLD);
    m_maxErrorThresholdSpin->setSuffix(" mm");
    m_maxErrorThresholdSpin->setDecimals(1);
    connect(m_maxErrorThresholdSpin,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &AlignmentControlPanel::onQualityThresholdsChanged);
    layout->addWidget(m_maxErrorThresholdSpin, 1, 1);

    // Auto-recompute
    m_autoRecomputeCheck = new QCheckBox("Auto-recompute on changes");
    m_autoRecomputeCheck->setChecked(true);
    connect(m_autoRecomputeCheck, &QCheckBox::toggled, this, &AlignmentControlPanel::onAutoRecomputeChanged);
    layout->addWidget(m_autoRecomputeCheck, 2, 0, 1, 2);

    return group;
}

void AlignmentControlPanel::setAlignmentEngine(AlignmentEngine* engine)
{
    // Disconnect from previous engine
    if (m_alignmentEngine)
    {
        disconnect(m_alignmentEngine, nullptr, this, nullptr);
        disconnect(this, nullptr, m_alignmentEngine, nullptr);
    }

    m_alignmentEngine = engine;

    if (m_alignmentEngine)
    {
        // Connect signals for real-time updates
        connect(
            m_alignmentEngine, &AlignmentEngine::qualityMetricsUpdated, this, &AlignmentControlPanel::updateRMSError);
        connect(m_alignmentEngine,
                &AlignmentEngine::alignmentResultUpdated,
                this,
                &AlignmentControlPanel::updateAlignmentResult);
        connect(m_alignmentEngine,
                &AlignmentEngine::alignmentStateChanged,
                this,
                &AlignmentControlPanel::updateAlignmentState);
        connect(m_alignmentEngine,
                &AlignmentEngine::correspondencesChanged,
                this,
                &AlignmentControlPanel::updateCorrespondenceCount);

        // Connect control signals
        connect(this,
                &AlignmentControlPanel::qualityThresholdsChanged,
                m_alignmentEngine,
                &AlignmentEngine::setQualityThresholds);
        connect(
            this, &AlignmentControlPanel::autoRecomputeChanged, m_alignmentEngine, &AlignmentEngine::setAutoRecompute);

        // Initialize UI with current state
        updateCorrespondenceCount(m_alignmentEngine->getCorrespondences().size());
        updateAlignmentResult(m_alignmentEngine->getCurrentResult());

        // Set initial configuration
        m_autoRecomputeCheck->setChecked(m_alignmentEngine->isAutoRecompute());
    }
}

void AlignmentControlPanel::updateRMSError(float error)
{
    m_rmsErrorLabel->setText(formatError(error));
    m_qualityLevelLabel->setText(getQualityLevel(error));

    // Update quality level color
    QString qualityLevel = getQualityLevel(error);
    QString color = "#666";  // Default gray
    if (qualityLevel == "Excellent")
        color = "#2E7D32";  // Green
    else if (qualityLevel == "Good")
        color = "#388E3C";  // Light green
    else if (qualityLevel == "Acceptable")
        color = "#F57C00";  // Orange
    else if (qualityLevel == "Poor")
        color = "#D32F2F";  // Red

    m_qualityLevelLabel->setStyleSheet(QString("QLabel { color: %1; font-weight: bold; }").arg(color));
}

void AlignmentControlPanel::updateAlignmentResult(const AlignmentEngine::AlignmentResult& result)
{
    m_lastResult = result;

    // Update quality metrics
    updateRMSError(result.errorStats.rmsError);
    m_maxErrorLabel->setText(formatError(result.errorStats.maxError));
    m_meanErrorLabel->setText(formatError(result.errorStats.meanError));
    m_computationTimeLabel->setText(QString("%1 ms").arg(result.computationTimeMs));

    // Update UI state
    updateUIState(result.state);

    // Enable/disable report button
    m_reportButton->setEnabled(result.state == AlignmentEngine::AlignmentState::Valid);
}

void AlignmentControlPanel::updateAlignmentState(AlignmentEngine::AlignmentState state, const QString& message)
{
    m_statusLabel->setText(message);
    updateUIState(state);

    // Show/hide progress bar
    m_progressBar->setVisible(state == AlignmentEngine::AlignmentState::Computing);
    if (state == AlignmentEngine::AlignmentState::Computing)
    {
        m_progressBar->setRange(0, 0);  // Indeterminate progress
    }
}

void AlignmentControlPanel::updateCorrespondenceCount(int count)
{
    m_correspondenceCountLabel->setText(QString::number(count));

    QString status;
    if (count == 0)
    {
        status = "No correspondences";
    }
    else if (count < 3)
    {
        status = "Insufficient (need ≥3)";
    }
    else
    {
        status = "Ready for alignment";
    }
    m_correspondenceStatusLabel->setText(status);

    // Update clear button state
    m_clearButton->setEnabled(count > 0);
}

void AlignmentControlPanel::onAlignmentButtonClicked()
{
    // Emit signal to request alignment computation
    // The MainPresenter will handle the actual computation logic
    emit alignmentRequested();
}

void AlignmentControlPanel::onClearCorrespondencesClicked()
{
    if (m_alignmentEngine)
    {
        int ret = QMessageBox::question(this,
                                        "Clear Correspondences",
                                        "Are you sure you want to clear all correspondences?",
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::No);
        if (ret == QMessageBox::Yes)
        {
            emit clearCorrespondencesRequested();
            m_alignmentEngine->clearCorrespondences();
        }
    }
}

void AlignmentControlPanel::onQualityThresholdsChanged()
{
    float rmsThreshold = static_cast<float>(m_rmsThresholdSpin->value());
    float maxErrorThreshold = static_cast<float>(m_maxErrorThresholdSpin->value());
    emit qualityThresholdsChanged(rmsThreshold, maxErrorThreshold);
}

void AlignmentControlPanel::onAutoRecomputeChanged()
{
    emit autoRecomputeChanged(m_autoRecomputeCheck->isChecked());
}

void AlignmentControlPanel::onShowDetailedReport()
{
    if (m_lastResult.state == AlignmentEngine::AlignmentState::Valid)
    {
        QString report = m_lastResult.errorStats.generateReport();

        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Detailed Alignment Report");
        msgBox.setText(report);
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
    }
}

void AlignmentControlPanel::onAcceptAlignmentClicked()
{
    emit acceptAlignmentRequested();
}

void AlignmentControlPanel::onCancelAlignmentClicked()
{
    emit cancelAlignmentRequested();
}

void AlignmentControlPanel::updateUIState(AlignmentEngine::AlignmentState state)
{
    bool canAlign =
        (state == AlignmentEngine::AlignmentState::Idle || state == AlignmentEngine::AlignmentState::Valid ||
         state == AlignmentEngine::AlignmentState::Insufficient);

    m_alignButton->setEnabled(canAlign && m_alignmentEngine && m_alignmentEngine->getCorrespondences().size() >= 3);

    // Update finalization button states
    // Accept button: enabled only when alignment is valid
    m_acceptButton->setEnabled(state == AlignmentEngine::AlignmentState::Valid);

    // Cancel button: enabled when manual alignment mode is active (not Idle)
    bool isAlignmentActive = (state != AlignmentEngine::AlignmentState::Idle);
    m_cancelButton->setEnabled(isAlignmentActive);

    // Update button text based on state
    switch (state)
    {
        case AlignmentEngine::AlignmentState::Computing:
            m_alignButton->setText("Computing...");
            break;
        case AlignmentEngine::AlignmentState::Valid:
            m_alignButton->setText("Recompute Alignment");
            break;
        default:
            m_alignButton->setText("Compute Alignment");
            break;
    }
}

QString AlignmentControlPanel::formatError(float error) const
{
    if (error <= 0.0f)
    {
        return "- mm";
    }
    return QString("%1 mm").arg(error, 0, 'f', 3);
}

QString AlignmentControlPanel::getQualityLevel(float rmsError) const
{
    if (rmsError <= 0.0f)
        return "-";
    if (rmsError <= 1.0f)
        return "Excellent";
    if (rmsError <= 3.0f)
        return "Good";
    if (rmsError <= 5.0f)
        return "Acceptable";
    return "Poor";
}
