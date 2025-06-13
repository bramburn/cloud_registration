#include "features/FeatureExtractor.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QSet>

#include <algorithm>
#include <cmath>
#include <random>

namespace Features
{
bool Plane::isSimilarTo(const Plane& other, float angleThreshold, float distanceThreshold) const
{
    // Check normal similarity
    float dotProduct = QVector3D::dotProduct(normal, other.normal);
    float angle = std::acos(std::abs(dotProduct));  // Use abs to handle flipped normals

    if (angle > angleThreshold)
    {
        return false;
    }

    // Check distance similarity
    float distanceDiff = std::abs(distance - other.distance);
    return distanceDiff <= distanceThreshold;
}

FeatureExtractor::FeatureExtractor(QObject* parent) : QObject(parent) {}

QList<Plane> FeatureExtractor::extractPlanes(const std::vector<Point3D>& points, const PlaneExtractionParams& params)
{
    QElapsedTimer timer;
    timer.start();

    QList<Plane> extractedPlanes;
    QSet<int> usedIndices;

    if (points.size() < 3)
    {
        qWarning() << "Insufficient points for plane extraction:" << points.size();
        emit extractionCompleted(0);
        return extractedPlanes;
    }

    qDebug() << "Starting plane extraction from" << points.size() << "points";
    emit extractionProgress(0);

    // Extract planes iteratively
    for (int planeIdx = 0; planeIdx < params.maxPlanes; ++planeIdx)
    {
        // Check if enough points remain
        int remainingPoints = points.size() - usedIndices.size();
        if (remainingPoints < params.minInliers)
        {
            qDebug() << "Insufficient remaining points:" << remainingPoints;
            break;
        }

        // Extract single plane
        Plane plane = ransacPlaneFitting(points, params, usedIndices);

        // Validate plane
        if (plane.inlierIndices.size() < params.minInliers)
        {
            qDebug() << "Plane" << planeIdx << "has insufficient inliers:" << plane.inlierIndices.size();
            break;
        }

        if (plane.confidence < params.minConfidence)
        {
            qDebug() << "Plane" << planeIdx << "has low confidence:" << plane.confidence;
            break;
        }

        // Check normal filter
        if (params.filterByNormal && !passesNormalFilter(plane, params))
        {
            qDebug() << "Plane" << planeIdx << "failed normal filter";
            // Don't break, try to find more planes
            if (params.removeInliers)
            {
                for (int idx : plane.inlierIndices)
                {
                    usedIndices.insert(idx);
                }
            }
            continue;
        }

        // Calculate plane properties
        auto [centroid, area] = calculatePlaneProperties(points, plane.inlierIndices);
        plane.centroid = centroid;

        if (area < params.minPlaneArea)
        {
            qDebug() << "Plane" << planeIdx << "has insufficient area:" << area;
            if (params.removeInliers)
            {
                for (int idx : plane.inlierIndices)
                {
                    usedIndices.insert(idx);
                }
            }
            continue;
        }

        // Add valid plane
        extractedPlanes.append(plane);
        emit planeExtracted(plane);

        qDebug() << "Extracted plane" << planeIdx << "with" << plane.inlierIndices.size()
                 << "inliers, confidence:" << plane.confidence << "area:" << area;

        // Remove inliers for next iteration
        if (params.removeInliers)
        {
            for (int idx : plane.inlierIndices)
            {
                usedIndices.insert(idx);
            }
        }

        // Update progress
        int progress = ((planeIdx + 1) * 100) / params.maxPlanes;
        emit extractionProgress(progress);
    }

    qDebug() << "Plane extraction completed:" << extractedPlanes.size() << "planes found in" << timer.elapsed() << "ms";

    emit extractionProgress(100);
    emit extractionCompleted(extractedPlanes.size());

    return extractedPlanes;
}

Plane FeatureExtractor::extractSinglePlane(const std::vector<Point3D>& points, const PlaneExtractionParams& params)
{
    return ransacPlaneFitting(points, params);
}

FeatureExtractor::PlaneExtractionParams
FeatureExtractor::getRecommendedParameters(const std::vector<Point3D>& points) const
{
    PlaneExtractionParams params;

    // Adjust parameters based on point cloud size
    size_t pointCount = points.size();

    if (pointCount > 1000000)
    {
        // Large point cloud
        params.maxIterations = 2000;
        params.distanceThreshold = 0.05f;  // 5cm
        params.minInliers = 500;
    }
    else if (pointCount > 100000)
    {
        // Medium point cloud
        params.maxIterations = 1500;
        params.distanceThreshold = 0.03f;  // 3cm
        params.minInliers = 200;
    }
    else
    {
        // Small point cloud
        params.maxIterations = 1000;
        params.distanceThreshold = 0.02f;  // 2cm
        params.minInliers = std::max(50, static_cast<int>(pointCount / 100));
    }

    return params;
}

float FeatureExtractor::validatePlaneQuality(const Plane& plane, const std::vector<Point3D>& points) const
{
    if (plane.inlierIndices.isEmpty() || points.empty())
    {
        return 0.0f;
    }

    // Calculate RMS error
    double sumSquaredErrors = 0.0;
    for (int idx : plane.inlierIndices)
    {
        if (idx >= 0 && idx < static_cast<int>(points.size()))
        {
            QVector3D point = toQVector3D(points[idx]);
            float error = plane.distanceToPoint(point);
            sumSquaredErrors += error * error;
        }
    }

    double rmsError = std::sqrt(sumSquaredErrors / plane.inlierIndices.size());

    // Quality score based on RMS error (lower error = higher quality)
    float quality = std::exp(-rmsError * 50.0f);  // Exponential decay

    return std::max(0.0f, std::min(1.0f, quality));
}

Plane FeatureExtractor::ransacPlaneFitting(const std::vector<Point3D>& points,
                                           const PlaneExtractionParams& params,
                                           const QSet<int>& usedIndices) const
{
    Plane bestPlane;
    int bestInlierCount = 0;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, points.size() - 1);

    // RANSAC iterations
    for (int iter = 0; iter < params.maxIterations; ++iter)
    {
        // Select three random points
        QVector3D p1, p2, p3;
        int attempts = 0;
        bool validTriple = false;

        while (attempts < 100 && !validTriple)
        {
            int idx1 = dis(gen);
            int idx2 = dis(gen);
            int idx3 = dis(gen);

            // Skip used indices
            if (usedIndices.contains(idx1) || usedIndices.contains(idx2) || usedIndices.contains(idx3))
            {
                attempts++;
                continue;
            }

            // Ensure indices are different
            if (idx1 == idx2 || idx2 == idx3 || idx1 == idx3)
            {
                attempts++;
                continue;
            }

            p1 = toQVector3D(points[idx1]);
            p2 = toQVector3D(points[idx2]);
            p3 = toQVector3D(points[idx3]);

            // Check if points are collinear
            QVector3D v1 = p2 - p1;
            QVector3D v2 = p3 - p1;
            QVector3D cross = QVector3D::crossProduct(v1, v2);

            if (cross.length() > 1e-6f)
            {
                validTriple = true;
            }
            attempts++;
        }

        if (!validTriple)
        {
            continue;
        }

        // Fit plane to three points
        auto [normal, distance] = fitPlaneToThreePoints(p1, p2, p3);

        if (normal.length() < 1e-6f)
        {
            continue;  // Invalid plane
        }

        // Count inliers
        QList<int> inliers = countInliers(points, normal, distance, params.distanceThreshold, usedIndices);

        if (inliers.size() > bestInlierCount)
        {
            bestInlierCount = inliers.size();
            bestPlane.normal = normal;
            bestPlane.distance = distance;
            bestPlane.inlierIndices = inliers;
        }
    }

    // Refine best plane if found
    if (bestInlierCount >= params.minInliers)
    {
        bestPlane = refinePlane(points, bestPlane.inlierIndices);
        bestPlane.confidence = validatePlaneQuality(bestPlane, points);
    }

    return bestPlane;
}

std::pair<QVector3D, float>
FeatureExtractor::fitPlaneToThreePoints(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3) const
{
    // Calculate normal vector
    QVector3D v1 = p2 - p1;
    QVector3D v2 = p3 - p1;
    QVector3D normal = QVector3D::crossProduct(v1, v2);

    if (normal.length() < 1e-6f)
    {
        return {QVector3D(), 0.0f};  // Degenerate case
    }

    normal.normalize();

    // Calculate distance from origin
    float distance = QVector3D::dotProduct(normal, p1);

    return {normal, distance};
}

QList<int> FeatureExtractor::countInliers(const std::vector<Point3D>& points,
                                          const QVector3D& normal,
                                          float distance,
                                          float threshold,
                                          const QSet<int>& usedIndices) const
{
    QList<int> inliers;

    for (int i = 0; i < static_cast<int>(points.size()); ++i)
    {
        if (usedIndices.contains(i))
        {
            continue;
        }

        QVector3D point = toQVector3D(points[i]);
        float pointDistance = std::abs(QVector3D::dotProduct(normal, point) - distance);

        if (pointDistance <= threshold)
        {
            inliers.append(i);
        }
    }

    return inliers;
}

Plane FeatureExtractor::refinePlane(const std::vector<Point3D>& points, const QList<int>& inlierIndices) const
{
    Plane refinedPlane;

    if (inlierIndices.size() < 3)
    {
        return refinedPlane;
    }

    // Calculate centroid
    QVector3D centroid(0, 0, 0);
    for (int idx : inlierIndices)
    {
        centroid += toQVector3D(points[idx]);
    }
    centroid /= inlierIndices.size();

    // Calculate covariance matrix
    float cxx = 0, cxy = 0, cxz = 0, cyy = 0, cyz = 0, czz = 0;

    for (int idx : inlierIndices)
    {
        QVector3D p = toQVector3D(points[idx]) - centroid;
        cxx += p.x() * p.x();
        cxy += p.x() * p.y();
        cxz += p.x() * p.z();
        cyy += p.y() * p.y();
        cyz += p.y() * p.z();
        czz += p.z() * p.z();
    }

    // Find normal as eigenvector of smallest eigenvalue (simplified approach)
    // This is a basic implementation - could be improved with proper SVD
    QVector3D normal;

    // Try different candidate normals and pick the one with smallest variance
    QVector3D candidates[] = {QVector3D(1, 0, 0),
                              QVector3D(0, 1, 0),
                              QVector3D(0, 0, 1),
                              QVector3D(1, 1, 0).normalized(),
                              QVector3D(1, 0, 1).normalized(),
                              QVector3D(0, 1, 1).normalized()};

    float minVariance = std::numeric_limits<float>::max();
    for (const QVector3D& candidate : candidates)
    {
        float variance = 0;
        for (int idx : inlierIndices)
        {
            QVector3D p = toQVector3D(points[idx]);
            float dist = QVector3D::dotProduct(candidate, p - centroid);
            variance += dist * dist;
        }

        if (variance < minVariance)
        {
            minVariance = variance;
            normal = candidate;
        }
    }

    refinedPlane.normal = normal;
    refinedPlane.distance = QVector3D::dotProduct(normal, centroid);
    refinedPlane.centroid = centroid;
    refinedPlane.inlierIndices = inlierIndices;

    return refinedPlane;
}

std::pair<QVector3D, float> FeatureExtractor::calculatePlaneProperties(const std::vector<Point3D>& points,
                                                                       const QList<int>& inlierIndices) const
{
    if (inlierIndices.isEmpty())
    {
        return {QVector3D(), 0.0f};
    }

    // Calculate centroid
    QVector3D centroid(0, 0, 0);
    for (int idx : inlierIndices)
    {
        centroid += toQVector3D(points[idx]);
    }
    centroid /= inlierIndices.size();

    // Estimate area (simplified as bounding box area)
    QVector3D minBounds = centroid;
    QVector3D maxBounds = centroid;

    for (int idx : inlierIndices)
    {
        QVector3D p = toQVector3D(points[idx]);
        minBounds.setX(std::min(minBounds.x(), p.x()));
        minBounds.setY(std::min(minBounds.y(), p.y()));
        minBounds.setZ(std::min(minBounds.z(), p.z()));
        maxBounds.setX(std::max(maxBounds.x(), p.x()));
        maxBounds.setY(std::max(maxBounds.y(), p.y()));
        maxBounds.setZ(std::max(maxBounds.z(), p.z()));
    }

    QVector3D size = maxBounds - minBounds;
    float area = std::max({size.x() * size.y(), size.y() * size.z(), size.x() * size.z()});

    return {centroid, area};
}

bool FeatureExtractor::passesNormalFilter(const Plane& plane, const PlaneExtractionParams& params) const
{
    if (!params.filterByNormal)
    {
        return true;
    }

    float dotProduct = QVector3D::dotProduct(plane.normal, params.preferredNormal);
    float angle = std::acos(std::abs(dotProduct));  // Use abs to handle flipped normals

    return angle <= params.normalTolerance;
}

}  // namespace Features
