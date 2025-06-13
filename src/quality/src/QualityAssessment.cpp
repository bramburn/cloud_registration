#include "quality/QualityAssessment.h"

#include <QDateTime>
#include <QDebug>
#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <numeric>

QualityAssessment::QualityAssessment(QObject* parent) : QObject(parent) {}

QualityAssessment::~QualityAssessment() = default;

QString QualityReport::generateSummary() const
{
    QString summary;
    summary += QString("Quality Assessment Summary\n");
    summary += QString("Project: %1\n").arg(projectName);
    summary += QString("Timestamp: %1\n").arg(timestamp);
    summary += QString("Overall Grade: %1\n").arg(metrics.qualityGrade);
    summary += QString("RMS Error: %1 mm\n").arg(metrics.rmsError * 1000, 0, 'f', 2);
    summary += QString("Overlap: %1%\n").arg(metrics.overlapPercentage, 0, 'f', 1);
    summary += QString("Correspondences: %1\n").arg(metrics.correspondenceCount);
    summary += QString("Confidence: %1%\n").arg(metrics.confidenceScore * 100, 0, 'f', 1);
    return summary;
}

QString QualityReport::generateDetailedReport() const
{
    QString report;
    report += QString("=== DETAILED QUALITY ASSESSMENT REPORT ===\n\n");
    report += QString("Project Information:\n");
    report += QString("  Name: %1\n").arg(projectName);
    report += QString("  Description: %1\n").arg(description);
    report += QString("  Assessment Time: %1\n").arg(timestamp);
    report += QString("  Processing Time: %1 seconds\n\n").arg(metrics.processingTime, 0, 'f', 2);

    report += QString("Alignment Accuracy:\n");
    report += QString("  RMS Error: %1 mm\n").arg(metrics.rmsError * 1000, 0, 'f', 3);
    report += QString("  Mean Error: %1 mm\n").arg(metrics.meanError * 1000, 0, 'f', 3);
    report += QString("  Standard Deviation: %1 mm\n").arg(metrics.standardDeviation * 1000, 0, 'f', 3);
    report += QString("  Min Error: %1 mm\n").arg(metrics.minError * 1000, 0, 'f', 3);
    report += QString("  Max Error: %1 mm\n\n").arg(metrics.maxError * 1000, 0, 'f', 3);

    report += QString("Coverage Analysis:\n");
    report += QString("  Overlap Percentage: %1%\n").arg(metrics.overlapPercentage, 0, 'f', 1);
    report += QString("  Total Correspondences: %1\n").arg(metrics.correspondenceCount);
    report += QString("  Valid Correspondences: %1\n\n").arg(metrics.validCorrespondences);

    report += QString("Point Cloud Statistics:\n");
    report += QString("  Total Points: %1\n").arg(metrics.totalPoints);
    report += QString("  Average Density: %1 points/m²\n").arg(metrics.averagePointDensity, 0, 'f', 1);
    report += QString("  Density Variation: %1\n\n").arg(metrics.densityVariation, 0, 'f', 3);

    report += QString("Geometric Features:\n");
    report += QString("  Planarity: %1\n").arg(metrics.planarity, 0, 'f', 3);
    report += QString("  Sphericity: %1\n").arg(metrics.sphericity, 0, 'f', 3);
    report += QString("  Linearity: %1\n\n").arg(metrics.linearity, 0, 'f', 3);

    report += QString("Overall Assessment:\n");
    report += QString("  Quality Grade: %1\n").arg(metrics.qualityGrade);
    report += QString("  Confidence Score: %1%\n\n").arg(metrics.confidenceScore * 100, 0, 'f', 1);

    if (!recommendations.isEmpty())
    {
        report += QString("Recommendations:\n");
        for (const QString& rec : recommendations)
        {
            report += QString("  • %1\n").arg(rec);
        }
    }

    return report;
}

QualityMetrics QualityAssessment::calculateErrorMetrics(const std::vector<QualityCorrespondence>& correspondences)
{
    QualityMetrics metrics;

    if (correspondences.empty())
    {
        qWarning() << "QualityAssessment: No correspondences provided";
        return metrics;
    }

    emit assessmentProgress(10, "Calculating error metrics");

    // Calculate all distances
    std::vector<float> errors = calculateAllDistances(correspondences);

    // Calculate statistics
    metrics = calculateStatistics(errors);
    metrics.correspondenceCount = correspondences.size();
    metrics.validCorrespondences = errors.size();

    emit assessmentProgress(30, "Error metrics calculated");
    return metrics;
}

float QualityAssessment::calculateOverlapPercentage(const std::vector<QualityPoint>& cloud1,
                                                    const std::vector<QualityPoint>& cloud2,
                                                    float tolerance)
{
    if (cloud1.empty() || cloud2.empty())
    {
        return 0.0f;
    }

    emit assessmentProgress(40, "Calculating overlap percentage");

    size_t overlapCount = 0;
    const float toleranceSquared = tolerance * tolerance;

    // For each point in cloud1, check if there's a nearby point in cloud2
    for (const auto& p1 : cloud1)
    {
        QVector3D point1 = p1.toVector3D();

        for (const auto& p2 : cloud2)
        {
            QVector3D point2 = p2.toVector3D();
            float distanceSquared = (point1 - point2).lengthSquared();

            if (distanceSquared <= toleranceSquared)
            {
                overlapCount++;
                break;  // Found a match, move to next point
            }
        }
    }

    float percentage = (static_cast<float>(overlapCount) / static_cast<float>(cloud1.size())) * 100.0f;
    emit assessmentProgress(60, "Overlap percentage calculated");

    return percentage;
}

QualityReport QualityAssessment::assessRegistration(const std::vector<QualityPoint>& sourceCloud,
                                                    const std::vector<QualityPoint>& targetCloud,
                                                    const QMatrix4x4& transformation,
                                                    const std::vector<QualityCorrespondence>& correspondences)
{
    QElapsedTimer timer;
    timer.start();

    m_isAssessing = true;
    emit assessmentProgress(0, "Starting comprehensive assessment");

    QualityReport report;
    report.timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);

    try
    {
        // Transform source cloud
        auto transformedSource = transformPointCloud(sourceCloud, transformation);

        // Calculate error metrics from correspondences
        if (!correspondences.empty())
        {
            report.metrics = calculateErrorMetrics(correspondences);
        }

        // Calculate overlap percentage
        report.metrics.overlapPercentage =
            calculateOverlapPercentage(transformedSource, targetCloud, m_toleranceThreshold);

        // Calculate density metrics
        report.metrics.averagePointDensity = calculateDensityMetrics(targetCloud);

        // Calculate geometric features
        calculateGeometricFeatures(
            targetCloud, report.metrics.planarity, report.metrics.sphericity, report.metrics.linearity);

        // Set point counts
        report.metrics.totalPoints = sourceCloud.size() + targetCloud.size();

        // Calculate overall quality
        report.metrics.qualityGrade = calculateQualityGrade(report.metrics);
        report.metrics.confidenceScore = calculateConfidenceScore(report.metrics);

        // Generate recommendations
        report.recommendations = generateRecommendations(report.metrics);

        report.metrics.processingTime = timer.elapsed() / 1000.0f;

        emit assessmentProgress(100, "Assessment completed");
        emit assessmentCompleted(report);
    }
    catch (const std::exception& e)
    {
        QString error = QString("Assessment failed: %1").arg(e.what());
        emit assessmentError(error);
        qCritical() << error;
    }

    m_isAssessing = false;
    return report;
}

float QualityAssessment::calculateRMSError(const std::vector<QualityCorrespondence>& correspondences)
{
    if (correspondences.empty())
    {
        return 0.0f;
    }

    float sumSquaredErrors = 0.0f;

    for (const auto& corr : correspondences)
    {
        float distance = calculatePointToPointDistance(corr.sourcePoint, corr.targetPoint);
        sumSquaredErrors += distance * distance;
    }

    return qSqrt(sumSquaredErrors / static_cast<float>(correspondences.size()));
}

QualityMetrics QualityAssessment::calculateStatistics(const std::vector<float>& errors)
{
    QualityMetrics metrics;

    if (errors.empty())
    {
        return metrics;
    }

    // Calculate basic statistics
    float sum = std::accumulate(errors.begin(), errors.end(), 0.0f);
    metrics.meanError = sum / static_cast<float>(errors.size());

    auto minMax = std::minmax_element(errors.begin(), errors.end());
    metrics.minError = *minMax.first;
    metrics.maxError = *minMax.second;

    // Calculate RMS error
    float sumSquared = 0.0f;
    for (float error : errors)
    {
        sumSquared += error * error;
    }
    metrics.rmsError = qSqrt(sumSquared / static_cast<float>(errors.size()));

    // Calculate standard deviation
    float sumSquaredDiff = 0.0f;
    for (float error : errors)
    {
        float diff = error - metrics.meanError;
        sumSquaredDiff += diff * diff;
    }
    metrics.standardDeviation = qSqrt(sumSquaredDiff / static_cast<float>(errors.size()));

    return metrics;
}

float QualityAssessment::calculateDensityMetrics(const std::vector<QualityPoint>& cloud)
{
    if (cloud.size() < 10)
    {
        return 0.0f;
    }

    // Sample a subset of points for density calculation
    size_t sampleSize = qMin(static_cast<size_t>(100), cloud.size());
    std::vector<float> densities;
    densities.reserve(sampleSize);

    for (size_t i = 0; i < sampleSize; ++i)
    {
        size_t index = (i * cloud.size()) / sampleSize;
        QVector3D point = cloud[index].toVector3D();
        float density = calculateLocalDensity(cloud, point, m_densityRadius);
        densities.push_back(density);
    }

    // Calculate average density
    float sum = std::accumulate(densities.begin(), densities.end(), 0.0f);
    return sum / static_cast<float>(densities.size());
}

void QualityAssessment::calculateGeometricFeatures(const std::vector<QualityPoint>& cloud,
                                                   float& planarity,
                                                   float& sphericity,
                                                   float& linearity)
{
    planarity = sphericity = linearity = 0.0f;

    if (cloud.size() < 3)
    {
        return;
    }

    // Calculate covariance matrix
    QMatrix3x3 covariance = calculateCovarianceMatrix(cloud);

    // Calculate eigenvalues
    std::vector<float> eigenvalues = calculateEigenvalues(covariance);

    if (eigenvalues.size() == 3)
    {
        std::sort(eigenvalues.begin(), eigenvalues.end(), std::greater<float>());

        float e1 = eigenvalues[0];  // Largest
        float e2 = eigenvalues[1];  // Middle
        float e3 = eigenvalues[2];  // Smallest

        float sum = e1 + e2 + e3;
        if (sum > 1e-6f)
        {
            linearity = (e1 - e2) / sum;
            planarity = (e2 - e3) / sum;
            sphericity = e3 / sum;
        }
    }
}

char QualityAssessment::calculateQualityGrade(const QualityMetrics& metrics)
{
    // Grading based on RMS error (in meters)
    float rmsErrorMM = metrics.rmsError * 1000.0f;  // Convert to mm

    if (rmsErrorMM <= 1.0f && metrics.overlapPercentage >= 80.0f)
    {
        return 'A';  // Excellent
    }
    else if (rmsErrorMM <= 2.0f && metrics.overlapPercentage >= 70.0f)
    {
        return 'B';  // Good
    }
    else if (rmsErrorMM <= 5.0f && metrics.overlapPercentage >= 60.0f)
    {
        return 'C';  // Acceptable
    }
    else if (rmsErrorMM <= 10.0f && metrics.overlapPercentage >= 50.0f)
    {
        return 'D';  // Poor
    }
    else
    {
        return 'F';  // Fail
    }
}

float QualityAssessment::calculateConfidenceScore(const QualityMetrics& metrics)
{
    float score = 1.0f;

    // Penalize high RMS error
    float rmsErrorMM = metrics.rmsError * 1000.0f;
    if (rmsErrorMM > 1.0f)
    {
        score *= qMax(0.1f, 1.0f - (rmsErrorMM - 1.0f) / 10.0f);
    }

    // Penalize low overlap
    if (metrics.overlapPercentage < 80.0f)
    {
        score *= metrics.overlapPercentage / 100.0f;
    }

    // Penalize insufficient correspondences
    if (metrics.correspondenceCount < m_minCorrespondences)
    {
        score *= static_cast<float>(metrics.correspondenceCount) / static_cast<float>(m_minCorrespondences);
    }

    return qBound(0.0f, score, 1.0f);
}

QStringList QualityAssessment::generateRecommendations(const QualityMetrics& metrics)
{
    QStringList recommendations;

    float rmsErrorMM = metrics.rmsError * 1000.0f;

    if (rmsErrorMM > 5.0f)
    {
        recommendations << "High RMS error detected. Consider adding more correspondence points.";
        recommendations << "Check for systematic errors in the registration process.";
    }

    if (metrics.overlapPercentage < 60.0f)
    {
        recommendations << "Low overlap percentage. Ensure sufficient overlap between scans.";
        recommendations << "Consider repositioning scans for better coverage.";
    }

    if (metrics.correspondenceCount < m_minCorrespondences)
    {
        recommendations << QString("Insufficient correspondences (%1). Minimum recommended: %2.")
                               .arg(metrics.correspondenceCount)
                               .arg(m_minCorrespondences);
    }

    if (metrics.standardDeviation > metrics.meanError * 2.0f)
    {
        recommendations << "High error variation detected. Check for outlier correspondences.";
    }

    if (metrics.qualityGrade >= 'C')
    {
        recommendations << "Registration quality is acceptable for most applications.";
    }
    else
    {
        recommendations << "Registration quality needs improvement before use.";
    }

    return recommendations;
}

// Helper methods implementation
std::vector<QualityPoint> QualityAssessment::transformPointCloud(const std::vector<QualityPoint>& cloud,
                                                                 const QMatrix4x4& transformation)
{
    std::vector<QualityPoint> transformedCloud;
    transformedCloud.reserve(cloud.size());

    for (const auto& point : cloud)
    {
        QVector3D original(point.x, point.y, point.z);
        QVector3D transformed = transformation * original;

        QualityPoint newPoint;
        newPoint.x = transformed.x();
        newPoint.y = transformed.y();
        newPoint.z = transformed.z();
        newPoint.intensity = point.intensity;

        transformedCloud.push_back(newPoint);
    }

    return transformedCloud;
}

float QualityAssessment::calculatePointToPointDistance(const QVector3D& p1, const QVector3D& p2)
{
    return (p1 - p2).length();
}

std::vector<float> QualityAssessment::calculateAllDistances(const std::vector<QualityCorrespondence>& correspondences)
{
    std::vector<float> distances;
    distances.reserve(correspondences.size());

    for (const auto& corr : correspondences)
    {
        float distance = calculatePointToPointDistance(corr.sourcePoint, corr.targetPoint);
        distances.push_back(distance);
    }

    return distances;
}

QMatrix3x3 QualityAssessment::calculateCovarianceMatrix(const std::vector<QualityPoint>& cloud)
{
    if (cloud.empty())
    {
        return QMatrix3x3();
    }

    // Calculate centroid
    QVector3D centroid(0, 0, 0);
    for (const auto& point : cloud)
    {
        centroid += point.toVector3D();
    }
    centroid /= static_cast<float>(cloud.size());

    // Calculate covariance matrix
    QMatrix3x3 covariance;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            covariance(i, j) = 0.0f;
        }
    }

    for (const auto& point : cloud)
    {
        QVector3D p = point.toVector3D() - centroid;

        covariance(0, 0) += p.x() * p.x();
        covariance(0, 1) += p.x() * p.y();
        covariance(0, 2) += p.x() * p.z();
        covariance(1, 0) += p.y() * p.x();
        covariance(1, 1) += p.y() * p.y();
        covariance(1, 2) += p.y() * p.z();
        covariance(2, 0) += p.z() * p.x();
        covariance(2, 1) += p.z() * p.y();
        covariance(2, 2) += p.z() * p.z();
    }

    float n = static_cast<float>(cloud.size());
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            covariance(i, j) /= n;
        }
    }

    return covariance;
}

std::vector<float> QualityAssessment::calculateEigenvalues(const QMatrix3x3& matrix)
{
    // Simplified eigenvalue calculation for 3x3 matrix
    // This is a basic implementation - for production use, consider using Eigen library
    std::vector<float> eigenvalues;

    // For now, return diagonal elements as approximation
    eigenvalues.push_back(matrix(0, 0));
    eigenvalues.push_back(matrix(1, 1));
    eigenvalues.push_back(matrix(2, 2));

    return eigenvalues;
}

float QualityAssessment::calculateLocalDensity(const std::vector<QualityPoint>& cloud,
                                               const QVector3D& point,
                                               float radius)
{
    size_t count = 0;
    float radiusSquared = radius * radius;

    for (const auto& p : cloud)
    {
        QVector3D diff = p.toVector3D() - point;
        if (diff.lengthSquared() <= radiusSquared)
        {
            count++;
        }
    }

    // Calculate density as points per unit area (assuming 2D projection)
    float area = M_PI * radius * radius;
    return static_cast<float>(count) / area;
}

#include "QualityAssessment.moc"
