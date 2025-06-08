#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QProgressBar>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include "../export/IFormatWriter.h"

/**
 * @brief Export options structure
 */
struct ExportOptions {
    QString outputPath;
    QString format;             // "e57", "las", "ply", "xyz"
    
    // Data options
    bool includeColor = true;
    bool includeIntensity = true;
    bool includeNormals = false;
    
    // Coordinate system
    QString sourceCRS = "EPSG:4326";
    QString targetCRS = "EPSG:4326";
    bool transformCoordinates = false;
    
    // Format-specific options
    QVariantMap formatOptions;
    
    // Processing options
    bool enableSubsampling = false;
    double subsamplingRatio = 1.0;
    bool enableFiltering = false;
    double filterRadius = 0.1;
    
    // Quality options
    int precision = 6;          // Decimal places for coordinates
    QString separator = " ";    // For text formats
    bool writeHeader = true;
    
    // Project information
    QString projectName;
    QString description;
    QString coordinateSystem;
};

/**
 * @brief Export Dialog for Point Cloud Data
 * 
 * Sprint 6 User Story 1: Multi-Format Point Cloud Export
 * Provides user-friendly interface for configuring and executing point cloud exports.
 */
class ExportDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ExportDialog(QWidget* parent = nullptr);
    ~ExportDialog() override;
    
    // Configuration
    void setPointCloudData(const std::vector<Point>& points);
    void setProjectInfo(const QString& name, const QString& description);
    void setAvailableFormats(const QStringList& formats);
    void setAvailableCRS(const QStringList& crsList);
    
    // Results
    ExportOptions getExportOptions() const;
    QString getSelectedFormat() const;
    QString getOutputPath() const;
    
    // State management
    void resetToDefaults();
    void loadSettings();
    void saveSettings();
    
public slots:
    void accept() override;
    void reject() override;
    
private slots:
    void onBrowseClicked();
    void onFormatChanged();
    void onCRSChanged();
    void onPreviewClicked();
    void onAdvancedToggled(bool enabled);
    void onTransformCoordsToggled(bool enabled);
    void updateEstimatedSize();
    void validateInput();
    
signals:
    void exportRequested(const ExportOptions& options);
    void previewRequested(const ExportOptions& options);
    
private:
    void setupUI();
    void setupBasicOptions();
    void setupFormatOptions();
    void setupCoordinateOptions();
    void setupAdvancedOptions();
    void setupButtons();
    
    void updateFormatSpecificOptions();
    void updateCoordinateSystemOptions();
    void updateEstimatedFileSize();
    QString formatFileSize(qint64 bytes) const;
    
    // UI Components
    QVBoxLayout* m_mainLayout;
    
    // Basic options
    QGroupBox* m_basicGroup;
    QLineEdit* m_pathEdit;
    QPushButton* m_browseButton;
    QComboBox* m_formatCombo;
    QLabel* m_estimatedSizeLabel;
    
    // Data options
    QGroupBox* m_dataGroup;
    QCheckBox* m_includeColorCheck;
    QCheckBox* m_includeIntensityCheck;
    QCheckBox* m_includeNormalsCheck;
    
    // Coordinate system options
    QGroupBox* m_coordinateGroup;
    QComboBox* m_sourceCRSCombo;
    QComboBox* m_targetCRSCombo;
    QCheckBox* m_transformCoordsCheck;
    QLabel* m_crsWarningLabel;
    
    // Format-specific options
    QGroupBox* m_formatGroup;
    QWidget* m_formatOptionsWidget;
    QVBoxLayout* m_formatOptionsLayout;
    
    // Advanced options
    QGroupBox* m_advancedGroup;
    QCheckBox* m_advancedToggle;
    QCheckBox* m_enableSubsamplingCheck;
    QDoubleSpinBox* m_subsamplingRatioSpin;
    QCheckBox* m_enableFilteringCheck;
    QDoubleSpinBox* m_filterRadiusSpin;
    QSpinBox* m_precisionSpin;
    QLineEdit* m_separatorEdit;
    QCheckBox* m_writeHeaderCheck;
    
    // Progress and status
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    
    // Buttons
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_previewButton;
    QPushButton* m_exportButton;
    QPushButton* m_cancelButton;
    
    // Data
    std::vector<Point> m_pointCloudData;
    QString m_projectName;
    QString m_projectDescription;
    QStringList m_availableFormats;
    QStringList m_availableCRS;
    
    // State
    bool m_isExporting = false;
    qint64 m_estimatedFileSize = 0;
};

#endif // EXPORTDIALOG_H
