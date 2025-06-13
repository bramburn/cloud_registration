#pragma once

#include <QList>
#include <QObject>
#include <QVector3D>

#include <memory>

#include "core/pointdata.h"

namespace Features
{
/**
 * @brief Represents a detected plane in 3D space
 */
struct Plane
{
    QVector3D normal;          // Unit normal vector
    float distance;            // Distance from origin
    QVector3D centroid;        // Center point of the plane
    QList<int> inlierIndices;  // Indices of points belonging to this plane
    float confidence;          // Quality/confidence score [0,1]

    Plane() : distance(0.0f), confidence(0.0f) {}

    /**
     * @brief Calculate distance from point to plane
     * @param point 3D point
     * @return Signed distance (positive = in front of plane)
     */
    float distanceToPoint(const QVector3D& point) const
    {
        return QVector3D::dotProduct(normal, point) - distance;
    }

    /**
     * @brief Check if two planes are similar (parallel and close)
     * @param other Other plane
     * @param angleThreshold Maximum angle difference in radians
     * @param distanceThreshold Maximum distance difference
     * @return True if planes are similar
     */
    bool isSimilarTo(const Plane& other,
                     float angleThreshold = 0.087f,  // ~5 degrees
                     float distanceThreshold = 0.5f) const;
};

/**
 * @brief Feature extraction algorithms for point clouds
 */
class FeatureExtractor : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Parameters for plane extraction
     */
    struct PlaneExtractionParams
    {
        int maxIterations = 1000;         // RANSAC iterations
        float distanceThreshold = 0.02f;  // Point-to-plane distance threshold (2cm)
        int minInliers = 100;             // Minimum points for valid plane
        float minPlaneArea = 1.0f;        // Minimum plane area (m²)
        int maxPlanes = 10;               // Maximum number of planes to extract
        bool removeInliers = true;        // Remove inliers after each plane detection

        // Quality filtering
        float minConfidence = 0.5f;                      // Minimum confidence score
        bool filterByNormal = true;                      // Filter planes by normal direction
        QVector3D preferredNormal = QVector3D(0, 0, 1);  // Preferred normal (e.g., vertical)
        float normalTolerance = 0.5f;                    // Tolerance for normal filtering
    };

    explicit FeatureExtractor(QObject* parent = nullptr);

    /**
     * @brief Extract planes from point cloud using RANSAC
     * @param points Input point cloud
     * @param params Extraction parameters
     * @return List of detected planes
     */
    QList<Plane> extractPlanes(const std::vector<Point3D>& points,
                               const PlaneExtractionParams& params = PlaneExtractionParams());

    /**
     * @brief Extract single best plane from point cloud
     * @param points Input point cloud
     * @param params Extraction parameters
     * @return Best plane or invalid plane if none found
     */
    Plane extractSinglePlane(const std::vector<Point3D>& points,
                             const PlaneExtractionParams& params = PlaneExtractionParams());

    /**
     * @brief Get recommended parameters based on point cloud characteristics
     * @param points Input point cloud
     * @return Recommended extraction parameters
     */
    PlaneExtractionParams getRecommendedParameters(const std::vector<Point3D>& points) const;

    /**
     * @brief Validate extracted plane quality
     * @param plane Plane to validate
     * @param points Original point cloud
     * @return Quality score [0,1]
     */
    float validatePlaneQuality(const Plane& plane, const std::vector<Point3D>& points) const;

signals:
    void extractionProgress(int percentage);
    void planeExtracted(const Plane& plane);
    void extractionCompleted(int planesFound);

private:
    /**
     * @brief RANSAC plane fitting for single plane
     * @param points Input points
     * @param params Extraction parameters
     * @param usedIndices Indices to exclude from consideration
     * @return Best plane found
     */
    Plane ransacPlaneFitting(const std::vector<Point3D>& points,
                             const PlaneExtractionParams& params,
                             const QSet<int>& usedIndices = QSet<int>()) const;

    /**
     * @brief Fit plane to three random points
     * @param p1, p2, p3 Three points
     * @return Plane parameters (normal and distance)
     */
    std::pair<QVector3D, float>
    fitPlaneToThreePoints(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3) const;

    /**
     * @brief Count inliers for given plane
     * @param points Input points
     * @param normal Plane normal
     * @param distance Plane distance
     * @param threshold Distance threshold
     * @param usedIndices Indices to exclude
     * @return List of inlier indices
     */
    QList<int> countInliers(const std::vector<Point3D>& points,
                            const QVector3D& normal,
                            float distance,
                            float threshold,
                            const QSet<int>& usedIndices = QSet<int>()) const;

    /**
     * @brief Refine plane parameters using least squares
     * @param points Input points
     * @param inlierIndices Inlier point indices
     * @return Refined plane
     */
    Plane refinePlane(const std::vector<Point3D>& points, const QList<int>& inlierIndices) const;

    /**
     * @brief Calculate plane centroid and area
     * @param points Input points
     * @param inlierIndices Inlier point indices
     * @return Centroid and area
     */
    std::pair<QVector3D, float> calculatePlaneProperties(const std::vector<Point3D>& points,
                                                         const QList<int>& inlierIndices) const;

    /**
     * @brief Filter plane by normal direction
     * @param plane Plane to check
     * @param params Extraction parameters
     * @return True if plane passes normal filter
     */
    bool passesNormalFilter(const Plane& plane, const PlaneExtractionParams& params) const;

    /**
     * @brief Convert Point3D to QVector3D
     */
    QVector3D toQVector3D(const Point3D& point) const
    {
        return QVector3D(point.x, point.y, point.z);
    }
};

}  // namespace Features
