#pragma once

#include <QString>
#include <QDateTime>
#include <QVector3D>
#include <QMatrix4x4>
#include <QList>
#include <QPair>

/**
 * @brief Quality assessment level enumeration
 */
enum class QualityLevel
{
    Poor,       // Poor quality (high error, low confidence)
    Fair,       // Fair quality (moderate error)
    Good,       // Good quality (low error)
    Excellent   // Excellent quality (very low error, high confidence)
};

/**
 * @brief Quality metric type enumeration
 */
enum class QualityMetricType
{
    RMSError,           // Root Mean Square error
    MaxError,           // Maximum error
    MeanError,          // Mean error
    StandardDeviation,  // Standard deviation of errors
    OverlapPercentage,  // Percentage of overlap between scans
    PointDensity,       // Average point density
    Planarity,          // Planarity measure
    Sphericity,         // Sphericity measure
    Linearity,          // Linearity measure
    CorrespondenceCount // Number of correspondences
};

/**
 * @brief Comprehensive quality metrics structure
 */
struct QualityMetrics
{
    // Error metrics
    float rmsError;                 // Root mean square error (meters)
    float maxError;                 // Maximum error (meters)
    float meanError;                // Mean error (meters)
    float standardDeviation;        // Standard deviation of errors (meters)
    
    // Correspondence metrics
    int correspondenceCount;        // Number of correspondences used
    float averageCorrespondenceDistance; // Average distance between correspondences
    float maxCorrespondenceDistance;     // Maximum correspondence distance
    
    // Overlap and coverage metrics
    float overlapPercentage;        // Percentage of overlap between scans (0-100)
    float coveragePercentage;       // Percentage of target covered (0-100)
    
    // Density metrics
    float averagePointDensity;      // Average point density (points/m²)
    float minPointDensity;          // Minimum point density
    float maxPointDensity;          // Maximum point density
    
    // Geometric feature metrics
    float planarity;                // Planarity measure (0-1)
    float sphericity;               // Sphericity measure (0-1)
    float linearity;                // Linearity measure (0-1)
    
    // Transformation metrics
    QVector3D translationMagnitude; // Translation vector magnitude
    QVector3D rotationAngles;       // Rotation angles (degrees)
    float transformationScale;      // Scale factor
    
    // Confidence and reliability
    float confidenceScore;          // Overall confidence score (0-1)
    float reliabilityScore;         // Reliability score (0-1)
    QualityLevel overallQuality;    // Overall quality assessment
    
    // Timing information
    QDateTime assessmentTime;       // When assessment was performed
    double processingTime;          // Time taken for assessment (seconds)
    
    QualityMetrics()
        : rmsError(0.0f)
        , maxError(0.0f)
        , meanError(0.0f)
        , standardDeviation(0.0f)
        , correspondenceCount(0)
        , averageCorrespondenceDistance(0.0f)
        , maxCorrespondenceDistance(0.0f)
        , overlapPercentage(0.0f)
        , coveragePercentage(0.0f)
        , averagePointDensity(0.0f)
        , minPointDensity(0.0f)
        , maxPointDensity(0.0f)
        , planarity(0.0f)
        , sphericity(0.0f)
        , linearity(0.0f)
        , transformationScale(1.0f)
        , confidenceScore(0.0f)
        , reliabilityScore(0.0f)
        , overallQuality(QualityLevel::Poor)
        , assessmentTime(QDateTime::currentDateTime())
        , processingTime(0.0)
    {}
};

/**
 * @brief Quality assessment parameters
 */
struct QualityAssessmentParameters
{
    // Error thresholds
    float excellentRMSThreshold;    // RMS threshold for excellent quality
    float goodRMSThreshold;         // RMS threshold for good quality
    float fairRMSThreshold;         // RMS threshold for fair quality
    
    // Overlap thresholds
    float minOverlapPercentage;     // Minimum acceptable overlap
    float goodOverlapPercentage;    // Good overlap threshold
    
    // Correspondence thresholds
    int minCorrespondences;         // Minimum number of correspondences
    float maxCorrespondenceDistance; // Maximum allowed correspondence distance
    
    // Density thresholds
    float minPointDensity;          // Minimum acceptable point density
    float maxPointDensity;          // Maximum expected point density
    
    // Geometric feature thresholds
    float minPlanarity;             // Minimum planarity for planar features
    float minSphericity;            // Minimum sphericity for spherical features
    
    // Processing parameters
    bool useRobustStatistics;       // Use robust statistical measures
    float outlierThreshold;         // Threshold for outlier detection
    bool generateDetailedReport;    // Generate detailed assessment report
    
    QualityAssessmentParameters()
        : excellentRMSThreshold(0.005f)  // 5mm
        , goodRMSThreshold(0.01f)        // 1cm
        , fairRMSThreshold(0.05f)        // 5cm
        , minOverlapPercentage(30.0f)
        , goodOverlapPercentage(70.0f)
        , minCorrespondences(3)
        , maxCorrespondenceDistance(0.1f)
        , minPointDensity(100.0f)        // 100 points/m²
        , maxPointDensity(10000.0f)      // 10k points/m²
        , minPlanarity(0.8f)
        , minSphericity(0.8f)
        , useRobustStatistics(true)
        , outlierThreshold(2.0f)         // 2 standard deviations
        , generateDetailedReport(true)
    {}
};

/**
 * @brief Quality assessment report structure
 */
struct QualityReport
{
    // Basic information
    QString reportId;
    QString projectName;
    QDateTime generationTime;
    QString assessmentVersion;
    
    // Metrics
    QualityMetrics metrics;
    QualityAssessmentParameters parameters;
    
    // Detailed analysis
    QList<QString> warnings;
    QList<QString> recommendations;
    QList<QString> criticalIssues;
    
    // Statistical data
    QList<float> errorDistribution;
    QList<QPair<QVector3D, float>> spatialErrorMap;
    
    // Summary
    QString summaryText;
    bool passesQualityCheck;
    float overallScore;             // 0-100 scale
    
    // Visualization data
    QList<QVector3D> correspondencePoints;
    QList<float> correspondenceErrors;
    QList<QVector3D> outlierPoints;
    
    QualityReport()
        : generationTime(QDateTime::currentDateTime())
        , passesQualityCheck(false)
        , overallScore(0.0f)
    {}
};

/**
 * @brief Quality threshold configuration
 */
struct QualityThresholds
{
    // Error thresholds by quality level
    struct ErrorThresholds
    {
        float excellent;
        float good;
        float fair;
        float poor;
    };
    
    ErrorThresholds rmsError;
    ErrorThresholds maxError;
    ErrorThresholds meanError;
    
    // Overlap thresholds
    float minOverlap;
    float goodOverlap;
    float excellentOverlap;
    
    // Correspondence thresholds
    int minCorrespondences;
    int goodCorrespondences;
    int excellentCorrespondences;
    
    QualityThresholds()
    {
        // RMS error thresholds (meters)
        rmsError.excellent = 0.005f;    // 5mm
        rmsError.good = 0.01f;          // 1cm
        rmsError.fair = 0.05f;          // 5cm
        rmsError.poor = 0.1f;           // 10cm
        
        // Max error thresholds (meters)
        maxError.excellent = 0.02f;     // 2cm
        maxError.good = 0.05f;          // 5cm
        maxError.fair = 0.1f;           // 10cm
        maxError.poor = 0.2f;           // 20cm
        
        // Mean error thresholds (meters)
        meanError.excellent = 0.003f;   // 3mm
        meanError.good = 0.007f;        // 7mm
        meanError.fair = 0.03f;         // 3cm
        meanError.poor = 0.07f;         // 7cm
        
        // Overlap thresholds (percentage)
        minOverlap = 30.0f;
        goodOverlap = 60.0f;
        excellentOverlap = 80.0f;
        
        // Correspondence count thresholds
        minCorrespondences = 3;
        goodCorrespondences = 6;
        excellentCorrespondences = 10;
    }
};

/**
 * @brief Utility functions for quality assessment
 */
namespace QualityUtils
{
    /**
     * @brief Convert quality level to string
     */
    QString qualityLevelToString(QualityLevel level);
    
    /**
     * @brief Convert metric type to string
     */
    QString metricTypeToString(QualityMetricType type);
    
    /**
     * @brief Determine quality level from RMS error
     */
    QualityLevel determineQualityFromRMS(float rmsError, const QualityThresholds& thresholds);
    
    /**
     * @brief Calculate overall quality score
     */
    float calculateOverallScore(const QualityMetrics& metrics, const QualityThresholds& thresholds);
    
    /**
     * @brief Generate quality recommendations
     */
    QList<QString> generateRecommendations(const QualityMetrics& metrics, const QualityThresholds& thresholds);
    
    /**
     * @brief Validate quality metrics
     */
    bool validateMetrics(const QualityMetrics& metrics);
    
    /**
     * @brief Create default thresholds
     */
    QualityThresholds createDefaultThresholds();
}
