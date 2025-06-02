#include "confirmationdialog.h"
#include <QIcon>
#include <QApplication>

ConfirmationDialog::ConfirmationDialog(const QString &title, const QString &message, QWidget *parent)
    : QDialog(parent)
    , m_messageLabel(nullptr)
    , m_detailsLabel(nullptr)
    , m_deleteFilesCheckbox(nullptr)
    , m_confirmButton(nullptr)
    , m_cancelButton(nullptr)
    , m_mainLayout(nullptr)
    , m_buttonLayout(nullptr)
{
    setWindowTitle(title);
    setModal(true);
    setMinimumWidth(400);
    
    setupUI();
    
    m_messageLabel->setText(message);
    
    // Connect signals
    connect(m_confirmButton, &QPushButton::clicked, this, &ConfirmationDialog::onConfirmClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &ConfirmationDialog::onCancelClicked);
}

bool ConfirmationDialog::confirm(const QString &title, const QString &message, QWidget *parent)
{
    ConfirmationDialog dialog(title, message, parent);
    return dialog.exec() == QDialog::Accepted;
}

void ConfirmationDialog::setDetailedText(const QString &details)
{
    if (!details.isEmpty()) {
        m_detailsLabel->setText(details);
        m_detailsLabel->setVisible(true);
    } else {
        m_detailsLabel->setVisible(false);
    }
}

void ConfirmationDialog::addPhysicalFileOption(const QString &optionText)
{
    if (!optionText.isEmpty()) {
        m_deleteFilesCheckbox->setText(optionText);
        m_deleteFilesCheckbox->setVisible(true);
    } else {
        m_deleteFilesCheckbox->setVisible(false);
    }
}

bool ConfirmationDialog::deletePhysicalFiles() const
{
    return m_deleteFilesCheckbox && m_deleteFilesCheckbox->isChecked();
}

void ConfirmationDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(16);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Message label
    m_messageLabel = new QLabel(this);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setStyleSheet("QLabel { font-size: 14px; }");
    m_mainLayout->addWidget(m_messageLabel);
    
    // Details label (initially hidden)
    m_detailsLabel = new QLabel(this);
    m_detailsLabel->setWordWrap(true);
    m_detailsLabel->setStyleSheet("QLabel { font-size: 12px; color: #666666; margin-top: 8px; }");
    m_detailsLabel->setVisible(false);
    m_mainLayout->addWidget(m_detailsLabel);
    
    // Physical file deletion checkbox (initially hidden)
    m_deleteFilesCheckbox = new QCheckBox(this);
    m_deleteFilesCheckbox->setStyleSheet("QCheckBox { font-size: 12px; margin-top: 8px; }");
    m_deleteFilesCheckbox->setVisible(false);
    m_mainLayout->addWidget(m_deleteFilesCheckbox);
    
    // Add stretch to push buttons to bottom
    m_mainLayout->addStretch();
    
    // Button layout
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setSpacing(8);
    
    // Add stretch to right-align buttons
    m_buttonLayout->addStretch();
    
    // Cancel button
    m_cancelButton = new QPushButton("Cancel", this);
    m_cancelButton->setMinimumWidth(80);
    m_cancelButton->setStyleSheet(R"(
        QPushButton {
            padding: 8px 16px;
            font-size: 12px;
            border: 1px solid #cccccc;
            border-radius: 4px;
            background-color: #f5f5f5;
        }
        QPushButton:hover {
            background-color: #e5e5e5;
        }
        QPushButton:pressed {
            background-color: #d5d5d5;
        }
    )");
    m_buttonLayout->addWidget(m_cancelButton);
    
    // Confirm button
    m_confirmButton = new QPushButton("Confirm", this);
    m_confirmButton->setMinimumWidth(80);
    m_confirmButton->setDefault(true);
    m_confirmButton->setStyleSheet(R"(
        QPushButton {
            padding: 8px 16px;
            font-size: 12px;
            border: 1px solid #d32f2f;
            border-radius: 4px;
            background-color: #f44336;
            color: white;
        }
        QPushButton:hover {
            background-color: #d32f2f;
        }
        QPushButton:pressed {
            background-color: #b71c1c;
        }
    )");
    m_buttonLayout->addWidget(m_confirmButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Set dialog styling
    setStyleSheet(R"(
        QDialog {
            background-color: white;
            border: 1px solid #cccccc;
        }
    )");
}

void ConfirmationDialog::onConfirmClicked()
{
    accept();
}

void ConfirmationDialog::onCancelClicked()
{
    reject();
}
