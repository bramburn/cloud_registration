#include "createprojectdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

CreateProjectDialog::CreateProjectDialog(QWidget *parent)
    : QDialog(parent)
    , m_nameEdit(nullptr)
    , m_pathEdit(nullptr)
    , m_browseBtn(nullptr)
    , m_okBtn(nullptr)
    , m_cancelBtn(nullptr)
    , m_errorLabel(nullptr)
{
    setupUI();
    
    // Set default path to Documents folder
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    m_pathEdit->setText(defaultPath);
    
    validateInput();
}

void CreateProjectDialog::setupUI()
{
    setWindowTitle("Create New Project");
    setModal(true);
    resize(500, 200);
    
    auto *mainLayout = new QVBoxLayout(this);
    
    // Form layout for inputs
    auto *formLayout = new QFormLayout();
    
    // Project name input
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Enter project name...");
    formLayout->addRow("Project Name:", m_nameEdit);
    
    // Project path input with browse button
    auto *pathLayout = new QHBoxLayout();
    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setPlaceholderText("Select project location...");
    m_browseBtn = new QPushButton("Browse...", this);
    m_browseBtn->setMaximumWidth(80);
    
    pathLayout->addWidget(m_pathEdit);
    pathLayout->addWidget(m_browseBtn);
    formLayout->addRow("Location:", pathLayout);
    
    // Error label
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: red; font-size: 12px;");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();
    
    // Button layout
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_okBtn = new QPushButton("Create", this);
    m_okBtn->setDefault(true);
    m_okBtn->setMinimumWidth(80);
    
    m_cancelBtn = new QPushButton("Cancel", this);
    m_cancelBtn->setMinimumWidth(80);
    
    buttonLayout->addWidget(m_okBtn);
    buttonLayout->addWidget(m_cancelBtn);
    
    // Add to main layout
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_errorLabel);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_browseBtn, &QPushButton::clicked, this, &CreateProjectDialog::onBrowseClicked);
    connect(m_okBtn, &QPushButton::clicked, this, &CreateProjectDialog::onAcceptClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_nameEdit, &QLineEdit::textChanged, this, &CreateProjectDialog::onNameChanged);
    connect(m_pathEdit, &QLineEdit::textChanged, this, &CreateProjectDialog::onPathChanged);
    
    // Style the dialog
    setStyleSheet(R"(
        QDialog {
            background-color: #f5f5f5;
        }
        QLineEdit {
            padding: 8px;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-size: 14px;
        }
        QLineEdit:focus {
            border-color: #4CAF50;
        }
        QPushButton {
            padding: 8px 16px;
            border: none;
            border-radius: 4px;
            font-size: 14px;
        }
        QPushButton#okBtn {
            background-color: #4CAF50;
            color: white;
        }
        QPushButton#okBtn:hover {
            background-color: #45a049;
        }
        QPushButton#okBtn:disabled {
            background-color: #cccccc;
            color: #666666;
        }
        QPushButton#cancelBtn {
            background-color: #f44336;
            color: white;
        }
        QPushButton#cancelBtn:hover {
            background-color: #da190b;
        }
        QPushButton#browseBtn {
            background-color: #2196F3;
            color: white;
        }
        QPushButton#browseBtn:hover {
            background-color: #1976D2;
        }
    )");
    
    m_okBtn->setObjectName("okBtn");
    m_cancelBtn->setObjectName("cancelBtn");
    m_browseBtn->setObjectName("browseBtn");
}

void CreateProjectDialog::onBrowseClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this,
        "Select Project Location",
        m_pathEdit->text(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    
    if (!dir.isEmpty()) {
        m_pathEdit->setText(dir);
    }
}

void CreateProjectDialog::onAcceptClicked()
{
    QString name = m_nameEdit->text().trimmed();
    QString path = m_pathEdit->text().trimmed();
    
    // Final validation
    if (name.isEmpty()) {
        m_errorLabel->setText("Project name cannot be empty.");
        m_errorLabel->show();
        m_nameEdit->setFocus();
        return;
    }
    
    if (path.isEmpty()) {
        m_errorLabel->setText("Project location cannot be empty.");
        m_errorLabel->show();
        m_pathEdit->setFocus();
        return;
    }
    
    // Check if directory exists and is writable
    QDir dir(path);
    if (!dir.exists()) {
        m_errorLabel->setText("Selected directory does not exist.");
        m_errorLabel->show();
        m_pathEdit->setFocus();
        return;
    }
    
    QFileInfo dirInfo(path);
    if (!dirInfo.isWritable()) {
        m_errorLabel->setText("You don't have write permission to the selected directory.");
        m_errorLabel->show();
        m_pathEdit->setFocus();
        return;
    }
    
    // Check for invalid characters in project name
    QString invalidChars = "<>:\"/\\|?*";
    for (const QChar &ch : invalidChars) {
        if (name.contains(ch)) {
            m_errorLabel->setText(QString("Project name contains invalid character: '%1'").arg(ch));
            m_errorLabel->show();
            m_nameEdit->setFocus();
            return;
        }
    }
    
    accept();
}

void CreateProjectDialog::onNameChanged()
{
    validateInput();
}

void CreateProjectDialog::onPathChanged()
{
    validateInput();
}

void CreateProjectDialog::validateInput()
{
    QString name = m_nameEdit->text().trimmed();
    QString path = m_pathEdit->text().trimmed();
    
    bool isValid = !name.isEmpty() && !path.isEmpty();
    
    m_okBtn->setEnabled(isValid);
    
    if (isValid) {
        m_errorLabel->hide();
    }
}
