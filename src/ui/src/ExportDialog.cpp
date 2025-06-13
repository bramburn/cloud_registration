#include "ExportDialog.h"

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>

const ExportOptions ExportDialog::DEFAULT_OPTIONS = ExportOptions();

ExportDialog::ExportDialog(QWidget* parent) : QDialog(parent), m_exporter(std::make_unique<PointCloudExporter>(this))
{
    setWindowTitle("Export Point Cloud");
    setModal(true);
    resize(600, 700);

    setupUI();

    // Connect exporter signals
    connect(m_exporter.get(), &PointCloudExporter::progressUpdated, this, &ExportDialog::onExportProgress);
    connect(m_exporter.get(), &PointCloudExporter::exportCompleted, this, &ExportDialog::onExportCompleted);
    connect(m_exporter.get(), &PointCloudExporter::errorOccurred, this, &ExportDialog::onExportError);

    // Set default options
    setDefaultOptions(DEFAULT_OPTIONS);
}

ExportDialog::~ExportDialog()
{
    if (m_exportInProgress)
    {
        m_exporter->cancelExport();
    }
}

void ExportDialog::setPointCloudData(const std::vector<Point>& points)
{
    m_pointCloudData = points;

    // Update UI with point count
    QString pointCountText = QString("Points to export: %1").arg(points.size());
    if (m_progressLabel)
    {
        m_progressLabel->setText(pointCountText);
    }
}

ExportOptions ExportDialog::getExportOptions() const
{
    ExportOptions options;

    // Format
    int formatIndex = m_formatCombo->currentIndex();
    options.format = static_cast<ExportFormat>(formatIndex);

    // Output
    options.outputPath = m_outputPathEdit->text();
    options.projectName = m_projectNameEdit->text();
    options.description = m_descriptionEdit->text();

    // Attributes
    options.includeColor = m_includeColorCheck->isChecked();
    options.includeIntensity = m_includeIntensityCheck->isChecked();

    // Coordinate systems
    options.sourceCRS = m_sourceCRSCombo->currentText();
    options.targetCRS = m_targetCRSCombo->currentText();

    // Format-specific
    options.compressE57 = m_compressE57Check->isChecked();
    options.asciiPLY = m_asciiPLYCheck->isChecked();
    options.precision = m_precisionSpin->value();
    options.xyzSeparator = m_xyzSeparatorEdit->text();

    // Processing
    options.validateOutput = m_validateOutputCheck->isChecked();
    options.batchSize = static_cast<size_t>(m_batchSizeSpin->value());

    return options;
}

void ExportDialog::setDefaultOptions(const ExportOptions& options)
{
    // Format
    m_formatCombo->setCurrentIndex(static_cast<int>(options.format));

    // Output
    m_outputPathEdit->setText(options.outputPath);
    m_projectNameEdit->setText(options.projectName);
    m_descriptionEdit->setText(options.description);

    // Attributes
    m_includeColorCheck->setChecked(options.includeColor);
    m_includeIntensityCheck->setChecked(options.includeIntensity);

    // Coordinate systems
    int sourceCRSIndex = m_sourceCRSCombo->findText(options.sourceCRS);
    if (sourceCRSIndex >= 0)
    {
        m_sourceCRSCombo->setCurrentIndex(sourceCRSIndex);
    }

    int targetCRSIndex = m_targetCRSCombo->findText(options.targetCRS);
    if (targetCRSIndex >= 0)
    {
        m_targetCRSCombo->setCurrentIndex(targetCRSIndex);
    }

    // Format-specific
    m_compressE57Check->setChecked(options.compressE57);
    m_asciiPLYCheck->setChecked(options.asciiPLY);
    m_precisionSpin->setValue(options.precision);
    m_xyzSeparatorEdit->setText(options.xyzSeparator);

    // Processing
    m_validateOutputCheck->setChecked(options.validateOutput);
    m_batchSizeSpin->setValue(static_cast<int>(options.batchSize));

    updateFormatSpecificOptions();
}

void ExportDialog::accept()
{
    if (m_exportInProgress)
    {
        return;
    }

    // Validate inputs
    validateInputs();
    if (!isValidOutputPath())
    {
        QMessageBox::warning(this, "Invalid Output Path", "Please specify a valid output file path.");
        return;
    }

    if (m_pointCloudData.empty())
    {
        QMessageBox::warning(this, "No Data", "No point cloud data available for export.");
        return;
    }

    // Start export
    m_exportInProgress = true;
    m_exportButton->setEnabled(false);
    m_progressBar->setVisible(true);
    m_progressLabel->setText("Starting export...");

    ExportOptions options = getExportOptions();
    m_exporter->exportPointCloudAsync(m_pointCloudData, options);
}

void ExportDialog::onBrowseOutputPath()
{
    QString currentPath = m_outputPathEdit->text();
    if (currentPath.isEmpty())
    {
        currentPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }

    // Get file extension for current format
    ExportFormat format = static_cast<ExportFormat>(m_formatCombo->currentIndex());
    QString extension = PointCloudExporter::getFileExtension(format);

    QString filter;
    switch (format)
    {
        case ExportFormat::E57:
            filter = "E57 Files (*.e57);;All Files (*.*)";
            break;
        case ExportFormat::LAS:
            filter = "LAS Files (*.las);;All Files (*.*)";
            break;
        case ExportFormat::PLY:
            filter = "PLY Files (*.ply);;All Files (*.*)";
            break;
        case ExportFormat::XYZ:
            filter = "XYZ Files (*.xyz);;Text Files (*.txt);;All Files (*.*)";
            break;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Export Point Cloud", currentPath, filter);
    if (!fileName.isEmpty())
    {
        m_outputPathEdit->setText(fileName);
        updateFileExtension();
    }
}

void ExportDialog::onFormatChanged()
{
    updateFormatSpecificOptions();
    updateFileExtension();
}

void ExportDialog::onExportProgress(int percentage, const QString& stage)
{
    m_progressBar->setValue(percentage);
    m_progressLabel->setText(stage);
}

void ExportDialog::onExportCompleted(const ExportResult& result)
{
    m_exportInProgress = false;
    m_exportButton->setEnabled(true);
    m_progressBar->setVisible(false);

    if (result.success)
    {
        QString message = QString("Export completed successfully!\n\n"
                                  "Points exported: %1\n"
                                  "File size: %2 bytes\n"
                                  "Export time: %3 seconds")
                              .arg(result.pointsExported)
                              .arg(result.fileSizeBytes)
                              .arg(result.exportTimeSeconds, 0, 'f', 2);

        QMessageBox::information(this, "Export Successful", message);
        QDialog::accept();  // Close dialog
    }
    else
    {
        QMessageBox::critical(this, "Export Failed", QString("Export failed: %1").arg(result.errorMessage));
        m_progressLabel->setText("Export failed");
    }
}

void ExportDialog::onExportError(const QString& errorMessage)
{
    m_exportInProgress = false;
    m_exportButton->setEnabled(true);
    m_progressBar->setVisible(false);

    QMessageBox::critical(this, "Export Error", errorMessage);
    m_progressLabel->setText("Export error occurred");
}

void ExportDialog::onPreviewSettings()
{
    ExportOptions options = getExportOptions();

    QString preview = QString("Export Preview:\n\n"
                              "Format: %1\n"
                              "Output: %2\n"
                              "Points: %3\n"
                              "Include Color: %4\n"
                              "Include Intensity: %5\n"
                              "Source CRS: %6\n"
                              "Target CRS: %7")
                          .arg(m_formatCombo->currentText())
                          .arg(options.outputPath)
                          .arg(m_pointCloudData.size())
                          .arg(options.includeColor ? "Yes" : "No")
                          .arg(options.includeIntensity ? "Yes" : "No")
                          .arg(options.sourceCRS)
                          .arg(options.targetCRS);

    QMessageBox::information(this, "Export Preview", preview);
}

void ExportDialog::onResetToDefaults()
{
    setDefaultOptions(DEFAULT_OPTIONS);
}

void ExportDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);

    setupFormatGroup();
    setupOutputGroup();
    setupAttributeGroup();
    setupCoordinateSystemGroup();
    setupFormatSpecificGroup();
    setupProgressGroup();
    setupButtonBox();

    m_mainLayout->addStretch();
}

void ExportDialog::setupFormatGroup()
{
    m_formatGroup = new QGroupBox("Export Format", this);
    QHBoxLayout* layout = new QHBoxLayout(m_formatGroup);

    m_formatCombo = new QComboBox();
    m_formatCombo->addItems(PointCloudExporter::getSupportedFormats());
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ExportDialog::onFormatChanged);

    m_formatHelpButton = new QPushButton("?");
    m_formatHelpButton->setMaximumWidth(30);
    connect(m_formatHelpButton, &QPushButton::clicked, this, &ExportDialog::showFormatHelp);

    layout->addWidget(new QLabel("Format:"));
    layout->addWidget(m_formatCombo);
    layout->addWidget(m_formatHelpButton);
    layout->addStretch();

    m_mainLayout->addWidget(m_formatGroup);
}

void ExportDialog::setupOutputGroup()
{
    m_outputGroup = new QGroupBox("Output Settings", this);
    QGridLayout* layout = new QGridLayout(m_outputGroup);

    // Output path
    layout->addWidget(new QLabel("Output File:"), 0, 0);
    m_outputPathEdit = new QLineEdit();
    layout->addWidget(m_outputPathEdit, 0, 1);
    m_browseButton = new QPushButton("Browse...");
    connect(m_browseButton, &QPushButton::clicked, this, &ExportDialog::onBrowseOutputPath);
    layout->addWidget(m_browseButton, 0, 2);

    // Project name
    layout->addWidget(new QLabel("Project Name:"), 1, 0);
    m_projectNameEdit = new QLineEdit();
    layout->addWidget(m_projectNameEdit, 1, 1, 1, 2);

    // Description
    layout->addWidget(new QLabel("Description:"), 2, 0);
    m_descriptionEdit = new QLineEdit();
    layout->addWidget(m_descriptionEdit, 2, 1, 1, 2);

    m_mainLayout->addWidget(m_outputGroup);
}

void ExportDialog::setupAttributeGroup()
{
    m_attributeGroup = new QGroupBox("Point Attributes", this);
    QVBoxLayout* layout = new QVBoxLayout(m_attributeGroup);

    m_includeColorCheck = new QCheckBox("Include Color (RGB)");
    m_includeIntensityCheck = new QCheckBox("Include Intensity");

    layout->addWidget(m_includeColorCheck);
    layout->addWidget(m_includeIntensityCheck);

    m_mainLayout->addWidget(m_attributeGroup);
}

void ExportDialog::setupCoordinateSystemGroup()
{
    m_coordinateGroup = new QGroupBox("Coordinate Systems", this);
    QGridLayout* layout = new QGridLayout(m_coordinateGroup);

    // Source CRS
    layout->addWidget(new QLabel("Source CRS:"), 0, 0);
    m_sourceCRSCombo = new QComboBox();
    m_sourceCRSCombo->addItems({"WGS84", "UTM Zone 10N", "UTM Zone 11N", "State Plane CA I", "Local"});
    layout->addWidget(m_sourceCRSCombo, 0, 1);

    // Target CRS
    layout->addWidget(new QLabel("Target CRS:"), 1, 0);
    m_targetCRSCombo = new QComboBox();
    m_targetCRSCombo->addItems({"WGS84", "UTM Zone 10N", "UTM Zone 11N", "State Plane CA I", "Local"});
    layout->addWidget(m_targetCRSCombo, 1, 1);

    // Warning label
    m_transformationWarning = new QLabel("⚠ Coordinate transformation will be applied");
    m_transformationWarning->setStyleSheet("color: orange;");
    m_transformationWarning->setVisible(false);
    layout->addWidget(m_transformationWarning, 2, 0, 1, 2);

    m_mainLayout->addWidget(m_coordinateGroup);
}

void ExportDialog::setupFormatSpecificGroup()
{
    m_formatSpecificGroup = new QGroupBox("Format-Specific Options", this);
    QGridLayout* layout = new QGridLayout(m_formatSpecificGroup);

    // E57 compression
    m_compressE57Check = new QCheckBox("Enable E57 compression");
    layout->addWidget(m_compressE57Check, 0, 0, 1, 2);

    // PLY ASCII format
    m_asciiPLYCheck = new QCheckBox("Use ASCII PLY format");
    layout->addWidget(m_asciiPLYCheck, 1, 0, 1, 2);

    // Precision
    layout->addWidget(new QLabel("Coordinate Precision:"), 2, 0);
    m_precisionSpin = new QSpinBox();
    m_precisionSpin->setRange(0, 15);
    m_precisionSpin->setValue(6);
    layout->addWidget(m_precisionSpin, 2, 1);

    // XYZ separator
    layout->addWidget(new QLabel("XYZ Field Separator:"), 3, 0);
    m_xyzSeparatorEdit = new QLineEdit(" ");
    m_xyzSeparatorEdit->setMaximumWidth(50);
    layout->addWidget(m_xyzSeparatorEdit, 3, 1);

    // Processing options
    m_validateOutputCheck = new QCheckBox("Validate exported file");
    layout->addWidget(m_validateOutputCheck, 4, 0, 1, 2);

    layout->addWidget(new QLabel("Batch Size:"), 5, 0);
    m_batchSizeSpin = new QSpinBox();
    m_batchSizeSpin->setRange(1000, 100000);
    m_batchSizeSpin->setValue(10000);
    layout->addWidget(m_batchSizeSpin, 5, 1);

    m_mainLayout->addWidget(m_formatSpecificGroup);
}

void ExportDialog::setupProgressGroup()
{
    m_progressGroup = new QGroupBox("Export Progress", this);
    QVBoxLayout* layout = new QVBoxLayout(m_progressGroup);

    m_progressBar = new QProgressBar();
    m_progressBar->setVisible(false);
    layout->addWidget(m_progressBar);

    m_progressLabel = new QLabel("Ready to export");
    layout->addWidget(m_progressLabel);

    m_mainLayout->addWidget(m_progressGroup);
}

void ExportDialog::setupButtonBox()
{
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_previewButton = new QPushButton("Preview");
    connect(m_previewButton, &QPushButton::clicked, this, &ExportDialog::onPreviewSettings);

    m_resetButton = new QPushButton("Reset");
    connect(m_resetButton, &QPushButton::clicked, this, &ExportDialog::onResetToDefaults);

    buttonLayout->addWidget(m_previewButton);
    buttonLayout->addWidget(m_resetButton);
    buttonLayout->addStretch();

    m_exportButton = new QPushButton("Export");
    m_exportButton->setDefault(true);
    connect(m_exportButton, &QPushButton::clicked, this, &ExportDialog::accept);

    m_cancelButton = new QPushButton("Cancel");
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    buttonLayout->addWidget(m_exportButton);
    buttonLayout->addWidget(m_cancelButton);

    m_mainLayout->addLayout(buttonLayout);
}

void ExportDialog::updateFormatSpecificOptions()
{
    ExportFormat format = static_cast<ExportFormat>(m_formatCombo->currentIndex());

    // Show/hide format-specific options
    m_compressE57Check->setVisible(format == ExportFormat::E57);
    m_asciiPLYCheck->setVisible(format == ExportFormat::PLY);

    // Find XYZ separator widgets and show/hide them
    QGridLayout* layout = qobject_cast<QGridLayout*>(m_formatSpecificGroup->layout());
    if (layout)
    {
        for (int i = 0; i < layout->count(); ++i)
        {
            QLayoutItem* item = layout->itemAt(i);
            if (QLabel* label = qobject_cast<QLabel*>(item->widget()))
            {
                if (label->text().contains("XYZ Field Separator"))
                {
                    label->setVisible(format == ExportFormat::XYZ);
                }
            }
        }
        m_xyzSeparatorEdit->setVisible(format == ExportFormat::XYZ);
    }
}

void ExportDialog::updateFileExtension()
{
    QString currentPath = m_outputPathEdit->text();
    if (currentPath.isEmpty())
    {
        return;
    }

    QFileInfo fileInfo(currentPath);
    QString baseName = fileInfo.completeBaseName();
    QString dir = fileInfo.absolutePath();

    ExportFormat format = static_cast<ExportFormat>(m_formatCombo->currentIndex());
    QString extension = PointCloudExporter::getFileExtension(format);

    QString newPath = dir + "/" + baseName + extension;
    m_outputPathEdit->setText(newPath);
}

void ExportDialog::validateInputs()
{
    // Check if coordinate transformation is needed
    bool needsTransformation = (m_sourceCRSCombo->currentText() != m_targetCRSCombo->currentText());
    m_transformationWarning->setVisible(needsTransformation);
}

bool ExportDialog::isValidOutputPath() const
{
    QString path = m_outputPathEdit->text();
    if (path.isEmpty())
    {
        return false;
    }

    QFileInfo fileInfo(path);
    return fileInfo.dir().exists();
}

void ExportDialog::showFormatHelp()
{
    QString help = "Export Format Information:\n\n"
                   "E57: Industry standard format with compression and metadata support\n"
                   "LAS: Laser scanning format with header and point classification\n"
                   "PLY: Simple polygon format, good for research and visualization\n"
                   "XYZ: Plain text format with coordinates only, widely compatible";

    QMessageBox::information(this, "Format Help", help);
}
