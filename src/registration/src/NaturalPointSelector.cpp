#include "registration/NaturalPointSelector.h"

#include <QDebug>

#include <algorithm>
#include <cmath>
#include <limits>

#include "registration/Target.h"
#include "core/profiling_macros.h"  // Sprint 7.3: Performance profiling

NaturalPointSelector::NaturalPointSelector(QObject* parent)
    : TargetDetectionBase(parent),
      m_defaultSelectionRadius(5.0f),
      m_defaultSearchRadius(0.1f),
      m_minConfidenceThreshold(0.3f)
{
}

TargetDetectionBase::DetectionResult NaturalPointSelector::detect(const std::vector<PointFullData>& points,
                                                                  const DetectionParams& params)
{
    DetectionResult result;
    result.success = false;
    result.errorMessage = "Natural point selection requires manual interaction. Use selectPoint() method instead.";
    return result;
}

NaturalPointSelector::SelectionResult NaturalPointSelector::selectPoint(const std::vector<PointFullData>& points,
                                                                        const QMatrix4x4& viewMatrix,
                                                                        const QMatrix4x4& projectionMatrix,
                                                                        const QPoint& screenPos,
                                                                        const QSize& viewportSize,
                                                                        float selectionRadius)
{
    PROFILE_FUNCTION();  // Sprint 7.3: Performance profiling

    SelectionResult result;

    if (points.empty())
    {
        return result;
    }

    // Create ray from screen coordinates
    Ray ray = createRayFromScreen(screenPos, viewportSize, viewMatrix, projectionMatrix);

    // Find closest point to ray
    int closestIndex = findClosestPointToRay(points, ray, selectionRadius / 100.0f);

    if (closestIndex < 0)
    {
        return result;
    }

    // Analyze local features
    QVector3D featureVector = analyzeLocalFeatures(points, closestIndex, m_defaultSearchRadius);

    // Calculate confidence
    float confidence = calculateSelectionConfidence(points, closestIndex, featureVector);

    if (confidence < m_minConfidenceThreshold)
    {
        return result;  // Low confidence selection
    }

    // Fill result
    result.success = true;
    result.selectedPoint = QVector3D(points[closestIndex].x, points[closestIndex].y, points[closestIndex].z);
    result.pointIndex = closestIndex;
    result.confidence = confidence;
    result.featureVector = featureVector;
    result.description = generatePointDescription(points, closestIndex, featureVector);

    qDebug() << "Selected natural point at" << result.selectedPoint << "with confidence" << result.confidence;

    return result;
}

NaturalPointSelector::SelectionResult NaturalPointSelector::selectClosestPoint(const std::vector<PointFullData>& points,
                                                                               const QVector3D& targetPosition,
                                                                               float maxDistance)
{
    SelectionResult result;

    if (points.empty())
    {
        return result;
    }

    int closestIndex = -1;
    float closestDistance = std::numeric_limits<float>::max();

    // Find closest point to target position
    for (size_t i = 0; i < points.size(); ++i)
    {
        QVector3D pointPos(points[i].x, points[i].y, points[i].z);
        float distance = (pointPos - targetPosition).length();

        if (distance < closestDistance && distance <= maxDistance)
        {
            closestDistance = distance;
            closestIndex = static_cast<int>(i);
        }
    }

    if (closestIndex < 0)
    {
        return result;
    }

    // Analyze features and calculate confidence
    QVector3D featureVector = analyzeLocalFeatures(points, closestIndex, m_defaultSearchRadius);
    float confidence = calculateSelectionConfidence(points, closestIndex, featureVector);

    // Fill result
    result.success = true;
    result.selectedPoint = QVector3D(points[closestIndex].x, points[closestIndex].y, points[closestIndex].z);
    result.pointIndex = closestIndex;
    result.confidence = confidence;
    result.featureVector = featureVector;
    result.description = generatePointDescription(points, closestIndex, featureVector);

    return result;
}

std::vector<NaturalPointSelector::SelectionResult>
NaturalPointSelector::suggestCorrespondences(const std::vector<PointFullData>& sourcePoints,
                                             const std::vector<PointFullData>& targetPoints,
                                             const SelectionResult& selectedPoint,
                                             float searchRadius)
{
    std::vector<SelectionResult> suggestions;

    if (!selectedPoint.isValid() || targetPoints.empty())
    {
        return suggestions;
    }

    // Find candidate points in target cloud within search radius
    std::vector<std::pair<int, float>> candidates;

    for (size_t i = 0; i < targetPoints.size(); ++i)
    {
        QVector3D targetPos(targetPoints[i].x, targetPoints[i].y, targetPoints[i].z);
        float distance = (targetPos - selectedPoint.selectedPoint).length();

        if (distance <= searchRadius)
        {
            // Analyze features for this candidate
            QVector3D candidateFeatures =
                analyzeLocalFeatures(targetPoints, static_cast<int>(i), m_defaultSearchRadius);

            // Calculate feature similarity
            float similarity = calculateFeatureSimilarity(selectedPoint.featureVector, candidateFeatures);

            if (similarity > 0.5f)
            {  // Minimum similarity threshold
                candidates.emplace_back(static_cast<int>(i), similarity);
            }
        }
    }

    // Sort candidates by similarity (best first)
    std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) { return a.second > b.second; });

    // Create selection results for top candidates (max 5)
    int maxSuggestions = std::min(5, static_cast<int>(candidates.size()));
    suggestions.reserve(maxSuggestions);

    for (int i = 0; i < maxSuggestions; ++i)
    {
        int candidateIndex = candidates[i].first;
        float similarity = candidates[i].second;

        SelectionResult suggestion;
        suggestion.success = true;
        suggestion.selectedPoint =
            QVector3D(targetPoints[candidateIndex].x, targetPoints[candidateIndex].y, targetPoints[candidateIndex].z);
        suggestion.pointIndex = candidateIndex;
        suggestion.confidence = similarity;
        suggestion.featureVector = analyzeLocalFeatures(targetPoints, candidateIndex, m_defaultSearchRadius);
        suggestion.description = generatePointDescription(targetPoints, candidateIndex, suggestion.featureVector);

        suggestions.push_back(suggestion);
    }

    return suggestions;
}

void NaturalPointSelector::onMouseClick(const std::vector<PointFullData>& points,
                                        const QMatrix4x4& viewMatrix,
                                        const QMatrix4x4& projectionMatrix,
                                        const QPoint& screenPos,
                                        const QSize& viewportSize)
{
    SelectionResult result =
        selectPoint(points, viewMatrix, projectionMatrix, screenPos, viewportSize, m_defaultSelectionRadius);

    if (result.isValid())
    {
        emit pointSelected(result);
    }
    else
    {
        emit selectionFailed("No suitable point found at clicked location");
    }
}

NaturalPointSelector::Ray NaturalPointSelector::createRayFromScreen(const QPoint& screenPos,
                                                                    const QSize& viewportSize,
                                                                    const QMatrix4x4& viewMatrix,
                                                                    const QMatrix4x4& projectionMatrix) const
{
    // Convert screen coordinates to normalized device coordinates
    float x = (2.0f * screenPos.x()) / viewportSize.width() - 1.0f;
    float y = 1.0f - (2.0f * screenPos.y()) / viewportSize.height();

    // Create points in clip space
    QVector4D nearPoint(x, y, -1.0f, 1.0f);
    QVector4D farPoint(x, y, 1.0f, 1.0f);

    // Transform to world space
    QMatrix4x4 invViewProj = (projectionMatrix * viewMatrix).inverted();

    QVector4D nearWorld = invViewProj * nearPoint;
    QVector4D farWorld = invViewProj * farPoint;

    // Perspective divide
    if (nearWorld.w() != 0.0f)
    {
        nearWorld /= nearWorld.w();
    }
    if (farWorld.w() != 0.0f)
    {
        farWorld /= farWorld.w();
    }

    // Create ray
    QVector3D origin = nearWorld.toVector3D();
    QVector3D direction = (farWorld.toVector3D() - origin).normalized();

    return Ray(origin, direction);
}

int NaturalPointSelector::findClosestPointToRay(const std::vector<PointFullData>& points,
                                                const Ray& ray,
                                                float maxDistance) const
{
    int closestIndex = -1;
    float closestDistance = std::numeric_limits<float>::max();

    for (size_t i = 0; i < points.size(); ++i)
    {
        QVector3D point(points[i].x, points[i].y, points[i].z);
        float distance = distancePointToRay(point, ray);

        if (distance < closestDistance && distance <= maxDistance)
        {
            closestDistance = distance;
            closestIndex = static_cast<int>(i);
        }
    }

    return closestIndex;
}

float NaturalPointSelector::distancePointToRay(const QVector3D& point, const Ray& ray) const
{
    QVector3D pointToOrigin = point - ray.origin;
    QVector3D projection = QVector3D::dotProduct(pointToOrigin, ray.direction) * ray.direction;
    QVector3D rejection = pointToOrigin - projection;

    return rejection.length();
}

QVector3D
NaturalPointSelector::analyzeLocalFeatures(const std::vector<PointFullData>& points, int pointIndex, float radius) const
{
    if (pointIndex < 0 || pointIndex >= static_cast<int>(points.size()))
    {
        return QVector3D(0, 0, 0);
    }

    // Find neighbors
    std::vector<int> neighbors = findNeighbors(points, pointIndex, radius);

    if (neighbors.size() < 3)
    {
        return QVector3D(0, 0, 0);  // Not enough neighbors for feature analysis
    }

    const auto& centerPoint = points[pointIndex];
    QVector3D center(centerPoint.x, centerPoint.y, centerPoint.z);

    // Calculate local geometric features
    QVector3D meanPos(0, 0, 0);
    for (int neighborIdx : neighbors)
    {
        const auto& neighbor = points[neighborIdx];
        meanPos += QVector3D(neighbor.x, neighbor.y, neighbor.z);
    }
    meanPos /= static_cast<float>(neighbors.size());

    // Calculate covariance matrix for principal component analysis
    float cxx = 0, cyy = 0, czz = 0, cxy = 0, cxz = 0, cyz = 0;

    for (int neighborIdx : neighbors)
    {
        const auto& neighbor = points[neighborIdx];
        QVector3D pos(neighbor.x, neighbor.y, neighbor.z);
        QVector3D diff = pos - meanPos;

        cxx += diff.x() * diff.x();
        cyy += diff.y() * diff.y();
        czz += diff.z() * diff.z();
        cxy += diff.x() * diff.y();
        cxz += diff.x() * diff.z();
        cyz += diff.y() * diff.z();
    }

    float count = static_cast<float>(neighbors.size());
    cxx /= count;
    cyy /= count;
    czz /= count;
    cxy /= count;
    cxz /= count;
    cyz /= count;

    // Calculate eigenvalues (simplified approach)
    float trace = cxx + cyy + czz;
    float det = cxx * cyy * czz + 2 * cxy * cxz * cyz - cxx * cyz * cyz - cyy * cxz * cxz - czz * cxy * cxy;

    // Feature vector: [planarity, linearity, sphericity]
    float planarity = std::abs(trace) > 1e-6f ? det / (trace * trace * trace) : 0.0f;
    float linearity = trace > 1e-6f ? std::sqrt(cxx + cyy + czz) / trace : 0.0f;
    float sphericity = 1.0f - planarity - linearity;

    return QVector3D(planarity, linearity, sphericity);
}

float NaturalPointSelector::calculateSelectionConfidence(const std::vector<PointFullData>& points,
                                                         int pointIndex,
                                                         const QVector3D& featureVector) const
{
    if (pointIndex < 0 || pointIndex >= static_cast<int>(points.size()))
    {
        return 0.0f;
    }

    // Confidence based on feature distinctiveness
    float planarity = featureVector.x();
    float linearity = featureVector.y();
    float sphericity = featureVector.z();

    // Higher confidence for more distinctive features (corners, edges)
    float distinctiveness = std::max({planarity, linearity}) * 2.0f + sphericity * 0.5f;

    // Normalize to 0-1 range
    float confidence = std::min(1.0f, distinctiveness);

    // Boost confidence if point has normal information
    if (points[pointIndex].hasNormal())
    {
        confidence *= 1.2f;
    }

    // Boost confidence if point has intensity information
    if (points[pointIndex].hasIntensity())
    {
        confidence *= 1.1f;
    }

    return std::min(1.0f, confidence);
}

QString NaturalPointSelector::generatePointDescription(const std::vector<PointFullData>& points,
                                                       int pointIndex,
                                                       const QVector3D& featureVector) const
{
    if (pointIndex < 0 || pointIndex >= static_cast<int>(points.size()))
    {
        return "Invalid point";
    }

    float planarity = featureVector.x();
    float linearity = featureVector.y();
    float sphericity = featureVector.z();

    QString description;

    if (planarity > 0.7f)
    {
        description = "Planar surface point";
    }
    else if (linearity > 0.7f)
    {
        description = "Edge/ridge point";
    }
    else if (sphericity > 0.7f)
    {
        description = "Corner/isolated point";
    }
    else if (planarity > linearity && planarity > sphericity)
    {
        description = "Surface feature";
    }
    else if (linearity > sphericity)
    {
        description = "Linear feature";
    }
    else
    {
        description = "Point feature";
    }

    // Add position information
    const auto& point = points[pointIndex];
    description += QString(" at (%.2f, %.2f, %.2f)").arg(point.x).arg(point.y).arg(point.z);

    return description;
}

float NaturalPointSelector::calculateFeatureSimilarity(const QVector3D& feature1, const QVector3D& feature2) const
{
    // Calculate cosine similarity between feature vectors
    float dot = QVector3D::dotProduct(feature1, feature2);
    float mag1 = feature1.length();
    float mag2 = feature2.length();

    if (mag1 < 1e-6f || mag2 < 1e-6f)
    {
        return 0.0f;
    }

    float similarity = dot / (mag1 * mag2);

    // Convert from [-1, 1] to [0, 1] range
    return (similarity + 1.0f) * 0.5f;
}

std::vector<int>
NaturalPointSelector::findNeighbors(const std::vector<PointFullData>& points, int centerIndex, float radius) const
{
    std::vector<int> neighbors;

    if (centerIndex < 0 || centerIndex >= static_cast<int>(points.size()))
    {
        return neighbors;
    }

    const auto& centerPoint = points[centerIndex];
    QVector3D center(centerPoint.x, centerPoint.y, centerPoint.z);

    neighbors.reserve(100);  // Reserve some space

    for (size_t i = 0; i < points.size(); ++i)
    {
        if (static_cast<int>(i) == centerIndex)
        {
            continue;  // Skip center point itself
        }

        const auto& point = points[i];
        QVector3D pos(point.x, point.y, point.z);

        if ((pos - center).length() <= radius)
        {
            neighbors.push_back(static_cast<int>(i));
        }
    }

    return neighbors;
}
