#ifndef OCTREE_H
#define OCTREE_H

#include <QMatrix4x4>
#include <QVector3D>
#include <QVector4D>

#include <array>
#include <chrono>
#include <memory>
#include <optional>
#include <vector>

// Forward declarations
struct ViewportInfo;

// Enhanced Point data structure for Sprint R3 - supports optional attributes
struct PointFullData
{
    // Position coordinates (required)
    float x, y, z;

    // Color attributes (optional, 0-255 range as specified in backlog)
    std::optional<uint8_t> r, g, b;

    // Intensity attribute (optional, normalized 0-1 range as specified in backlog)
    std::optional<float> intensity;

    // Normal attribute (optional, for Sprint R4 lighting)
    std::optional<QVector3D> normal;

    // Default constructor (XYZ only)
    PointFullData(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    // Constructor with color (uint8_t as per backlog Task R3.1.1)
    PointFullData(float x, float y, float z, uint8_t r, uint8_t g, uint8_t b) : x(x), y(y), z(z), r(r), g(g), b(b) {}

    // Constructor with intensity (float 0-1 as per backlog Task R3.2.1)
    PointFullData(float x, float y, float z, float intensity) : x(x), y(y), z(z), intensity(intensity) {}

    // Constructor with both color and intensity
    PointFullData(float x, float y, float z, uint8_t r, uint8_t g, uint8_t b, float intensity)
        : x(x), y(y), z(z), r(r), g(g), b(b), intensity(intensity)
    {
    }

    // Utility methods as required by backlog
    bool hasColor() const
    {
        return r.has_value() && g.has_value() && b.has_value();
    }
    bool hasIntensity() const
    {
        return intensity.has_value();
    }
    bool hasNormal() const
    {
        return normal.has_value();
    }

    // Get normalized color for shader (0-1 range)
    void getNormalizedColor(float& nr, float& ng, float& nb) const
    {
        nr = hasColor() ? r.value() / 255.0f : 1.0f;
        ng = hasColor() ? g.value() / 255.0f : 1.0f;
        nb = hasColor() ? b.value() / 255.0f : 1.0f;
    }
};

// Sprint R4: Aggregate data structure for point splatting (Task R4.1.2)
struct AggregateNodeData
{
    QVector3D center;
    QVector3D averageColor;
    float averageIntensity;
    QVector3D averageNormal;
    float boundingRadius;
    int pointCount;
    float screenSpaceSize;  // For splat sizing

    AggregateNodeData()
        : center(0, 0, 0),
          averageColor(1, 1, 1),
          averageIntensity(1.0f),
          averageNormal(0, 0, 1),
          boundingRadius(0.0f),
          pointCount(0),
          screenSpaceSize(0.0f)
    {
    }
};

// Axis-Aligned Bounding Box for spatial subdivision
struct AxisAlignedBoundingBox
{
    QVector3D min, max;

    AxisAlignedBoundingBox() = default;
    AxisAlignedBoundingBox(const QVector3D& min, const QVector3D& max) : min(min), max(max) {}

    bool contains(float x, float y, float z) const
    {
        return x >= min.x() && x <= max.x() && y >= min.y() && y <= max.y() && z >= min.z() && z <= max.z();
    }

    QVector3D center() const
    {
        return (min + max) * 0.5f;
    }

    float distanceToPoint(const QVector3D& point) const
    {
        // Calculate distance from point to closest point on AABB
        QVector3D closest;
        closest.setX(qMax(min.x(), qMin(point.x(), max.x())));
        closest.setY(qMax(min.y(), qMin(point.y(), max.y())));
        closest.setZ(qMax(min.z(), qMin(point.z(), max.z())));
        return (point - closest).length();
    }

    QVector3D size() const
    {
        return max - min;
    }
};

// Forward declaration
class OctreeNode;

// Octree node for spatial subdivision
class OctreeNode
{
public:
    AxisAlignedBoundingBox bounds;
    std::vector<PointFullData> points;
    std::array<std::unique_ptr<OctreeNode>, 8> children;
    bool isLeaf = true;
    int depth = 0;

    OctreeNode(const AxisAlignedBoundingBox& bounds, int depth = 0) : bounds(bounds), depth(depth) {}

    // Insert a point into the octree
    void insert(const PointFullData& point, int maxDepth = 8, int maxPointsPerNode = 100);

    // Subdivide this node into 8 children
    void subdivide();

    // Get the child index for a point (0-7)
    int getChildIndex(const PointFullData& point) const;

    // Collect visible points based on frustum culling and LOD
    void collectVisiblePoints(const std::array<QVector4D, 6>& frustumPlanes,
                              const QVector3D& cameraPos,
                              float lodDistance1,
                              float lodDistance2,
                              std::vector<PointFullData>& visiblePoints) const;

    // Sprint R2: Point sampling methods for refined LOD
    std::vector<PointFullData> getSampledPoints(int maxPoints) const;
    std::vector<PointFullData> getSampledPointsByPercentage(float percentage) const;
    std::vector<PointFullData> getRepresentativePoints() const;

    // Sprint R2: Screen-space error based traversal
    void collectVisiblePointsWithScreenSpaceError(const std::array<QVector4D, 6>& frustumPlanes,
                                                  const QMatrix4x4& mvpMatrix,
                                                  const ViewportInfo& viewport,
                                                  float primaryThreshold,
                                                  float cullThreshold,
                                                  std::vector<PointFullData>& visiblePoints) const;

    // Sprint R4: Splatting and lighting methods (Task R4.1.2, R4.1.4, R4.1.5)
    const AggregateNodeData& getAggregateData() const;
    void calculateAggregateData() const;
    bool shouldRenderAsSplat(float screenSpaceError, float splatThreshold) const;

    // Enhanced traversal for splat rendering (Task R4.1.4, R4.1.5)
    void collectRenderData(const std::array<QVector4D, 6>& frustumPlanes,
                           const QMatrix4x4& mvpMatrix,
                           const ViewportInfo& viewport,
                           float splatThreshold,
                           bool splattingEnabled,
                           std::vector<PointFullData>& individualPoints,
                           std::vector<AggregateNodeData>& splatData) const;

private:
    // Test if this node's bounding box intersects with the view frustum
    bool intersectsFrustum(const std::array<QVector4D, 6>& frustumPlanes) const;

    // Sprint R2: Pre-calculated representative points for coarse LOD
    mutable std::vector<PointFullData> m_representativePoints;
    mutable bool m_representativePointsCalculated = false;

    // Sprint R4: Aggregate data for splatting (Task R4.1.2)
    mutable AggregateNodeData m_aggregateData;
    mutable bool m_aggregateDataCalculated = false;

    void calculateRepresentativePoints() const;
    QVector3D estimateNormalFromPoints() const;
};

// Main octree class for managing the spatial data structure
class Octree
{
public:
    std::unique_ptr<OctreeNode> root;

    // Build the octree from a vector of points
    void build(const std::vector<PointFullData>& points, int maxDepth = 8, int maxPointsPerNode = 100);

    // Build from existing point data format (flat float array)
    void buildFromFloatArray(const std::vector<float>& pointData, int maxDepth = 8, int maxPointsPerNode = 100);

    // Get visible points using frustum culling and LOD
    void getVisiblePoints(const std::array<QVector4D, 6>& frustumPlanes,
                          const QVector3D& cameraPos,
                          float lodDistance1,
                          float lodDistance2,
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
namespace FrustumUtils
{
// Extract frustum planes from view-projection matrix
std::array<QVector4D, 6> extractFrustumPlanes(const QMatrix4x4& viewProjection);

// Test point against frustum
bool pointInFrustum(const QVector3D& point, const std::array<QVector4D, 6>& frustumPlanes);

// Test AABB against frustum
bool aabbInFrustum(const AxisAlignedBoundingBox& aabb, const std::array<QVector4D, 6>& frustumPlanes);
}  // namespace FrustumUtils

#endif  // OCTREE_H
