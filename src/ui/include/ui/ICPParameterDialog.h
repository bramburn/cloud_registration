#ifndef ICPPARAMETERDIALOG_H
#define ICPPARAMETERDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>

#include "algorithms/ICPRegistration.h"

/**
 * @brief Dialog for configuring ICP algorithm parameters
 * 
 * This dialog provides a user interface for setting ICP parameters such as
 * maximum iterations, convergence threshold, correspondence distance, and
 * outlier rejection settings. It also provides intelligent default values
 * based on the input point clouds.
 * 
 * Sprint 4.1 Requirements:
 * - Configurable ICP parameters with sensible defaults
 * - Reset to defaults functionality
 * - Parameter validation and user feedback
 * - Integration with ICPRegistration::getRecommendedParameters()
 */
class ICPParameterDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param sourceCloud Source point cloud for parameter calculation
     * @param targetCloud Target point cloud for parameter calculation
     * @param parent Parent widget
     */
    explicit ICPParameterDialog(const PointCloud& sourceCloud,
                               const PointCloud& targetCloud,
                               QWidget* parent = nullptr);

    /**
     * @brief Get configured ICP parameters
     * @return ICPParams struct with current dialog values
     */
    ICPParams getICPParameters() const;

    /**
     * @brief Set ICP parameters in the dialog
     * @param params Parameters to set
     */
    void setICPParameters(const ICPParams& params);

    /**
     * @brief Get source scan ID
     * @return Source scan identifier
     */
    QString getSourceScanId() const { return m_sourceScanId; }

    /**
     * @brief Get target scan ID
     * @return Target scan identifier
     */
    QString getTargetScanId() const { return m_targetScanId; }

    /**
     * @brief Set scan IDs for the alignment
     * @param sourceScanId Source scan identifier
     * @param targetScanId Target scan identifier
     */
    void setScanIds(const QString& sourceScanId, const QString& targetScanId);

signals:
    /**
     * @brief Emitted when user clicks "Run ICP" button
     * @param params Configured ICP parameters
     * @param sourceScanId Source scan identifier
     * @param targetScanId Target scan identifier
     */
    void runICPRequested(const ICPParams& params, const QString& sourceScanId, const QString& targetScanId);

private slots:
    /**
     * @brief Handle "Run ICP" button click
     */
    void onRunICPClicked();

    /**
     * @brief Handle "Reset to Defaults" button click
     */
    void onResetToDefaultsClicked();

    /**
     * @brief Handle outlier rejection checkbox state change
     * @param enabled Whether outlier rejection is enabled
     */
    void onOutlierRejectionToggled(bool enabled);

private:
    /**
     * @brief Setup the user interface
     */
    void setupUI();

    /**
     * @brief Setup signal connections
     */
    void setupConnections();

    /**
     * @brief Load default parameters based on point clouds
     */
    void loadDefaultParameters();

    /**
     * @brief Validate current parameter values
     * @return true if parameters are valid
     */
    bool validateParameters() const;

    /**
     * @brief Update UI state based on current settings
     */
    void updateUIState();

    // UI Components
    QSpinBox* m_maxIterationsSpinBox;
    QDoubleSpinBox* m_convergenceThresholdSpinBox;
    QDoubleSpinBox* m_maxCorrespondenceDistanceSpinBox;
    QCheckBox* m_enableOutlierRejectionCheckBox;
    QDoubleSpinBox* m_outlierThresholdSpinBox;
    
    QPushButton* m_runICPButton;
    QPushButton* m_cancelButton;
    QPushButton* m_resetToDefaultsButton;
    
    QLabel* m_statusLabel;

    // Point cloud data for parameter calculation
    PointCloud m_sourceCloud;
    PointCloud m_targetCloud;
    
    // Scan identifiers
    QString m_sourceScanId;
    QString m_targetScanId;
    
    // Default parameters
    ICPParams m_defaultParams;
};

#endif // ICPPARAMETERDIALOG_H
