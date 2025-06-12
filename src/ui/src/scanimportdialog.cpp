#include "ui/scanimportdialog.h"
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>

ScanImportDialog::ScanImportDialog(QWidget *parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_fileSelectionGroup(nullptr)
    , m_fileList(nullptr)
    , m_addFilesButton(nullptr)
    , m_removeFileButton(nullptr)
    , m_clearAllButton(nullptr)
    , m_settingsGroup(nullptr)
    , m_enableLODCheckbox(nullptr)
    , m_lodThresholdSpinBox(nullptr)
    , m_preserveColorsCheckbox(nullptr)
    , m_preserveIntensityCheckbox(nullptr)
    , m_maxPointsSpinBox(nullptr)
    , m_coordinateSystemCombo(nullptr)
    , m_progressGroup(nullptr)
    , m_progressBar(nullptr)
    , m_progressLabel(nullptr)
    , m_currentFileLabel(nullptr)
    , m_buttonLayout(nullptr)
    , m_importButton(nullptr)
    , m_cancelButton(nullptr)
    , m_importInProgress(false)
{
    setWindowTitle("Import Scans");
    setModal(true);
    resize(600, 500);
    
    setupUI();
    
    // Initialize default settings
    m_currentSettings.enableLOD = true;
    m_currentSettings.lodThreshold = 0.1f;
    m_currentSettings.preserveColors = true;
    m_currentSettings.preserveIntensity = true;
    m_currentSettings.maxPointsPerScan = 1000000;
    m_currentSettings.targetCoordinateSystem = "WGS84";
}

ScanImportDialog::~ScanImportDialog()
{
    // Qt handles cleanup automatically
}

void ScanImportDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    setupFileSelection();
    setupImportSettings();
    setupProgressArea();
    
    // Dialog buttons
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();
    
    m_importButton = new QPushButton("Import", this);
    m_cancelButton = new QPushButton("Cancel", this);
    
    m_buttonLayout->addWidget(m_importButton);
    m_buttonLayout->addWidget(m_cancelButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Connect signals
    connect(m_addFilesButton, &QPushButton::clicked, this, &ScanImportDialog::onAddFilesClicked);
    connect(m_removeFileButton, &QPushButton::clicked, this, &ScanImportDialog::onRemoveFileClicked);
    connect(m_clearAllButton, &QPushButton::clicked, this, &ScanImportDialog::onClearAllClicked);
    connect(m_importButton, &QPushButton::clicked, this, &ScanImportDialog::onImportClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ScanImportDialog::onCancelClicked);
    
    // Settings change signals
    connect(m_enableLODCheckbox, &QCheckBox::toggled, this, &ScanImportDialog::onSettingsChanged);
    connect(m_preserveColorsCheckbox, &QCheckBox::toggled, this, &ScanImportDialog::onSettingsChanged);
    connect(m_preserveIntensityCheckbox, &QCheckBox::toggled, this, &ScanImportDialog::onSettingsChanged);
    
    updateImportButton();
}

void ScanImportDialog::setupFileSelection()
{
    m_fileSelectionGroup = new QGroupBox("Select Scan Files", this);
    QVBoxLayout* layout = new QVBoxLayout(m_fileSelectionGroup);
    
    m_fileList = new QListWidget(this);
    m_fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    layout->addWidget(m_fileList);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_addFilesButton = new QPushButton("Add Files...", this);
    m_removeFileButton = new QPushButton("Remove Selected", this);
    m_clearAllButton = new QPushButton("Clear All", this);
    
    buttonLayout->addWidget(m_addFilesButton);
    buttonLayout->addWidget(m_removeFileButton);
    buttonLayout->addWidget(m_clearAllButton);
    buttonLayout->addStretch();
    
    layout->addLayout(buttonLayout);
    m_mainLayout->addWidget(m_fileSelectionGroup);
}

void ScanImportDialog::setupImportSettings()
{
    m_settingsGroup = new QGroupBox("Import Settings", this);
    QVBoxLayout* layout = new QVBoxLayout(m_settingsGroup);
    
    // LOD settings
    m_enableLODCheckbox = new QCheckBox("Enable Level of Detail (LOD)", this);
    m_enableLODCheckbox->setChecked(true);
    layout->addWidget(m_enableLODCheckbox);
    
    // Data preservation settings
    m_preserveColorsCheckbox = new QCheckBox("Preserve Color Data", this);
    m_preserveColorsCheckbox->setChecked(true);
    layout->addWidget(m_preserveColorsCheckbox);
    
    m_preserveIntensityCheckbox = new QCheckBox("Preserve Intensity Data", this);
    m_preserveIntensityCheckbox->setChecked(true);
    layout->addWidget(m_preserveIntensityCheckbox);
    
    // Max points setting
    QHBoxLayout* maxPointsLayout = new QHBoxLayout();
    maxPointsLayout->addWidget(new QLabel("Max Points per Scan:", this));
    m_maxPointsSpinBox = new QSpinBox(this);
    m_maxPointsSpinBox->setRange(10000, 10000000);
    m_maxPointsSpinBox->setValue(1000000);
    m_maxPointsSpinBox->setSuffix(" points");
    maxPointsLayout->addWidget(m_maxPointsSpinBox);
    maxPointsLayout->addStretch();
    layout->addLayout(maxPointsLayout);
    
    m_mainLayout->addWidget(m_settingsGroup);
}

void ScanImportDialog::setupProgressArea()
{
    m_progressGroup = new QGroupBox("Import Progress", this);
    QVBoxLayout* layout = new QVBoxLayout(m_progressGroup);
    
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    layout->addWidget(m_progressBar);
    
    m_progressLabel = new QLabel("Ready to import", this);
    layout->addWidget(m_progressLabel);
    
    m_currentFileLabel = new QLabel("", this);
    m_currentFileLabel->setVisible(false);
    layout->addWidget(m_currentFileLabel);
    
    m_mainLayout->addWidget(m_progressGroup);
}

QStringList ScanImportDialog::getSelectedFiles() const
{
    return m_selectedFiles;
}

ScanImportDialog::ImportSettings ScanImportDialog::getImportSettings() const
{
    ImportSettings settings;
    settings.enableLOD = m_enableLODCheckbox->isChecked();
    settings.preserveColors = m_preserveColorsCheckbox->isChecked();
    settings.preserveIntensity = m_preserveIntensityCheckbox->isChecked();
    settings.maxPointsPerScan = m_maxPointsSpinBox->value();
    return settings;
}

void ScanImportDialog::setProjectPath(const QString& projectPath)
{
    m_projectPath = projectPath;
}

void ScanImportDialog::updateProgress(int percentage, const QString& currentFile)
{
    m_progressBar->setValue(percentage);
    m_currentFileLabel->setText(QString("Processing: %1").arg(currentFile));
    m_progressLabel->setText(QString("Import progress: %1%").arg(percentage));
}

void ScanImportDialog::showImportResult(bool success, const QString& message)
{
    m_importInProgress = false;
    m_progressBar->setVisible(false);
    m_currentFileLabel->setVisible(false);
    
    if (success) {
        m_progressLabel->setText("Import completed successfully");
        QMessageBox::information(this, "Import Complete", message);
        accept();
    } else {
        m_progressLabel->setText("Import failed");
        QMessageBox::warning(this, "Import Failed", message);
    }
    
    updateImportButton();
}

void ScanImportDialog::onAddFilesClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select Scan Files",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Scan Files (*.e57 *.las *.laz *.ply *.xyz);;E57 Files (*.e57);;LAS Files (*.las *.laz);;All Files (*)"
    );
    
    for (const QString& file : files) {
        if (!m_selectedFiles.contains(file)) {
            m_selectedFiles.append(file);
            m_fileList->addItem(QDir::toNativeSeparators(file));
        }
    }
    
    updateImportButton();
}

void ScanImportDialog::onRemoveFileClicked()
{
    QList<QListWidgetItem*> selectedItems = m_fileList->selectedItems();
    for (QListWidgetItem* item : selectedItems) {
        int row = m_fileList->row(item);
        m_selectedFiles.removeAt(row);
        delete m_fileList->takeItem(row);
    }
    
    updateImportButton();
}

void ScanImportDialog::onClearAllClicked()
{
    m_selectedFiles.clear();
    m_fileList->clear();
    updateImportButton();
}

void ScanImportDialog::onImportClicked()
{
    if (m_selectedFiles.isEmpty()) {
        QMessageBox::warning(this, "No Files Selected", "Please select at least one scan file to import.");
        return;
    }
    
    m_importInProgress = true;
    m_progressBar->setVisible(true);
    m_currentFileLabel->setVisible(true);
    m_progressBar->setValue(0);
    
    updateImportButton();
    
    emit importRequested(m_selectedFiles, getImportSettings());
}

void ScanImportDialog::onCancelClicked()
{
    if (m_importInProgress) {
        emit importCancelled();
    }
    reject();
}

void ScanImportDialog::onSettingsChanged()
{
    m_currentSettings = getImportSettings();
    validateSettings();
}

void ScanImportDialog::updateImportButton()
{
    m_importButton->setEnabled(!m_selectedFiles.isEmpty() && !m_importInProgress);
    m_importButton->setText(m_importInProgress ? "Importing..." : "Import");
}

void ScanImportDialog::validateSettings()
{
    // Add any validation logic here
    // For now, all settings are valid
}
