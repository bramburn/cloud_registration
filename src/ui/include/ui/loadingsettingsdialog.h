#ifndef LOADINGSETTINGSDIALOG_H
#define LOADINGSETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QCheckBox>
#include "core/loadingsettings.h"

class LoadingSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingSettingsDialog(QWidget *parent = nullptr);
    ~LoadingSettingsDialog();

    // Public method to retrieve the currently configured loading settings
    LoadingSettings getSettings() const;
    void setSettings(const LoadingSettings& settings);

    // Task 1.4.3.4: E57/LAS compatibility handling
    void configureForFileType(const QString& fileExtension);

private slots:
    // Slots for button clicks and combo box changes
    void onApplyClicked();
    void onOkClicked();
    void onCancelClicked();
    void onMethodChanged(int index);
    void onVoxelSettingsChanged();

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

    // Voxel Grid specific controls
    QGroupBox *m_voxelParametersGroup;
    QVBoxLayout *m_voxelParametersLayout;
    QLabel *m_leafSizeLabel;
    QDoubleSpinBox *m_leafSizeSpinBox;
    QLabel *m_minPointsLabel;
    QSpinBox *m_minPointsSpinBox;

    // File type specific controls (Sprint 1.4)
    QGroupBox *m_e57Group;
    QGroupBox *m_lasGroup;
    QCheckBox *m_e57TransformCheck;
    QCheckBox *m_e57LoadColorsCheck;
    QCheckBox *m_lasValidateCheck;
    QCheckBox *m_lasLoadIntensityCheck;
    QCheckBox *m_lasLoadColorsCheck;

    // Internal state
    LoadingSettings m_currentSettings;
    QSettings m_qSettings;
};

#endif // LOADINGSETTINGSDIALOG_H
