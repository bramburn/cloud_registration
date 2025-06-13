#pragma once

#include <QMatrix4x4>
#include <QObject>
#include <QVector>

#include <memory>

#include "core/pointdata.h"

namespace Analysis
{
/**
 * @brief Analysis tools for comparing point clouds and assessing registration quality
 */
class DifferenceAnalysis : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Parameters for difference analysis
     */
    struct Parameters
    {
        float maxSearchDistance = 1.0f;  // Maximum distance for nearest neighbor search
        bool useKDTree = true;           // Use KD-tree for fast search
        int subsampleRatio = 1;          // Subsample points (1 = no subsampling)
        bool bidirectional = false;      // Calculate distances both ways

        // Quality assessment
        float outlierThreshold = 0.1f;  // Distance threshold for outliers (10cm)
        bool removeOutliers = true;     // Remove outliers from statistics
    };

    /**
     * @brief Statistics for difference analysis
     */
    struct Statistics
    {
        float meanDistance = 0.0f;
        float medianDistance = 0.0f;
        float rmsDistance = 0.0f;
        float maxDistance = 0.0f;
        float minDistance = 0.0f;
        float standardDeviation = 0.0f;

        // Quality metrics
        int totalPoints = 0;
        int validDistances = 0;
        int outliers = 0;
        float outlierPercentage = 0.0f;

        // Percentiles
        float percentile90 = 0.0f;
        float percentile95 = 0.0f;
        float percentile99 = 0.0f;
    };

    explicit DifferenceAnalysis(QObject* parent = nullptr);

    /**
     * @brief Calculate distances between source and target point clouds
     * @param sourcePoints Source point cloud
     * @param targetPoints Target point cloud
     * @param transform Optional transformation to apply to source points
     * @param params Analysis parameters
     * @return Vector of distances (one per source point)
     */
    QVector<float> calculateDistances(const std::vector<Point3D>& sourcePoints,
                                      const std::vector<Point3D>& targetPoints,
                                      const QMatrix4x4& transform = QMatrix4x4(),
                                      const Parameters& params = Parameters());

    /**
     * @brief Calculate comprehensive statistics from distance vector
     * @param distances Vector of distances
     * @param params Analysis parameters
     * @return Statistical analysis
     */
    Statistics calculateStatistics(const QVector<float>& distances, const Parameters& params = Parameters()) const;

    /**
     * @brief Generate color map values for visualization
     * @param distances Vector of distances
     * @param maxDistance Maximum distance for color mapping
     * @return Vector of normalized values [0,1] for color mapping
     */
    QVector<float> generateColorMapValues(const QVector<float>& distances, float maxDistance = -1.0f) const;

    /**
     * @brief Assess registration quality based on distance analysis
     * @param statistics Distance statistics
     * @param params Analysis parameters
     * @return Quality score [0,1] where 1 is perfect alignment
     */
    float assessRegistrationQuality(const Statistics& statistics, const Parameters& params = Parameters()) const;

    /**
     * @brief Get recommended parameters based on point cloud characteristics
     * @param sourcePoints Source point cloud
     * @param targetPoints Target point cloud
     * @return Recommended analysis parameters
     */
    Parameters getRecommendedParameters(const std::vector<Point3D>& sourcePoints,
                                        const std::vector<Point3D>& targetPoints) const;

    /**
     * @brief Create detailed analysis report
     * @param statistics Distance statistics
     * @param params Analysis parameters
     * @return Human-readable analysis report
     */
    QString generateAnalysisReport(const Statistics& statistics, const Parameters& params = Parameters()) const;

signals:
    void analysisProgress(int percentage);
    void analysisCompleted(const Statistics& statistics);

private:
    /**
     * @brief Simple KD-tree implementation for nearest neighbor search
     */
    class KDTree
    {
    public:
        explicit KDTree(const std::vector<Point3D>& points);

        /**
         * @brief Find nearest neighbor to query point
         * @param query Query point
         * @param maxDistance Maximum search distance
         * @return Distance to nearest neighbor (or maxDistance if none found)
         */
        float findNearestDistance(const Point3D& query, float maxDistance) const;

    private:
        struct Node
        {
            Point3D point;
            int axis;
            std::unique_ptr<Node> left;
            std::unique_ptr<Node> right;

            Node(const Point3D& p, int a) : point(p), axis(a) {}
        };

        std::unique_ptr<Node> m_root;

        std::unique_ptr<Node> buildTree(std::vector<Point3D>& points, int depth) const;
        float searchNearest(const Node* node, const Point3D& query, float maxDistance, int depth) const;
        float pointDistance(const Point3D& p1, const Point3D& p2) const;
    };

    /**
     * @brief Brute force nearest neighbor search (fallback)
     * @param query Query point
     * @param targetPoints Target point cloud
     * @param maxDistance Maximum search distance
     * @return Distance to nearest neighbor
     */
    float
    bruteForceNearestDistance(const Point3D& query, const std::vector<Point3D>& targetPoints, float maxDistance) const;

    /**
     * @brief Apply transformation to point
     * @param point Input point
     * @param transform Transformation matrix
     * @return Transformed point
     */
    Point3D transformPoint(const Point3D& point, const QMatrix4x4& transform) const;

    /**
     * @brief Calculate percentile from sorted distances
     * @param sortedDistances Sorted distance vector
     * @param percentile Percentile to calculate (0-100)
     * @return Percentile value
     */
    float calculatePercentile(const QVector<float>& sortedDistances, float percentile) const;

    /**
     * @brief Remove outliers from distance vector
     * @param distances Input distances
     * @param threshold Outlier threshold
     * @return Filtered distances
     */
    QVector<float> removeOutliers(const QVector<float>& distances, float threshold) const;

    /**
     * @brief Calculate point cloud bounds for parameter estimation
     * @param points Input point cloud
     * @return Bounding box diagonal length
     */
    float calculateBounds(const std::vector<Point3D>& points) const;
};

}  // namespace Analysis
