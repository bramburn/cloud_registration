#ifndef LOADINGSETTINGSDIALOG_H
#define LOADINGSETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include "loadingsettings.h"

class LoadingSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingSettingsDialog(QWidget *parent = nullptr);
    ~LoadingSettingsDialog();

    // Public method to retrieve the currently configured loading settings
    LoadingSettings getSettings() const;

private slots:
    // Slots for button clicks and combo box changes
    void onApplyClicked();
    void onOkClicked();
    void onCancelClicked();
    void onMethodChanged(int index);

private:
    // Private helper methods for managing settings persistence
    void loadSettings();
    void saveSettings();
    void updateUIForMethod(LoadingMethod method);
    void setupUI();

    // UI Components
    QComboBox *m_methodComboBox;
    QPushButton *m_applyButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_buttonLayout;
    QLabel *m_methodLabel;

    // Internal state
    LoadingSettings m_currentSettings;
    QSettings m_qSettings;
};

#endif // LOADINGSETTINGSDIALOG_H
