#include "scanimportdialog.h"
#include "scanimportmanager.h"
#include "projectmanager.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QStyle>

ScanImportDialog::ScanImportDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Import Scan Files");
    setModal(true);
    resize(600, 500);
    
    setupUI();
    setupConnections();
    updateUI();
}

void ScanImportDialog::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    
    // Info section
    m_infoLabel = new QLabel("Select scan files (.las, .e57) to import into your project:", this);
    m_infoLabel->setWordWrap(true);
    
    // Target location display
    m_targetLabel = new QLabel(this);
    m_targetLabel->setStyleSheet("font-style: italic; color: #666;");
    m_targetLabel->setWordWrap(true);
    
    // File selection section
    auto *fileGroup = new QGroupBox("Selected Files", this);
    auto *fileLayout = new QVBoxLayout(fileGroup);
    
    m_fileList = new QListWidget(this);
    m_fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_fileList->setMinimumHeight(200);
    
    auto *fileButtonLayout = new QHBoxLayout();
    m_browseButton = new QPushButton("Browse Files...", this);
    m_removeButton = new QPushButton("Remove Selected", this);
    m_removeButton->setEnabled(false);
    
    fileButtonLayout->addWidget(m_browseButton);
    fileButtonLayout->addWidget(m_removeButton);
    fileButtonLayout->addStretch();
    
    fileLayout->addWidget(m_fileList);
    fileLayout->addLayout(fileButtonLayout);
    
    // Import mode section
    auto *modeGroup = new QGroupBox("Import Mode", this);
    auto *modeLayout = new QVBoxLayout(modeGroup);
    
    m_copyModeRadio = new QRadioButton("Copy to Project Folder", this);
    m_copyModeRadio->setToolTip("Files are copied to the project. Original files remain in their location.");
    m_copyModeRadio->setChecked(true); // Default to copy mode
    
    m_moveModeRadio = new QRadioButton("Move to Project Folder", this);
    m_moveModeRadio->setToolTip("Files are moved to the project. Original files are removed from their location.");
    
    modeLayout->addWidget(m_copyModeRadio);
    modeLayout->addWidget(m_moveModeRadio);
    
    // Dialog buttons
    auto *buttonLayout = new QHBoxLayout();
    m_okButton = new QPushButton("Import", this);
    m_okButton->setDefault(true);
    m_okButton->setEnabled(false);
    
    m_cancelButton = new QPushButton("Cancel", this);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    // Assembly
    mainLayout->addWidget(m_infoLabel);
    mainLayout->addWidget(m_targetLabel);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(fileGroup);
    mainLayout->addWidget(modeGroup);
    mainLayout->addLayout(buttonLayout);
}

void ScanImportDialog::setupConnections()
{
    connect(m_browseButton, &QPushButton::clicked, this, &ScanImportDialog::browseFiles);
    connect(m_removeButton, &QPushButton::clicked, this, &ScanImportDialog::removeSelectedFiles);
    connect(m_fileList, &QListWidget::itemSelectionChanged, this, &ScanImportDialog::updateUI);
    connect(m_okButton, &QPushButton::clicked, this, &ScanImportDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ScanImportDialog::browseFiles()
{
    QStringList extensions = ScanImportManager::getSupportedExtensions();
    QStringList filters;
    filters << "Scan Files (*.las *.e57)";
    filters << "LAS Files (*.las)";
    filters << "E57 Files (*.e57)";
    filters << "All Files (*)";
    
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select Scan Files",
        QString(),
        filters.join(";;")
    );
    
    if (!files.isEmpty()) {
        addFilesToList(files);
    }
}

void ScanImportDialog::addFilesToList(const QStringList &files)
{
    QStringList validFiles;
    QStringList invalidFiles;
    
    for (const QString &file : files) {
        if (!m_selectedFiles.contains(file)) {
            if (ScanImportManager::isValidScanFile(file)) {
                m_selectedFiles.append(file);
                validFiles.append(file);
                
                auto *item = new QListWidgetItem(QFileInfo(file).fileName(), m_fileList);
                item->setData(Qt::UserRole, file);
                item->setToolTip(file);
            } else {
                invalidFiles.append(file);
            }
        }
    }
    
    if (!invalidFiles.isEmpty()) {
        QMessageBox::warning(this, "Invalid Files",
            QString("The following files are not supported scan formats:\n\n%1")
            .arg(invalidFiles.join("\n")));
    }
    
    updateUI();
}

void ScanImportDialog::removeSelectedFiles()
{
    auto selectedItems = m_fileList->selectedItems();
    
    for (auto *item : selectedItems) {
        QString filePath = item->data(Qt::UserRole).toString();
        m_selectedFiles.removeAll(filePath);
        delete item;
    }
    
    updateUI();
}

void ScanImportDialog::updateUI()
{
    bool hasFiles = !m_selectedFiles.isEmpty();
    bool hasSelection = !m_fileList->selectedItems().isEmpty();
    
    m_okButton->setEnabled(hasFiles);
    m_removeButton->setEnabled(hasSelection);
    
    // Update file count display
    if (hasFiles) {
        m_fileList->setToolTip(QString("%1 files selected for import").arg(m_selectedFiles.size()));
    } else {
        m_fileList->setToolTip("No files selected");
    }
}

void ScanImportDialog::setProjectPath(const QString &projectPath)
{
    m_projectPath = projectPath;
    
    QString scansPath = ProjectManager::getScansSubfolder(projectPath);
    m_targetLabel->setText(QString("Files will be imported to: %1").arg(scansPath));
}

ImportMode ScanImportDialog::importMode() const
{
    return m_copyModeRadio->isChecked() ? ImportMode::Copy : ImportMode::Move;
}

void ScanImportDialog::accept()
{
    if (validateSelection()) {
        QDialog::accept();
    }
}

bool ScanImportDialog::validateSelection()
{
    if (m_selectedFiles.isEmpty()) {
        QMessageBox::warning(this, "No Files Selected", 
                           "Please select at least one scan file to import.");
        return false;
    }
    
    // Check if all selected files still exist
    QStringList missingFiles;
    for (const QString &file : m_selectedFiles) {
        if (!QFileInfo::exists(file)) {
            missingFiles.append(file);
        }
    }
    
    if (!missingFiles.isEmpty()) {
        QMessageBox::warning(this, "Missing Files",
            QString("The following files no longer exist:\n\n%1\n\nPlease remove them and try again.")
            .arg(missingFiles.join("\n")));
        return false;
    }
    
    // Warn about move operation
    if (importMode() == ImportMode::Move) {
        auto reply = QMessageBox::question(this, "Confirm Move Operation",
            "You have selected to MOVE files to the project folder.\n"
            "This will remove the original files from their current location.\n\n"
            "Do you want to continue?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            
        if (reply != QMessageBox::Yes) {
            return false;
        }
    }
    
    return true;
}
