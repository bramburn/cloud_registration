#include "relinkfiledialog.h"
#include <QMessageBox>
#include <QDir>

namespace SceneRegistration {

RelinkFileDialog::RelinkFileDialog(QWidget* parent)
    : QDialog(parent)
    , m_instructionLabel(nullptr)
    , m_scanNameLabel(nullptr)
    , m_originalPathLabel(nullptr)
    , m_newPathLabel(nullptr)
    , m_pathEdit(nullptr)
    , m_browseButton(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_validationLabel(nullptr)
    , m_mainLayout(nullptr)
    , m_pathLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_isValidPath(false)
{
    setupUI();
}

void RelinkFileDialog::setupUI() {
    setWindowTitle("Relink Scan File");
    setModal(true);
    setMinimumWidth(500);
    
    m_mainLayout = new QVBoxLayout(this);
    
    // Instruction label
    m_instructionLabel = new QLabel("The original scan file could not be found. Please locate the file in its new location.");
    m_instructionLabel->setWordWrap(true);
    m_instructionLabel->setStyleSheet("QLabel { color: #666; margin-bottom: 10px; }");
    m_mainLayout->addWidget(m_instructionLabel);
    
    // Scan name
    m_scanNameLabel = new QLabel();
    QFont boldFont = m_scanNameLabel->font();
    boldFont.setBold(true);
    m_scanNameLabel->setFont(boldFont);
    m_mainLayout->addWidget(m_scanNameLabel);
    
    // Original path
    m_originalPathLabel = new QLabel();
    m_originalPathLabel->setWordWrap(true);
    m_originalPathLabel->setStyleSheet("QLabel { color: #888; font-style: italic; margin-bottom: 15px; }");
    m_mainLayout->addWidget(m_originalPathLabel);
    
    // New path selection
    m_newPathLabel = new QLabel("New file location:");
    m_mainLayout->addWidget(m_newPathLabel);
    
    m_pathLayout = new QHBoxLayout();
    
    m_pathEdit = new QLineEdit();
    m_pathEdit->setPlaceholderText("Select the new location of the scan file...");
    connect(m_pathEdit, &QLineEdit::textChanged, this, &RelinkFileDialog::onPathChanged);
    m_pathLayout->addWidget(m_pathEdit);
    
    m_browseButton = new QPushButton("Browse...");
    connect(m_browseButton, &QPushButton::clicked, this, &RelinkFileDialog::onBrowseClicked);
    m_pathLayout->addWidget(m_browseButton);
    
    m_mainLayout->addLayout(m_pathLayout);
    
    // Validation label
    m_validationLabel = new QLabel();
    m_validationLabel->setWordWrap(true);
    m_validationLabel->setVisible(false);
    m_mainLayout->addWidget(m_validationLabel);
    
    // Spacer
    m_mainLayout->addStretch();
    
    // Button layout
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->addStretch();
    
    m_cancelButton = new QPushButton("Cancel");
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    m_buttonLayout->addWidget(m_cancelButton);
    
    m_okButton = new QPushButton("OK");
    m_okButton->setDefault(true);
    m_okButton->setEnabled(false);
    connect(m_okButton, &QPushButton::clicked, this, &RelinkFileDialog::onAccept);
    m_buttonLayout->addWidget(m_okButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

void RelinkFileDialog::setScanInfo(const QString& scanName, const QString& originalPath) {
    m_scanName = scanName;
    m_originalPath = originalPath;
    
    m_scanNameLabel->setText(QString("Scan: %1").arg(scanName));
    m_originalPathLabel->setText(QString("Original location: %1").arg(originalPath));
}

QString RelinkFileDialog::getSelectedFilePath() const {
    return m_selectedPath;
}

void RelinkFileDialog::onBrowseClicked() {
    QString filter = "Point Cloud Files (*.e57 *.las *.laz *.ply *.pcd);;All Files (*.*)";
    QString fileName = QFileDialog::getOpenFileName(this, "Locate Scan File", 
                                                   QDir::homePath(), filter);
    
    if (!fileName.isEmpty()) {
        m_pathEdit->setText(fileName);
    }
}

void RelinkFileDialog::onPathChanged() {
    updateValidation();
}

void RelinkFileDialog::updateValidation() {
    QString path = m_pathEdit->text().trimmed();
    m_isValidPath = false;
    
    if (path.isEmpty()) {
        m_validationLabel->setVisible(false);
        m_okButton->setEnabled(false);
        return;
    }
    
    QFileInfo fileInfo(path);
    
    if (!fileInfo.exists()) {
        m_validationLabel->setText("⚠ File does not exist");
        m_validationLabel->setStyleSheet("QLabel { color: #d32f2f; }");
        m_validationLabel->setVisible(true);
        m_okButton->setEnabled(false);
        return;
    }
    
    if (!fileInfo.isFile()) {
        m_validationLabel->setText("⚠ Path is not a file");
        m_validationLabel->setStyleSheet("QLabel { color: #d32f2f; }");
        m_validationLabel->setVisible(true);
        m_okButton->setEnabled(false);
        return;
    }
    
    if (!fileInfo.isReadable()) {
        m_validationLabel->setText("⚠ File is not readable");
        m_validationLabel->setStyleSheet("QLabel { color: #d32f2f; }");
        m_validationLabel->setVisible(true);
        m_okButton->setEnabled(false);
        return;
    }
    
    // Check if it's a supported file type
    QString suffix = fileInfo.suffix().toLower();
    QStringList supportedFormats = {"e57", "las", "laz", "ply", "pcd"};
    
    if (!supportedFormats.contains(suffix)) {
        m_validationLabel->setText("⚠ File format may not be supported");
        m_validationLabel->setStyleSheet("QLabel { color: #ff9800; }");
        m_validationLabel->setVisible(true);
    } else {
        m_validationLabel->setText("✓ File is valid and accessible");
        m_validationLabel->setStyleSheet("QLabel { color: #388e3c; }");
        m_validationLabel->setVisible(true);
    }
    
    m_isValidPath = true;
    m_okButton->setEnabled(true);
}

void RelinkFileDialog::onAccept() {
    if (!m_isValidPath) {
        QMessageBox::warning(this, "Invalid File", 
                           "Please select a valid, accessible file.");
        return;
    }
    
    m_selectedPath = QDir::toNativeSeparators(m_pathEdit->text().trimmed());
    accept();
}

QString RelinkFileDialog::relinkScanFile(QWidget* parent, const QString& scanName, 
                                        const QString& originalPath) {
    RelinkFileDialog dialog(parent);
    dialog.setScanInfo(scanName, originalPath);
    
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getSelectedFilePath();
    }
    
    return QString();
}

} // namespace SceneRegistration
