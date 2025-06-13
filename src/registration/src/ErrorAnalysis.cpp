#include "ErrorAnalysis.h"
#include <QDebug>
#include <cmath>
#include <algorithm>

float ErrorAnalysis::calculateRMSError(
    const QList<QPair<QVector3D, QVector3D>>& correspondences,
    const QMatrix4x4& transform)
{
    if (correspondences.isEmpty()) {
        qWarning() << "No correspondences provided for RMS error calculation";
        return 0.0f;
    }
    
    float sumOfSquares = 0.0f;
    
    for (const auto& pair : correspondences) {
        // Apply transformation to source point
        QVector3D transformedSource = transformPoint(pair.first, transform);
        
        // Calculate Euclidean distance to target point
        QVector3D delta = transformedSource - pair.second;
        float distance = delta.length();
        
        // Accumulate squared distance
        sumOfSquares += distance * distance;
    }
    
    // Calculate RMS: sqrt(sum of squares / count)
    float rmsError = std::sqrt(sumOfSquares / correspondences.size());
    
    qDebug() << "RMS error calculated:" << rmsError << "mm for" << correspondences.size() << "correspondences";
    return rmsError;
}

ErrorAnalysis::ErrorStatistics ErrorAnalysis::calculateErrorStatistics(
    const QList<QPair<QVector3D, QVector3D>>& correspondences,
    const QMatrix4x4& transform)
{
    ErrorStatistics stats;
    
    if (correspondences.isEmpty()) {
        qWarning() << "No correspondences provided for error statistics";
        return stats;
    }
    
    // Calculate individual errors
    QList<float> errors = calculateIndividualErrors(correspondences, transform);
    stats.numCorrespondences = errors.size();
    
    if (errors.isEmpty()) {
        return stats;
    }
    
    // Calculate basic statistics
    float sum = 0.0f;
    stats.minError = errors.first();
    stats.maxError = errors.first();
    
    for (float error : errors) {
        sum += error;
        stats.minError = std::min(stats.minError, error);
        stats.maxError = std::max(stats.maxError, error);
    }
    
    stats.meanError = sum / errors.size();
    
    // Calculate RMS error
    float sumOfSquares = 0.0f;
    for (float error : errors) {
        sumOfSquares += error * error;
    }
    stats.rmsError = std::sqrt(sumOfSquares / errors.size());
    
    // Calculate standard deviation
    stats.standardDeviation = calculateStandardDeviation(errors, stats.meanError);
    
    qDebug() << "Error statistics calculated:" << stats.generateReport();
    return stats;
}

QList<float> ErrorAnalysis::calculateIndividualErrors(
    const QList<QPair<QVector3D, QVector3D>>& correspondences,
    const QMatrix4x4& transform)
{
    QList<float> errors;
    errors.reserve(correspondences.size());
    
    for (const auto& pair : correspondences) {
        // Apply transformation to source point
        QVector3D transformedSource = transformPoint(pair.first, transform);
        
        // Calculate Euclidean distance to target point
        QVector3D delta = transformedSource - pair.second;
        float distance = delta.length();
        
        errors.append(distance);
    }
    
    return errors;
}

QList<int> ErrorAnalysis::identifyOutliers(
    const QList<QPair<QVector3D, QVector3D>>& correspondences,
    const QMatrix4x4& transform,
    float outlierThreshold)
{
    QList<int> outlierIndices;
    
    // Calculate error statistics
    ErrorStatistics stats = calculateErrorStatistics(correspondences, transform);
    
    if (stats.numCorrespondences == 0) {
        return outlierIndices;
    }
    
    // Calculate outlier threshold: mean + (threshold * standard deviation)
    float threshold = stats.meanError + (outlierThreshold * stats.standardDeviation);
    
    // Get individual errors
    QList<float> errors = calculateIndividualErrors(correspondences, transform);
    
    // Identify outliers
    for (int i = 0; i < errors.size(); ++i) {
        if (errors[i] > threshold) {
            outlierIndices.append(i);
            qDebug() << "Outlier detected at index" << i << "with error" << errors[i] << "mm (threshold:" << threshold << "mm)";
        }
    }
    
    qDebug() << "Identified" << outlierIndices.size() << "outliers out of" << correspondences.size() << "correspondences";
    return outlierIndices;
}

bool ErrorAnalysis::validateTransformation(const QMatrix4x4& transform)
{
    // Extract rotation matrix (top-left 3x3) and calculate determinant manually
    float r00 = transform(0, 0), r01 = transform(0, 1), r02 = transform(0, 2);
    float r10 = transform(1, 0), r11 = transform(1, 1), r12 = transform(1, 2);
    float r20 = transform(2, 0), r21 = transform(2, 1), r22 = transform(2, 2);

    // Calculate determinant manually: det(R) = r00(r11*r22 - r12*r21) - r01(r10*r22 - r12*r20) + r02(r10*r21 - r11*r20)
    float determinant = r00 * (r11 * r22 - r12 * r21) -
                       r01 * (r10 * r22 - r12 * r20) +
                       r02 * (r10 * r21 - r11 * r20);
    if (determinant < MIN_DETERMINANT || determinant > MAX_DETERMINANT) {
        qWarning() << "Invalid rotation matrix determinant:" << determinant;
        return false;
    }
    
    // Check translation magnitude (should be reasonable)
    QVector3D translation(transform(0, 3), transform(1, 3), transform(2, 3));
    float translationMagnitude = translation.length();
    if (translationMagnitude > MAX_TRANSLATION_MAGNITUDE) {
        qWarning() << "Excessive translation magnitude:" << translationMagnitude << "m";
        return false;
    }
    
    // Check for NaN or infinite values
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float value = transform(row, col);
            if (std::isnan(value) || std::isinf(value)) {
                qWarning() << "Invalid transformation matrix value at (" << row << "," << col << "):" << value;
                return false;
            }
        }
    }
    
    qDebug() << "Transformation matrix validation passed";
    return true;
}

float ErrorAnalysis::calculateConditionNumber(
    const QList<QPair<QVector3D, QVector3D>>& correspondences)
{
    if (correspondences.size() < 3) {
        return std::numeric_limits<float>::infinity();
    }
    
    // Calculate point spread for source points
    QVector3D minPoint = correspondences.first().first;
    QVector3D maxPoint = correspondences.first().first;
    
    for (const auto& pair : correspondences) {
        const QVector3D& point = pair.first;
        minPoint.setX(std::min(minPoint.x(), point.x()));
        minPoint.setY(std::min(minPoint.y(), point.y()));
        minPoint.setZ(std::min(minPoint.z(), point.z()));
        maxPoint.setX(std::max(maxPoint.x(), point.x()));
        maxPoint.setY(std::max(maxPoint.y(), point.y()));
        maxPoint.setZ(std::max(maxPoint.z(), point.z()));
    }
    
    QVector3D spread = maxPoint - minPoint;
    float maxSpread = std::max({spread.x(), spread.y(), spread.z()});
    float minSpread = std::min({spread.x(), spread.y(), spread.z()});
    
    if (minSpread < 1e-6f) {
        return std::numeric_limits<float>::infinity(); // Degenerate case
    }
    
    return maxSpread / minSpread;
}

QVector3D ErrorAnalysis::transformPoint(const QVector3D& point, const QMatrix4x4& transform)
{
    return transform.map(point);
}

float ErrorAnalysis::calculateStandardDeviation(const QList<float>& values, float mean)
{
    if (values.size() <= 1) {
        return 0.0f;
    }
    
    float sumOfSquaredDeviations = 0.0f;
    for (float value : values) {
        float deviation = value - mean;
        sumOfSquaredDeviations += deviation * deviation;
    }
    
    return std::sqrt(sumOfSquaredDeviations / (values.size() - 1));
}

bool ErrorAnalysis::ErrorStatistics::meetsQualityThresholds(float rmsThreshold, float maxThreshold) const
{
    return (rmsError <= rmsThreshold) && (maxError <= maxThreshold);
}

QString ErrorAnalysis::ErrorStatistics::generateReport() const
{
    QString qualityLevel;
    if (rmsError <= EXCELLENT_RMS_THRESHOLD) {
        qualityLevel = "Excellent";
    } else if (rmsError <= GOOD_RMS_THRESHOLD) {
        qualityLevel = "Good";
    } else if (rmsError <= ACCEPTABLE_RMS_THRESHOLD) {
        qualityLevel = "Acceptable";
    } else {
        qualityLevel = "Poor";
    }
    
    return QString("Alignment Quality Report:\n"
                  "Quality Level: %1\n"
                  "RMS Error: %2 mm\n"
                  "Mean Error: %3 mm\n"
                  "Max Error: %4 mm\n"
                  "Min Error: %5 mm\n"
                  "Std Deviation: %6 mm\n"
                  "Correspondences: %7")
           .arg(qualityLevel)
           .arg(rmsError, 0, 'f', 3)
           .arg(meanError, 0, 'f', 3)
           .arg(maxError, 0, 'f', 3)
           .arg(minError, 0, 'f', 3)
           .arg(standardDeviation, 0, 'f', 3)
           .arg(numCorrespondences);
}
