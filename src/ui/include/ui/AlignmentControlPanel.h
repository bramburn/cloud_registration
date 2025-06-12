#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QProgressBar>
#include <QTextEdit>
#include <QGroupBox>
#include <QCheckBox>
#include "registration/AlignmentEngine.h"

/**
 * @brief AlignmentControlPanel - UI controls for manual alignment workflow
 * 
 * This widget provides comprehensive controls for the manual alignment process,
 * including correspondence management, quality monitoring, and alignment execution.
 * It integrates with AlignmentEngine to provide real-time feedback and control.
 * 
 * Sprint 4 Requirements:
 * - Display real-time RMS error and quality metrics
 * - Provide controls for alignment parameters and execution
 * - Show correspondence count and validation status
 * - Enable quality threshold configuration
 * - Display comprehensive alignment reports
 * - Support alignment history and undo/redo operations
 */
class AlignmentControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit AlignmentControlPanel(QWidget* parent = nullptr);
    virtual ~AlignmentControlPanel() = default;

    /**
     * @brief Connect to alignment engine for real-time updates
     * @param engine AlignmentEngine instance to monitor and control
     */
    void setAlignmentEngine(AlignmentEngine* engine);

    /**
     * @brief Get current alignment engine
     * @return Pointer to current alignment engine (may be nullptr)
     */
    AlignmentEngine* getAlignmentEngine() const { return m_alignmentEngine; }

public slots:
    /**
     * @brief Update RMS error display
     * @param error Current RMS error in mm
     */
    void updateRMSError(float error);

    /**
     * @brief Update alignment result display
     * @param result Complete alignment result with all metrics
     */
    void updateAlignmentResult(const AlignmentEngine::AlignmentResult& result);

    /**
     * @brief Update alignment state display
     * @param state Current alignment state
     * @param message Status message
     */
    void updateAlignmentState(AlignmentEngine::AlignmentState state, const QString& message);

    /**
     * @brief Update correspondence count display
     * @param count Number of correspondence pairs
     */
    void updateCorrespondenceCount(int count);

signals:
    /**
     * @brief Emitted when user requests alignment computation
     */
    void alignmentRequested();

    /**
     * @brief Emitted when user requests correspondence clearing
     */
    void clearCorrespondencesRequested();

    /**
     * @brief Emitted when quality thresholds are changed
     * @param rmsThreshold New RMS error threshold (mm)
     * @param maxErrorThreshold New maximum error threshold (mm)
     */
    void qualityThresholdsChanged(float rmsThreshold, float maxErrorThreshold);

    /**
     * @brief Emitted when auto-recompute setting is changed
     * @param enabled New auto-recompute setting
     */
    void autoRecomputeChanged(bool enabled);

private slots:
    /**
     * @brief Handle alignment button click
     */
    void onAlignmentButtonClicked();

    /**
     * @brief Handle clear correspondences button click
     */
    void onClearCorrespondencesClicked();

    /**
     * @brief Handle quality threshold changes
     */
    void onQualityThresholdsChanged();

    /**
     * @brief Handle auto-recompute checkbox change
     */
    void onAutoRecomputeChanged();

    /**
     * @brief Show detailed error report
     */
    void onShowDetailedReport();

private:
    /**
     * @brief Setup the user interface
     */
    void setupUI();

    /**
     * @brief Create correspondence status group
     * @return QGroupBox with correspondence controls
     */
    QGroupBox* createCorrespondenceGroup();

    /**
     * @brief Create quality metrics group
     * @return QGroupBox with quality displays
     */
    QGroupBox* createQualityGroup();

    /**
     * @brief Create alignment controls group
     * @return QGroupBox with alignment controls
     */
    QGroupBox* createControlsGroup();

    /**
     * @brief Create configuration group
     * @return QGroupBox with configuration options
     */
    QGroupBox* createConfigurationGroup();

    /**
     * @brief Update UI state based on alignment state
     * @param state Current alignment state
     */
    void updateUIState(AlignmentEngine::AlignmentState state);

    /**
     * @brief Format error value for display
     * @param error Error value in mm
     * @return Formatted string with appropriate precision and units
     */
    QString formatError(float error) const;

    /**
     * @brief Get quality level description from RMS error
     * @param rmsError RMS error in mm
     * @return Quality level string (Excellent, Good, Acceptable, Poor)
     */
    QString getQualityLevel(float rmsError) const;

private:
    // Core components
    AlignmentEngine* m_alignmentEngine = nullptr;

    // Layout
    QVBoxLayout* m_mainLayout;

    // Correspondence status
    QLabel* m_correspondenceCountLabel;
    QLabel* m_correspondenceStatusLabel;

    // Quality metrics
    QLabel* m_rmsErrorLabel;
    QLabel* m_qualityLevelLabel;
    QLabel* m_maxErrorLabel;
    QLabel* m_meanErrorLabel;
    QLabel* m_computationTimeLabel;

    // Controls
    QPushButton* m_alignButton;
    QPushButton* m_clearButton;
    QPushButton* m_reportButton;
    QProgressBar* m_progressBar;

    // Configuration
    QDoubleSpinBox* m_rmsThresholdSpin;
    QDoubleSpinBox* m_maxErrorThresholdSpin;
    QCheckBox* m_autoRecomputeCheck;

    // Status
    QLabel* m_statusLabel;
    QTextEdit* m_detailsText;

    // Current state
    AlignmentEngine::AlignmentResult m_lastResult;

    // Constants
    static constexpr float DEFAULT_RMS_THRESHOLD = 5.0f;
    static constexpr float DEFAULT_MAX_ERROR_THRESHOLD = 10.0f;
};
