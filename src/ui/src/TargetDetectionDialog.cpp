#include "TargetDetectionDialog.h"

#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QStandardPaths>

TargetDetectionDialog::TargetDetectionDialog(TargetManager* targetManager, QWidget* parent)
    : QDialog(parent),
      m_targetManager(targetManager),
      m_sphereDetector(new SphereDetector(this)),
      m_naturalPointSelector(new NaturalPointSelector(this)),
      m_detectionRunning(false)
{
    setWindowTitle("Target Detection");
    setModal(true);
    resize(800, 600);

    setupUI();

    // Connect detection signals
    connect(
        m_sphereDetector, &TargetDetectionBase::detectionProgress, this, &TargetDetectionDialog::onDetectionProgress);
    connect(
        m_sphereDetector, &TargetDetectionBase::detectionCompleted, this, &TargetDetectionDialog::onDetectionCompleted);
    connect(m_sphereDetector, &TargetDetectionBase::detectionError, this, &TargetDetectionDialog::onDetectionError);

    connect(m_naturalPointSelector,
            &TargetDetectionBase::detectionProgress,
            this,
            &TargetDetectionDialog::onDetectionProgress);
    connect(m_naturalPointSelector,
            &TargetDetectionBase::detectionCompleted,
            this,
            &TargetDetectionDialog::onDetectionCompleted);
    connect(
        m_naturalPointSelector, &TargetDetectionBase::detectionError, this, &TargetDetectionDialog::onDetectionError);

    // Set default parameters
    resetToDefaults();
}

void TargetDetectionDialog::setPointCloudData(const QString& scanId, const std::vector<PointFullData>& points)
{
    m_currentScanId = scanId;
    m_currentPoints = points;

    m_statusLabel->setText(QString("Loaded %1 points from scan: %2").arg(points.size()).arg(scanId));

    // Clear previous results
    m_resultsTable->setRowCount(0);
    m_logTextEdit->clear();

    // Enable detection if we have data
    m_startButton->setEnabled(!points.empty() && !m_detectionRunning);
}

TargetDetectionBase::DetectionParams TargetDetectionDialog::getDetectionParameters() const
{
    return getParametersFromUI();
}

void TargetDetectionDialog::setDetectionParameters(const TargetDetectionBase::DetectionParams& params)
{
    setUIFromParameters(params);
}

TargetDetectionDialog::DetectionMode TargetDetectionDialog::getDetectionMode() const
{
    return static_cast<DetectionMode>(m_detectionModeCombo->currentIndex());
}

void TargetDetectionDialog::startDetection()
{
    if (m_currentPoints.empty())
    {
        QMessageBox::warning(this, "No Data", "Please load point cloud data first.");
        return;
    }

    if (!validateParameters())
    {
        QMessageBox::warning(this, "Invalid Parameters", "Please check your detection parameters.");
        return;
    }

    m_detectionRunning = true;
    m_startButton->setEnabled(false);
    m_cancelButton->setEnabled(true);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(true);
    m_statusLabel->setText("Starting detection...");

    // Clear previous results
    m_resultsTable->setRowCount(0);
    m_logTextEdit->append(QString("Starting detection on scan: %1").arg(m_currentScanId));

    TargetDetectionBase::DetectionParams params = getParametersFromUI();
    DetectionMode mode = getDetectionMode();

    // Emit signal to start detection through AlignmentEngine
    emit detectionStartRequested(m_currentScanId, static_cast<int>(mode), params.toVariantMap());

    switch (mode)
    {
        case AutomaticSpheres:
            m_logTextEdit->append("Starting automatic sphere detection...");
            break;

        case ManualNaturalPoints:
            m_logTextEdit->append("Manual natural point selection mode activated.");
            m_statusLabel->setText("Ready for manual point selection");
            m_detectionRunning = false;
            m_startButton->setEnabled(true);
            m_cancelButton->setEnabled(false);
            m_progressBar->setVisible(false);
            emit manualSelectionRequested(m_currentScanId);
            return; // Don't set detection running for manual mode

        case Both:
            m_logTextEdit->append("Starting automatic sphere detection first...");
            break;
    }
}

void TargetDetectionDialog::cancelDetection()
{
    if (m_detectionRunning)
    {
        m_logTextEdit->append("Cancelling detection...");
        emit cancelDetectionRequested();
    }

    m_detectionRunning = false;
    m_startButton->setEnabled(true);
    m_cancelButton->setEnabled(false);
    m_progressBar->setVisible(false);
    m_statusLabel->setText("Detection cancelled");
    m_logTextEdit->append("Detection cancelled by user.");
}

void TargetDetectionDialog::resetToDefaults()
{
    TargetDetectionBase::DetectionParams defaultParams;

    // Use sphere detector defaults for sphere-specific parameters
    if (m_sphereDetector)
    {
        defaultParams = m_sphereDetector->getDefaultParameters();
    }

    setUIFromParameters(defaultParams);
    m_detectionModeCombo->setCurrentIndex(0);  // AutomaticSpheres
    onDetectionModeChanged();
}

void TargetDetectionDialog::loadParameters()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Load Detection Parameters",
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    "JSON Files (*.json);;All Files (*)");

    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Error", "Cannot open file for reading.");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError)
    {
        QMessageBox::warning(this, "Error", "Invalid JSON file: " + error.errorString());
        return;
    }

    QVariantMap paramMap = doc.toVariant().toMap();
    TargetDetectionBase::DetectionParams params;
    params.fromVariantMap(paramMap);

    setUIFromParameters(params);

    m_logTextEdit->append(QString("Loaded parameters from: %1").arg(fileName));
}

void TargetDetectionDialog::saveParameters()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Save Detection Parameters",
                                                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                                    "JSON Files (*.json);;All Files (*)");

    if (fileName.isEmpty())
    {
        return;
    }

    TargetDetectionBase::DetectionParams params = getParametersFromUI();
    QVariantMap paramMap = params.toVariantMap();

    QJsonDocument doc = QJsonDocument::fromVariant(paramMap);
    QByteArray data = doc.toJson();

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, "Error", "Cannot open file for writing.");
        return;
    }

    file.write(data);
    file.close();

    m_logTextEdit->append(QString("Saved parameters to: %1").arg(fileName));
}

void TargetDetectionDialog::onDetectionProgress(int percentage, const QString& stage)
{
    m_progressBar->setValue(percentage);
    m_statusLabel->setText(stage);
    m_logTextEdit->append(QString("Progress: %1% - %2").arg(percentage).arg(stage));
    QApplication::processEvents();  // Keep UI responsive
}

void TargetDetectionDialog::onDetectionCompleted(const TargetDetectionBase::DetectionResult& result)
{
    m_detectionRunning = false;
    m_startButton->setEnabled(true);
    m_cancelButton->setEnabled(false);
    m_progressBar->setVisible(false);

    m_lastResult = result;

    if (result.success)
    {
        m_statusLabel->setText(QString("Detection completed: %1 targets found").arg(result.targets.size()));

        m_logTextEdit->append(QString("Detection completed successfully:"));
        m_logTextEdit->append(QString("- Found %1 targets").arg(result.targets.size()));
        m_logTextEdit->append(QString("- Processed %1 points").arg(result.processedPoints));
        m_logTextEdit->append(QString("- Processing time: %1 seconds").arg(result.processingTime, 0, 'f', 2));

        updateResultsTable(result);

        // Enable accept/reject buttons if we have results
        m_acceptButton->setEnabled(!result.targets.isEmpty());
        m_rejectButton->setEnabled(!result.targets.isEmpty());
    }
    else
    {
        m_statusLabel->setText("Detection failed");
        m_logTextEdit->append(QString("Detection failed: %1").arg(result.errorMessage));

        QMessageBox::warning(this, "Detection Failed", result.errorMessage);
    }
}

void TargetDetectionDialog::onDetectionError(const QString& error)
{
    m_detectionRunning = false;
    m_startButton->setEnabled(true);
    m_cancelButton->setEnabled(false);
    m_progressBar->setVisible(false);

    m_statusLabel->setText("Detection error");
    m_logTextEdit->append(QString("Detection error: %1").arg(error));

    QMessageBox::critical(this, "Detection Error", error);
}

void TargetDetectionDialog::onDetectionModeChanged()
{
    updateParameterControls();
}

void TargetDetectionDialog::onParametersChanged()
{
    // Validate parameters when they change
    bool valid = validateParameters();
    m_startButton->setEnabled(valid && !m_currentPoints.empty() && !m_detectionRunning);
}

void TargetDetectionDialog::onTargetSelected()
{
    // Handle target selection in results table
    int currentRow = m_resultsTable->currentRow();
    if (currentRow >= 0 && currentRow < m_lastResult.targets.size())
    {
        // Could emit signal to highlight target in 3D view
        auto target = m_lastResult.targets[currentRow];
        m_logTextEdit->append(QString("Selected target: %1 at %2")
                                  .arg(target->getTargetId())
                                  .arg(QString("(%1, %2, %3)")
                                           .arg(target->getPosition().x())
                                           .arg(target->getPosition().y())
                                           .arg(target->getPosition().z())));
    }
}

void TargetDetectionDialog::onAcceptTargets()
{
    if (m_lastResult.targets.isEmpty())
    {
        return;
    }

    // Add all detected targets to the target manager
    int addedCount = 0;
    for (const auto& target : m_lastResult.targets)
    {
        if (m_targetManager->addTarget(m_currentScanId, target))
        {
            addedCount++;
        }
    }

    m_logTextEdit->append(QString("Added %1 targets to scan %2").arg(addedCount).arg(m_currentScanId));

    QMessageBox::information(
        this, "Targets Added", QString("Successfully added %1 targets to the scan.").arg(addedCount));

    emit detectionCompleted(m_currentScanId, m_lastResult);
    accept();  // Close dialog
}

void TargetDetectionDialog::onRejectTargets()
{
    m_logTextEdit->append("Rejected all detected targets.");

    // Clear results
    m_resultsTable->setRowCount(0);
    m_lastResult = TargetDetectionBase::DetectionResult();

    m_acceptButton->setEnabled(false);
    m_rejectButton->setEnabled(false);
}

void TargetDetectionDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Create tab widget
    m_tabWidget = new QTabWidget();

    // Parameters tab
    QWidget* parametersTab = new QWidget();
    QVBoxLayout* parametersLayout = new QVBoxLayout(parametersTab);
    parametersLayout->addWidget(createParameterControls());
    parametersLayout->addStretch();
    m_tabWidget->addTab(parametersTab, "Parameters");

    // Detection tab
    QWidget* detectionTab = new QWidget();
    QVBoxLayout* detectionLayout = new QVBoxLayout(detectionTab);
    detectionLayout->addWidget(createDetectionControls());
    detectionLayout->addStretch();
    m_tabWidget->addTab(detectionTab, "Detection");

    // Results tab
    QWidget* resultsTab = new QWidget();
    QVBoxLayout* resultsLayout = new QVBoxLayout(resultsTab);
    resultsLayout->addWidget(createResultsDisplay());
    m_tabWidget->addTab(resultsTab, "Results");

    mainLayout->addWidget(m_tabWidget);

    // Dialog buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QPushButton* closeButton = new QPushButton("Close");
    connect(closeButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(closeButton);

    mainLayout->addLayout(buttonLayout);
}

QWidget* TargetDetectionDialog::createParameterControls()
{
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);

    // Detection mode selection
    QGroupBox* modeGroup = new QGroupBox("Detection Mode");
    QVBoxLayout* modeLayout = new QVBoxLayout(modeGroup);

    m_detectionModeCombo = new QComboBox();
    m_detectionModeCombo->addItem("Automatic Sphere Detection");
    m_detectionModeCombo->addItem("Manual Natural Point Selection");
    m_detectionModeCombo->addItem("Both");
    connect(m_detectionModeCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &TargetDetectionDialog::onDetectionModeChanged);

    modeLayout->addWidget(m_detectionModeCombo);
    layout->addWidget(modeGroup);

    // Common parameters
    m_commonParamsGroup = new QGroupBox("Common Parameters");
    QGridLayout* commonLayout = new QGridLayout(m_commonParamsGroup);

    commonLayout->addWidget(new QLabel("Distance Threshold (m):"), 0, 0);
    m_distanceThresholdSpin = new QDoubleSpinBox();
    m_distanceThresholdSpin->setRange(0.001, 1.0);
    m_distanceThresholdSpin->setSingleStep(0.001);
    m_distanceThresholdSpin->setDecimals(3);
    connect(m_distanceThresholdSpin,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &TargetDetectionDialog::onParametersChanged);
    commonLayout->addWidget(m_distanceThresholdSpin, 0, 1);

    commonLayout->addWidget(new QLabel("Max Iterations:"), 1, 0);
    m_maxIterationsSpin = new QSpinBox();
    m_maxIterationsSpin->setRange(100, 10000);
    m_maxIterationsSpin->setSingleStep(100);
    connect(m_maxIterationsSpin,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &TargetDetectionDialog::onParametersChanged);
    commonLayout->addWidget(m_maxIterationsSpin, 1, 1);

    commonLayout->addWidget(new QLabel("Min Quality:"), 2, 0);
    m_minQualitySpin = new QDoubleSpinBox();
    m_minQualitySpin->setRange(0.0, 1.0);
    m_minQualitySpin->setSingleStep(0.1);
    m_minQualitySpin->setDecimals(2);
    connect(m_minQualitySpin,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &TargetDetectionDialog::onParametersChanged);
    commonLayout->addWidget(m_minQualitySpin, 2, 1);

    m_enablePreprocessingCheck = new QCheckBox("Enable Preprocessing");
    connect(m_enablePreprocessingCheck, &QCheckBox::toggled, this, &TargetDetectionDialog::onParametersChanged);
    commonLayout->addWidget(m_enablePreprocessingCheck, 3, 0, 1, 2);

    layout->addWidget(m_commonParamsGroup);

    // Sphere-specific parameters
    m_sphereParamsGroup = new QGroupBox("Sphere Detection Parameters");
    QGridLayout* sphereLayout = new QGridLayout(m_sphereParamsGroup);

    sphereLayout->addWidget(new QLabel("Min Radius (m):"), 0, 0);
    m_minRadiusSpin = new QDoubleSpinBox();
    m_minRadiusSpin->setRange(0.01, 10.0);
    m_minRadiusSpin->setSingleStep(0.01);
    m_minRadiusSpin->setDecimals(3);
    connect(m_minRadiusSpin,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &TargetDetectionDialog::onParametersChanged);
    sphereLayout->addWidget(m_minRadiusSpin, 0, 1);

    sphereLayout->addWidget(new QLabel("Max Radius (m):"), 1, 0);
    m_maxRadiusSpin = new QDoubleSpinBox();
    m_maxRadiusSpin->setRange(0.01, 10.0);
    m_maxRadiusSpin->setSingleStep(0.01);
    m_maxRadiusSpin->setDecimals(3);
    connect(m_maxRadiusSpin,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &TargetDetectionDialog::onParametersChanged);
    sphereLayout->addWidget(m_maxRadiusSpin, 1, 1);

    sphereLayout->addWidget(new QLabel("Min Inliers:"), 2, 0);
    m_minInliersSpin = new QSpinBox();
    m_minInliersSpin->setRange(10, 1000);
    m_minInliersSpin->setSingleStep(10);
    connect(m_minInliersSpin,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &TargetDetectionDialog::onParametersChanged);
    sphereLayout->addWidget(m_minInliersSpin, 2, 1);

    layout->addWidget(m_sphereParamsGroup);

    // Natural point parameters
    m_naturalPointParamsGroup = new QGroupBox("Natural Point Parameters");
    QGridLayout* naturalLayout = new QGridLayout(m_naturalPointParamsGroup);

    naturalLayout->addWidget(new QLabel("Neighborhood Radius (m):"), 0, 0);
    m_neighborhoodRadiusSpin = new QDoubleSpinBox();
    m_neighborhoodRadiusSpin->setRange(0.01, 1.0);
    m_neighborhoodRadiusSpin->setSingleStep(0.01);
    m_neighborhoodRadiusSpin->setDecimals(3);
    connect(m_neighborhoodRadiusSpin,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &TargetDetectionDialog::onParametersChanged);
    naturalLayout->addWidget(m_neighborhoodRadiusSpin, 0, 1);

    naturalLayout->addWidget(new QLabel("Curvature Threshold:"), 1, 0);
    m_curvatureThresholdSpin = new QDoubleSpinBox();
    m_curvatureThresholdSpin->setRange(0.0, 1.0);
    m_curvatureThresholdSpin->setSingleStep(0.01);
    m_curvatureThresholdSpin->setDecimals(3);
    connect(m_curvatureThresholdSpin,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &TargetDetectionDialog::onParametersChanged);
    naturalLayout->addWidget(m_curvatureThresholdSpin, 1, 1);

    layout->addWidget(m_naturalPointParamsGroup);

    return widget;
}

QWidget* TargetDetectionDialog::createDetectionControls()
{
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);

    // Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_startButton = new QPushButton("Start Detection");
    m_startButton->setEnabled(false);
    connect(m_startButton, &QPushButton::clicked, this, &TargetDetectionDialog::startDetection);
    buttonLayout->addWidget(m_startButton);

    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setEnabled(false);
    connect(m_cancelButton, &QPushButton::clicked, this, &TargetDetectionDialog::cancelDetection);
    buttonLayout->addWidget(m_cancelButton);

    buttonLayout->addStretch();

    m_resetButton = new QPushButton("Reset to Defaults");
    connect(m_resetButton, &QPushButton::clicked, this, &TargetDetectionDialog::resetToDefaults);
    buttonLayout->addWidget(m_resetButton);

    m_loadParamsButton = new QPushButton("Load Parameters");
    connect(m_loadParamsButton, &QPushButton::clicked, this, &TargetDetectionDialog::loadParameters);
    buttonLayout->addWidget(m_loadParamsButton);

    m_saveParamsButton = new QPushButton("Save Parameters");
    connect(m_saveParamsButton, &QPushButton::clicked, this, &TargetDetectionDialog::saveParameters);
    buttonLayout->addWidget(m_saveParamsButton);

    layout->addLayout(buttonLayout);

    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    layout->addWidget(m_progressBar);

    // Status label
    m_statusLabel = new QLabel("Ready");
    layout->addWidget(m_statusLabel);

    // Manual selection button
    m_manualSelectionButton = new QPushButton("Enable Manual Selection Mode");
    connect(
        m_manualSelectionButton, &QPushButton::clicked, [this]() { emit manualSelectionRequested(m_currentScanId); });
    layout->addWidget(m_manualSelectionButton);

    return widget;
}

QWidget* TargetDetectionDialog::createResultsDisplay()
{
    QWidget* widget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(widget);

    // Results table
    m_resultsTable = new QTableWidget();
    m_resultsTable->setColumnCount(6);
    QStringList headers = {"Type", "ID", "Position", "Quality", "Radius/Size", "Details"};
    m_resultsTable->setHorizontalHeaderLabels(headers);
    m_resultsTable->horizontalHeader()->setStretchLastSection(true);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(m_resultsTable, &QTableWidget::currentRowChanged, this, &TargetDetectionDialog::onTargetSelected);

    layout->addWidget(m_resultsTable);

    // Action buttons
    QHBoxLayout* actionLayout = new QHBoxLayout();

    m_acceptButton = new QPushButton("Accept All Targets");
    m_acceptButton->setEnabled(false);
    connect(m_acceptButton, &QPushButton::clicked, this, &TargetDetectionDialog::onAcceptTargets);
    actionLayout->addWidget(m_acceptButton);

    m_rejectButton = new QPushButton("Reject All Targets");
    m_rejectButton->setEnabled(false);
    connect(m_rejectButton, &QPushButton::clicked, this, &TargetDetectionDialog::onRejectTargets);
    actionLayout->addWidget(m_rejectButton);

    actionLayout->addStretch();

    layout->addLayout(actionLayout);

    // Log text edit
    QLabel* logLabel = new QLabel("Detection Log:");
    layout->addWidget(logLabel);

    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setMaximumHeight(150);
    m_logTextEdit->setReadOnly(true);
    layout->addWidget(m_logTextEdit);

    return widget;
}

void TargetDetectionDialog::updateParameterControls()
{
    DetectionMode mode = getDetectionMode();

    // Show/hide parameter groups based on detection mode
    m_sphereParamsGroup->setVisible(mode == AutomaticSpheres || mode == Both);
    m_naturalPointParamsGroup->setVisible(mode == ManualNaturalPoints || mode == Both);

    // Update manual selection button visibility
    m_manualSelectionButton->setVisible(mode == ManualNaturalPoints || mode == Both);
}

void TargetDetectionDialog::updateResultsTable(const TargetDetectionBase::DetectionResult& result)
{
    m_resultsTable->setRowCount(result.targets.size());

    for (int i = 0; i < result.targets.size(); ++i)
    {
        const auto& target = result.targets[i];

        // Type
        m_resultsTable->setItem(i, 0, new QTableWidgetItem(target->getType()));

        // ID
        m_resultsTable->setItem(i, 1, new QTableWidgetItem(target->getTargetId()));

        // Position
        QVector3D pos = target->getPosition();
        QString posStr =
            QString("(%1, %2, %3)").arg(pos.x(), 0, 'f', 3).arg(pos.y(), 0, 'f', 3).arg(pos.z(), 0, 'f', 3);
        m_resultsTable->setItem(i, 2, new QTableWidgetItem(posStr));

        // Quality
        m_resultsTable->setItem(i, 3, new QTableWidgetItem(QString::number(target->getQuality(), 'f', 3)));

        // Type-specific information
        QString sizeInfo;
        QString details;

        if (target->getType() == "Sphere")
        {
            auto sphereTarget = std::dynamic_pointer_cast<SphereTarget>(target);
            if (sphereTarget)
            {
                sizeInfo = QString("%1 m").arg(sphereTarget->getRadius(), 0, 'f', 3);
                details = QString("RMS: %1, Inliers: %2")
                              .arg(sphereTarget->getRMSError(), 0, 'f', 4)
                              .arg(sphereTarget->getInlierCount());
            }
        }
        else if (target->getType() == "Natural Point")
        {
            auto naturalTarget = std::dynamic_pointer_cast<NaturalPointTarget>(target);
            if (naturalTarget)
            {
                sizeInfo = "Point";
                details = naturalTarget->getDescription();
            }
        }

        m_resultsTable->setItem(i, 4, new QTableWidgetItem(sizeInfo));
        m_resultsTable->setItem(i, 5, new QTableWidgetItem(details));
    }

    // Resize columns to content
    m_resultsTable->resizeColumnsToContents();
}

bool TargetDetectionDialog::validateParameters() const
{
    TargetDetectionBase::DetectionParams params = getParametersFromUI();

    DetectionMode mode = getDetectionMode();

    if (mode == AutomaticSpheres || mode == Both)
    {
        return m_sphereDetector->validateParameters(params);
    }
    else if (mode == ManualNaturalPoints)
    {
        return m_naturalPointSelector->validateParameters(params);
    }

    return true;
}

TargetDetectionBase::DetectionParams TargetDetectionDialog::getParametersFromUI() const
{
    TargetDetectionBase::DetectionParams params;

    // Common parameters
    params.distanceThreshold = static_cast<float>(m_distanceThresholdSpin->value());
    params.maxIterations = m_maxIterationsSpin->value();
    params.minQuality = static_cast<float>(m_minQualitySpin->value());
    params.enablePreprocessing = m_enablePreprocessingCheck->isChecked();

    // Sphere parameters
    params.minRadius = static_cast<float>(m_minRadiusSpin->value());
    params.maxRadius = static_cast<float>(m_maxRadiusSpin->value());
    params.minInliers = m_minInliersSpin->value();

    // Natural point parameters
    params.neighborhoodRadius = static_cast<float>(m_neighborhoodRadiusSpin->value());
    params.curvatureThreshold = static_cast<float>(m_curvatureThresholdSpin->value());

    return params;
}

void TargetDetectionDialog::setUIFromParameters(const TargetDetectionBase::DetectionParams& params)
{
    // Common parameters
    m_distanceThresholdSpin->setValue(params.distanceThreshold);
    m_maxIterationsSpin->setValue(params.maxIterations);
    m_minQualitySpin->setValue(params.minQuality);
    m_enablePreprocessingCheck->setChecked(params.enablePreprocessing);

    // Sphere parameters
    m_minRadiusSpin->setValue(params.minRadius);
    m_maxRadiusSpin->setValue(params.maxRadius);
    m_minInliersSpin->setValue(params.minInliers);

    // Natural point parameters
    m_neighborhoodRadiusSpin->setValue(params.neighborhoodRadius);
    m_curvatureThresholdSpin->setValue(params.curvatureThreshold);
}
