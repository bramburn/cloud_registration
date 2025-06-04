#ifndef SCREENSPACEERROR_H
#define SCREENSPACEERROR_H

#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <array>
#include <vector>

// Forward declaration
struct AxisAlignedBoundingBox;

struct ViewportInfo {
    int width;
    int height;
    float nearPlane;
    float farPlane;
};

/**
 * @brief Screen-space error calculator for LOD selection
 * 
 * This class provides static methods to calculate screen-space error metrics
 * for octree nodes, enabling more accurate LOD selection based on visual impact
 * rather than just distance.
 */
class ScreenSpaceErrorCalculator {
public:
    /**
     * @brief Calculate the screen-space error for an AABB
     * @param aabb The axis-aligned bounding box to project
     * @param mvpMatrix The model-view-projection matrix
     * @param viewport The viewport information
     * @return Screen-space error in pixels (diagonal extent)
     */
    static float calculateAABBScreenSpaceError(
        const AxisAlignedBoundingBox& aabb,
        const QMatrix4x4& mvpMatrix,
        const ViewportInfo& viewport
    );
    
    /**
     * @brief Calculate the screen-space extent of a node
     * @param aabb The axis-aligned bounding box
     * @param mvpMatrix The model-view-projection matrix
     * @param viewport The viewport information
     * @return Screen-space extent in pixels
     */
    static float calculateNodeScreenSpaceExtent(
        const AxisAlignedBoundingBox& aabb,
        const QMatrix4x4& mvpMatrix,
        const ViewportInfo& viewport
    );
    
    /**
     * @brief Determine if a node should be culled based on screen-space error
     * @param screenSpaceError The calculated screen-space error
     * @param threshold The culling threshold in pixels
     * @return True if the node should be culled
     */
    static bool shouldCullNode(float screenSpaceError, float threshold);
    
    /**
     * @brief Determine if recursion should stop for a node
     * @param screenSpaceError The calculated screen-space error
     * @param primaryThreshold The primary threshold for stopping recursion
     * @return True if recursion should stop
     */
    static bool shouldStopRecursion(float screenSpaceError, float primaryThreshold);

private:
    /**
     * @brief Get all 8 corners of an AABB
     * @param aabb The axis-aligned bounding box
     * @return Array of 8 corner points
     */
    static std::array<QVector3D, 8> getAABBCorners(const AxisAlignedBoundingBox& aabb);
    
    /**
     * @brief Project a world position to screen coordinates
     * @param worldPos The world position
     * @param mvpMatrix The model-view-projection matrix
     * @param viewport The viewport information
     * @return Screen position (x, y in pixels, z as depth)
     */
    static QVector3D projectToScreen(const QVector3D& worldPos, 
                                   const QMatrix4x4& mvpMatrix, 
                                   const ViewportInfo& viewport);
};

#endif // SCREENSPACEERROR_H
