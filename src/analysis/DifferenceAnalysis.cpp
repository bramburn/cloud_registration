#include "DifferenceAnalysis.h"
#include <QDebug>
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>
#include <limits>

namespace Analysis {

DifferenceAnalysis::DifferenceAnalysis(QObject* parent) : QObject(parent) {
}

QVector<float> DifferenceAnalysis::calculateDistances(
    const std::vector<Point3D>& sourcePoints,
    const std::vector<Point3D>& targetPoints,
    const QMatrix4x4& transform,
    const Parameters& params) {
    
    QElapsedTimer timer;
    timer.start();
    
    QVector<float> distances;
    
    if (sourcePoints.empty() || targetPoints.empty()) {
        qWarning() << "Empty point clouds provided for distance calculation";
        emit analysisCompleted(Statistics());
        return distances;
    }
    
    qDebug() << "Calculating distances between" << sourcePoints.size() 
             << "source and" << targetPoints.size() << "target points";
    
    emit analysisProgress(0);
    
    // Build KD-tree for target points if requested
    std::unique_ptr<KDTree> kdTree;
    if (params.useKDTree) {
        qDebug() << "Building KD-tree for target points";
        kdTree = std::make_unique<KDTree>(targetPoints);
    }
    
    emit analysisProgress(20);
    
    // Calculate distances
    distances.reserve(sourcePoints.size() / params.subsampleRatio);
    
    for (size_t i = 0; i < sourcePoints.size(); i += params.subsampleRatio) {
        // Transform source point if transformation provided
        Point3D sourcePoint = sourcePoints[i];
        if (!transform.isIdentity()) {
            sourcePoint = transformPoint(sourcePoint, transform);
        }
        
        // Find nearest distance
        float nearestDistance;
        if (kdTree) {
            nearestDistance = kdTree->findNearestDistance(sourcePoint, params.maxSearchDistance);
        } else {
            nearestDistance = bruteForceNearestDistance(sourcePoint, targetPoints, params.maxSearchDistance);
        }
        
        distances.append(nearestDistance);
        
        // Update progress
        if (i % 10000 == 0) {
            int progress = 20 + (i * 60) / sourcePoints.size();
            emit analysisProgress(progress);
        }
    }
    
    emit analysisProgress(80);
    
    // Bidirectional analysis if requested
    if (params.bidirectional) {
        qDebug() << "Performing bidirectional distance analysis";
        
        // Build KD-tree for source points
        std::unique_ptr<KDTree> sourceKdTree;
        if (params.useKDTree) {
            // Create transformed source points for KD-tree
            std::vector<Point3D> transformedSource;
            transformedSource.reserve(sourcePoints.size());
            for (const auto& point : sourcePoints) {
                transformedSource.push_back(transform.isIdentity() ? point : transformPoint(point, transform));
            }
            sourceKdTree = std::make_unique<KDTree>(transformedSource);
        }
        
        // Calculate distances from target to source
        for (size_t i = 0; i < targetPoints.size(); i += params.subsampleRatio) {
            const Point3D& targetPoint = targetPoints[i];
            
            float nearestDistance;
            if (sourceKdTree) {
                nearestDistance = sourceKdTree->findNearestDistance(targetPoint, params.maxSearchDistance);
            } else {
                // Transform source points on-the-fly for brute force search
                std::vector<Point3D> transformedSource;
                transformedSource.reserve(sourcePoints.size());
                for (const auto& point : sourcePoints) {
                    transformedSource.push_back(transform.isIdentity() ? point : transformPoint(point, transform));
                }
                nearestDistance = bruteForceNearestDistance(targetPoint, transformedSource, params.maxSearchDistance);
            }
            
            distances.append(nearestDistance);
        }
    }
    
    emit analysisProgress(100);
    
    qDebug() << "Distance calculation completed:" << distances.size() 
             << "distances calculated in" << timer.elapsed() << "ms";
    
    // Calculate and emit statistics
    Statistics stats = calculateStatistics(distances, params);
    emit analysisCompleted(stats);
    
    return distances;
}

DifferenceAnalysis::Statistics DifferenceAnalysis::calculateStatistics(
    const QVector<float>& distances, const Parameters& params) const {
    
    Statistics stats;
    
    if (distances.isEmpty()) {
        return stats;
    }
    
    // Filter out invalid distances and outliers
    QVector<float> validDistances;
    for (float distance : distances) {
        if (distance >= 0.0f && distance < params.maxSearchDistance) {
            if (!params.removeOutliers || distance <= params.outlierThreshold) {
                validDistances.append(distance);
            }
        }
    }
    
    stats.totalPoints = distances.size();
    stats.validDistances = validDistances.size();
    stats.outliers = stats.totalPoints - stats.validDistances;
    stats.outlierPercentage = (stats.totalPoints > 0) ? 
        (static_cast<float>(stats.outliers) / stats.totalPoints) * 100.0f : 0.0f;
    
    if (validDistances.isEmpty()) {
        return stats;
    }
    
    // Sort distances for percentile calculations
    QVector<float> sortedDistances = validDistances;
    std::sort(sortedDistances.begin(), sortedDistances.end());
    
    // Basic statistics
    stats.minDistance = sortedDistances.first();
    stats.maxDistance = sortedDistances.last();
    stats.medianDistance = calculatePercentile(sortedDistances, 50.0f);
    
    // Calculate mean
    double sum = 0.0;
    for (float distance : validDistances) {
        sum += distance;
    }
    stats.meanDistance = static_cast<float>(sum / validDistances.size());
    
    // Calculate RMS and standard deviation
    double sumSquares = 0.0;
    double sumSquaredDeviations = 0.0;
    for (float distance : validDistances) {
        sumSquares += distance * distance;
        double deviation = distance - stats.meanDistance;
        sumSquaredDeviations += deviation * deviation;
    }
    
    stats.rmsDistance = static_cast<float>(std::sqrt(sumSquares / validDistances.size()));
    stats.standardDeviation = static_cast<float>(std::sqrt(sumSquaredDeviations / validDistances.size()));
    
    // Calculate percentiles
    stats.percentile90 = calculatePercentile(sortedDistances, 90.0f);
    stats.percentile95 = calculatePercentile(sortedDistances, 95.0f);
    stats.percentile99 = calculatePercentile(sortedDistances, 99.0f);
    
    return stats;
}

QVector<float> DifferenceAnalysis::generateColorMapValues(
    const QVector<float>& distances, float maxDistance) const {
    
    QVector<float> colorValues;
    colorValues.reserve(distances.size());
    
    // Determine maximum distance for normalization
    if (maxDistance < 0.0f) {
        maxDistance = *std::max_element(distances.begin(), distances.end());
    }
    
    if (maxDistance <= 0.0f) {
        // All distances are zero or invalid
        colorValues.fill(0.0f, distances.size());
        return colorValues;
    }
    
    // Normalize distances to [0,1] range
    for (float distance : distances) {
        float normalizedValue = std::max(0.0f, std::min(1.0f, distance / maxDistance));
        colorValues.append(normalizedValue);
    }
    
    return colorValues;
}

float DifferenceAnalysis::assessRegistrationQuality(
    const Statistics& statistics, const Parameters& params) const {
    
    if (statistics.validDistances == 0) {
        return 0.0f;
    }
    
    // Quality based on RMS error (lower is better)
    float rmsQuality = std::exp(-statistics.rmsDistance * 20.0f);
    
    // Quality based on outlier percentage (lower is better)
    float outlierQuality = 1.0f - (statistics.outlierPercentage / 100.0f);
    
    // Quality based on 95th percentile (lower is better)
    float percentileQuality = std::exp(-statistics.percentile95 * 15.0f);
    
    // Combined quality score
    float quality = (rmsQuality + outlierQuality + percentileQuality) / 3.0f;
    
    return std::max(0.0f, std::min(1.0f, quality));
}

DifferenceAnalysis::Parameters DifferenceAnalysis::getRecommendedParameters(
    const std::vector<Point3D>& sourcePoints,
    const std::vector<Point3D>& targetPoints) const {
    
    Parameters params;
    
    // Estimate point cloud scale
    float sourceBounds = calculateBounds(sourcePoints);
    float targetBounds = calculateBounds(targetPoints);
    float avgBounds = (sourceBounds + targetBounds) / 2.0f;
    
    // Adjust parameters based on point cloud size and scale
    size_t totalPoints = sourcePoints.size() + targetPoints.size();
    
    if (totalPoints > 2000000) {
        // Large point clouds
        params.subsampleRatio = 10;
        params.maxSearchDistance = avgBounds * 0.01f; // 1% of bounds
        params.useKDTree = true;
    } else if (totalPoints > 500000) {
        // Medium point clouds
        params.subsampleRatio = 5;
        params.maxSearchDistance = avgBounds * 0.02f; // 2% of bounds
        params.useKDTree = true;
    } else {
        // Small point clouds
        params.subsampleRatio = 1;
        params.maxSearchDistance = avgBounds * 0.05f; // 5% of bounds
        params.useKDTree = (totalPoints > 10000);
    }
    
    // Set outlier threshold based on scale
    params.outlierThreshold = avgBounds * 0.001f; // 0.1% of bounds
    
    return params;
}

QString DifferenceAnalysis::generateAnalysisReport(
    const Statistics& statistics, const Parameters& params) const {
    
    QString report;
    report += "=== Point Cloud Difference Analysis Report ===\n\n";
    
    report += QString("Total Points Analyzed: %1\n").arg(statistics.totalPoints);
    report += QString("Valid Distances: %1\n").arg(statistics.validDistances);
    report += QString("Outliers: %1 (%2%)\n\n")
        .arg(statistics.outliers)
        .arg(statistics.outlierPercentage, 0, 'f', 1);
    
    report += "Distance Statistics:\n";
    report += QString("  Mean Distance: %1 m\n").arg(statistics.meanDistance, 0, 'f', 4);
    report += QString("  Median Distance: %1 m\n").arg(statistics.medianDistance, 0, 'f', 4);
    report += QString("  RMS Distance: %1 m\n").arg(statistics.rmsDistance, 0, 'f', 4);
    report += QString("  Standard Deviation: %1 m\n").arg(statistics.standardDeviation, 0, 'f', 4);
    report += QString("  Min Distance: %1 m\n").arg(statistics.minDistance, 0, 'f', 4);
    report += QString("  Max Distance: %1 m\n\n").arg(statistics.maxDistance, 0, 'f', 4);
    
    report += "Percentiles:\n";
    report += QString("  90th Percentile: %1 m\n").arg(statistics.percentile90, 0, 'f', 4);
    report += QString("  95th Percentile: %1 m\n").arg(statistics.percentile95, 0, 'f', 4);
    report += QString("  99th Percentile: %1 m\n\n").arg(statistics.percentile99, 0, 'f', 4);
    
    float quality = assessRegistrationQuality(statistics, params);
    report += QString("Registration Quality Score: %1/1.0 (%2%)\n")
        .arg(quality, 0, 'f', 3)
        .arg(quality * 100.0f, 0, 'f', 1);
    
    return report;
}

// KDTree implementation
DifferenceAnalysis::KDTree::KDTree(const std::vector<Point3D>& points) {
    if (points.empty()) {
        return;
    }

    std::vector<Point3D> pointsCopy = points;
    m_root = buildTree(pointsCopy, 0);
}

float DifferenceAnalysis::KDTree::findNearestDistance(const Point3D& query, float maxDistance) const {
    if (!m_root) {
        return maxDistance;
    }

    return searchNearest(m_root.get(), query, maxDistance, 0);
}

std::unique_ptr<DifferenceAnalysis::KDTree::Node> DifferenceAnalysis::KDTree::buildTree(
    std::vector<Point3D>& points, int depth) const {

    if (points.empty()) {
        return nullptr;
    }

    int axis = depth % 3;

    // Sort points by current axis
    std::sort(points.begin(), points.end(), [axis](const Point3D& a, const Point3D& b) {
        switch (axis) {
            case 0: return a.x < b.x;
            case 1: return a.y < b.y;
            case 2: return a.z < b.z;
            default: return false;
        }
    });

    // Find median
    size_t medianIdx = points.size() / 2;
    auto node = std::make_unique<Node>(points[medianIdx], axis);

    // Recursively build left and right subtrees
    if (medianIdx > 0) {
        std::vector<Point3D> leftPoints(points.begin(), points.begin() + medianIdx);
        node->left = buildTree(leftPoints, depth + 1);
    }

    if (medianIdx + 1 < points.size()) {
        std::vector<Point3D> rightPoints(points.begin() + medianIdx + 1, points.end());
        node->right = buildTree(rightPoints, depth + 1);
    }

    return node;
}

float DifferenceAnalysis::KDTree::searchNearest(const Node* node, const Point3D& query,
                                               float maxDistance, int depth) const {
    if (!node) {
        return maxDistance;
    }

    float bestDistance = pointDistance(query, node->point);
    bestDistance = std::min(bestDistance, maxDistance);

    int axis = depth % 3;
    float queryValue, nodeValue;

    switch (axis) {
        case 0: queryValue = query.x; nodeValue = node->point.x; break;
        case 1: queryValue = query.y; nodeValue = node->point.y; break;
        case 2: queryValue = query.z; nodeValue = node->point.z; break;
        default: queryValue = nodeValue = 0.0f; break;
    }

    // Determine which subtree to search first
    const Node* firstChild = (queryValue < nodeValue) ? node->left.get() : node->right.get();
    const Node* secondChild = (queryValue < nodeValue) ? node->right.get() : node->left.get();

    // Search the closer subtree first
    if (firstChild) {
        float childDistance = searchNearest(firstChild, query, bestDistance, depth + 1);
        bestDistance = std::min(bestDistance, childDistance);
    }

    // Check if we need to search the other subtree
    float axisDiff = std::abs(queryValue - nodeValue);
    if (axisDiff < bestDistance && secondChild) {
        float childDistance = searchNearest(secondChild, query, bestDistance, depth + 1);
        bestDistance = std::min(bestDistance, childDistance);
    }

    return bestDistance;
}

float DifferenceAnalysis::KDTree::pointDistance(const Point3D& p1, const Point3D& p2) const {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    float dz = p1.z - p2.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

// Helper methods
float DifferenceAnalysis::bruteForceNearestDistance(
    const Point3D& query,
    const std::vector<Point3D>& targetPoints,
    float maxDistance) const {

    float minDistance = maxDistance;

    for (const auto& target : targetPoints) {
        float dx = query.x - target.x;
        float dy = query.y - target.y;
        float dz = query.z - target.z;
        float distance = std::sqrt(dx*dx + dy*dy + dz*dz);

        if (distance < minDistance) {
            minDistance = distance;
        }
    }

    return minDistance;
}

Point3D DifferenceAnalysis::transformPoint(const Point3D& point, const QMatrix4x4& transform) const {
    QVector3D qPoint(point.x, point.y, point.z);
    QVector3D transformed = transform.map(qPoint);

    Point3D result;
    result.x = transformed.x();
    result.y = transformed.y();
    result.z = transformed.z();
    result.intensity = point.intensity;
    result.hasIntensity = point.hasIntensity;

    return result;
}

float DifferenceAnalysis::calculatePercentile(const QVector<float>& sortedDistances, float percentile) const {
    if (sortedDistances.isEmpty()) {
        return 0.0f;
    }

    if (percentile <= 0.0f) {
        return sortedDistances.first();
    }

    if (percentile >= 100.0f) {
        return sortedDistances.last();
    }

    float index = (percentile / 100.0f) * (sortedDistances.size() - 1);
    int lowerIndex = static_cast<int>(std::floor(index));
    int upperIndex = static_cast<int>(std::ceil(index));

    if (lowerIndex == upperIndex) {
        return sortedDistances[lowerIndex];
    }

    float weight = index - lowerIndex;
    return sortedDistances[lowerIndex] * (1.0f - weight) + sortedDistances[upperIndex] * weight;
}

QVector<float> DifferenceAnalysis::removeOutliers(const QVector<float>& distances, float threshold) const {
    QVector<float> filtered;
    filtered.reserve(distances.size());

    for (float distance : distances) {
        if (distance <= threshold) {
            filtered.append(distance);
        }
    }

    return filtered;
}

float DifferenceAnalysis::calculateBounds(const std::vector<Point3D>& points) const {
    if (points.empty()) {
        return 1.0f; // Default bounds
    }

    Point3D minBounds = points[0];
    Point3D maxBounds = points[0];

    for (const auto& point : points) {
        minBounds.x = std::min(minBounds.x, point.x);
        minBounds.y = std::min(minBounds.y, point.y);
        minBounds.z = std::min(minBounds.z, point.z);
        maxBounds.x = std::max(maxBounds.x, point.x);
        maxBounds.y = std::max(maxBounds.y, point.y);
        maxBounds.z = std::max(maxBounds.z, point.z);
    }

    float dx = maxBounds.x - minBounds.x;
    float dy = maxBounds.y - minBounds.y;
    float dz = maxBounds.z - minBounds.z;

    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

} // namespace Analysis
