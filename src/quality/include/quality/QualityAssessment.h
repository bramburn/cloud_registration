#ifndef QUALITYASSESSMENT_H
#define QUALITYASSESSMENT_H

#include <QMatrix4x4>
#include <QObject>
#include <QVariant>
#include <QVector3D>

#include <memory>
#include <vector>

/**
 * @brief Point structure for quality assessment
 */
struct QualityPoint
{
    float x, y, z;
    float intensity = 0.0f;

    QualityPoint() = default;
    QualityPoint(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    QualityPoint(float x_, float y_, float z_, float intensity_) : x(x_), y(y_), z(z_), intensity(intensity_) {}

    QVector3D toVector3D() const
    {
        return QVector3D(x, y, z);
    }
};

/**
 * @brief Target correspondence for quality assessment
 */
struct QualityCorrespondence
{
    QVector3D sourcePoint;
    QVector3D targetPoint;
    float confidence = 1.0f;
    QString description;

    QualityCorrespondence() = default;
    QualityCorrespondence(const QVector3D& src, const QVector3D& tgt, float conf = 1.0f)
        : sourcePoint(src), targetPoint(tgt), confidence(conf)
    {
    }
};

/**
 * @brief Quality metrics structure
 */
struct QualityMetrics
{
    // Alignment accuracy
    float rmsError = 0.0f;
    float meanError = 0.0f;
    float standardDeviation = 0.0f;
    float minError = 0.0f;
    float maxError = 0.0f;

    // Coverage analysis
    float overlapPercentage = 0.0f;
    size_t correspondenceCount = 0;

    // Density metrics
    float averagePointDensity = 0.0f;
    float densityVariation = 0.0f;

    // Geometric features
    float planarity = 0.0f;
    float sphericity = 0.0f;
    float linearity = 0.0f;

    // Overall quality
    char qualityGrade = 'F';  // A-F grading
    float confidenceScore = 0.0f;

    // Additional statistics
    size_t totalPoints = 0;
    size_t validCorrespondences = 0;
    float processingTime = 0.0f;
};

/**
 * @brief Quality report container
 */
struct QualityReport
{
    QualityMetrics metrics;
    QString projectName;
    QString description;
    QString timestamp;
    QStringList recommendations;
    QVariantMap additionalData;

    QString generateSummary() const;
    QString generateDetailedReport() const;
};

/**
 * @brief Registration Quality Assessment Engine
 *
 * Sprint 6 User Story 2: Registration Quality Assessment and Reporting
 * Provides comprehensive quality analysis for point cloud registration results.
 */
class QualityAssessment : public QObject
{
    Q_OBJECT

public:
    explicit QualityAssessment(QObject* parent = nullptr);
    ~QualityAssessment() override;

    // Main assessment methods
    QualityMetrics calculateErrorMetrics(const std::vector<QualityCorrespondence>& correspondences);
    float calculateOverlapPercentage(const std::vector<QualityPoint>& cloud1,
                                     const std::vector<QualityPoint>& cloud2,
                                     float tolerance = 0.1f);

    // Comprehensive analysis
    QualityReport assessRegistration(const std::vector<QualityPoint>& sourceCloud,
                                     const std::vector<QualityPoint>& targetCloud,
                                     const QMatrix4x4& transformation,
                                     const std::vector<QualityCorrespondence>& correspondences);

    // Individual metric calculations
    float calculateRMSError(const std::vector<QualityCorrespondence>& correspondences);
    QualityMetrics calculateStatistics(const std::vector<float>& errors);
    float calculateDensityMetrics(const std::vector<QualityPoint>& cloud);
    void calculateGeometricFeatures(const std::vector<QualityPoint>& cloud,
                                    float& planarity,
                                    float& sphericity,
                                    float& linearity);

    // Quality grading
    char calculateQualityGrade(const QualityMetrics& metrics);
    float calculateConfidenceScore(const QualityMetrics& metrics);
    QStringList generateRecommendations(const QualityMetrics& metrics);

    // Configuration
    void setToleranceThreshold(float tolerance)
    {
        m_toleranceThreshold = tolerance;
    }
    void setDensityRadius(float radius)
    {
        m_densityRadius = radius;
    }
    void setMinCorrespondences(size_t count)
    {
        m_minCorrespondences = count;
    }

signals:
    void assessmentProgress(int percentage, const QString& stage);
    void assessmentCompleted(const QualityReport& report);
    void assessmentError(const QString& error);

private:
    // Helper methods
    std::vector<QualityPoint> transformPointCloud(const std::vector<QualityPoint>& cloud,
                                                  const QMatrix4x4& transformation);
    float calculatePointToPointDistance(const QVector3D& p1, const QVector3D& p2);
    std::vector<float> calculateAllDistances(const std::vector<QualityCorrespondence>& correspondences);

    // Geometric analysis helpers
    QMatrix3x3 calculateCovarianceMatrix(const std::vector<QualityPoint>& cloud);
    std::vector<float> calculateEigenvalues(const QMatrix3x3& matrix);
    float calculateLocalDensity(const std::vector<QualityPoint>& cloud, const QVector3D& point, float radius);

    // Configuration parameters
    float m_toleranceThreshold = 0.05f;  // 5cm default tolerance
    float m_densityRadius = 0.1f;        // 10cm radius for density calculation
    size_t m_minCorrespondences = 3;     // Minimum correspondences for valid assessment

    // Internal state
    bool m_isAssessing = false;
};

#endif  // QUALITYASSESSMENT_H
