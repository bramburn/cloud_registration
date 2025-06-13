#include "ui/ICPParameterDialog.h"

#include <QMessageBox>
#include <QDebug>

ICPParameterDialog::ICPParameterDialog(const PointCloud& sourceCloud,
                                     const PointCloud& targetCloud,
                                     QWidget* parent)
    : QDialog(parent),
      m_sourceCloud(sourceCloud),
      m_targetCloud(targetCloud),
      m_maxIterationsSpinBox(nullptr),
      m_convergenceThresholdSpinBox(nullptr),
      m_maxCorrespondenceDistanceSpinBox(nullptr),
      m_enableOutlierRejectionCheckBox(nullptr),
      m_outlierThresholdSpinBox(nullptr),
      m_runICPButton(nullptr),
      m_cancelButton(nullptr),
      m_resetToDefaultsButton(nullptr),
      m_statusLabel(nullptr)
{
    setWindowTitle("ICP Parameter Configuration");
    setModal(true);
    resize(400, 350);

    setupUI();
    setupConnections();
    loadDefaultParameters();
    updateUIState();

    qDebug() << "ICPParameterDialog created with" << sourceCloud.size() << "source points and" 
             << targetCloud.size() << "target points";
}

ICPParams ICPParameterDialog::getICPParameters() const
{
    ICPParams params;
    params.maxIterations = m_maxIterationsSpinBox->value();
    params.convergenceThreshold = static_cast<float>(m_convergenceThresholdSpinBox->value());
    params.maxCorrespondenceDistance = static_cast<float>(m_maxCorrespondenceDistanceSpinBox->value());
    params.useOutlierRejection = m_enableOutlierRejectionCheckBox->isChecked();
    params.outlierThreshold = static_cast<float>(m_outlierThresholdSpinBox->value());
    params.subsamplingRatio = 1.0f; // Default to no subsampling for now
    
    return params;
}

void ICPParameterDialog::setICPParameters(const ICPParams& params)
{
    m_maxIterationsSpinBox->setValue(params.maxIterations);
    m_convergenceThresholdSpinBox->setValue(static_cast<double>(params.convergenceThreshold));
    m_maxCorrespondenceDistanceSpinBox->setValue(static_cast<double>(params.maxCorrespondenceDistance));
    m_enableOutlierRejectionCheckBox->setChecked(params.useOutlierRejection);
    m_outlierThresholdSpinBox->setValue(static_cast<double>(params.outlierThreshold));
    
    updateUIState();
}

void ICPParameterDialog::setScanIds(const QString& sourceScanId, const QString& targetScanId)
{
    m_sourceScanId = sourceScanId;
    m_targetScanId = targetScanId;
    
    // Update window title to show scan information
    setWindowTitle(QString("ICP Configuration - %1 → %2").arg(sourceScanId, targetScanId));
}

void ICPParameterDialog::onRunICPClicked()
{
    if (!validateParameters())
    {
        return;
    }
    
    ICPParams params = getICPParameters();
    emit runICPRequested(params, m_sourceScanId, m_targetScanId);
    accept();
}

void ICPParameterDialog::onResetToDefaultsClicked()
{
    setICPParameters(m_defaultParams);
    m_statusLabel->setText("Parameters reset to recommended defaults");
    
    qDebug() << "ICP parameters reset to defaults";
}

void ICPParameterDialog::onOutlierRejectionToggled(bool enabled)
{
    m_outlierThresholdSpinBox->setEnabled(enabled);
    updateUIState();
}

void ICPParameterDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Parameter configuration group
    QGroupBox* parameterGroup = new QGroupBox("ICP Parameters");
    QFormLayout* parameterLayout = new QFormLayout(parameterGroup);
    
    // Max Iterations
    m_maxIterationsSpinBox = new QSpinBox();
    m_maxIterationsSpinBox->setRange(1, 1000);
    m_maxIterationsSpinBox->setValue(50);
    m_maxIterationsSpinBox->setToolTip("Maximum number of ICP iterations");
    parameterLayout->addRow("Max Iterations:", m_maxIterationsSpinBox);
    
    // Convergence Threshold
    m_convergenceThresholdSpinBox = new QDoubleSpinBox();
    m_convergenceThresholdSpinBox->setRange(1e-8, 1e-2);
    m_convergenceThresholdSpinBox->setDecimals(8);
    m_convergenceThresholdSpinBox->setValue(1e-5);
    m_convergenceThresholdSpinBox->setToolTip("Convergence threshold for RMS error change");
    parameterLayout->addRow("Convergence Threshold:", m_convergenceThresholdSpinBox);
    
    // Max Correspondence Distance
    m_maxCorrespondenceDistanceSpinBox = new QDoubleSpinBox();
    m_maxCorrespondenceDistanceSpinBox->setRange(0.001, 10.0);
    m_maxCorrespondenceDistanceSpinBox->setDecimals(3);
    m_maxCorrespondenceDistanceSpinBox->setValue(0.1);
    m_maxCorrespondenceDistanceSpinBox->setSuffix(" m");
    m_maxCorrespondenceDistanceSpinBox->setToolTip("Maximum distance for point correspondences");
    parameterLayout->addRow("Max Correspondence Distance:", m_maxCorrespondenceDistanceSpinBox);
    
    // Outlier Rejection
    m_enableOutlierRejectionCheckBox = new QCheckBox("Enable Outlier Rejection");
    m_enableOutlierRejectionCheckBox->setChecked(true);
    m_enableOutlierRejectionCheckBox->setToolTip("Enable statistical outlier rejection");
    parameterLayout->addRow(m_enableOutlierRejectionCheckBox);
    
    // Outlier Threshold
    m_outlierThresholdSpinBox = new QDoubleSpinBox();
    m_outlierThresholdSpinBox->setRange(0.5, 5.0);
    m_outlierThresholdSpinBox->setDecimals(1);
    m_outlierThresholdSpinBox->setValue(2.0);
    m_outlierThresholdSpinBox->setSuffix(" σ");
    m_outlierThresholdSpinBox->setToolTip("Standard deviation threshold for outlier rejection");
    parameterLayout->addRow("Outlier Threshold:", m_outlierThresholdSpinBox);
    
    mainLayout->addWidget(parameterGroup);
    
    // Status label
    m_statusLabel = new QLabel("Configure parameters and click 'Run ICP' to start alignment");
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");
    mainLayout->addWidget(m_statusLabel);
    
    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_resetToDefaultsButton = new QPushButton("Reset to Defaults");
    m_resetToDefaultsButton->setToolTip("Reset all parameters to recommended defaults");
    buttonLayout->addWidget(m_resetToDefaultsButton);
    
    buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton("Cancel");
    buttonLayout->addWidget(m_cancelButton);
    
    m_runICPButton = new QPushButton("Run ICP");
    m_runICPButton->setDefault(true);
    m_runICPButton->setToolTip("Start ICP alignment with current parameters");
    buttonLayout->addWidget(m_runICPButton);
    
    mainLayout->addLayout(buttonLayout);
}

void ICPParameterDialog::setupConnections()
{
    connect(m_runICPButton, &QPushButton::clicked, this, &ICPParameterDialog::onRunICPClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_resetToDefaultsButton, &QPushButton::clicked, this, &ICPParameterDialog::onResetToDefaultsClicked);
    connect(m_enableOutlierRejectionCheckBox, &QCheckBox::toggled, this, &ICPParameterDialog::onOutlierRejectionToggled);
}

void ICPParameterDialog::loadDefaultParameters()
{
    // Get recommended parameters from ICPRegistration
    m_defaultParams = ICPRegistration::getRecommendedParameters(m_sourceCloud, m_targetCloud);
    setICPParameters(m_defaultParams);
    
    qDebug() << "Loaded default ICP parameters - Max iterations:" << m_defaultParams.maxIterations
             << "Convergence:" << m_defaultParams.convergenceThreshold
             << "Max distance:" << m_defaultParams.maxCorrespondenceDistance;
}

bool ICPParameterDialog::validateParameters() const
{
    // Basic validation
    if (m_maxIterationsSpinBox->value() < 1)
    {
        QMessageBox::warning(const_cast<ICPParameterDialog*>(this), "Invalid Parameters", 
                           "Maximum iterations must be at least 1");
        return false;
    }
    
    if (m_convergenceThresholdSpinBox->value() <= 0)
    {
        QMessageBox::warning(const_cast<ICPParameterDialog*>(this), "Invalid Parameters", 
                           "Convergence threshold must be positive");
        return false;
    }
    
    if (m_maxCorrespondenceDistanceSpinBox->value() <= 0)
    {
        QMessageBox::warning(const_cast<ICPParameterDialog*>(this), "Invalid Parameters", 
                           "Maximum correspondence distance must be positive");
        return false;
    }
    
    if (m_sourceScanId.isEmpty() || m_targetScanId.isEmpty())
    {
        QMessageBox::warning(const_cast<ICPParameterDialog*>(this), "Invalid Configuration", 
                           "Source and target scan IDs must be specified");
        return false;
    }
    
    return true;
}

void ICPParameterDialog::updateUIState()
{
    // Enable/disable outlier threshold based on checkbox
    m_outlierThresholdSpinBox->setEnabled(m_enableOutlierRejectionCheckBox->isChecked());
    
    // Update status based on current settings
    ICPParams params = getICPParameters();
    QString status = QString("Ready to run ICP with %1 iterations, %2m max distance")
                        .arg(params.maxIterations)
                        .arg(params.maxCorrespondenceDistance, 0, 'f', 3);
    
    if (params.useOutlierRejection)
    {
        status += QString(", outlier rejection at %1σ").arg(params.outlierThreshold, 0, 'f', 1);
    }
    
    m_statusLabel->setText(status);
}
