#include "core/screenspaceerror.h"
#include "core/octree.h"
#include <QVector3D>
#include <cmath>

float ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
    const AxisAlignedBoundingBox& bounds,
    const QMatrix4x4& mvpMatrix,
    const ViewportInfo& viewport)
{
    // Calculate the center and size of the bounding box
    QVector3D center = bounds.center();
    QVector3D size = bounds.size();
    float boundingRadius = size.length() * 0.5f;

    // Transform center to clip space
    QVector4D centerClip = mvpMatrix * QVector4D(center, 1.0f);

    // Avoid division by zero
    if (std::abs(centerClip.w()) < 1e-6f) {
        return 1000.0f; // Very high error for degenerate cases
    }

    // Convert to normalized device coordinates
    QVector3D centerNDC = centerClip.toVector3D() / centerClip.w();

    // Calculate distance from camera (using Z component)
    float distance = std::abs(centerNDC.z());

    // Avoid division by zero
    if (distance < 1e-6f) {
        return 1000.0f; // Very high error for very close objects
    }

    // Calculate projected size in screen space
    float projectedSize = (boundingRadius / distance) * viewport.height * 0.5f;

    // Screen space error is the projected size in pixels
    return projectedSize;
}

bool ScreenSpaceErrorCalculator::shouldCullNode(float error, float threshold)
{
    // Cull if the screen space error is below the threshold
    return error < threshold;
}

bool ScreenSpaceErrorCalculator::shouldStopRecursion(float error, float threshold)
{
    // Stop recursion if the screen space error is below the threshold
    return error < threshold;
}
