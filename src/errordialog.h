#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QApplication>
#include <QClipboard>

namespace SceneRegistration {

enum class ErrorSeverity {
    Information,
    Warning,
    Critical,
    Fatal
};

struct ErrorDetails {
    QString title;
    QString message;
    QString technicalDetails;
    ErrorSeverity severity;
    QStringList suggestedActions;
};

class ErrorDialog : public QDialog {
    Q_OBJECT

public:
    explicit ErrorDialog(QWidget* parent = nullptr);
    
    // Static convenience methods for Sprint 3.1 specific errors
    static void showProjectLoadError(QWidget* parent, const QString& projectPath, 
                                   const QString& error, const QString& details = QString());
    static void showProjectSaveError(QWidget* parent, const QString& error, 
                                   const QString& details = QString());
    static void showCorruptedFileError(QWidget* parent, const QString& fileName, 
                                     const QString& suggestedAction = QString());
    static void showMissingFileError(QWidget* parent, const QString& fileName, 
                                   const QString& originalPath);
    
    // General error display
    static void showError(QWidget* parent, const ErrorDetails& details);

private slots:
    void onShowDetailsToggled(bool show);
    void onCopyToClipboard();

private:
    void setupUI();
    void setErrorDetails(const ErrorDetails& details);
    void updateIconForSeverity(ErrorSeverity severity);
    
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QLabel* m_messageLabel;
    QTextEdit* m_detailsText;
    QCheckBox* m_showDetailsCheck;
    QPushButton* m_okButton;
    QPushButton* m_copyButton;
    
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;
    QHBoxLayout* m_headerLayout;
};

} // namespace SceneRegistration

#endif // ERRORDIALOG_H
