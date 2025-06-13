#ifndef SCREENSPACEERROR_H
#define SCREENSPACEERROR_H

#include <QMatrix4x4>
#include "core/pointdata.h"

// Forward declarations
struct AxisAlignedBoundingBox;

/**
 * @brief ScreenSpaceErrorCalculator - Utility class for screen space error calculations
 *
 * Provides methods for calculating screen space error for octree nodes and determining
 * when to cull or stop recursion based on error thresholds.
 */
class ScreenSpaceErrorCalculator
{
public:
    /**
     * @brief Calculate screen space error for an AABB
     * @param bounds The axis-aligned bounding box
     * @param mvpMatrix Model-view-projection matrix
     * @param viewport Viewport information
     * @return Screen space error value
     */
    static float calculateAABBScreenSpaceError(const AxisAlignedBoundingBox& bounds,
                                               const QMatrix4x4& mvpMatrix,
                                               const ViewportInfo& viewport);

    /**
     * @brief Determine if a node should be culled based on error threshold
     * @param error Screen space error value
     * @param threshold Culling threshold
     * @return True if node should be culled
     */
    static bool shouldCullNode(float error, float threshold);

    /**
     * @brief Determine if recursion should stop based on error threshold
     * @param error Screen space error value
     * @param threshold Primary threshold for stopping recursion
     * @return True if recursion should stop
     */
    static bool shouldStopRecursion(float error, float threshold);

private:
    ScreenSpaceErrorCalculator() = default;
};

// Legacy compatibility
using ScreenSpaceError = ScreenSpaceErrorCalculator;

#endif  // SCREENSPACEERROR_H
