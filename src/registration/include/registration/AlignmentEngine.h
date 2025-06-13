#pragma once

#include <QList>
#include <QMatrix4x4>
#include <QObject>
#include <QPair>
#include <QTimer>
#include <QVector3D>

#include <memory>
#include <vector>

#include "ErrorAnalysis.h"
#include "core/octree.h"
#include "registration/TargetDetectionBase.h"

// Forward declarations
class ICPRegistration;
struct ICPParams;
class SphereDetector;
class PointCloudLoadManager;
class TargetManager;

/**
 * @brief AlignmentEngine - High-level coordination for manual alignment workflow
 *
 * This class orchestrates the manual alignment process by managing correspondence
 * points, computing transformations, and providing real-time feedback. It serves
 * as the central coordinator between the UI, algorithms, and visualization components.
 *
 * Sprint 4 Requirements:
 * - Manages correspondence point pairs for alignment
 * - Provides real-time transformation computation and preview
 * - Emits quality metrics for immediate user feedback
 * - Supports incremental correspondence addition/removal
 * - Integrates with PointCloudViewerWidget for dynamic visualization
 * - Maintains alignment history for undo/redo functionality
 */
class AlignmentEngine : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Alignment state enumeration
     */
    enum class AlignmentState
    {
        Idle,          ///< No correspondences defined
        Insufficient,  ///< Less than 3 correspondences
        Computing,     ///< Transformation computation in progress
        Valid,         ///< Valid transformation computed
        Error,         ///< Error in computation
        Cancelled      ///< Computation cancelled by user
    };

    /**
     * @brief Alignment result structure
     */
    struct AlignmentResult
    {
        QMatrix4x4 transformation;                    ///< Computed transformation matrix
        ErrorAnalysis::ErrorStatistics errorStats;    ///< Comprehensive error analysis
        AlignmentState state = AlignmentState::Idle;  ///< Current alignment state
        QString message;                              ///< Status or error message
        qint64 computationTimeMs = 0;                 ///< Computation time in milliseconds

        /**
         * @brief Check if alignment result is valid for application
         * @return true if transformation can be safely applied
         */
        bool isValid() const
        {
            return state == AlignmentState::Valid;
        }
    };

public:
    explicit AlignmentEngine(QObject* parent = nullptr);
    virtual ~AlignmentEngine() = default;

    // --- Dependency Injection ---

    /**
     * @brief Set point cloud load manager for data access
     * @param loadManager Pointer to load manager
     */
    void setPointCloudLoadManager(PointCloudLoadManager* loadManager)
    {
        m_loadManager = loadManager;
    }

    /**
     * @brief Set target manager for storing detection results
     * @param targetManager Pointer to target manager
     */
    void setTargetManager(TargetManager* targetManager)
    {
        m_targetManager = targetManager;
    }

    // --- Correspondence Management ---

    /**
     * @brief Set complete list of correspondence pairs
     * @param correspondences List of (source, target) point pairs
     */
    void setCorrespondences(const QList<QPair<QVector3D, QVector3D>>& correspondences);

    /**
     * @brief Add a single correspondence pair
     * @param sourcePoint Point in source scan
     * @param targetPoint Corresponding point in target scan
     */
    void addCorrespondence(const QVector3D& sourcePoint, const QVector3D& targetPoint);

    /**
     * @brief Remove correspondence at specified index
     * @param index Index of correspondence to remove
     */
    void removeCorrespondence(int index);

    /**
     * @brief Clear all correspondences
     */
    void clearCorrespondences();

    /**
     * @brief Get current correspondence list
     * @return List of correspondence pairs
     */
    const QList<QPair<QVector3D, QVector3D>>& getCorrespondences() const
    {
        return m_correspondences;
    }

    // --- Alignment Computation ---

    /**
     * @brief Trigger alignment computation with current correspondences
     *
     * Computes transformation and emits signals for real-time update.
     * Computation is performed asynchronously to maintain UI responsiveness.
     */
    void recomputeAlignment();

    /**
     * @brief Get the most recent alignment result
     * @return Current alignment result with transformation and quality metrics
     */
    const AlignmentResult& getCurrentResult() const
    {
        return m_currentResult;
    }

    // --- Sprint 6.1: Deviation Analysis ---

    /**
     * @brief Analyze deviation between source and target point clouds
     * @param source Source point cloud
     * @param target Target point cloud
     * @param transform Transformation to apply to source points
     * @return Colorized point cloud with deviation colors
     */
    std::vector<PointFullData> analyzeDeviation(const std::vector<PointFullData>& source,
                                                const std::vector<PointFullData>& target,
                                                const QMatrix4x4& transform);

    /**
     * @brief Get the maximum deviation distance from the last analysis
     * @return Maximum deviation distance used for color mapping
     */
    float getLastDeviationMaxDistance() const
    {
        return m_lastDeviationMaxDistance;
    }

    /**
     * @brief Get current transformation matrix
     * @return 4x4 transformation matrix (identity if no valid alignment)
     */
    QMatrix4x4 getCurrentTransformation() const
    {
        return m_currentResult.transformation;
    }

    /**
     * @brief Get current RMS error
     * @return RMS error in mm (0 if no valid alignment)
     */
    float getCurrentRMSError() const
    {
        return m_currentResult.errorStats.rmsError;
    }

    // --- Configuration ---

    /**
     * @brief Enable/disable automatic recomputation on correspondence changes
     * @param enabled If true, alignment recomputes automatically when correspondences change
     */
    void setAutoRecompute(bool enabled)
    {
        m_autoRecompute = enabled;
    }

    /**
     * @brief Check if auto-recomputation is enabled
     * @return true if automatic recomputation is enabled
     */
    bool isAutoRecompute() const
    {
        return m_autoRecompute;
    }

    /**
     * @brief Set quality thresholds for validation
     * @param rmsThreshold Maximum acceptable RMS error (mm)
     * @param maxErrorThreshold Maximum acceptable individual error (mm)
     */
    void setQualityThresholds(float rmsThreshold, float maxErrorThreshold);

    // --- Target Detection Integration ---

    /**
     * @brief Start target detection process
     * @param scanId ID of the scan to process
     * @param mode Detection mode (automatic, manual, or both)
     * @param params Detection parameters
     */
    void startTargetDetection(const QString& scanId,
                             int mode,
                             const QVariantMap& params);

    /**
     * @brief Cancel currently running target detection
     */
    void cancelTargetDetection();

    // --- Automatic ICP Alignment ---

    /**
     * @brief Start automatic ICP alignment between two scans
     * @param sourceScanId Identifier of the source scan to be transformed
     * @param targetScanId Identifier of the target/reference scan
     * @param params ICP algorithm parameters
     */
    void startAutomaticAlignment(const QString& sourceScanId,
                                const QString& targetScanId,
                                const ICPParams& params);

    /**
     * @brief Cancel currently running automatic alignment
     */
    void cancelAutomaticAlignment();

signals:
    /**
     * @brief Emitted when transformation is updated
     * @param transform New transformation matrix
     */
    void transformationUpdated(const QMatrix4x4& transform);

    /**
     * @brief Emitted when quality metrics are updated
     * @param rmsError Current RMS error in mm
     */
    void qualityMetricsUpdated(float rmsError);

    /**
     * @brief Emitted when complete alignment result is available
     * @param result Complete alignment result with all metrics
     */
    void alignmentResultUpdated(const AlignmentResult& result);

    /**
     * @brief Emitted when alignment state changes
     * @param state New alignment state
     * @param message Status message
     */
    void alignmentStateChanged(AlignmentState state, const QString& message);

    /**
     * @brief Emitted when correspondences are modified
     * @param count New correspondence count
     */
    void correspondencesChanged(int count);

    // Target detection signals
    /**
     * @brief Emitted when target detection progress updates
     * @param percentage Progress percentage (0-100)
     * @param stage Current processing stage
     */
    void targetDetectionProgress(int percentage, const QString& stage);

    /**
     * @brief Emitted when target detection completes successfully
     * @param result Complete detection result with targets and metadata
     */
    void targetDetectionCompleted(const TargetDetectionBase::DetectionResult& result);

    /**
     * @brief Emitted when target detection encounters an error
     * @param error Error message
     */
    void targetDetectionError(const QString& error);

    // ICP Progress signals
    /**
     * @brief Emitted during ICP iterations to report progress
     * @param iteration Current iteration number
     * @param rmsError Current RMS error
     * @param transformation Current transformation estimate
     */
    void progressUpdated(int iteration, float rmsError, const QMatrix4x4& transformation);

    /**
     * @brief Emitted when ICP computation completes
     * @param success True if converged successfully
     * @param finalTransformation Final transformation matrix
     * @param finalRMSError Final RMS error
     * @param iterations Number of iterations performed
     */
    void computationFinished(bool success, const QMatrix4x4& finalTransformation, float finalRMSError, int iterations);

private slots:
    /**
     * @brief Perform actual alignment computation (called by timer for async execution)
     */
    void performAlignment();

    /**
     * @brief Handle detection progress from SphereDetector
     * @param percentage Progress percentage (0-100)
     * @param stage Current processing stage
     */
    void onDetectionProgress(int percentage, const QString& stage);

    /**
     * @brief Handle detection completion from SphereDetector
     * @param result Detection result
     */
    void onDetectionCompleted(const TargetDetectionBase::DetectionResult& result);

    /**
     * @brief Handle detection error from SphereDetector
     * @param error Error message
     */
    void onDetectionError(const QString& error);

private:
    /**
     * @brief Validate correspondences before computation
     * @return true if correspondences are valid for alignment
     */
    bool validateCorrespondences() const;

    /**
     * @brief Update alignment state and emit appropriate signals
     * @param state New alignment state
     * @param message Status message
     */
    void updateAlignmentState(AlignmentState state, const QString& message = QString());

    /**
     * @brief Trigger recomputation if auto-recompute is enabled
     */
    void triggerRecomputeIfEnabled();

private:
    // Core data
    QList<QPair<QVector3D, QVector3D>> m_correspondences;  ///< Current correspondence pairs
    AlignmentResult m_currentResult;                       ///< Most recent alignment result

    // Configuration
    bool m_autoRecompute = true;        ///< Auto-recompute on changes
    float m_rmsThreshold = 5.0f;        ///< RMS error threshold (mm)
    float m_maxErrorThreshold = 10.0f;  ///< Max individual error threshold (mm)

    // Async computation
    QTimer* m_computationTimer;         ///< Timer for async computation
    bool m_computationPending = false;  ///< Computation request pending

    // ICP-specific members
    std::unique_ptr<ICPRegistration> m_icpAlgorithm;  ///< Current ICP algorithm instance
    QString m_currentSourceScanId;                    ///< Current source scan ID for ICP
    QString m_currentTargetScanId;                    ///< Current target scan ID for ICP

    // Target detection members
    std::unique_ptr<SphereDetector> m_sphereDetector;  ///< Current sphere detector instance
    PointCloudLoadManager* m_loadManager;              ///< Point cloud load manager for data access
    TargetManager* m_targetManager;                    ///< Target manager for storing results

    // Sprint 6.1: Deviation analysis
    float m_lastDeviationMaxDistance = 0.05f;  ///< Last max deviation distance used

    // Constants
    static constexpr int COMPUTATION_DELAY_MS = 100;  ///< Delay before computation (ms)
    static constexpr int MIN_CORRESPONDENCES = 3;     ///< Minimum correspondences required
};
