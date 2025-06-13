#ifndef TARGETDETECTIONDIALOG_H
#define TARGETDETECTIONDIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>

#include <memory>
#include <vector>

#include "core/pointdata.h"
#include "registration/TargetDetectionBase.h"

// Forward declarations
class TargetManager;
class SphereDetector;
class NaturalPointSelector;
class SphereTarget;
class NaturalPointTarget;

/**
 * @brief Target Detection Dialog for Point Cloud Registration
 *
 * Sprint 5.1: Target Detection UI & Mode Activation
 * Provides user interface for configuring and initiating target detection
 * on point cloud data for registration purposes.
 */
class TargetDetectionDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Detection mode enumeration
     */
    enum DetectionMode
    {
        AutomaticSpheres = 0,      ///< Automatic sphere detection only
        ManualNaturalPoints = 1,   ///< Manual natural point selection only
        Both = 2                   ///< Both automatic and manual detection
    };

public:
    /**
     * @brief Constructor
     * @param targetManager Pointer to target manager for storing results
     * @param parent Parent widget
     */
    explicit TargetDetectionDialog(TargetManager* targetManager, QWidget* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~TargetDetectionDialog() override = default;

    // Configuration methods
    /**
     * @brief Set point cloud data for detection
     * @param scanId ID of the scan
     * @param points Point cloud data
     */
    void setPointCloudData(const QString& scanId, const std::vector<PointFullData>& points);

    /**
     * @brief Get current detection parameters from UI
     * @return Detection parameters structure
     */
    TargetDetectionBase::DetectionParams getDetectionParameters() const;

    /**
     * @brief Set detection parameters in UI
     * @param params Detection parameters to set
     */
    void setDetectionParameters(const TargetDetectionBase::DetectionParams& params);

    /**
     * @brief Get current detection mode
     * @return Selected detection mode
     */
    DetectionMode getDetectionMode() const;

public slots:
    /**
     * @brief Start the detection process
     */
    void startDetection();

    /**
     * @brief Cancel ongoing detection
     */
    void cancelDetection();

    /**
     * @brief Reset parameters to default values
     */
    void resetToDefaults();

    /**
     * @brief Load parameters from file
     */
    void loadParameters();

    /**
     * @brief Save parameters to file
     */
    void saveParameters();

private slots:
    // Detection progress and completion handlers
    void onDetectionProgress(int percentage, const QString& stage);
    void onDetectionCompleted(const TargetDetectionBase::DetectionResult& result);
    void onDetectionError(const QString& error);

    // UI event handlers
    void onDetectionModeChanged();
    void onParametersChanged();
    void onTargetSelected();
    void onAcceptTargets();
    void onRejectTargets();

signals:
    /**
     * @brief Emitted when detection is completed and targets are accepted
     * @param scanId ID of the scan
     * @param result Detection result
     */
    void detectionCompleted(const QString& scanId, const TargetDetectionBase::DetectionResult& result);

    /**
     * @brief Emitted when manual selection mode is requested
     * @param scanId ID of the scan for manual selection
     */
    void manualSelectionRequested(const QString& scanId);

private:
    // UI setup methods
    void setupUI();
    QWidget* createParameterControls();
    QWidget* createDetectionControls();
    QWidget* createResultsDisplay();

    // Parameter management
    void updateParameterControls();
    void updateResultsTable(const TargetDetectionBase::DetectionResult& result);
    bool validateParameters() const;
    TargetDetectionBase::DetectionParams getParametersFromUI() const;
    void setUIFromParameters(const TargetDetectionBase::DetectionParams& params);

private:
    // Core components
    TargetManager* m_targetManager;
    SphereDetector* m_sphereDetector;
    NaturalPointSelector* m_naturalPointSelector;

    // Data
    QString m_currentScanId;
    std::vector<PointFullData> m_currentPoints;
    TargetDetectionBase::DetectionResult m_lastResult;

    // State
    bool m_detectionRunning;

    // UI Components
    QTabWidget* m_tabWidget;

    // Parameter controls
    QComboBox* m_detectionModeCombo;
    QGroupBox* m_commonParamsGroup;
    QGroupBox* m_sphereParamsGroup;
    QGroupBox* m_naturalPointParamsGroup;

    // Common parameter controls
    QDoubleSpinBox* m_distanceThresholdSpin;
    QSpinBox* m_maxIterationsSpin;
    QDoubleSpinBox* m_minQualitySpin;
    QCheckBox* m_enablePreprocessingCheck;

    // Sphere parameter controls
    QDoubleSpinBox* m_minRadiusSpin;
    QDoubleSpinBox* m_maxRadiusSpin;
    QSpinBox* m_minInliersSpin;

    // Natural point parameter controls
    QDoubleSpinBox* m_neighborhoodRadiusSpin;
    QDoubleSpinBox* m_curvatureThresholdSpin;

    // Detection controls
    QPushButton* m_startButton;
    QPushButton* m_cancelButton;
    QPushButton* m_resetButton;
    QPushButton* m_loadParamsButton;
    QPushButton* m_saveParamsButton;
    QPushButton* m_manualSelectionButton;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;

    // Results display
    QTableWidget* m_resultsTable;
    QPushButton* m_acceptButton;
    QPushButton* m_rejectButton;
    QTextEdit* m_logTextEdit;
};

#endif  // TARGETDETECTIONDIALOG_H
