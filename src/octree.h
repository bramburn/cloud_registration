#ifndef OCTREE_H
#define OCTREE_H

#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <memory>
#include <vector>
#include <array>
#include <chrono>

// Point data structure compatible with existing PointCloudViewerWidget
struct PointFullData {
    float x, y, z;
    float r, g, b;  // Color (0-1 range)
    float intensity;
    
    PointFullData(float x = 0, float y = 0, float z = 0, 
                  float r = 1, float g = 1, float b = 1, float intensity = 1.0f)
        : x(x), y(y), z(z), r(r), g(g), b(b), intensity(intensity) {}
    
    // Constructor from existing point data format (XYZ only)
    PointFullData(float x, float y, float z)
        : x(x), y(y), z(z), r(1.0f), g(1.0f), b(1.0f), intensity(1.0f) {}
};

// Axis-Aligned Bounding Box for spatial subdivision
struct AxisAlignedBoundingBox {
    QVector3D min, max;
    
    AxisAlignedBoundingBox() = default;
    AxisAlignedBoundingBox(const QVector3D& min, const QVector3D& max) 
        : min(min), max(max) {}
    
    bool contains(float x, float y, float z) const {
        return x >= min.x() && x <= max.x() && 
               y >= min.y() && y <= max.y() && 
               z >= min.z() && z <= max.z();
    }
    
    QVector3D center() const {
        return (min + max) * 0.5f;
    }
    
    float distanceToPoint(const QVector3D& point) const {
        // Calculate distance from point to closest point on AABB
        QVector3D closest;
        closest.setX(qMax(min.x(), qMin(point.x(), max.x())));
        closest.setY(qMax(min.y(), qMin(point.y(), max.y())));
        closest.setZ(qMax(min.z(), qMin(point.z(), max.z())));
        return (point - closest).length();
    }
    
    QVector3D size() const {
        return max - min;
    }
};

// Forward declaration
class OctreeNode;

// Octree node for spatial subdivision
class OctreeNode {
public:
    AxisAlignedBoundingBox bounds;
    std::vector<PointFullData> points;
    std::array<std::unique_ptr<OctreeNode>, 8> children;
    bool isLeaf = true;
    int depth = 0;
    
    OctreeNode(const AxisAlignedBoundingBox& bounds, int depth = 0)
        : bounds(bounds), depth(depth) {}
    
    // Insert a point into the octree
    void insert(const PointFullData& point, int maxDepth = 8, int maxPointsPerNode = 100);
    
    // Subdivide this node into 8 children
    void subdivide();
    
    // Get the child index for a point (0-7)
    int getChildIndex(const PointFullData& point) const;
    
    // Collect visible points based on frustum culling and LOD
    void collectVisiblePoints(const std::array<QVector4D, 6>& frustumPlanes,
                             const QVector3D& cameraPos,
                             float lodDistance1, float lodDistance2,
                             std::vector<PointFullData>& visiblePoints) const;
    
private:
    // Test if this node's bounding box intersects with the view frustum
    bool intersectsFrustum(const std::array<QVector4D, 6>& frustumPlanes) const;
};

// Main octree class for managing the spatial data structure
class Octree {
public:
    std::unique_ptr<OctreeNode> root;
    
    // Build the octree from a vector of points
    void build(const std::vector<PointFullData>& points, int maxDepth = 8, int maxPointsPerNode = 100);
    
    // Build from existing point data format (flat float array)
    void buildFromFloatArray(const std::vector<float>& pointData, int maxDepth = 8, int maxPointsPerNode = 100);
    
    // Get visible points using frustum culling and LOD
    void getVisiblePoints(const std::array<QVector4D, 6>& frustumPlanes,
                         const QVector3D& cameraPos,
                         float lodDistance1, float lodDistance2,
                         std::vector<PointFullData>& visiblePoints) const;
    
    // Get all points (for fallback rendering)
    void getAllPoints(std::vector<PointFullData>& allPoints) const;
    
    // Statistics
    size_t getTotalPointCount() const;
    int getMaxDepth() const;
    size_t getNodeCount() const;
    
private:
    // Calculate bounding box for a set of points
    AxisAlignedBoundingBox calculateBounds(const std::vector<PointFullData>& points) const;
    
    // Helper functions for statistics
    void collectAllPoints(const OctreeNode* node, std::vector<PointFullData>& allPoints) const;
    size_t countPoints(const OctreeNode* node) const;
    int getDepth(const OctreeNode* node) const;
    size_t countNodes(const OctreeNode* node) const;
};

// Utility functions for frustum extraction
namespace FrustumUtils {
    // Extract frustum planes from view-projection matrix
    std::array<QVector4D, 6> extractFrustumPlanes(const QMatrix4x4& viewProjection);
    
    // Test point against frustum
    bool pointInFrustum(const QVector3D& point, const std::array<QVector4D, 6>& frustumPlanes);
    
    // Test AABB against frustum
    bool aabbInFrustum(const AxisAlignedBoundingBox& aabb, const std::array<QVector4D, 6>& frustumPlanes);
}

#endif // OCTREE_H
