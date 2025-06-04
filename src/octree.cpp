#include "octree.h"
#include "screenspaceerror.h"
#include <QDebug>
#include <algorithm>
#include <random>
#include <cmath>

void OctreeNode::insert(const PointFullData& point, int maxDepth, int maxPointsPerNode) {
    if (!bounds.contains(point.x, point.y, point.z)) {
        return;
    }
    
    if (isLeaf) {
        if (static_cast<int>(points.size()) < maxPointsPerNode || depth >= maxDepth) {
            points.push_back(point);
        } else {
            // Subdivide
            subdivide();
            
            // Redistribute existing points
            for (const auto& p : points) {
                int childIndex = getChildIndex(p);
                if (childIndex >= 0 && childIndex < 8 && children[childIndex]) {
                    children[childIndex]->insert(p, maxDepth, maxPointsPerNode);
                }
            }
            points.clear();
            
            // Insert new point
            int childIndex = getChildIndex(point);
            if (childIndex >= 0 && childIndex < 8 && children[childIndex]) {
                children[childIndex]->insert(point, maxDepth, maxPointsPerNode);
            }
        }
    } else {
        // Internal node - distribute to appropriate child
        int childIndex = getChildIndex(point);
        if (childIndex >= 0 && childIndex < 8 && children[childIndex]) {
            children[childIndex]->insert(point, maxDepth, maxPointsPerNode);
        }
    }
}

void OctreeNode::subdivide() {
    isLeaf = false;
    QVector3D center = bounds.center();
    QVector3D min = bounds.min;
    QVector3D max = bounds.max;
    
    // Create 8 child nodes with subdivided bounds
    children[0] = std::make_unique<OctreeNode>(
        AxisAlignedBoundingBox(min, center), depth + 1);
    children[1] = std::make_unique<OctreeNode>(
        AxisAlignedBoundingBox(QVector3D(center.x(), min.y(), min.z()), 
                              QVector3D(max.x(), center.y(), center.z())), depth + 1);
    children[2] = std::make_unique<OctreeNode>(
        AxisAlignedBoundingBox(QVector3D(min.x(), center.y(), min.z()), 
                              QVector3D(center.x(), max.y(), center.z())), depth + 1);
    children[3] = std::make_unique<OctreeNode>(
        AxisAlignedBoundingBox(QVector3D(center.x(), center.y(), min.z()), 
                              QVector3D(max.x(), max.y(), center.z())), depth + 1);
    children[4] = std::make_unique<OctreeNode>(
        AxisAlignedBoundingBox(QVector3D(min.x(), min.y(), center.z()), 
                              QVector3D(center.x(), center.y(), max.z())), depth + 1);
    children[5] = std::make_unique<OctreeNode>(
        AxisAlignedBoundingBox(QVector3D(center.x(), min.y(), center.z()), 
                              QVector3D(max.x(), center.y(), max.z())), depth + 1);
    children[6] = std::make_unique<OctreeNode>(
        AxisAlignedBoundingBox(QVector3D(min.x(), center.y(), center.z()), 
                              QVector3D(center.x(), max.y(), max.z())), depth + 1);
    children[7] = std::make_unique<OctreeNode>(
        AxisAlignedBoundingBox(center, max), depth + 1);
}

int OctreeNode::getChildIndex(const PointFullData& point) const {
    QVector3D center = bounds.center();
    int index = 0;
    
    if (point.x > center.x()) index |= 1;
    if (point.y > center.y()) index |= 2;
    if (point.z > center.z()) index |= 4;
    
    return index;
}

void OctreeNode::collectVisiblePoints(const std::array<QVector4D, 6>& frustumPlanes,
                                     const QVector3D& cameraPos,
                                     float lodDistance1, float lodDistance2,
                                     std::vector<PointFullData>& visiblePoints) const {
    // Check if node intersects with view frustum
    if (!intersectsFrustum(frustumPlanes)) {
        return;
    }
    
    // Calculate distance from camera to node
    float distance = bounds.distanceToPoint(cameraPos);
    
    if (isLeaf) {
        // Apply LOD based on distance
        if (distance < lodDistance1) {
            // Close: render all points
            visiblePoints.insert(visiblePoints.end(), points.begin(), points.end());
        } else if (distance < lodDistance2) {
            // Medium distance: render 50% of points
            for (size_t i = 0; i < points.size(); i += 2) {
                visiblePoints.push_back(points[i]);
            }
        } else {
            // Far distance: render 10% of points
            for (size_t i = 0; i < points.size(); i += 10) {
                visiblePoints.push_back(points[i]);
            }
        }
    } else {
        // Internal node: recurse to children if not too far
        if (distance < lodDistance2) {
            for (const auto& child : children) {
                if (child) {
                    child->collectVisiblePoints(frustumPlanes, cameraPos, 
                                              lodDistance1, lodDistance2, visiblePoints);
                }
            }
        } else {
            // Very far: stop recursion and render a sample from this level
            // For now, just skip rendering at this distance
        }
    }
}

bool OctreeNode::intersectsFrustum(const std::array<QVector4D, 6>& frustumPlanes) const {
    // Test AABB against all 6 frustum planes
    for (const auto& plane : frustumPlanes) {
        QVector3D normal(plane.x(), plane.y(), plane.z());
        float distance = plane.w();
        
        // Find the positive vertex (farthest along plane normal)
        QVector3D positiveVertex;
        positiveVertex.setX(normal.x() >= 0 ? bounds.max.x() : bounds.min.x());
        positiveVertex.setY(normal.y() >= 0 ? bounds.max.y() : bounds.min.y());
        positiveVertex.setZ(normal.z() >= 0 ? bounds.max.z() : bounds.min.z());
        
        // If positive vertex is behind plane, AABB is completely outside
        if (QVector3D::dotProduct(normal, positiveVertex) + distance < 0) {
            return false;
        }
    }
    return true;
}

void Octree::build(const std::vector<PointFullData>& points, int maxDepth, int maxPointsPerNode) {
    if (points.empty()) return;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    AxisAlignedBoundingBox rootBounds = calculateBounds(points);
    root = std::make_unique<OctreeNode>(rootBounds);
    
    for (const auto& point : points) {
        root->insert(point, maxDepth, maxPointsPerNode);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    qDebug() << "Octree built in" << duration.count() << "ms for" << points.size() << "points";
}

void Octree::buildFromFloatArray(const std::vector<float>& pointData, int maxDepth, int maxPointsPerNode) {
    if (pointData.empty() || pointData.size() % 3 != 0) {
        qWarning() << "Invalid point data for octree construction";
        return;
    }
    
    std::vector<PointFullData> points;
    points.reserve(pointData.size() / 3);
    
    for (size_t i = 0; i < pointData.size(); i += 3) {
        points.emplace_back(pointData[i], pointData[i + 1], pointData[i + 2]);
    }
    
    build(points, maxDepth, maxPointsPerNode);
}

void Octree::getVisiblePoints(const std::array<QVector4D, 6>& frustumPlanes,
                             const QVector3D& cameraPos,
                             float lodDistance1, float lodDistance2,
                             std::vector<PointFullData>& visiblePoints) const {
    if (root) {
        root->collectVisiblePoints(frustumPlanes, cameraPos, 
                                  lodDistance1, lodDistance2, visiblePoints);
    }
}

void Octree::getAllPoints(std::vector<PointFullData>& allPoints) const {
    if (root) {
        collectAllPoints(root.get(), allPoints);
    }
}

AxisAlignedBoundingBox Octree::calculateBounds(const std::vector<PointFullData>& points) const {
    if (points.empty()) return AxisAlignedBoundingBox();
    
    float minX = points[0].x, maxX = points[0].x;
    float minY = points[0].y, maxY = points[0].y;
    float minZ = points[0].z, maxZ = points[0].z;
    
    for (const auto& point : points) {
        minX = std::min(minX, point.x);
        maxX = std::max(maxX, point.x);
        minY = std::min(minY, point.y);
        maxY = std::max(maxY, point.y);
        minZ = std::min(minZ, point.z);
        maxZ = std::max(maxZ, point.z);
    }
    
    return AxisAlignedBoundingBox(QVector3D(minX, minY, minZ), QVector3D(maxX, maxY, maxZ));
}

void Octree::collectAllPoints(const OctreeNode* node, std::vector<PointFullData>& allPoints) const {
    if (!node) return;
    
    if (node->isLeaf) {
        allPoints.insert(allPoints.end(), node->points.begin(), node->points.end());
    } else {
        for (const auto& child : node->children) {
            if (child) {
                collectAllPoints(child.get(), allPoints);
            }
        }
    }
}

size_t Octree::getTotalPointCount() const {
    return root ? countPoints(root.get()) : 0;
}

size_t Octree::countPoints(const OctreeNode* node) const {
    if (!node) return 0;
    
    if (node->isLeaf) {
        return node->points.size();
    } else {
        size_t count = 0;
        for (const auto& child : node->children) {
            if (child) {
                count += countPoints(child.get());
            }
        }
        return count;
    }
}

int Octree::getMaxDepth() const {
    return root ? getDepth(root.get()) : 0;
}

int Octree::getDepth(const OctreeNode* node) const {
    if (!node) return 0;
    
    if (node->isLeaf) {
        return node->depth;
    } else {
        int maxChildDepth = 0;
        for (const auto& child : node->children) {
            if (child) {
                maxChildDepth = std::max(maxChildDepth, getDepth(child.get()));
            }
        }
        return maxChildDepth;
    }
}

size_t Octree::getNodeCount() const {
    return root ? countNodes(root.get()) : 0;
}

size_t Octree::countNodes(const OctreeNode* node) const {
    if (!node) return 0;
    
    size_t count = 1; // Count this node
    if (!node->isLeaf) {
        for (const auto& child : node->children) {
            if (child) {
                count += countNodes(child.get());
            }
        }
    }
    return count;
}

// Frustum utility functions
namespace FrustumUtils {
    std::array<QVector4D, 6> extractFrustumPlanes(const QMatrix4x4& viewProjection) {
        std::array<QVector4D, 6> planes;
        
        // Extract frustum planes from view-projection matrix
        // Left plane
        planes[0] = QVector4D(viewProjection(3, 0) + viewProjection(0, 0),
                             viewProjection(3, 1) + viewProjection(0, 1),
                             viewProjection(3, 2) + viewProjection(0, 2),
                             viewProjection(3, 3) + viewProjection(0, 3));
        
        // Right plane
        planes[1] = QVector4D(viewProjection(3, 0) - viewProjection(0, 0),
                             viewProjection(3, 1) - viewProjection(0, 1),
                             viewProjection(3, 2) - viewProjection(0, 2),
                             viewProjection(3, 3) - viewProjection(0, 3));
        
        // Bottom plane
        planes[2] = QVector4D(viewProjection(3, 0) + viewProjection(1, 0),
                             viewProjection(3, 1) + viewProjection(1, 1),
                             viewProjection(3, 2) + viewProjection(1, 2),
                             viewProjection(3, 3) + viewProjection(1, 3));
        
        // Top plane
        planes[3] = QVector4D(viewProjection(3, 0) - viewProjection(1, 0),
                             viewProjection(3, 1) - viewProjection(1, 1),
                             viewProjection(3, 2) - viewProjection(1, 2),
                             viewProjection(3, 3) - viewProjection(1, 3));
        
        // Near plane
        planes[4] = QVector4D(viewProjection(3, 0) + viewProjection(2, 0),
                             viewProjection(3, 1) + viewProjection(2, 1),
                             viewProjection(3, 2) + viewProjection(2, 2),
                             viewProjection(3, 3) + viewProjection(2, 3));
        
        // Far plane
        planes[5] = QVector4D(viewProjection(3, 0) - viewProjection(2, 0),
                             viewProjection(3, 1) - viewProjection(2, 1),
                             viewProjection(3, 2) - viewProjection(2, 2),
                             viewProjection(3, 3) - viewProjection(2, 3));
        
        // Normalize planes
        for (auto& plane : planes) {
            float length = std::sqrt(plane.x() * plane.x() + plane.y() * plane.y() + plane.z() * plane.z());
            if (length > 0) {
                plane /= length;
            }
        }
        
        return planes;
    }
    
    bool pointInFrustum(const QVector3D& point, const std::array<QVector4D, 6>& frustumPlanes) {
        for (const auto& plane : frustumPlanes) {
            QVector3D normal(plane.x(), plane.y(), plane.z());
            float distance = plane.w();
            if (QVector3D::dotProduct(normal, point) + distance < 0) {
                return false;
            }
        }
        return true;
    }
    
    bool aabbInFrustum(const AxisAlignedBoundingBox& aabb, const std::array<QVector4D, 6>& frustumPlanes) {
        for (const auto& plane : frustumPlanes) {
            QVector3D normal(plane.x(), plane.y(), plane.z());
            float distance = plane.w();
            
            // Find the positive vertex (farthest along plane normal)
            QVector3D positiveVertex;
            positiveVertex.setX(normal.x() >= 0 ? aabb.max.x() : aabb.min.x());
            positiveVertex.setY(normal.y() >= 0 ? aabb.max.y() : aabb.min.y());
            positiveVertex.setZ(normal.z() >= 0 ? aabb.max.z() : aabb.min.z());
            
            // If positive vertex is behind plane, AABB is completely outside
            if (QVector3D::dotProduct(normal, positiveVertex) + distance < 0) {
                return false;
            }
        }
        return true;
    }
}

// Sprint R2: Point sampling methods implementation
std::vector<PointFullData> OctreeNode::getSampledPoints(int maxPoints) const {
    if (points.empty()) {
        return {};
    }

    if (static_cast<int>(points.size()) <= maxPoints) {
        return points;
    }

    std::vector<PointFullData> sampledPoints;
    sampledPoints.reserve(maxPoints);

    // Use deterministic sampling based on point index for consistency
    int step = static_cast<int>(points.size()) / maxPoints;
    for (int i = 0; i < maxPoints && i * step < static_cast<int>(points.size()); ++i) {
        sampledPoints.push_back(points[i * step]);
    }

    return sampledPoints;
}

std::vector<PointFullData> OctreeNode::getSampledPointsByPercentage(float percentage) const {
    int maxPoints = static_cast<int>(points.size() * percentage);
    return getSampledPoints(maxPoints);
}

std::vector<PointFullData> OctreeNode::getRepresentativePoints() const {
    if (!m_representativePointsCalculated) {
        calculateRepresentativePoints();
        m_representativePointsCalculated = true;
    }
    return m_representativePoints;
}

void OctreeNode::calculateRepresentativePoints() const {
    if (isLeaf) {
        // For leaf nodes, use a subset of points
        m_representativePoints = getSampledPoints(std::min(100, static_cast<int>(points.size())));
    } else {
        // For internal nodes, collect representative points from children
        m_representativePoints.clear();
        for (const auto& child : children) {
            if (child) {
                auto childRepPoints = child->getRepresentativePoints();
                m_representativePoints.insert(m_representativePoints.end(),
                                            childRepPoints.begin(), childRepPoints.end());
            }
        }

        // Limit total representative points for internal nodes
        if (m_representativePoints.size() > 200) {
            m_representativePoints.resize(200);
        }
    }
}

void OctreeNode::collectVisiblePointsWithScreenSpaceError(
    const std::array<QVector4D, 6>& frustumPlanes,
    const QMatrix4x4& mvpMatrix,
    const ViewportInfo& viewport,
    float primaryThreshold,
    float cullThreshold,
    std::vector<PointFullData>& visiblePoints) const {

    // Check frustum culling first
    if (!intersectsFrustum(frustumPlanes)) {
        return;
    }

    // Calculate screen-space error
    float screenSpaceError = ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
        bounds, mvpMatrix, viewport);

    // Cull if error is too small
    if (ScreenSpaceErrorCalculator::shouldCullNode(screenSpaceError, cullThreshold)) {
        return;
    }

    // Stop recursion if error is below primary threshold
    if (ScreenSpaceErrorCalculator::shouldStopRecursion(screenSpaceError, primaryThreshold)) {
        // Render representative points for this coarse LOD level
        auto representativePoints = getRepresentativePoints();
        visiblePoints.insert(visiblePoints.end(),
                           representativePoints.begin(), representativePoints.end());
        return;
    }

    if (isLeaf) {
        // Leaf node: add all points
        visiblePoints.insert(visiblePoints.end(), points.begin(), points.end());
    } else {
        // Internal node: recurse to children
        for (const auto& child : children) {
            if (child) {
                child->collectVisiblePointsWithScreenSpaceError(
                    frustumPlanes, mvpMatrix, viewport,
                    primaryThreshold, cullThreshold, visiblePoints);
            }
        }
    }
}
