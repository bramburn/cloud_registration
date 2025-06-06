#include "errordialog.h"
#include <QStyle>
#include <QApplication>
#include <QClipboard>
#include <QDebug>

namespace SceneRegistration {

ErrorDialog::ErrorDialog(QWidget* parent)
    : QDialog(parent)
    , m_iconLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_messageLabel(nullptr)
    , m_detailsText(nullptr)
    , m_showDetailsCheck(nullptr)
    , m_okButton(nullptr)
    , m_copyButton(nullptr)
    , m_mainLayout(nullptr)
    , m_buttonLayout(nullptr)
    , m_headerLayout(nullptr)
{
    setupUI();
}

void ErrorDialog::setupUI() {
    setWindowTitle("Error");
    setModal(true);
    setMinimumWidth(400);
    
    m_mainLayout = new QVBoxLayout(this);
    
    // Header layout with icon and title
    m_headerLayout = new QHBoxLayout();
    
    m_iconLabel = new QLabel();
    m_iconLabel->setFixedSize(32, 32);
    m_iconLabel->setScaledContents(true);
    m_headerLayout->addWidget(m_iconLabel);
    
    m_titleLabel = new QLabel();
    m_titleLabel->setWordWrap(true);
    QFont titleFont = m_titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 2);
    m_titleLabel->setFont(titleFont);
    m_headerLayout->addWidget(m_titleLabel, 1);
    
    m_mainLayout->addLayout(m_headerLayout);
    
    // Message label
    m_messageLabel = new QLabel();
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setMargin(10);
    m_mainLayout->addWidget(m_messageLabel);
    
    // Show details checkbox
    m_showDetailsCheck = new QCheckBox("Show technical details");
    connect(m_showDetailsCheck, &QCheckBox::toggled, this, &ErrorDialog::onShowDetailsToggled);
    m_mainLayout->addWidget(m_showDetailsCheck);
    
    // Details text (initially hidden)
    m_detailsText = new QTextEdit();
    m_detailsText->setReadOnly(true);
    m_detailsText->setMaximumHeight(150);
    m_detailsText->setVisible(false);
    m_mainLayout->addWidget(m_detailsText);
    
    // Button layout
    m_buttonLayout = new QHBoxLayout();
    
    m_copyButton = new QPushButton("Copy to Clipboard");
    connect(m_copyButton, &QPushButton::clicked, this, &ErrorDialog::onCopyToClipboard);
    m_copyButton->setVisible(false);
    m_buttonLayout->addWidget(m_copyButton);
    
    m_buttonLayout->addStretch();
    
    m_okButton = new QPushButton("OK");
    m_okButton->setDefault(true);
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    m_buttonLayout->addWidget(m_okButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

void ErrorDialog::setErrorDetails(const ErrorDetails& details) {
    m_titleLabel->setText(details.title);
    m_messageLabel->setText(details.message);
    
    if (!details.technicalDetails.isEmpty()) {
        QString detailsText = details.technicalDetails;
        if (!details.suggestedActions.isEmpty()) {
            detailsText += "\n\nSuggested actions:\n";
            for (const QString& action : details.suggestedActions) {
                detailsText += "• " + action + "\n";
            }
        }
        m_detailsText->setPlainText(detailsText);
        m_showDetailsCheck->setVisible(true);
    } else {
        m_showDetailsCheck->setVisible(false);
        m_detailsText->setVisible(false);
    }
    
    updateIconForSeverity(details.severity);
}

void ErrorDialog::updateIconForSeverity(ErrorSeverity severity) {
    QStyle::StandardPixmap iconType = QStyle::SP_MessageBoxInformation; // Default value

    switch (severity) {
        case ErrorSeverity::Information:
            iconType = QStyle::SP_MessageBoxInformation;
            break;
        case ErrorSeverity::Warning:
            iconType = QStyle::SP_MessageBoxWarning;
            break;
        case ErrorSeverity::Critical:
            iconType = QStyle::SP_MessageBoxCritical;
            break;
        case ErrorSeverity::Fatal:
            iconType = QStyle::SP_MessageBoxCritical;
            break;
        default:
            iconType = QStyle::SP_MessageBoxInformation;
            break;
    }
    
    QIcon icon = style()->standardIcon(iconType);
    m_iconLabel->setPixmap(icon.pixmap(32, 32));
}

void ErrorDialog::onShowDetailsToggled(bool show) {
    m_detailsText->setVisible(show);
    m_copyButton->setVisible(show);
    
    if (show) {
        resize(width(), sizeHint().height());
    } else {
        resize(width(), minimumSizeHint().height());
    }
}

void ErrorDialog::onCopyToClipboard() {
    QString clipboardText = m_titleLabel->text() + "\n\n";
    clipboardText += m_messageLabel->text() + "\n\n";
    clipboardText += "Technical Details:\n" + m_detailsText->toPlainText();
    
    QApplication::clipboard()->setText(clipboardText);
}

// Static convenience methods
void ErrorDialog::showProjectLoadError(QWidget* parent, const QString& projectPath, 
                                     const QString& error, const QString& details) {
    ErrorDetails errorDetails;
    errorDetails.title = "Project Load Error";
    errorDetails.message = QString("Failed to load project from:\n%1\n\nError: %2").arg(projectPath, error);
    errorDetails.technicalDetails = details;
    errorDetails.severity = ErrorSeverity::Critical;
    errorDetails.suggestedActions = {
        "Check if the project directory exists and is accessible",
        "Verify that project files are not corrupted",
        "Try opening a different project"
    };
    
    showError(parent, errorDetails);
}

void ErrorDialog::showProjectSaveError(QWidget* parent, const QString& error, const QString& details) {
    ErrorDetails errorDetails;
    errorDetails.title = "Project Save Error";
    errorDetails.message = QString("Failed to save project.\n\nError: %1").arg(error);
    errorDetails.technicalDetails = details;
    errorDetails.severity = ErrorSeverity::Critical;
    errorDetails.suggestedActions = {
        "Check if you have write permissions to the project directory",
        "Ensure there is sufficient disk space",
        "Try saving to a different location"
    };
    
    showError(parent, errorDetails);
}

void ErrorDialog::showCorruptedFileError(QWidget* parent, const QString& fileName, const QString& suggestedAction) {
    ErrorDetails errorDetails;
    errorDetails.title = "Corrupted File";
    errorDetails.message = QString("The file '%1' is corrupted or unreadable.\n\nThe project cannot be opened.").arg(fileName);
    errorDetails.severity = ErrorSeverity::Critical;
    
    if (!suggestedAction.isEmpty()) {
        errorDetails.suggestedActions = { suggestedAction };
    } else {
        errorDetails.suggestedActions = {
            "Check if the file exists and is not corrupted",
            "Try restoring from a backup if available",
            "Contact support if the problem persists"
        };
    }
    
    showError(parent, errorDetails);
}

void ErrorDialog::showMissingFileError(QWidget* parent, const QString& fileName, const QString& originalPath) {
    ErrorDetails errorDetails;
    errorDetails.title = "Missing File";
    errorDetails.message = QString("The linked file '%1' could not be found at its original location:\n%2").arg(fileName, originalPath);
    errorDetails.severity = ErrorSeverity::Warning;
    errorDetails.suggestedActions = {
        "Use 'Relink Scan File...' to locate the file in its new location",
        "Use 'Remove Missing Scan Reference' to remove the reference from the project",
        "Check if the file was moved or renamed"
    };
    
    showError(parent, errorDetails);
}

void ErrorDialog::showError(QWidget* parent, const ErrorDetails& details) {
    ErrorDialog dialog(parent);
    dialog.setErrorDetails(details);
    dialog.exec();
}

} // namespace SceneRegistration
