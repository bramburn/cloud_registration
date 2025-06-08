#pragma once

#include <QList>
#include <QVector3D>
#include <QMatrix4x4>
#include <QPair>

/**
 * @brief ErrorAnalysis - Comprehensive error analysis and quality assessment for alignment
 * 
 * This class provides static methods for analyzing the quality of point cloud alignments
 * by computing various error metrics and statistical measures. It supports both
 * correspondence-based error analysis and general transformation quality assessment.
 * 
 * Sprint 4 Requirements:
 * - Calculate Root Mean Square (RMS) error for correspondence quality
 * - Provide statistical analysis of alignment accuracy
 * - Support real-time quality monitoring during alignment
 * - Enable threshold-based quality validation
 * - Generate comprehensive error reports for documentation
 */
class ErrorAnalysis
{
public:
    /**
     * @brief Error statistics structure for comprehensive analysis
     */
    struct ErrorStatistics {
        float rmsError = 0.0f;          ///< Root Mean Square error
        float meanError = 0.0f;         ///< Mean absolute error
        float maxError = 0.0f;          ///< Maximum error
        float minError = 0.0f;          ///< Minimum error
        float standardDeviation = 0.0f; ///< Standard deviation of errors
        int numCorrespondences = 0;     ///< Number of correspondences analyzed
        
        /**
         * @brief Check if error statistics meet quality thresholds
         * @param rmsThreshold Maximum acceptable RMS error (mm)
         * @param maxThreshold Maximum acceptable individual error (mm)
         * @return true if quality meets thresholds
         */
        bool meetsQualityThresholds(float rmsThreshold = 5.0f, float maxThreshold = 10.0f) const;
        
        /**
         * @brief Generate human-readable quality report
         * @return Formatted string with error statistics
         */
        QString generateReport() const;
    };

    /**
     * @brief Calculate RMS error for correspondence pairs with transformation
     * 
     * Computes the Root Mean Square error by:
     * 1. Applying transformation to each source point
     * 2. Computing Euclidean distance to corresponding target point
     * 3. Calculating RMS of all distances
     * 
     * Formula: RMS = sqrt(sum(distance_i^2) / n)
     * 
     * @param correspondences List of point pairs (source, target)
     * @param transform Transformation matrix to apply to source points
     * @return RMS error in same units as input points (typically mm)
     */
    static float calculateRMSError(
        const QList<QPair<QVector3D, QVector3D>>& correspondences,
        const QMatrix4x4& transform);

    /**
     * @brief Calculate comprehensive error statistics
     * 
     * Provides detailed statistical analysis including RMS, mean, max, min,
     * and standard deviation of alignment errors.
     * 
     * @param correspondences List of point pairs (source, target)
     * @param transform Transformation matrix to apply to source points
     * @return Complete error statistics structure
     */
    static ErrorStatistics calculateErrorStatistics(
        const QList<QPair<QVector3D, QVector3D>>& correspondences,
        const QMatrix4x4& transform);

    /**
     * @brief Calculate individual correspondence errors
     * 
     * Returns the Euclidean distance error for each correspondence pair
     * after applying the transformation.
     * 
     * @param correspondences List of point pairs (source, target)
     * @param transform Transformation matrix to apply to source points
     * @return List of individual errors for each correspondence
     */
    static QList<float> calculateIndividualErrors(
        const QList<QPair<QVector3D, QVector3D>>& correspondences,
        const QMatrix4x4& transform);

    /**
     * @brief Identify outlier correspondences based on error threshold
     * 
     * Finds correspondence pairs with errors significantly higher than
     * the mean, which may indicate incorrect target identification.
     * 
     * @param correspondences List of point pairs (source, target)
     * @param transform Transformation matrix to apply to source points
     * @param outlierThreshold Multiple of standard deviation for outlier detection
     * @return Indices of correspondence pairs identified as outliers
     */
    static QList<int> identifyOutliers(
        const QList<QPair<QVector3D, QVector3D>>& correspondences,
        const QMatrix4x4& transform,
        float outlierThreshold = 2.0f);

    /**
     * @brief Validate transformation matrix for numerical stability
     * 
     * Checks transformation matrix for:
     * - Proper rotation matrix properties (orthogonal, determinant = 1)
     * - Reasonable translation values
     * - Numerical stability
     * 
     * @param transform Transformation matrix to validate
     * @return true if transformation is valid and stable
     */
    static bool validateTransformation(const QMatrix4x4& transform);

    /**
     * @brief Calculate transformation condition number for stability assessment
     * 
     * Higher condition numbers indicate potential numerical instability
     * in the transformation computation.
     * 
     * @param correspondences List of point pairs used for transformation
     * @return Condition number (lower is better, >1000 indicates problems)
     */
    static float calculateConditionNumber(
        const QList<QPair<QVector3D, QVector3D>>& correspondences);

private:
    /**
     * @brief Apply transformation to a 3D point
     * @param point Input point
     * @param transform Transformation matrix
     * @return Transformed point
     */
    static QVector3D transformPoint(const QVector3D& point, const QMatrix4x4& transform);

    /**
     * @brief Calculate standard deviation from a list of values
     * @param values List of numerical values
     * @param mean Pre-calculated mean of the values
     * @return Standard deviation
     */
    static float calculateStandardDeviation(const QList<float>& values, float mean);

    // Quality thresholds for professional surveying applications
    static constexpr float EXCELLENT_RMS_THRESHOLD = 1.0f;  ///< Excellent quality: <1mm RMS
    static constexpr float GOOD_RMS_THRESHOLD = 3.0f;       ///< Good quality: <3mm RMS
    static constexpr float ACCEPTABLE_RMS_THRESHOLD = 5.0f; ///< Acceptable: <5mm RMS
    static constexpr float POOR_RMS_THRESHOLD = 10.0f;      ///< Poor quality: >10mm RMS
    
    static constexpr float MAX_TRANSLATION_MAGNITUDE = 1000.0f; ///< Max reasonable translation (m)
    static constexpr float MIN_DETERMINANT = 0.9f;             ///< Min determinant for valid rotation
    static constexpr float MAX_DETERMINANT = 1.1f;             ///< Max determinant for valid rotation
};
