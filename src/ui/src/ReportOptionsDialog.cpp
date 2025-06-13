#include "ui/ReportOptionsDialog.h"
#include "quality/PDFReportGenerator.h"
#include "interfaces/IMainView.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QLabel>

ReportOptionsDialog::ReportOptionsDialog(IMainView* mainView, QWidget* parent)
    : QDialog(parent)
    , m_mainView(mainView)
    , m_isGenerating(false)
{
    setWindowTitle("PDF Report Options");
    setModal(true);
    setMinimumSize(500, 600);
    
    setupUI();
}

void ReportOptionsDialog::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    
    // Add groups
    mainLayout->addWidget(createGeneralInfoGroup());
    mainLayout->addWidget(createContentOptionsGroup());
    mainLayout->addWidget(createOutputPathGroup());
    mainLayout->addWidget(createProgressAndButtonsGroup());
    
    setLayout(mainLayout);
}

QGroupBox* ReportOptionsDialog::createGeneralInfoGroup()
{
    auto* group = new QGroupBox("General Information", this);
    auto* layout = new QGridLayout(group);
    
    // Report Title
    layout->addWidget(new QLabel("Report Title:"), 0, 0);
    m_reportTitleEdit = new QLineEdit(this);
    layout->addWidget(m_reportTitleEdit, 0, 1);
    
    // Company Name
    layout->addWidget(new QLabel("Company Name:"), 1, 0);
    m_companyNameEdit = new QLineEdit(this);
    layout->addWidget(m_companyNameEdit, 1, 1);
    
    // Operator Name
    layout->addWidget(new QLabel("Operator Name:"), 2, 0);
    m_operatorNameEdit = new QLineEdit(this);
    layout->addWidget(m_operatorNameEdit, 2, 1);
    
    // Company Logo
    layout->addWidget(new QLabel("Company Logo:"), 3, 0);
    auto* logoLayout = new QHBoxLayout();
    m_logoPathEdit = new QLineEdit(this);
    m_browseLogoButton = new QPushButton("Browse...", this);
    logoLayout->addWidget(m_logoPathEdit);
    logoLayout->addWidget(m_browseLogoButton);
    layout->addLayout(logoLayout, 3, 1);
    
    // Connect logo browse button
    connect(m_browseLogoButton, &QPushButton::clicked,
            this, &ReportOptionsDialog::onBrowseLogoPathClicked);
    
    return group;
}

QGroupBox* ReportOptionsDialog::createContentOptionsGroup()
{
    auto* group = new QGroupBox("Content Options", this);
    auto* layout = new QVBoxLayout(group);
    
    m_includeChartsCheckBox = new QCheckBox("Include Charts", this);
    m_includeScreenshotsCheckBox = new QCheckBox("Include Screenshots", this);
    m_includeRecommendationsCheckBox = new QCheckBox("Include Recommendations", this);
    m_includeDetailedMetricsCheckBox = new QCheckBox("Include Detailed Metrics", this);
    
    // Set default checked state for detailed metrics
    m_includeDetailedMetricsCheckBox->setChecked(true);
    
    layout->addWidget(m_includeChartsCheckBox);
    layout->addWidget(m_includeScreenshotsCheckBox);
    layout->addWidget(m_includeRecommendationsCheckBox);
    layout->addWidget(m_includeDetailedMetricsCheckBox);
    
    return group;
}

QGroupBox* ReportOptionsDialog::createOutputPathGroup()
{
    auto* group = new QGroupBox("Output Path", this);
    auto* layout = new QHBoxLayout(group);
    
    layout->addWidget(new QLabel("Save to:"));
    m_outputPathEdit = new QLineEdit(this);
    m_browseOutputButton = new QPushButton("Browse...", this);
    
    layout->addWidget(m_outputPathEdit);
    layout->addWidget(m_browseOutputButton);
    
    // Connect output browse button
    connect(m_browseOutputButton, &QPushButton::clicked,
            this, &ReportOptionsDialog::onBrowseOutputPathClicked);
    
    return group;
}

QWidget* ReportOptionsDialog::createProgressAndButtonsGroup()
{
    auto* widget = new QWidget(this);
    auto* layout = new QVBoxLayout(widget);
    
    // Progress section
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_statusLabel = new QLabel(this);
    m_statusLabel->setVisible(false);
    
    layout->addWidget(m_progressBar);
    layout->addWidget(m_statusLabel);
    
    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_generateButton = new QPushButton("Generate Report", this);
    m_cancelButton = new QPushButton("Cancel", this);
    
    buttonLayout->addWidget(m_generateButton);
    buttonLayout->addWidget(m_cancelButton);
    
    layout->addLayout(buttonLayout);
    
    // Connect buttons
    connect(m_generateButton, &QPushButton::clicked,
            this, &ReportOptionsDialog::onGenerateButtonClicked);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &ReportOptionsDialog::onCancelButtonClicked);
    
    return widget;
}

PDFReportGenerator::ReportOptions ReportOptionsDialog::getReportOptions() const
{
    PDFReportGenerator::ReportOptions options;
    
    options.reportTitle = m_reportTitleEdit->text();
    options.companyName = m_companyNameEdit->text();
    options.operatorName = m_operatorNameEdit->text();
    options.logoPath = m_logoPathEdit->text();
    options.outputPath = m_outputPathEdit->text();
    
    options.includeCharts = m_includeChartsCheckBox->isChecked();
    options.includeScreenshots = m_includeScreenshotsCheckBox->isChecked();
    options.includeRecommendations = m_includeRecommendationsCheckBox->isChecked();
    options.includeDetailedMetrics = m_includeDetailedMetricsCheckBox->isChecked();
    
    return options;
}

void ReportOptionsDialog::setReportOptions(const PDFReportGenerator::ReportOptions& options)
{
    m_reportTitleEdit->setText(options.reportTitle);
    m_companyNameEdit->setText(options.companyName);
    m_operatorNameEdit->setText(options.operatorName);
    m_logoPathEdit->setText(options.logoPath);
    m_outputPathEdit->setText(options.outputPath);
    
    m_includeChartsCheckBox->setChecked(options.includeCharts);
    m_includeScreenshotsCheckBox->setChecked(options.includeScreenshots);
    m_includeRecommendationsCheckBox->setChecked(options.includeRecommendations);
    m_includeDetailedMetricsCheckBox->setChecked(options.includeDetailedMetrics);
}

void ReportOptionsDialog::onReportProgress(int percentage, const QString& stage)
{
    m_progressBar->setValue(percentage);
    m_statusLabel->setText(stage);
    
    if (!m_progressBar->isVisible()) {
        m_progressBar->setVisible(true);
        m_statusLabel->setVisible(true);
    }
}

void ReportOptionsDialog::onReportFinished(bool success, const QString& message)
{
    m_isGenerating = false;
    m_generateButton->setEnabled(true);
    
    if (success) {
        m_statusLabel->setText("Report generated successfully!");
        m_progressBar->setValue(100);
        
        // Show success message and close dialog after a brief delay
        QMessageBox::information(this, "Report Generated", message);
        accept(); // Close dialog with success
    } else {
        m_statusLabel->setText(QString("Error: %1").arg(message));
        m_progressBar->setValue(0);
        
        // Show error message but keep dialog open
        QMessageBox::critical(this, "Report Generation Failed", message);
    }
}

void ReportOptionsDialog::onGenerateButtonClicked()
{
    if (!validateInputs()) {
        return;
    }
    
    m_isGenerating = true;
    m_generateButton->setEnabled(false);
    m_progressBar->setValue(0);
    m_statusLabel->setText("Preparing to generate report...");
    m_progressBar->setVisible(true);
    m_statusLabel->setVisible(true);
    
    emit generateReportRequested(getReportOptions());
}

void ReportOptionsDialog::onBrowseOutputPathClicked()
{
    if (!m_mainView) {
        return;
    }
    
    QString defaultName = QString("%1.pdf").arg(m_reportTitleEdit->text().isEmpty() ? 
                                                "QualityReport" : m_reportTitleEdit->text());
    
    QString filePath = m_mainView->askForSaveFilePath("Save Quality Report",
                                                      "PDF files (*.pdf)",
                                                      defaultName);
    
    if (!filePath.isEmpty()) {
        m_outputPathEdit->setText(filePath);
    }
}

void ReportOptionsDialog::onBrowseLogoPathClicked()
{
    if (!m_mainView) {
        return;
    }
    
    QString filePath = m_mainView->askForOpenFilePath("Select Company Logo",
                                                      "Image files (*.png *.jpg *.jpeg *.bmp *.gif)");
    
    if (!filePath.isEmpty()) {
        m_logoPathEdit->setText(filePath);
    }
}

void ReportOptionsDialog::onCancelButtonClicked()
{
    if (m_isGenerating) {
        // TODO: Implement cancellation of report generation if supported
        // For now, just warn the user
        int ret = QMessageBox::question(this, "Cancel Report Generation",
                                       "Report generation is in progress. Are you sure you want to cancel?",
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    
    reject(); // Close dialog with cancellation
}

bool ReportOptionsDialog::validateInputs() const
{
    if (m_reportTitleEdit->text().trimmed().isEmpty()) {
        showValidationError("Report title is required.");
        return false;
    }
    
    if (m_outputPathEdit->text().trimmed().isEmpty()) {
        showValidationError("Output path is required.");
        return false;
    }
    
    // Validate output path directory exists
    QFileInfo outputInfo(m_outputPathEdit->text());
    QDir outputDir = outputInfo.dir();
    if (!outputDir.exists()) {
        showValidationError("Output directory does not exist.");
        return false;
    }
    
    // Validate logo path if provided
    if (!m_logoPathEdit->text().trimmed().isEmpty()) {
        QFileInfo logoInfo(m_logoPathEdit->text());
        if (!logoInfo.exists() || !logoInfo.isFile()) {
            showValidationError("Logo file does not exist or is not a valid file.");
            return false;
        }
    }
    
    return true;
}

void ReportOptionsDialog::showValidationError(const QString& message) const
{
    QMessageBox::warning(const_cast<ReportOptionsDialog*>(this), "Validation Error", message);
}
