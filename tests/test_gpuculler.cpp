#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFormat>
#include <QGuiApplication>
#include <memory>

#include "../src/rendering/GpuCuller.h"
#include "../src/octree.h"

using ::testing::_;
using ::testing::Return;
using ::testing::AtLeast;

/**
 * @brief Unit tests for GpuCuller class
 * 
 * Tests Sprint 6 requirements:
 * - GPU-based culling functionality
 * - Compute shader integration
 * - Performance characteristics
 * - Large dataset handling
 * 
 * Note: These tests require OpenGL context and compute shader support
 */
class GpuCullerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize Qt application if not already done
        if (!QGuiApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            m_app = std::make_unique<QGuiApplication>(argc, argv);
        }

        // Create OpenGL context for testing
        setupOpenGLContext();
        
        // Create GPU culler instance
        m_gpuCuller = std::make_unique<GpuCuller>();
    }

    void TearDown() override {
        m_gpuCuller.reset();
        
        if (m_context) {
            m_context->doneCurrent();
            m_context.reset();
        }
        
        m_surface.reset();
    }

    void setupOpenGLContext() {
        // Create offscreen surface for testing
        QOpenGLFormat format;
        format.setVersion(4, 3);
        format.setProfile(QOpenGLFormat::CoreProfile);
        
        m_surface = std::make_unique<QOffscreenSurface>();
        m_surface->setFormat(format);
        m_surface->create();
        
        if (!m_surface->isValid()) {
            GTEST_SKIP() << "Cannot create valid OpenGL surface";
            return;
        }
        
        // Create OpenGL context
        m_context = std::make_unique<QOpenGLContext>();
        m_context->setFormat(format);
        
        if (!m_context->create()) {
            GTEST_SKIP() << "Cannot create OpenGL context";
            return;
        }
        
        if (!m_context->makeCurrent(m_surface.get())) {
            GTEST_SKIP() << "Cannot make OpenGL context current";
            return;
        }
        
        // Check for compute shader support
        if (!m_context->hasExtension("GL_ARB_compute_shader")) {
            GTEST_SKIP() << "Compute shaders not supported";
            return;
        }
        
        m_hasValidContext = true;
    }

    std::vector<GpuCuller::CullingNode> createTestNodes(size_t count) {
        std::vector<GpuCuller::CullingNode> nodes;
        nodes.reserve(count);
        
        for (size_t i = 0; i < count; ++i) {
            GpuCuller::CullingNode node;
            
            // Create nodes in a grid pattern
            float x = static_cast<float>(i % 10) * 10.0f;
            float y = static_cast<float>((i / 10) % 10) * 10.0f;
            float z = static_cast<float>(i / 100) * 10.0f;
            
            node.minBounds = QVector3D(x, y, z);
            node.maxBounds = QVector3D(x + 5.0f, y + 5.0f, z + 5.0f);
            node.nodeIndex = static_cast<uint32_t>(i);
            node.pointCount = 1000 + (i % 500); // Varying point counts
            node.childMask = 0; // Leaf nodes for simplicity
            
            nodes.push_back(node);
        }
        
        return nodes;
    }

    GpuCuller::CullingParams createTestParams() {
        GpuCuller::CullingParams params;
        
        // Create a simple view-projection matrix
        QMatrix4x4 view;
        view.lookAt(QVector3D(50, 50, 50), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
        
        QMatrix4x4 projection;
        projection.perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
        
        params.viewProjectionMatrix = projection * view;
        params.cameraPosition = QVector3D(50, 50, 50);
        params.nearPlane = 0.1f;
        params.farPlane = 1000.0f;
        params.screenSpaceErrorThreshold = 1.0f;
        params.viewportWidth = 1920;
        params.viewportHeight = 1080;
        params.maxNodes = 10000;
        
        return params;
    }

protected:
    std::unique_ptr<QGuiApplication> m_app;
    std::unique_ptr<QOffscreenSurface> m_surface;
    std::unique_ptr<QOpenGLContext> m_context;
    std::unique_ptr<GpuCuller> m_gpuCuller;
    bool m_hasValidContext = false;
};

// ============================================================================
// Test Case 1: Initialization and Setup
// ============================================================================

TEST_F(GpuCullerTest, InitializationSuccess) {
    if (!m_hasValidContext) {
        GTEST_SKIP() << "No valid OpenGL context available";
    }
    
    EXPECT_FALSE(m_gpuCuller->isInitialized());
    
    bool initResult = m_gpuCuller->initialize();
    EXPECT_TRUE(initResult);
    EXPECT_TRUE(m_gpuCuller->isInitialized());
    
    // Check initial state
    EXPECT_GT(m_gpuCuller->getGpuMemoryUsage(), 0u);
    EXPECT_EQ(m_gpuCuller->getLastCullingTime(), 0.0f);
}

TEST_F(GpuCullerTest, InitializationWithoutContext) {
    // Test initialization without valid OpenGL context
    m_context->doneCurrent();
    
    bool initResult = m_gpuCuller->initialize();
    EXPECT_FALSE(initResult);
    EXPECT_FALSE(m_gpuCuller->isInitialized());
}

// ============================================================================
// Test Case 2: Octree Data Upload
// ============================================================================

TEST_F(GpuCullerTest, UpdateOctreeDataSuccess) {
    if (!m_hasValidContext) {
        GTEST_SKIP() << "No valid OpenGL context available";
    }
    
    ASSERT_TRUE(m_gpuCuller->initialize());
    
    // Create test nodes
    auto testNodes = createTestNodes(1000);
    
    bool uploadResult = m_gpuCuller->updateOctreeData(testNodes);
    EXPECT_TRUE(uploadResult);
}

TEST_F(GpuCullerTest, UpdateOctreeDataTooManyNodes) {
    if (!m_hasValidContext) {
        GTEST_SKIP() << "No valid OpenGL context available";
    }
    
    ASSERT_TRUE(m_gpuCuller->initialize());
    
    // Set small max nodes limit
    m_gpuCuller->setMaxNodes(100);
    
    // Try to upload more nodes than limit
    auto testNodes = createTestNodes(200);
    
    bool uploadResult = m_gpuCuller->updateOctreeData(testNodes);
    EXPECT_FALSE(uploadResult);
}

TEST_F(GpuCullerTest, UpdateOctreeDataNotInitialized) {
    auto testNodes = createTestNodes(100);
    
    bool uploadResult = m_gpuCuller->updateOctreeData(testNodes);
    EXPECT_FALSE(uploadResult);
}

// ============================================================================
// Test Case 3: Culling Operations
// ============================================================================

TEST_F(GpuCullerTest, PerformCullingBasic) {
    if (!m_hasValidContext) {
        GTEST_SKIP() << "No valid OpenGL context available";
    }
    
    ASSERT_TRUE(m_gpuCuller->initialize());
    
    // Upload test data
    auto testNodes = createTestNodes(100);
    ASSERT_TRUE(m_gpuCuller->updateOctreeData(testNodes));
    
    // Perform culling
    auto params = createTestParams();
    auto result = m_gpuCuller->performCulling(params);
    
    // Verify results
    EXPECT_GE(result.visibleNodeIndices.size(), 0u);
    EXPECT_EQ(result.visibleNodeIndices.size(), result.visiblePointCounts.size());
    EXPECT_GE(result.cullingTimeMs, 0.0f);
    
    // Check that culling time was recorded
    EXPECT_EQ(m_gpuCuller->getLastCullingTime(), result.cullingTimeMs);
}

TEST_F(GpuCullerTest, PerformCullingLargeDataset) {
    if (!m_hasValidContext) {
        GTEST_SKIP() << "No valid OpenGL context available";
    }
    
    ASSERT_TRUE(m_gpuCuller->initialize());
    
    // Create large dataset
    auto testNodes = createTestNodes(10000);
    ASSERT_TRUE(m_gpuCuller->updateOctreeData(testNodes));
    
    // Perform culling
    auto params = createTestParams();
    auto result = m_gpuCuller->performCulling(params);
    
    // Verify performance - should complete within reasonable time
    EXPECT_LT(result.cullingTimeMs, 100.0f) << "GPU culling took too long: " << result.cullingTimeMs << "ms";
    
    // Verify some nodes are visible (camera is positioned to see some nodes)
    EXPECT_GT(result.visibleNodeIndices.size(), 0u);
    EXPECT_GT(result.totalVisiblePoints, 0u);
}

TEST_F(GpuCullerTest, PerformCullingNoData) {
    if (!m_hasValidContext) {
        GTEST_SKIP() << "No valid OpenGL context available";
    }
    
    ASSERT_TRUE(m_gpuCuller->initialize());
    
    // Perform culling without uploading data
    auto params = createTestParams();
    auto result = m_gpuCuller->performCulling(params);
    
    // Should return empty results
    EXPECT_EQ(result.visibleNodeIndices.size(), 0u);
    EXPECT_EQ(result.totalVisiblePoints, 0u);
    EXPECT_EQ(result.cullingTimeMs, 0.0f);
}

// ============================================================================
// Test Case 4: Octree Conversion
// ============================================================================

TEST_F(GpuCullerTest, ConvertOctreeToGpuFormat) {
    // Create a simple octree
    auto octree = std::make_unique<Octree>();
    
    // Add some test points
    std::vector<PointFullData> testPoints;
    for (int i = 0; i < 100; ++i) {
        PointFullData point;
        point.x = static_cast<float>(i % 10);
        point.y = static_cast<float>((i / 10) % 10);
        point.z = static_cast<float>(i / 100);
        point.intensity = 1.0f;
        point.red = point.green = point.blue = 255;
        testPoints.push_back(point);
    }
    
    octree->build(testPoints);
    
    // Convert to GPU format
    auto gpuNodes = GpuCuller::convertOctreeToGpuFormat(octree->root.get());
    
    // Verify conversion
    EXPECT_GT(gpuNodes.size(), 0u);
    
    // Check that root node has reasonable bounds
    if (!gpuNodes.empty()) {
        const auto& rootNode = gpuNodes[0];
        EXPECT_LE(rootNode.minBounds.x(), rootNode.maxBounds.x());
        EXPECT_LE(rootNode.minBounds.y(), rootNode.maxBounds.y());
        EXPECT_LE(rootNode.minBounds.z(), rootNode.maxBounds.z());
        EXPECT_GT(rootNode.pointCount, 0u);
    }
}

TEST_F(GpuCullerTest, ConvertNullOctree) {
    auto gpuNodes = GpuCuller::convertOctreeToGpuFormat(nullptr);
    EXPECT_EQ(gpuNodes.size(), 0u);
}

// ============================================================================
// Test Case 5: Configuration and Settings
// ============================================================================

TEST_F(GpuCullerTest, SetMaxNodes) {
    if (!m_hasValidContext) {
        GTEST_SKIP() << "No valid OpenGL context available";
    }
    
    uint32_t newMaxNodes = 5000;
    m_gpuCuller->setMaxNodes(newMaxNodes);
    
    ASSERT_TRUE(m_gpuCuller->initialize());
    
    // Memory usage should reflect new max nodes
    size_t expectedMinMemory = newMaxNodes * sizeof(GpuCuller::CullingNode);
    EXPECT_GE(m_gpuCuller->getGpuMemoryUsage(), expectedMinMemory);
}

TEST_F(GpuCullerTest, SetOcclusionCullingEnabled) {
    m_gpuCuller->setOcclusionCullingEnabled(true);
    m_gpuCuller->setOcclusionCullingEnabled(false);
    
    // Test passes if no exceptions are thrown
    SUCCEED();
}

// ============================================================================
// Test Case 6: Performance Benchmarks
// ============================================================================

TEST_F(GpuCullerTest, PerformanceBenchmark) {
    if (!m_hasValidContext) {
        GTEST_SKIP() << "No valid OpenGL context available";
    }
    
    ASSERT_TRUE(m_gpuCuller->initialize());
    
    // Test with increasing dataset sizes
    std::vector<size_t> testSizes = {1000, 5000, 10000, 50000};
    
    for (size_t testSize : testSizes) {
        if (testSize > 50000) {
            // Skip very large tests in unit testing
            continue;
        }
        
        auto testNodes = createTestNodes(testSize);
        ASSERT_TRUE(m_gpuCuller->updateOctreeData(testNodes));
        
        auto params = createTestParams();
        auto result = m_gpuCuller->performCulling(params);
        
        // Performance should scale reasonably
        float timePerNode = result.cullingTimeMs / static_cast<float>(testSize);
        EXPECT_LT(timePerNode, 0.01f) << "Performance degraded for " << testSize << " nodes";
        
        qDebug() << "GPU Culling Performance:" << testSize << "nodes in" << result.cullingTimeMs << "ms";
    }
}
