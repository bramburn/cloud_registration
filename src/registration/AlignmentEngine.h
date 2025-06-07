#ifndef ALIGNMENTENGINE_H
#define ALIGNMENTENGINE_H

#include <QObject>
#include <QMatrix4x4>
#include <QVector3D>
#include <QList>
#include <QPair>
#include <memory>
#include "../algorithms/ICPRegistration.h"
#include "../algorithms/PointToPlaneICP.h"

class ICPProgressWidget;

/**
 * @brief High-level alignment coordination and workflow management
 * 
 * The AlignmentEngine orchestrates the complete registration workflow,
 * integrating manual alignment with automatic ICP refinement. It manages
 * algorithm selection, parameter configuration, and provides a unified
 * interface for the registration system.
 */
class AlignmentEngine : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Available ICP algorithm types
     */
    enum class ICPAlgorithmType {
        PointToPoint,    ///< Standard point-to-point ICP
        PointToPlane     ///< Point-to-plane ICP (requires normals)
    };

    explicit AlignmentEngine(QObject* parent = nullptr);
    ~AlignmentEngine();

    /**
     * @brief Set correspondence points for manual alignment
     * @param correspondences List of point correspondences (source, target)
     */
    void setCorrespondences(const QList<QPair<QVector3D, QVector3D>>& correspondences);

    /**
     * @brief Recompute manual alignment from current correspondences
     */
    void recomputeAlignment();

    /**
     * @brief Run ICP refinement on point clouds
     * @param sourcePoints Source point cloud data (x,y,z interleaved)
     * @param targetPoints Target point cloud data (x,y,z interleaved)
     * @param algorithmType Type of ICP algorithm to use
     * @param params ICP algorithm parameters
     * @param showProgress Whether to show progress dialog
     */
    void runICP(const std::vector<float>& sourcePoints,
                const std::vector<float>& targetPoints,
                ICPAlgorithmType algorithmType = ICPAlgorithmType::PointToPoint,
                const ICPParams& params = ICPParams(),
                bool showProgress = true);

    /**
     * @brief Cancel currently running ICP computation
     */
    void cancelICP();

    /**
     * @brief Check if ICP is currently running
     */
    bool isICPRunning() const;

    /**
     * @brief Get current transformation matrix
     */
    QMatrix4x4 getCurrentTransformation() const { return m_currentTransform; }

    /**
     * @brief Get current RMS error
     */
    float getCurrentRMSError() const { return m_currentRMSError; }

    /**
     * @brief Get number of correspondences
     */
    int getCorrespondenceCount() const { return m_correspondences.size(); }

signals:
    /**
     * @brief Emitted when transformation is updated (manual or ICP)
     * @param transformation New transformation matrix
     */
    void transformationUpdated(const QMatrix4x4& transformation);

    /**
     * @brief Emitted when quality metrics are updated
     * @param rmsError Current RMS error
     * @param correspondenceCount Number of correspondences used
     */
    void qualityMetricsUpdated(float rmsError, int correspondenceCount);

    /**
     * @brief Emitted when ICP computation starts
     * @param algorithmType Type of ICP algorithm being used
     * @param maxIterations Maximum number of iterations
     */
    void icpStarted(ICPAlgorithmType algorithmType, int maxIterations);

    /**
     * @brief Emitted when ICP computation completes
     * @param success True if ICP converged successfully
     * @param finalTransformation Final transformation matrix
     * @param finalRMSError Final RMS error
     * @param iterations Number of iterations performed
     * @param improvementPercent Percentage improvement in RMS error
     */
    void icpFinished(bool success, const QMatrix4x4& finalTransformation,
                    float finalRMSError, int iterations, float improvementPercent);

    /**
     * @brief Emitted when an error occurs
     * @param message Error message
     */
    void errorOccurred(const QString& message);

private slots:
    void onICPProgressUpdated(int iteration, float rmsError, const QMatrix4x4& transformation);
    void onICPFinished(bool success, const QMatrix4x4& finalTransformation,
                      float finalRMSError, int iterations);
    void onProgressWidgetClosed(bool success, const QString& message);

private:
    void calculateManualAlignmentError();
    std::unique_ptr<ICPRegistration> createICPAlgorithm(ICPAlgorithmType type);
    QString algorithmTypeToString(ICPAlgorithmType type) const;

private:
    // Manual alignment data
    QList<QPair<QVector3D, QVector3D>> m_correspondences;
    QMatrix4x4 m_currentTransform;
    float m_currentRMSError;
    float m_manualRMSError;

    // ICP computation
    std::unique_ptr<ICPRegistration> m_icpAlgorithm;
    ICPAlgorithmType m_currentAlgorithmType;
    ICPParams m_currentICPParams;
    
    // Progress monitoring
    ICPProgressWidget* m_progressWidget;
    
    // State tracking
    bool m_hasValidAlignment;
    bool m_icpInProgress;
};

#endif // ALIGNMENTENGINE_H
