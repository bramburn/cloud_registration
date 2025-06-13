#pragma once

#include <QString>
#include <QMatrix4x4>
#include <QVector3D>
#include <QDateTime>
#include <QList>
#include <QPair>

/**
 * @brief Registration step enumeration for workflow management
 */
enum class RegistrationStep
{
    SelectScans,        // Select scans to register
    TargetDetection,    // Detect registration targets
    ManualAlignment,    // Manual correspondence establishment
    ICPRegistration,    // ICP refinement
    QualityReview,      // Quality assessment
    Export              // Export results
};

/**
 * @brief Registration algorithm type
 */
enum class RegistrationAlgorithm
{
    Manual,             // Manual correspondence only
    ICP,                // Iterative Closest Point
    FeatureBased,       // Feature-based registration
    Hybrid              // Combination of methods
};

/**
 * @brief Registration quality level
 */
enum class RegistrationQuality
{
    Poor,               // Poor quality (high error)
    Fair,               // Fair quality
    Good,               // Good quality
    Excellent           // Excellent quality (low error)
};

/**
 * @brief Registration result structure
 */
struct RegistrationResult
{
    bool success;
    QString errorMessage;
    
    // Transformation
    QMatrix4x4 transformation;
    QVector3D translation;
    QVector3D rotation;  // Euler angles in degrees
    
    // Quality metrics
    float rmsError;
    float maxError;
    float meanError;
    float standardDeviation;
    int correspondenceCount;
    float overlapPercentage;
    RegistrationQuality quality;
    
    // Algorithm information
    RegistrationAlgorithm algorithm;
    int iterations;
    float convergenceThreshold;
    bool converged;
    
    // Timing
    QDateTime timestamp;
    double processingTime;
    
    // Additional metrics
    QList<QPair<QVector3D, QVector3D>> correspondences;
    QList<float> residuals;
    
    RegistrationResult()
        : success(false)
        , transformation(QMatrix4x4())
        , rmsError(0.0f)
        , maxError(0.0f)
        , meanError(0.0f)
        , standardDeviation(0.0f)
        , correspondenceCount(0)
        , overlapPercentage(0.0f)
        , quality(RegistrationQuality::Poor)
        , algorithm(RegistrationAlgorithm::Manual)
        , iterations(0)
        , convergenceThreshold(0.001f)
        , converged(false)
        , timestamp(QDateTime::currentDateTime())
        , processingTime(0.0)
    {}
};

/**
 * @brief Registration parameters structure
 */
struct RegistrationParameters
{
    // ICP parameters
    float maxDistance;
    float convergenceThreshold;
    int maxIterations;
    bool usePointToPlane;
    bool useRobustEstimation;
    
    // Feature-based parameters
    float featureRadius;
    float correspondenceDistance;
    int minCorrespondences;
    bool useRANSAC;
    float ransacThreshold;
    int ransacIterations;
    
    // Quality thresholds
    float maxAcceptableRMS;
    float minOverlapPercentage;
    
    // Performance
    bool useMultiThreading;
    int threadCount;
    
    RegistrationParameters()
        : maxDistance(1.0f)
        , convergenceThreshold(0.001f)
        , maxIterations(50)
        , usePointToPlane(true)
        , useRobustEstimation(false)
        , featureRadius(0.1f)
        , correspondenceDistance(0.05f)
        , minCorrespondences(3)
        , useRANSAC(true)
        , ransacThreshold(0.01f)
        , ransacIterations(1000)
        , maxAcceptableRMS(0.01f)
        , minOverlapPercentage(50.0f)
        , useMultiThreading(true)
        , threadCount(-1)  // Auto-detect
    {}
};

/**
 * @brief Scan information for registration
 */
struct ScanInfo
{
    QString scanId;
    QString filePath;
    QString name;
    QString description;
    QDateTime acquisitionTime;
    QVector3D position;
    QVector3D orientation;
    QMatrix4x4 localTransformation;
    bool isReference;
    int pointCount;
    QVector3D boundingBoxMin;
    QVector3D boundingBoxMax;
    
    ScanInfo()
        : isReference(false)
        , pointCount(0)
        , localTransformation(QMatrix4x4())
    {}
};

/**
 * @brief Target correspondence structure
 */
struct TargetCorrespondence
{
    QString sourceTargetId;
    QString targetTargetId;
    QVector3D sourcePosition;
    QVector3D targetPosition;
    float distance;
    float confidence;
    bool isManual;
    bool isValid;
    
    TargetCorrespondence()
        : distance(0.0f)
        , confidence(1.0f)
        , isManual(false)
        , isValid(true)
    {}
};

/**
 * @brief Registration project state
 */
enum class ProjectState
{
    Created,            // Project created but no scans loaded
    ScansLoaded,        // Scans loaded
    TargetsDetected,    // Targets detected
    CorrespondencesSet, // Correspondences established
    Registered,         // Registration completed
    QualityChecked,     // Quality assessment done
    Exported            // Results exported
};

/**
 * @brief Registration workflow state
 */
struct WorkflowState
{
    ProjectState currentState;
    RegistrationStep currentStep;
    bool canProceedToNext;
    bool canGoToPrevious;
    QString statusMessage;
    float overallProgress;  // 0.0 to 1.0
    
    WorkflowState()
        : currentState(ProjectState::Created)
        , currentStep(RegistrationStep::SelectScans)
        , canProceedToNext(false)
        , canGoToPrevious(false)
        , overallProgress(0.0f)
    {}
};

/**
 * @brief Utility functions for registration types
 */
namespace RegistrationUtils
{
    /**
     * @brief Convert registration step to string
     */
    QString stepToString(RegistrationStep step);
    
    /**
     * @brief Convert string to registration step
     */
    RegistrationStep stringToStep(const QString& str);
    
    /**
     * @brief Convert algorithm to string
     */
    QString algorithmToString(RegistrationAlgorithm algorithm);
    
    /**
     * @brief Convert quality to string
     */
    QString qualityToString(RegistrationQuality quality);
    
    /**
     * @brief Determine quality from RMS error
     */
    RegistrationQuality determineQuality(float rmsError);
    
    /**
     * @brief Validate registration result
     */
    bool isResultValid(const RegistrationResult& result);
    
    /**
     * @brief Calculate transformation from correspondences
     */
    QMatrix4x4 calculateTransformation(const QList<TargetCorrespondence>& correspondences);
    
    /**
     * @brief Apply transformation to point
     */
    QVector3D transformPoint(const QVector3D& point, const QMatrix4x4& transformation);
    
    /**
     * @brief Calculate RMS error from correspondences
     */
    float calculateRMSError(const QList<TargetCorrespondence>& correspondences, 
                           const QMatrix4x4& transformation);
}
