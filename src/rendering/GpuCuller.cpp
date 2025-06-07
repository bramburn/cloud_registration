#include "GpuCuller.h"
#include "../octree.h"
#include <QOpenGLContext>
#include <QElapsedTimer>
#include <QDebug>
#include <QFile>
#include <QTextStream>

GpuCuller::GpuCuller()
    : m_computeShader(nullptr)
    , m_nodeBuffer(QOpenGLBuffer::ShaderStorageBuffer)
    , m_resultBuffer(QOpenGLBuffer::ShaderStorageBuffer)
    , m_uniformBuffer(QOpenGLBuffer::UniformBuffer)
    , m_initialized(false)
    , m_maxNodes(MAX_NODES_DEFAULT)
    , m_currentNodeCount(0)
    , m_occlusionCullingEnabled(false)
    , m_lastCullingTime(0.0f)
    , m_gpuMemoryUsage(0)
{
}

GpuCuller::~GpuCuller() {
    cleanup();
}

bool GpuCuller::initialize() {
    if (m_initialized) {
        return true;
    }

    // Initialize OpenGL functions
    if (!initializeOpenGLFunctions()) {
        qWarning() << "GpuCuller: Failed to initialize OpenGL functions";
        return false;
    }

    // Check for compute shader support
    QOpenGLContext* context = QOpenGLContext::currentContext();
    if (!context) {
        qWarning() << "GpuCuller: No current OpenGL context";
        return false;
    }

    if (!context->hasExtension("GL_ARB_compute_shader")) {
        qWarning() << "GpuCuller: Compute shaders not supported";
        return false;
    }

    // Load and compile compute shader
    if (!loadComputeShader()) {
        qWarning() << "GpuCuller: Failed to load compute shader";
        return false;
    }

    // Create GPU buffers
    if (!createBuffers()) {
        qWarning() << "GpuCuller: Failed to create GPU buffers";
        return false;
    }

    m_initialized = true;
    qDebug() << "GpuCuller: Successfully initialized with max nodes:" << m_maxNodes;
    return true;
}

void GpuCuller::cleanup() {
    if (m_nodeBuffer.isCreated()) {
        m_nodeBuffer.destroy();
    }
    if (m_resultBuffer.isCreated()) {
        m_resultBuffer.destroy();
    }
    if (m_uniformBuffer.isCreated()) {
        m_uniformBuffer.destroy();
    }
    
    m_computeShader.reset();
    m_initialized = false;
    m_gpuMemoryUsage = 0;
}

bool GpuCuller::isInitialized() const {
    return m_initialized;
}

bool GpuCuller::loadComputeShader() {
    m_computeShader = std::make_unique<QOpenGLShaderProgram>();

    // Load compute shader source
    QFile shaderFile(":/shaders/culling.comp");
    if (!shaderFile.open(QIODevice::ReadOnly)) {
        // Fallback to file system path
        shaderFile.setFileName("shaders/culling.comp");
        if (!shaderFile.open(QIODevice::ReadOnly)) {
            qWarning() << "GpuCuller: Cannot open compute shader file";
            return false;
        }
    }

    QTextStream stream(&shaderFile);
    QString shaderSource = stream.readAll();
    shaderFile.close();

    // Compile compute shader
    if (!m_computeShader->addShaderFromSourceCode(QOpenGLShader::Compute, shaderSource)) {
        qWarning() << "GpuCuller: Failed to compile compute shader:" << m_computeShader->log();
        return false;
    }

    // Link shader program
    if (!m_computeShader->link()) {
        qWarning() << "GpuCuller: Failed to link compute shader:" << m_computeShader->log();
        return false;
    }

    return true;
}

bool GpuCuller::createBuffers() {
    // Create node buffer
    if (!m_nodeBuffer.create()) {
        qWarning() << "GpuCuller: Failed to create node buffer";
        return false;
    }

    m_nodeBuffer.bind();
    m_nodeBuffer.allocate(m_maxNodes * sizeof(CullingNode));
    m_nodeBuffer.release();

    // Create result buffer
    if (!m_resultBuffer.create()) {
        qWarning() << "GpuCuller: Failed to create result buffer";
        return false;
    }

    m_resultBuffer.bind();
    m_resultBuffer.allocate(m_maxNodes * sizeof(uint32_t) * 2); // indices + counts
    m_resultBuffer.release();

    // Create uniform buffer
    if (!m_uniformBuffer.create()) {
        qWarning() << "GpuCuller: Failed to create uniform buffer";
        return false;
    }

    m_uniformBuffer.bind();
    m_uniformBuffer.allocate(sizeof(CullingParams));
    m_uniformBuffer.release();

    // Calculate GPU memory usage
    m_gpuMemoryUsage = (m_maxNodes * sizeof(CullingNode)) + 
                       (m_maxNodes * sizeof(uint32_t) * 2) + 
                       sizeof(CullingParams);

    return true;
}

bool GpuCuller::updateOctreeData(const std::vector<CullingNode>& octreeNodes) {
    if (!m_initialized) {
        qWarning() << "GpuCuller: Not initialized";
        return false;
    }

    if (octreeNodes.size() > m_maxNodes) {
        qWarning() << "GpuCuller: Too many nodes:" << octreeNodes.size() << "max:" << m_maxNodes;
        return false;
    }

    m_currentNodeCount = static_cast<uint32_t>(octreeNodes.size());

    // Upload node data to GPU
    m_nodeBuffer.bind();
    void* nodeData = m_nodeBuffer.map(QOpenGLBuffer::WriteOnly);
    if (!nodeData) {
        qWarning() << "GpuCuller: Failed to map node buffer";
        m_nodeBuffer.release();
        return false;
    }

    memcpy(nodeData, octreeNodes.data(), octreeNodes.size() * sizeof(CullingNode));
    m_nodeBuffer.unmap();
    m_nodeBuffer.release();

    return true;
}

GpuCuller::CullingResult GpuCuller::performCulling(const CullingParams& params) {
    CullingResult result;
    result.totalVisiblePoints = 0;
    result.cullingTimeMs = 0.0f;

    if (!m_initialized || m_currentNodeCount == 0) {
        return result;
    }

    QElapsedTimer timer;
    timer.start();

    // Update uniforms
    updateUniforms(params);

    // Bind buffers to shader storage buffer objects
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, NODE_BUFFER_BINDING, m_nodeBuffer.bufferId());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, RESULT_BUFFER_BINDING, m_resultBuffer.bufferId());
    glBindBufferBase(GL_UNIFORM_BUFFER, UNIFORM_BUFFER_BINDING, m_uniformBuffer.bufferId());

    // Bind and execute compute shader
    m_computeShader->bind();
    
    // Calculate work groups
    uint32_t workGroups = (m_currentNodeCount + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE;
    
    // Dispatch compute shader
    glDispatchCompute(workGroups, 1, 1);
    
    // Wait for completion
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
    m_computeShader->release();

    // Read back results
    result = readResults();
    result.cullingTimeMs = timer.nsecsElapsed() / 1000000.0f;
    m_lastCullingTime = result.cullingTimeMs;

    return result;
}

void GpuCuller::updateUniforms(const CullingParams& params) {
    m_uniformBuffer.bind();
    void* uniformData = m_uniformBuffer.map(QOpenGLBuffer::WriteOnly);
    if (uniformData) {
        memcpy(uniformData, &params, sizeof(CullingParams));
        m_uniformBuffer.unmap();
    }
    m_uniformBuffer.release();
}

GpuCuller::CullingResult GpuCuller::readResults() {
    CullingResult result;
    
    m_resultBuffer.bind();
    const uint32_t* resultData = static_cast<const uint32_t*>(
        m_resultBuffer.map(QOpenGLBuffer::ReadOnly));
    
    if (resultData) {
        // First part contains visible node indices
        // Second part contains point counts for each visible node
        uint32_t visibleNodeCount = resultData[0];
        
        if (visibleNodeCount > 0 && visibleNodeCount <= m_currentNodeCount) {
            result.visibleNodeIndices.reserve(visibleNodeCount);
            result.visiblePointCounts.reserve(visibleNodeCount);
            
            for (uint32_t i = 0; i < visibleNodeCount; ++i) {
                uint32_t nodeIndex = resultData[1 + i];
                uint32_t pointCount = resultData[1 + m_maxNodes + i];
                
                result.visibleNodeIndices.push_back(nodeIndex);
                result.visiblePointCounts.push_back(pointCount);
                result.totalVisiblePoints += pointCount;
            }
        }
        
        m_resultBuffer.unmap();
    }
    
    m_resultBuffer.release();
    return result;
}

std::vector<GpuCuller::CullingNode> GpuCuller::convertOctreeToGpuFormat(const OctreeNode* rootNode) {
    std::vector<CullingNode> nodes;
    if (!rootNode) {
        return nodes;
    }

    // Reserve space for efficiency
    nodes.reserve(10000); // Reasonable initial size
    
    convertNodeRecursive(rootNode, nodes, 0);
    return nodes;
}

uint32_t GpuCuller::convertNodeRecursive(const OctreeNode* node, 
                                        std::vector<CullingNode>& nodes, 
                                        uint32_t nodeIndex) {
    if (!node || nodeIndex >= MAX_NODES_DEFAULT) {
        return nodeIndex;
    }

    // Ensure we have space for this node
    if (nodeIndex >= nodes.size()) {
        nodes.resize(nodeIndex + 1);
    }

    CullingNode& gpuNode = nodes[nodeIndex];
    gpuNode.minBounds = QVector3D(node->bounds.min.x(), node->bounds.min.y(), node->bounds.min.z());
    gpuNode.maxBounds = QVector3D(node->bounds.max.x(), node->bounds.max.y(), node->bounds.max.z());
    gpuNode.nodeIndex = nodeIndex;
    gpuNode.pointCount = static_cast<uint32_t>(node->points.size());
    gpuNode.childMask = calculateChildMask(node);

    uint32_t nextIndex = nodeIndex + 1;

    // Process children recursively
    for (int i = 0; i < 8; ++i) {
        if (node->children[i]) {
            nextIndex = convertNodeRecursive(node->children[i].get(), nodes, nextIndex);
        }
    }

    return nextIndex;
}

uint32_t GpuCuller::calculateChildMask(const OctreeNode* node) {
    uint32_t mask = 0;
    for (int i = 0; i < 8; ++i) {
        if (node->children[i]) {
            mask |= (1u << i);
        }
    }
    return mask;
}

float GpuCuller::getLastCullingTime() const {
    return m_lastCullingTime;
}

size_t GpuCuller::getGpuMemoryUsage() const {
    return m_gpuMemoryUsage;
}

void GpuCuller::setMaxNodes(uint32_t maxNodes) {
    if (maxNodes != m_maxNodes) {
        m_maxNodes = maxNodes;
        if (m_initialized) {
            // Recreate buffers with new size
            cleanup();
            initialize();
        }
    }
}

void GpuCuller::setOcclusionCullingEnabled(bool enabled) {
    m_occlusionCullingEnabled = enabled;
}
