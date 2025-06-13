#include "rendering/LODManager.h"

#include <QDebug>

#include <algorithm>
#include <cmath>

#include "core/octree.h"
#include "rendering/CameraController.h"

LODManager::LODManager(QObject* parent)
    : QObject(parent),
      m_nearDistance(10.0f),
      m_farDistance(100.0f),
      m_maxPointsPerFrame(1000000),
      m_qualityLevel(1.0f),
      m_frustumCullingEnabled(true),
      m_distanceLODEnabled(true),
      m_adaptiveQualityEnabled(false),
      m_targetFPS(30.0f),
      m_currentFPS(30.0f),
      m_fpsHistoryIndex(0),
      m_octree(nullptr),
      m_camera(nullptr),
      m_lastTotalPoints(0),
      m_lastVisiblePoints(0),
      m_lastCulledNodes(0)
{
    // Initialize FPS history
    for (int i = 0; i < 10; ++i)
    {
        m_fpsHistory[i] = 30.0f;
    }
}

LODManager::~LODManager()
{
    // Note: We don't own the octree or camera, so we don't delete them
}

void LODManager::setOctree(Octree* octree)
{
    m_octree = octree;
    qDebug() << "LODManager: Octree set";
}

void LODManager::setCameraController(CameraController* camera)
{
    m_camera = camera;
    qDebug() << "LODManager: Camera controller set";
}

void LODManager::configureLOD(float nearDistance, float farDistance, int maxPointsPerFrame)
{
    m_nearDistance = qMax(0.1f, nearDistance);
    m_farDistance = qMax(m_nearDistance + 1.0f, farDistance);
    m_maxPointsPerFrame = qMax(1000, maxPointsPerFrame);

    qDebug() << "LODManager configured - Near:" << m_nearDistance << "Far:" << m_farDistance
             << "Max points:" << m_maxPointsPerFrame;

    emit lodParametersChanged();
}

std::vector<int> LODManager::getVisiblePoints(float aspectRatio)
{
    if (!m_octree || !m_camera)
    {
        qWarning() << "LODManager: Octree or camera not set";
        return std::vector<int>();
    }

    return traverseOctreeForLOD(aspectRatio);
}

std::vector<float> LODManager::getVisiblePointData(float aspectRatio, const std::vector<float>& pointData)
{
    std::vector<int> visibleIndices = getVisiblePoints(aspectRatio);
    std::vector<float> visibleData;

    if (pointData.size() % 3 != 0)
    {
        qWarning() << "LODManager: Invalid point data size";
        return visibleData;
    }

    visibleData.reserve(visibleIndices.size() * 3);

    for (int index : visibleIndices)
    {
        if (index * 3 + 2 < static_cast<int>(pointData.size()))
        {
            visibleData.push_back(pointData[index * 3]);
            visibleData.push_back(pointData[index * 3 + 1]);
            visibleData.push_back(pointData[index * 3 + 2]);
        }
    }

    m_lastVisiblePoints = static_cast<int>(visibleIndices.size());
    m_lastTotalPoints = static_cast<int>(pointData.size() / 3);

    float reductionRatio = m_lastTotalPoints > 0 ? static_cast<float>(m_lastVisiblePoints) / m_lastTotalPoints : 0.0f;

    emit lodStatsUpdated(m_lastVisiblePoints, m_lastTotalPoints, reductionRatio);

    return visibleData;
}

void LODManager::getLastLODStats(int& totalPoints, int& visiblePoints, int& culledNodes) const
{
    totalPoints = m_lastTotalPoints;
    visiblePoints = m_lastVisiblePoints;
    culledNodes = m_lastCulledNodes;
}

void LODManager::setQualityLevel(float quality)
{
    m_qualityLevel = qBound(0.0f, quality, 1.0f);
    qDebug() << "LODManager: Quality level set to" << m_qualityLevel;
    emit lodParametersChanged();
}

void LODManager::setAdaptiveQuality(bool enabled, float targetFPS)
{
    m_adaptiveQualityEnabled = enabled;
    m_targetFPS = qMax(10.0f, targetFPS);
    qDebug() << "LODManager: Adaptive quality" << (enabled ? "enabled" : "disabled")
             << "with target FPS:" << m_targetFPS;
}

void LODManager::updatePerformanceMetrics(float currentFPS)
{
    m_currentFPS = currentFPS;
    m_fpsHistory[m_fpsHistoryIndex] = currentFPS;
    m_fpsHistoryIndex = (m_fpsHistoryIndex + 1) % 10;

    if (m_adaptiveQualityEnabled)
    {
        updateAdaptiveQuality();
    }
}

bool LODManager::isNodeInFrustum(const QVector3D& nodeCenter,
                                 float nodeSize,
                                 const QMatrix4x4& viewProjectionMatrix) const
{
    if (!m_frustumCullingEnabled)
    {
        return true;
    }

    // Extract frustum planes
    std::vector<QVector4D> planes;
    extractFrustumPlanes(viewProjectionMatrix, planes);

    // Check if node bounding box intersects with frustum
    float halfSize = nodeSize * 0.5f;
    QVector3D corners[8] = {nodeCenter + QVector3D(-halfSize, -halfSize, -halfSize),
                            nodeCenter + QVector3D(halfSize, -halfSize, -halfSize),
                            nodeCenter + QVector3D(-halfSize, halfSize, -halfSize),
                            nodeCenter + QVector3D(halfSize, halfSize, -halfSize),
                            nodeCenter + QVector3D(-halfSize, -halfSize, halfSize),
                            nodeCenter + QVector3D(halfSize, -halfSize, halfSize),
                            nodeCenter + QVector3D(-halfSize, halfSize, halfSize),
                            nodeCenter + QVector3D(halfSize, halfSize, halfSize)};

    // Check if all corners are outside any plane
    for (const QVector4D& plane : planes)
    {
        bool allOutside = true;
        for (int i = 0; i < 8; ++i)
        {
            float distance =
                plane.x() * corners[i].x() + plane.y() * corners[i].y() + plane.z() * corners[i].z() + plane.w();
            if (distance >= 0)
            {
                allOutside = false;
                break;
            }
        }
        if (allOutside)
        {
            return false;
        }
    }

    return true;
}

float LODManager::calculateNodeDistance(const QVector3D& nodeCenter) const
{
    if (!m_camera)
    {
        return 0.0f;
    }

    QVector3D cameraPos = m_camera->getCameraPosition();
    return (nodeCenter - cameraPos).length();
}

int LODManager::calculateLODLevel(float distance) const
{
    if (!m_distanceLODEnabled)
    {
        return 0;  // Highest detail
    }

    if (distance <= m_nearDistance)
    {
        return 0;  // High detail
    }
    else if (distance <= m_farDistance)
    {
        return 1;  // Medium detail
    }
    else
    {
        return 2;  // Low detail
    }
}

void LODManager::updateAdaptiveQuality()
{
    // Calculate average FPS over history
    float avgFPS = 0.0f;
    for (int i = 0; i < 10; ++i)
    {
        avgFPS += m_fpsHistory[i];
    }
    avgFPS /= 10.0f;

    // Adjust quality based on performance
    if (avgFPS < m_targetFPS * 0.8f)
    {
        // Performance is poor, reduce quality
        m_qualityLevel = qMax(0.1f, m_qualityLevel - 0.05f);
    }
    else if (avgFPS > m_targetFPS * 1.2f)
    {
        // Performance is good, increase quality
        m_qualityLevel = qMin(1.0f, m_qualityLevel + 0.02f);
    }
}

std::vector<int> LODManager::traverseOctreeForLOD(float aspectRatio)
{
    std::vector<int> visiblePoints;

    if (!m_octree || !m_octree->root)
    {
        return visiblePoints;
    }

    // Get view-projection matrix for frustum culling
    QMatrix4x4 viewMatrix = m_camera->getViewMatrix();
    QMatrix4x4 projMatrix = m_camera->getProjectionMatrix(aspectRatio);
    QMatrix4x4 viewProjMatrix = projMatrix * viewMatrix;

    // Simple implementation: traverse octree and collect visible points
    // This is a placeholder - in a real implementation, you would traverse
    // the octree structure and apply LOD logic

    m_lastCulledNodes = 0;

    // For now, return a simplified result based on quality level
    int maxPoints = static_cast<int>(m_maxPointsPerFrame * m_qualityLevel);
    visiblePoints.reserve(maxPoints);

    // This would be replaced with actual octree traversal
    for (int i = 0; i < maxPoints && i < 100000; ++i)
    {
        visiblePoints.push_back(i);
    }

    return visiblePoints;
}

void LODManager::extractFrustumPlanes(const QMatrix4x4& viewProjectionMatrix, std::vector<QVector4D>& planes) const
{
    planes.clear();
    planes.reserve(6);

    const float* m = viewProjectionMatrix.constData();

    // Left plane
    planes.emplace_back(m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12]);
    // Right plane
    planes.emplace_back(m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12]);
    // Bottom plane
    planes.emplace_back(m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13]);
    // Top plane
    planes.emplace_back(m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13]);
    // Near plane
    planes.emplace_back(m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14]);
    // Far plane
    planes.emplace_back(m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14]);

    // Normalize planes
    for (QVector4D& plane : planes)
    {
        float length = std::sqrt(plane.x() * plane.x() + plane.y() * plane.y() + plane.z() * plane.z());
        if (length > 0.0f)
        {
            plane /= length;
        }
    }
}

bool LODManager::isPointInFrustum(const QVector3D& point, const std::vector<QVector4D>& planes) const
{
    for (const QVector4D& plane : planes)
    {
        float distance = plane.x() * point.x() + plane.y() * point.y() + plane.z() * point.z() + plane.w();
        if (distance < 0)
        {
            return false;
        }
    }
    return true;
}
