#ifndef ICPREGISTRATION_H
#define ICPREGISTRATION_H

#include <QList>
#include <QMatrix4x4>
#include <QObject>
#include <QPair>
#include <QVector3D>

#include <atomic>
#include <memory>
#include <vector>

/**
 * @brief Parameters for ICP algorithm configuration
 */
struct ICPParams
{
    int maxIterations = 50;
    float convergenceThreshold = 1e-5f;
    float maxCorrespondenceDistance = 0.1f;
    bool useOutlierRejection = true;
    float outlierThreshold = 2.0f;  // Standard deviations for outlier rejection
    float subsamplingRatio = 1.0f;  // 1.0 = no subsampling, 0.1 = 10% of points
};

/**
 * @brief Point cloud data structure for ICP processing
 */
struct PointCloud
{
    std::vector<QVector3D> points;
    std::vector<QVector3D> normals;  // Optional, for point-to-plane ICP

    PointCloud() = default;
    PointCloud(const std::vector<float>& pointData);
    PointCloud(const std::vector<QVector3D>& pts) : points(pts) {}

    size_t size() const
    {
        return points.size();
    }
    bool empty() const
    {
        return points.empty();
    }
    void clear()
    {
        points.clear();
        normals.clear();
    }

    // Transform all points by the given matrix
    void transform(const QMatrix4x4& transformation);

    // Subsample the point cloud
    PointCloud subsample(float ratio) const;
};

/**
 * @brief Correspondence between two points
 */
struct Correspondence
{
    QVector3D sourcePoint;
    QVector3D targetPoint;
    QVector3D targetNormal;  // Optional, for point-to-plane ICP
    float distance;
    bool isValid;

    Correspondence() : distance(0.0f), isValid(false) {}
    Correspondence(const QVector3D& src, const QVector3D& tgt, float dist = 0.0f)
        : sourcePoint(src), targetPoint(tgt), distance(dist), isValid(true)
    {
    }
};

/**
 * @brief K-D Tree for efficient nearest neighbor search
 */
class KDTree
{
public:
    explicit KDTree(const PointCloud& cloud);
    ~KDTree();

    // Find nearest neighbor to query point
    bool findNearestNeighbor(const QVector3D& query, QVector3D& nearest, float& distance) const;

    // Find nearest neighbor within max distance
    bool findNearestNeighbor(const QVector3D& query, float maxDistance, QVector3D& nearest, float& distance) const;

private:
    struct Node;
    std::unique_ptr<Node> root_;

    std::unique_ptr<Node> buildTree(std::vector<std::pair<QVector3D, int>>& points, int depth = 0);
    void findNearest(const std::unique_ptr<Node>& node,
                     const QVector3D& query,
                     QVector3D& bestPoint,
                     float& bestDistance,
                     int depth = 0) const;
};

/**
 * @brief Core ICP Registration Algorithm
 *
 * Implements the Iterative Closest Point algorithm for point cloud registration.
 * This is the base class that provides point-to-point ICP functionality.
 */
class ICPRegistration : public QObject
{
    Q_OBJECT

public:
    explicit ICPRegistration(QObject* parent = nullptr);
    virtual ~ICPRegistration() = default;

    /**
     * @brief Compute transformation between source and target point clouds
     * @param source Source point cloud to be transformed
     * @param target Target point cloud (reference)
     * @param initialGuess Initial transformation estimate
     * @param params ICP algorithm parameters
     * @return Final transformation matrix
     */
    virtual QMatrix4x4 compute(const PointCloud& source,
                               const PointCloud& target,
                               const QMatrix4x4& initialGuess,
                               const ICPParams& params);

    /**
     * @brief Cancel the currently running ICP computation
     */
    void cancel();

    /**
     * @brief Check if ICP computation is currently running
     */
    bool isRunning() const
    {
        return m_isRunning;
    }

    /**
     * @brief Get recommended ICP parameters based on input point clouds
     * @param source Source point cloud
     * @param target Target point cloud
     * @return Recommended ICPParams with intelligent defaults
     */
    static ICPParams getRecommendedParameters(const PointCloud& source, const PointCloud& target);

signals:
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

protected:
    /**
     * @brief Find correspondences between source and target points
     * @param source Source point cloud
     * @param target Target point cloud
     * @param maxDistance Maximum correspondence distance
     * @return List of valid correspondences
     */
    virtual std::vector<Correspondence>
    findCorrespondences(const PointCloud& source, const PointCloud& target, float maxDistance);

    /**
     * @brief Compute transformation from correspondences
     * @param correspondences List of point correspondences
     * @return Transformation matrix
     */
    virtual QMatrix4x4 computeTransformation(const std::vector<Correspondence>& correspondences);

    /**
     * @brief Calculate RMS error for current correspondences
     * @param correspondences List of point correspondences
     * @return RMS error value
     */
    virtual float calculateRMSError(const std::vector<Correspondence>& correspondences);

    /**
     * @brief Remove outlier correspondences
     * @param correspondences Input correspondences
     * @param threshold Outlier threshold in standard deviations
     * @return Filtered correspondences
     */
    virtual std::vector<Correspondence> removeOutliers(const std::vector<Correspondence>& correspondences,
                                                       float threshold);

    /**
     * @brief Check if convergence criteria are met
     * @param currentError Current RMS error
     * @param previousError Previous RMS error
     * @param threshold Convergence threshold
     * @return True if converged
     */
    virtual bool hasConverged(float currentError, float previousError, float threshold);

protected:
    // Build K-D tree for target point cloud
    std::unique_ptr<KDTree> buildKDTree(const PointCloud& target);

private:
    std::atomic<bool> m_isCancelled;
    std::atomic<bool> m_isRunning;
};

#endif  // ICPREGISTRATION_H
