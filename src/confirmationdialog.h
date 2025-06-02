#ifndef CONFIRMATIONDIALOG_H
#define CONFIRMATIONDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ConfirmationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfirmationDialog(const QString &title, const QString &message, 
                               QWidget *parent = nullptr);
    
    // Static convenience method for simple confirmations
    static bool confirm(const QString &title, const QString &message, 
                       QWidget *parent = nullptr);
    
    // Add detailed text and physical file deletion option
    void setDetailedText(const QString &details);
    void addPhysicalFileOption(const QString &optionText);
    bool deletePhysicalFiles() const;

private slots:
    void onConfirmClicked();
    void onCancelClicked();

private:
    void setupUI();
    
    QLabel *m_messageLabel;
    QLabel *m_detailsLabel;
    QCheckBox *m_deleteFilesCheckbox;
    QPushButton *m_confirmButton;
    QPushButton *m_cancelButton;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_buttonLayout;
};

#endif // CONFIRMATIONDIALOG_H
