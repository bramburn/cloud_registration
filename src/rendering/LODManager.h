#ifndef LODMANAGER_H
#define LODMANAGER_H

#include <QObject>
#include <QVector3D>
#include <QMatrix4x4>
#include <vector>
#include <memory>

// Forward declarations
class Octree;
class CameraController;

/**
 * @brief LODManager - Level of Detail management for performance optimization
 * 
 * This class implements the Level of Detail (LOD) logic required for rendering 
 * large datasets performantly. It acts as the decision-maker for what gets 
 * rendered each frame.
 * 
 * Sprint 1 Requirements:
 * - Distance-based culling and detail selection
 * - Frustum culling for out-of-view geometry
 * - Dynamic point selection based on camera parameters
 * - Octree traversal for efficient spatial queries
 * - Performance optimization for large point clouds (50M+ points)
 */
class LODManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     */
    explicit LODManager(QObject* parent = nullptr);

    /**
     * @brief Destructor
     */
    ~LODManager();

    /**
     * @brief Set the octree data structure to use for LOD
     * @param octree Pointer to octree (not owned by LODManager)
     */
    void setOctree(Octree* octree);

    /**
     * @brief Set the camera controller for view information
     * @param camera Pointer to camera controller (not owned by LODManager)
     */
    void setCameraController(CameraController* camera);

    /**
     * @brief Configure LOD parameters
     * @param nearDistance Distance threshold for high detail
     * @param farDistance Distance threshold for low detail
     * @param maxPointsPerFrame Maximum points to render per frame
     */
    void configureLOD(float nearDistance, float farDistance, int maxPointsPerFrame);

    /**
     * @brief Enable or disable frustum culling
     * @param enabled True to enable frustum culling
     */
    void setFrustumCullingEnabled(bool enabled) { m_frustumCullingEnabled = enabled; }

    /**
     * @brief Enable or disable distance-based LOD
     * @param enabled True to enable distance-based LOD
     */
    void setDistanceLODEnabled(bool enabled) { m_distanceLODEnabled = enabled; }

    /**
     * @brief Get visible points for current frame
     * @param aspectRatio Viewport aspect ratio
     * @return Vector of point indices to render
     */
    std::vector<int> getVisiblePoints(float aspectRatio);

    /**
     * @brief Get visible point data for current frame
     * @param aspectRatio Viewport aspect ratio
     * @param pointData Original point cloud data
     * @return Vector of visible point coordinates
     */
    std::vector<float> getVisiblePointData(float aspectRatio, const std::vector<float>& pointData);

    /**
     * @brief Get statistics about last LOD operation
     * @param totalPoints Total points in dataset
     * @param visiblePoints Points selected for rendering
     * @param culledNodes Octree nodes culled
     */
    void getLastLODStats(int& totalPoints, int& visiblePoints, int& culledNodes) const;

    /**
     * @brief Set LOD quality level (0.0 = lowest quality, 1.0 = highest quality)
     * @param quality Quality level between 0.0 and 1.0
     */
    void setQualityLevel(float quality);

    /**
     * @brief Get current quality level
     * @return Quality level between 0.0 and 1.0
     */
    float getQualityLevel() const { return m_qualityLevel; }

    /**
     * @brief Enable adaptive quality based on performance
     * @param enabled True to enable adaptive quality
     * @param targetFPS Target frame rate for adaptation
     */
    void setAdaptiveQuality(bool enabled, float targetFPS = 30.0f);

    /**
     * @brief Update performance metrics for adaptive quality
     * @param currentFPS Current frame rate
     */
    void updatePerformanceMetrics(float currentFPS);

signals:
    /**
     * @brief Emitted when LOD parameters change
     */
    void lodParametersChanged();

    /**
     * @brief Emitted with LOD statistics
     * @param visiblePoints Number of visible points
     * @param totalPoints Total points in dataset
     * @param reductionRatio Reduction ratio (0.0-1.0)
     */
    void lodStatsUpdated(int visiblePoints, int totalPoints, float reductionRatio);

private:
    // LOD parameters
    float m_nearDistance;
    float m_farDistance;
    int m_maxPointsPerFrame;
    float m_qualityLevel;

    // Feature flags
    bool m_frustumCullingEnabled;
    bool m_distanceLODEnabled;
    bool m_adaptiveQualityEnabled;

    // Adaptive quality
    float m_targetFPS;
    float m_currentFPS;
    float m_fpsHistory[10];
    int m_fpsHistoryIndex;

    // References (not owned)
    Octree* m_octree;
    CameraController* m_camera;

    // Statistics
    mutable int m_lastTotalPoints;
    mutable int m_lastVisiblePoints;
    mutable int m_lastCulledNodes;

    // Helper methods
    bool isNodeInFrustum(const QVector3D& nodeCenter, float nodeSize, 
                        const QMatrix4x4& viewProjectionMatrix) const;
    float calculateNodeDistance(const QVector3D& nodeCenter) const;
    int calculateLODLevel(float distance) const;
    void updateAdaptiveQuality();
    std::vector<int> traverseOctreeForLOD(float aspectRatio);
    void extractFrustumPlanes(const QMatrix4x4& viewProjectionMatrix, 
                             std::vector<QVector4D>& planes) const;
    bool isPointInFrustum(const QVector3D& point, 
                         const std::vector<QVector4D>& planes) const;
};

#endif // LODMANAGER_H
