#ifndef SCANIMPORTDIALOG_H
#define SCANIMPORTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QListWidget>
#include <QProgressBar>
#include <QCheckBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QComboBox>

/**
 * @brief Dialog for importing scans into a project
 * 
 * This dialog allows users to:
 * - Select scan files (E57, LAS, etc.)
 * - Configure import settings
 * - Monitor import progress
 * - Manage scan metadata
 */
class ScanImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScanImportDialog(QWidget *parent = nullptr);
    ~ScanImportDialog();

    /**
     * @brief Get the list of selected scan files
     * @return List of file paths to import
     */
    QStringList getSelectedFiles() const;

    /**
     * @brief Get import settings configured by user
     * @return Import configuration
     */
    struct ImportSettings {
        bool enableLOD = true;
        float lodThreshold = 0.1f;
        bool preserveColors = true;
        bool preserveIntensity = true;
        int maxPointsPerScan = 1000000;
        QString targetCoordinateSystem;
    };
    
    ImportSettings getImportSettings() const;

    /**
     * @brief Set the current project path for context
     * @param projectPath Path to the current project
     */
    void setProjectPath(const QString& projectPath);

public slots:
    /**
     * @brief Update import progress
     * @param percentage Progress percentage (0-100)
     * @param currentFile Currently processing file
     */
    void updateProgress(int percentage, const QString& currentFile);

    /**
     * @brief Show import completion status
     * @param success Whether import was successful
     * @param message Status message
     */
    void showImportResult(bool success, const QString& message);

signals:
    /**
     * @brief Emitted when user starts the import process
     * @param files List of files to import
     * @param settings Import configuration
     */
    void importRequested(const QStringList& files, const ImportSettings& settings);

    /**
     * @brief Emitted when user cancels the import
     */
    void importCancelled();

private slots:
    void onAddFilesClicked();
    void onRemoveFileClicked();
    void onClearAllClicked();
    void onImportClicked();
    void onCancelClicked();
    void onSettingsChanged();

private:
    void setupUI();
    void setupFileSelection();
    void setupImportSettings();
    void setupProgressArea();
    void updateImportButton();
    void validateSettings();

    // UI Components
    QVBoxLayout* m_mainLayout;
    
    // File selection area
    QGroupBox* m_fileSelectionGroup;
    QListWidget* m_fileList;
    QPushButton* m_addFilesButton;
    QPushButton* m_removeFileButton;
    QPushButton* m_clearAllButton;
    
    // Import settings area
    QGroupBox* m_settingsGroup;
    QCheckBox* m_enableLODCheckbox;
    QSpinBox* m_lodThresholdSpinBox;
    QCheckBox* m_preserveColorsCheckbox;
    QCheckBox* m_preserveIntensityCheckbox;
    QSpinBox* m_maxPointsSpinBox;
    QComboBox* m_coordinateSystemCombo;
    
    // Progress area
    QGroupBox* m_progressGroup;
    QProgressBar* m_progressBar;
    QLabel* m_progressLabel;
    QLabel* m_currentFileLabel;
    
    // Dialog buttons
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_importButton;
    QPushButton* m_cancelButton;
    
    // State
    QString m_projectPath;
    QStringList m_selectedFiles;
    ImportSettings m_currentSettings;
    bool m_importInProgress;
};

#endif // SCANIMPORTDIALOG_H
