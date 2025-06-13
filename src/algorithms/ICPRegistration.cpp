#include "ICPRegistration.h"

#include <QDebug>

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

#include "LeastSquaresAlignment.h"

// PointCloud implementation
PointCloud::PointCloud(const std::vector<float>& pointData)
{
    if (pointData.size() % 3 != 0)
    {
        qWarning() << "Point data size must be multiple of 3";
        return;
    }

    points.reserve(pointData.size() / 3);
    for (size_t i = 0; i < pointData.size(); i += 3)
    {
        points.emplace_back(pointData[i], pointData[i + 1], pointData[i + 2]);
    }
}

void PointCloud::transform(const QMatrix4x4& transformation)
{
    for (auto& point : points)
    {
        point = transformation.map(point);
    }

    // Transform normals if present (only rotation part)
    if (!normals.empty())
    {
        QMatrix3x3 rotationMatrix = transformation.normalMatrix();
        for (auto& normal : normals)
        {
            // Manual matrix-vector multiplication for QMatrix3x3 * QVector3D
            float x = rotationMatrix(0, 0) * normal.x() + rotationMatrix(0, 1) * normal.y() +
                      rotationMatrix(0, 2) * normal.z();
            float y = rotationMatrix(1, 0) * normal.x() + rotationMatrix(1, 1) * normal.y() +
                      rotationMatrix(1, 2) * normal.z();
            float z = rotationMatrix(2, 0) * normal.x() + rotationMatrix(2, 1) * normal.y() +
                      rotationMatrix(2, 2) * normal.z();
            normal = QVector3D(x, y, z);
            normal.normalize();
        }
    }
}

PointCloud PointCloud::subsample(float ratio) const
{
    if (ratio >= 1.0f)
        return *this;
    if (ratio <= 0.0f)
        return PointCloud();

    PointCloud result;
    size_t targetSize = static_cast<size_t>(points.size() * ratio);

    if (targetSize == 0)
        return result;

    // Random subsampling
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    result.points.reserve(targetSize);
    if (!normals.empty())
    {
        result.normals.reserve(targetSize);
    }

    for (size_t i = 0; i < points.size() && result.points.size() < targetSize; ++i)
    {
        if (dis(gen) < ratio)
        {
            result.points.push_back(points[i]);
            if (!normals.empty())
            {
                result.normals.push_back(normals[i]);
            }
        }
    }

    return result;
}

// KDTree implementation
struct KDTree::Node
{
    QVector3D point;
    int axis;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;

    Node(const QVector3D& pt, int ax) : point(pt), axis(ax) {}
};

KDTree::KDTree(const PointCloud& cloud)
{
    if (cloud.empty())
        return;

    std::vector<std::pair<QVector3D, int>> points;
    points.reserve(cloud.size());
    for (size_t i = 0; i < cloud.size(); ++i)
    {
        points.emplace_back(cloud.points[i], static_cast<int>(i));
    }

    root_ = buildTree(points, 0);
}

KDTree::~KDTree() = default;

std::unique_ptr<KDTree::Node> KDTree::buildTree(std::vector<std::pair<QVector3D, int>>& points, int depth)
{
    if (points.empty())
        return nullptr;

    int axis = depth % 3;

    // Sort points by current axis
    std::sort(
        points.begin(), points.end(), [axis](const auto& a, const auto& b) { return a.first[axis] < b.first[axis]; });

    size_t median = points.size() / 2;
    auto node = std::make_unique<Node>(points[median].first, axis);

    // Split points for left and right subtrees
    std::vector<std::pair<QVector3D, int>> leftPoints(points.begin(), points.begin() + median);
    std::vector<std::pair<QVector3D, int>> rightPoints(points.begin() + median + 1, points.end());

    node->left = buildTree(leftPoints, depth + 1);
    node->right = buildTree(rightPoints, depth + 1);

    return node;
}

bool KDTree::findNearestNeighbor(const QVector3D& query, QVector3D& nearest, float& distance) const
{
    if (!root_)
        return false;

    distance = std::numeric_limits<float>::max();
    findNearest(root_, query, nearest, distance, 0);
    return distance < std::numeric_limits<float>::max();
}

bool KDTree::findNearestNeighbor(const QVector3D& query, float maxDistance, QVector3D& nearest, float& distance) const
{
    if (!findNearestNeighbor(query, nearest, distance))
    {
        return false;
    }
    return distance <= maxDistance;
}

void KDTree::findNearest(const std::unique_ptr<Node>& node,
                         const QVector3D& query,
                         QVector3D& bestPoint,
                         float& bestDistance,
                         int depth) const
{
    if (!node)
        return;

    // Calculate distance to current node
    float dist = (node->point - query).length();
    if (dist < bestDistance)
    {
        bestDistance = dist;
        bestPoint = node->point;
    }

    int axis = depth % 3;
    float diff = query[axis] - node->point[axis];

    // Choose which subtree to search first
    auto& firstChild = (diff < 0) ? node->left : node->right;
    auto& secondChild = (diff < 0) ? node->right : node->left;

    // Search the closer subtree first
    findNearest(firstChild, query, bestPoint, bestDistance, depth + 1);

    // Check if we need to search the other subtree
    if (std::abs(diff) < bestDistance)
    {
        findNearest(secondChild, query, bestPoint, bestDistance, depth + 1);
    }
}

// ICPRegistration implementation
ICPRegistration::ICPRegistration(QObject* parent) : QObject(parent), m_isCancelled(false), m_isRunning(false) {}

QMatrix4x4 ICPRegistration::compute(const PointCloud& source,
                                    const PointCloud& target,
                                    const QMatrix4x4& initialGuess,
                                    const ICPParams& params)
{
    if (source.empty() || target.empty())
    {
        qWarning() << "Cannot perform ICP on empty point clouds";
        emit computationFinished(false, QMatrix4x4(), 0.0f, 0);
        return QMatrix4x4();
    }

    m_isRunning = true;
    m_isCancelled = false;

    qDebug() << "Starting ICP with" << source.size() << "source points and" << target.size() << "target points";

    // Subsample if requested
    PointCloud workingSource = (params.subsamplingRatio < 1.0f) ? source.subsample(params.subsamplingRatio) : source;
    PointCloud workingTarget = (params.subsamplingRatio < 1.0f) ? target.subsample(params.subsamplingRatio) : target;

    qDebug() << "Working with" << workingSource.size() << "source points and" << workingTarget.size()
             << "target points after subsampling";

    // Build K-D tree for target
    auto kdTree = buildKDTree(workingTarget);

    // Initialize transformation
    QMatrix4x4 currentTransform = initialGuess;
    PointCloud transformedSource = workingSource;
    transformedSource.transform(currentTransform);

    float previousError = std::numeric_limits<float>::max();
    int iteration = 0;

    for (iteration = 0; iteration < params.maxIterations; ++iteration)
    {
        if (m_isCancelled)
        {
            qDebug() << "ICP cancelled at iteration" << iteration;
            break;
        }

        // Find correspondences
        auto correspondences = findCorrespondences(transformedSource, workingTarget, params.maxCorrespondenceDistance);

        if (correspondences.empty())
        {
            qWarning() << "No correspondences found at iteration" << iteration;
            break;
        }

        // Remove outliers if enabled
        if (params.useOutlierRejection)
        {
            correspondences = removeOutliers(correspondences, params.outlierThreshold);
        }

        if (correspondences.size() < 3)
        {
            qWarning() << "Insufficient correspondences after outlier removal:" << correspondences.size();
            break;
        }

        // Calculate current error
        float currentError = calculateRMSError(correspondences);

        // Emit progress
        emit progressUpdated(iteration, currentError, currentTransform);

        // Check convergence
        if (hasConverged(currentError, previousError, params.convergenceThreshold))
        {
            qDebug() << "ICP converged at iteration" << iteration << "with RMS error" << currentError;
            break;
        }

        // Compute incremental transformation
        QMatrix4x4 incrementalTransform = computeTransformation(correspondences);

        // Update overall transformation
        currentTransform = incrementalTransform * currentTransform;

        // Apply transformation to source
        transformedSource = workingSource;
        transformedSource.transform(currentTransform);

        previousError = currentError;
    }

    float finalError = 0.0f;
    if (!m_isCancelled)
    {
        // Calculate final error
        auto finalCorrespondences =
            findCorrespondences(transformedSource, workingTarget, params.maxCorrespondenceDistance);
        if (!finalCorrespondences.empty())
        {
            finalError = calculateRMSError(finalCorrespondences);
        }
    }

    m_isRunning = false;

    bool success = !m_isCancelled && iteration < params.maxIterations;
    emit computationFinished(success, currentTransform, finalError, iteration);

    qDebug() << "ICP finished. Success:" << success << "Final RMS:" << finalError << "Iterations:" << iteration;

    return currentTransform;
}

void ICPRegistration::cancel()
{
    m_isCancelled = true;
}

std::unique_ptr<KDTree> ICPRegistration::buildKDTree(const PointCloud& target)
{
    return std::make_unique<KDTree>(target);
}

std::vector<Correspondence>
ICPRegistration::findCorrespondences(const PointCloud& source, const PointCloud& target, float maxDistance)
{
    std::vector<Correspondence> correspondences;
    correspondences.reserve(source.size());

    auto kdTree = buildKDTree(target);

    for (const auto& sourcePoint : source.points)
    {
        QVector3D nearestPoint;
        float distance;

        if (kdTree->findNearestNeighbor(sourcePoint, maxDistance, nearestPoint, distance))
        {
            correspondences.emplace_back(sourcePoint, nearestPoint, distance);
        }
    }

    qDebug() << "Found" << correspondences.size() << "correspondences out of" << source.size() << "source points";

    return correspondences;
}

QMatrix4x4 ICPRegistration::computeTransformation(const std::vector<Correspondence>& correspondences)
{
    if (correspondences.size() < 3)
    {
        qWarning() << "Insufficient correspondences for transformation computation:" << correspondences.size();
        return QMatrix4x4();
    }

    // Convert correspondences to point pairs for LeastSquaresAlignment
    QList<QPair<QVector3D, QVector3D>> pointPairs;
    pointPairs.reserve(static_cast<int>(correspondences.size()));

    for (const auto& corr : correspondences)
    {
        if (corr.isValid)
        {
            pointPairs.append(qMakePair(corr.sourcePoint, corr.targetPoint));
        }
    }

    if (pointPairs.size() < 3)
    {
        qWarning() << "Insufficient valid correspondences for transformation computation:" << pointPairs.size();
        return QMatrix4x4();
    }

    // Use LeastSquaresAlignment from Sprint 4
    return LeastSquaresAlignment::computeTransformation(pointPairs);
}

float ICPRegistration::calculateRMSError(const std::vector<Correspondence>& correspondences)
{
    if (correspondences.empty())
        return 0.0f;

    float sumSquaredErrors = 0.0f;
    int validCount = 0;

    for (const auto& corr : correspondences)
    {
        if (corr.isValid)
        {
            float error = (corr.sourcePoint - corr.targetPoint).lengthSquared();
            sumSquaredErrors += error;
            validCount++;
        }
    }

    if (validCount == 0)
        return 0.0f;

    return std::sqrt(sumSquaredErrors / validCount);
}

std::vector<Correspondence> ICPRegistration::removeOutliers(const std::vector<Correspondence>& correspondences,
                                                            float threshold)
{
    if (correspondences.empty())
        return correspondences;

    // Calculate mean and standard deviation of distances
    std::vector<float> distances;
    distances.reserve(correspondences.size());

    for (const auto& corr : correspondences)
    {
        if (corr.isValid)
        {
            distances.push_back(corr.distance);
        }
    }

    if (distances.empty())
        return correspondences;

    float mean = std::accumulate(distances.begin(), distances.end(), 0.0f) / distances.size();

    float variance = 0.0f;
    for (float dist : distances)
    {
        variance += (dist - mean) * (dist - mean);
    }
    variance /= distances.size();
    float stdDev = std::sqrt(variance);

    // Filter correspondences based on threshold
    float maxDistance = mean + threshold * stdDev;

    std::vector<Correspondence> filtered;
    filtered.reserve(correspondences.size());

    int removedCount = 0;
    for (const auto& corr : correspondences)
    {
        if (corr.isValid && corr.distance <= maxDistance)
        {
            filtered.push_back(corr);
        }
        else
        {
            removedCount++;
        }
    }

    qDebug() << "Outlier removal: kept" << filtered.size() << "removed" << removedCount << "threshold:" << maxDistance;

    return filtered;
}

bool ICPRegistration::hasConverged(float currentError, float previousError, float threshold)
{
    if (previousError == std::numeric_limits<float>::max())
    {
        return false;  // First iteration
    }

    float errorChange = std::abs(previousError - currentError);
    return errorChange < threshold;
}

ICPParams ICPRegistration::getRecommendedParameters(const PointCloud& source, const PointCloud& target)
{
    ICPParams params;

    if (source.empty() || target.empty())
    {
        qWarning() << "Cannot calculate recommended parameters for empty point clouds";
        return params; // Return default parameters
    }

    // Calculate bounding box diagonal for both clouds to estimate scale
    auto calculateBoundingBoxDiagonal = [](const PointCloud& cloud) -> float {
        if (cloud.empty()) return 1.0f;

        QVector3D minPoint = cloud.points[0];
        QVector3D maxPoint = cloud.points[0];

        for (const auto& point : cloud.points)
        {
            minPoint.setX(std::min(minPoint.x(), point.x()));
            minPoint.setY(std::min(minPoint.y(), point.y()));
            minPoint.setZ(std::min(minPoint.z(), point.z()));

            maxPoint.setX(std::max(maxPoint.x(), point.x()));
            maxPoint.setY(std::max(maxPoint.y(), point.y()));
            maxPoint.setZ(std::max(maxPoint.z(), point.z()));
        }

        return (maxPoint - minPoint).length();
    };

    float sourceDiagonal = calculateBoundingBoxDiagonal(source);
    float targetDiagonal = calculateBoundingBoxDiagonal(target);
    float avgDiagonal = (sourceDiagonal + targetDiagonal) / 2.0f;

    // Set parameters based on point cloud characteristics
    size_t totalPoints = source.size() + target.size();

    // Max iterations: more for larger/denser clouds
    if (totalPoints > 1000000) // > 1M points
    {
        params.maxIterations = 100;
    }
    else if (totalPoints > 100000) // > 100K points
    {
        params.maxIterations = 75;
    }
    else
    {
        params.maxIterations = 50;
    }

    // Convergence threshold: tighter for larger clouds
    if (totalPoints > 500000)
    {
        params.convergenceThreshold = 1e-6f;
    }
    else
    {
        params.convergenceThreshold = 1e-5f;
    }

    // Max correspondence distance: 5-10% of average bounding box diagonal
    params.maxCorrespondenceDistance = std::max(0.01f, avgDiagonal * 0.075f);

    // Outlier rejection: always recommended
    params.useOutlierRejection = true;
    params.outlierThreshold = 2.5f; // 2.5 standard deviations

    // Subsampling: for very large clouds
    if (totalPoints > 2000000) // > 2M points
    {
        params.subsamplingRatio = 0.5f; // Use 50% of points
    }
    else if (totalPoints > 1000000) // > 1M points
    {
        params.subsamplingRatio = 0.75f; // Use 75% of points
    }
    else
    {
        params.subsamplingRatio = 1.0f; // Use all points
    }

    qDebug() << "Recommended ICP parameters calculated:"
             << "maxIterations:" << params.maxIterations
             << "convergenceThreshold:" << params.convergenceThreshold
             << "maxCorrespondenceDistance:" << params.maxCorrespondenceDistance
             << "subsamplingRatio:" << params.subsamplingRatio
             << "for" << totalPoints << "total points, avg diagonal:" << avgDiagonal;

    return params;
}
