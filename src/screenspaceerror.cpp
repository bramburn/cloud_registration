#include "screenspaceerror.h"
#include "octree.h"
#include <QDebug>
#include <algorithm>
#include <cmath>

float ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
    const AxisAlignedBoundingBox& aabb,
    const QMatrix4x4& mvpMatrix,
    const ViewportInfo& viewport) {
    
    // Get all 8 corners of the AABB
    auto corners = getAABBCorners(aabb);
    
    // Project all corners to screen space
    std::vector<QVector3D> screenCorners;
    screenCorners.reserve(8);
    
    bool allBehindCamera = true;
    for (const auto& corner : corners) {
        QVector4D clipSpace = mvpMatrix * QVector4D(corner, 1.0f);
        
        // Check if point is in front of camera
        if (clipSpace.w() > 0.001f) {
            allBehindCamera = false;
            QVector3D screenPos = projectToScreen(corner, mvpMatrix, viewport);
            screenCorners.push_back(screenPos);
        }
    }
    
    // If all corners are behind camera, return zero error to cull
    if (allBehindCamera || screenCorners.empty()) {
        return 0.0f;
    }
    
    // Find screen-space bounding box
    float minX = screenCorners[0].x();
    float maxX = screenCorners[0].x();
    float minY = screenCorners[0].y();
    float maxY = screenCorners[0].y();
    
    for (const auto& screenPos : screenCorners) {
        minX = std::min(minX, screenPos.x());
        maxX = std::max(maxX, screenPos.x());
        minY = std::min(minY, screenPos.y());
        maxY = std::max(maxY, screenPos.y());
    }
    
    // Calculate screen-space extent (diagonal in pixels)
    float width = maxX - minX;
    float height = maxY - minY;
    return std::sqrt(width * width + height * height);
}

float ScreenSpaceErrorCalculator::calculateNodeScreenSpaceExtent(
    const AxisAlignedBoundingBox& aabb,
    const QMatrix4x4& mvpMatrix,
    const ViewportInfo& viewport) {
    
    return calculateAABBScreenSpaceError(aabb, mvpMatrix, viewport);
}

bool ScreenSpaceErrorCalculator::shouldCullNode(float screenSpaceError, float threshold) {
    return screenSpaceError < threshold;
}

bool ScreenSpaceErrorCalculator::shouldStopRecursion(float screenSpaceError, float primaryThreshold) {
    return screenSpaceError < primaryThreshold;
}

std::array<QVector3D, 8> ScreenSpaceErrorCalculator::getAABBCorners(const AxisAlignedBoundingBox& aabb) {
    return {{
        QVector3D(aabb.min.x(), aabb.min.y(), aabb.min.z()),
        QVector3D(aabb.max.x(), aabb.min.y(), aabb.min.z()),
        QVector3D(aabb.min.x(), aabb.max.y(), aabb.min.z()),
        QVector3D(aabb.max.x(), aabb.max.y(), aabb.min.z()),
        QVector3D(aabb.min.x(), aabb.min.y(), aabb.max.z()),
        QVector3D(aabb.max.x(), aabb.min.y(), aabb.max.z()),
        QVector3D(aabb.min.x(), aabb.max.y(), aabb.max.z()),
        QVector3D(aabb.max.x(), aabb.max.y(), aabb.max.z())
    }};
}

QVector3D ScreenSpaceErrorCalculator::projectToScreen(
    const QVector3D& worldPos, 
    const QMatrix4x4& mvpMatrix, 
    const ViewportInfo& viewport) {
    
    QVector4D clipSpace = mvpMatrix * QVector4D(worldPos, 1.0f);
    
    if (std::abs(clipSpace.w()) < 0.001f) {
        // Avoid division by zero
        return QVector3D(0, 0, 0);
    }
    
    // Perspective divide
    QVector3D ndc = QVector3D(clipSpace.x(), clipSpace.y(), clipSpace.z()) / clipSpace.w();
    
    // Convert to screen coordinates
    float screenX = (ndc.x() + 1.0f) * 0.5f * viewport.width;
    float screenY = (1.0f - ndc.y()) * 0.5f * viewport.height; // Flip Y for screen coordinates
    float screenZ = ndc.z();
    
    return QVector3D(screenX, screenY, screenZ);
}
