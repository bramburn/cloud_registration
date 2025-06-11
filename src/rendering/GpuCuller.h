#ifndef GPUCULLER_H
#define GPUCULLER_H

#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QVector3D>
#include <QVector4D>
#include <vector>
#include <memory>

// Forward declarations
class OctreeNode;
struct AxisAlignedBoundingBox;

/**
 * @brief GPU-based culling module for high-performance point cloud rendering
 * 
 * This class implements GPU-based frustum and occlusion culling using compute shaders
 * to achieve interactive frame rates with large point cloud datasets (50+ million points).
 * 
 * Sprint 6 Requirements:
 * - GPU-based culling for massive datasets
 * - Compute shader integration
 * - Interactive frame rates (30+ FPS)
 * - Octree structure optimization
 */
class GpuCuller : protected QOpenGLExtraFunctions {
public:
    /**
     * @brief Structure representing a culling node for GPU processing
     */
    struct CullingNode {
        QVector3D minBounds;
        float padding1;
        QVector3D maxBounds;
        float padding2;
        uint32_t nodeIndex;
        uint32_t pointCount;
        uint32_t childMask;  // Bitmask indicating which children exist
        uint32_t padding3;
    };

    /**
     * @brief Culling result structure
     */
    struct CullingResult {
        std::vector<uint32_t> visibleNodeIndices;
        std::vector<uint32_t> visiblePointCounts;
        uint32_t totalVisiblePoints;
        float cullingTimeMs;
    };

    /**
     * @brief Culling parameters for GPU shader
     */
    struct CullingParams {
        QMatrix4x4 viewProjectionMatrix;
        QVector3D cameraPosition;
        float nearPlane;
        float farPlane;
        float screenSpaceErrorThreshold;
        uint32_t viewportWidth;
        uint32_t viewportHeight;
        uint32_t maxNodes;
    };

public:
    explicit GpuCuller();
    ~GpuCuller();

    /**
     * @brief Initialize GPU resources and compile compute shaders
     * @return true if initialization successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Clean up GPU resources
     */
    void cleanup();

    /**
     * @brief Check if GPU culling is available and initialized
     * @return true if ready for use, false otherwise
     */
    bool isInitialized() const;

    /**
     * @brief Update octree data for GPU culling
     * @param octreeNodes Vector of octree nodes to upload to GPU
     * @return true if upload successful, false otherwise
     */
    bool updateOctreeData(const std::vector<CullingNode>& octreeNodes);

    /**
     * @brief Perform GPU-based culling operation
     * @param params Culling parameters including camera and viewport info
     * @return Culling results with visible node indices
     */
    CullingResult performCulling(const CullingParams& params);

    /**
     * @brief Convert octree structure to GPU-friendly format
     * @param rootNode Root of the octree to convert
     * @return Vector of culling nodes for GPU processing
     */
    static std::vector<CullingNode> convertOctreeToGpuFormat(const OctreeNode* rootNode);

    /**
     * @brief Get performance statistics
     * @return Last culling operation time in milliseconds
     */
    float getLastCullingTime() const;

    /**
     * @brief Get GPU memory usage for culling buffers
     * @return Memory usage in bytes
     */
    size_t getGpuMemoryUsage() const;

    /**
     * @brief Set maximum number of nodes that can be processed
     * @param maxNodes Maximum node count
     */
    void setMaxNodes(uint32_t maxNodes);

    /**
     * @brief Enable or disable occlusion culling (in addition to frustum culling)
     * @param enabled true to enable occlusion culling, false for frustum only
     */
    void setOcclusionCullingEnabled(bool enabled);

private:
    /**
     * @brief Load and compile the compute shader
     * @return true if successful, false otherwise
     */
    bool loadComputeShader();

    /**
     * @brief Create GPU buffers for culling data
     * @return true if successful, false otherwise
     */
    bool createBuffers();

    /**
     * @brief Update uniform buffer with culling parameters
     * @param params Culling parameters to upload
     */
    void updateUniforms(const CullingParams& params);

    /**
     * @brief Read back results from GPU
     * @return Culling results
     */
    CullingResult readResults();

    /**
     * @brief Convert octree node recursively to GPU format
     * @param node Current octree node
     * @param nodes Output vector of GPU nodes
     * @param nodeIndex Current node index
     * @return Next available node index
     */
    static uint32_t convertNodeRecursive(const OctreeNode* node, 
                                       std::vector<CullingNode>& nodes, 
                                       uint32_t nodeIndex);

    /**
     * @brief Calculate child mask for octree node
     * @param node Octree node to analyze
     * @return Bitmask indicating which children exist
     */
    static uint32_t calculateChildMask(const OctreeNode* node);

private:
    // OpenGL resources
    std::unique_ptr<QOpenGLShaderProgram> m_computeShader;
    QOpenGLBuffer m_nodeBuffer;
    QOpenGLBuffer m_resultBuffer;
    QOpenGLBuffer m_uniformBuffer;
    
    // GPU state
    bool m_initialized;
    uint32_t m_maxNodes;
    uint32_t m_currentNodeCount;
    bool m_occlusionCullingEnabled;
    
    // Performance tracking
    float m_lastCullingTime;
    size_t m_gpuMemoryUsage;
    
    // Compute shader work group size
    static constexpr uint32_t WORK_GROUP_SIZE = 64;
    static constexpr uint32_t MAX_NODES_DEFAULT = 1000000;
    
    // Buffer binding points
    static constexpr uint32_t NODE_BUFFER_BINDING = 0;
    static constexpr uint32_t RESULT_BUFFER_BINDING = 1;
    static constexpr uint32_t UNIFORM_BUFFER_BINDING = 2;
};

#endif // GPUCULLER_H
