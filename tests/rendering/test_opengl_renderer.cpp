#include <gtest/gtest.h>
#include <QApplication>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFunctions>
#include "rendering/OpenGLRenderer.h"

class OpenGLRendererTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create minimal Qt application if not already created
        if (!QApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app = new QApplication(argc, argv);
        }

        // Create OpenGL context for testing
        surface = new QOffscreenSurface();
        surface->create();
        
        context = new QOpenGLContext();
        context->create();
        context->makeCurrent(surface);

        renderer = new OpenGLRenderer();
    }

    void TearDown() override
    {
        delete renderer;
        context->doneCurrent();
        delete context;
        delete surface;
        // Note: Don't delete app as it might be shared
    }

    QApplication* app = nullptr;
    QOffscreenSurface* surface = nullptr;
    QOpenGLContext* context = nullptr;
    OpenGLRenderer* renderer = nullptr;
};

TEST_F(OpenGLRendererTest, InitializationTest)
{
    EXPECT_FALSE(renderer->isInitialized());
    EXPECT_FALSE(renderer->areShadersReady());
    
    bool result = renderer->initialize();
    EXPECT_TRUE(result);
    EXPECT_TRUE(renderer->isInitialized());
    EXPECT_EQ(renderer->getPointCount(), 0);
}

TEST_F(OpenGLRendererTest, ShaderLoadingTest)
{
    ASSERT_TRUE(renderer->initialize());
    
    // Test loading valid shaders
    bool result = renderer->loadShaders("shaders/pointcloud.vert", "shaders/pointcloud.frag");
    EXPECT_TRUE(result);
    EXPECT_TRUE(renderer->areShadersReady());
    
    if (!result) {
        qDebug() << "Shader loading failed:" << renderer->getLastError();
    }
}

TEST_F(OpenGLRendererTest, ShaderLoadingFailureTest)
{
    ASSERT_TRUE(renderer->initialize());
    
    // Test loading non-existent shaders
    bool result = renderer->loadShaders("nonexistent.vert", "nonexistent.frag");
    EXPECT_FALSE(result);
    EXPECT_FALSE(renderer->areShadersReady());
    EXPECT_FALSE(renderer->getLastError().isEmpty());
}

TEST_F(OpenGLRendererTest, PointDataUploadTest)
{
    ASSERT_TRUE(renderer->initialize());
    ASSERT_TRUE(renderer->loadShaders("shaders/pointcloud.vert", "shaders/pointcloud.frag"));
    
    // Test uploading valid point data
    std::vector<float> points = {
        0.0f, 0.0f, 0.0f,  // Point 1
        1.0f, 0.0f, 0.0f,  // Point 2
        0.0f, 1.0f, 0.0f   // Point 3
    };
    
    bool result = renderer->uploadPointData(points);
    EXPECT_TRUE(result);
    EXPECT_EQ(renderer->getPointCount(), 3);
}

TEST_F(OpenGLRendererTest, InvalidPointDataTest)
{
    ASSERT_TRUE(renderer->initialize());
    ASSERT_TRUE(renderer->loadShaders("shaders/pointcloud.vert", "shaders/pointcloud.frag"));
    
    // Test uploading invalid point data (not multiple of 3)
    std::vector<float> invalidPoints = {0.0f, 0.0f}; // Only 2 coordinates
    
    bool result = renderer->uploadPointData(invalidPoints);
    EXPECT_FALSE(result);
    EXPECT_EQ(renderer->getPointCount(), 0);
    EXPECT_FALSE(renderer->getLastError().isEmpty());
}

TEST_F(OpenGLRendererTest, EmptyPointDataTest)
{
    ASSERT_TRUE(renderer->initialize());
    ASSERT_TRUE(renderer->loadShaders("shaders/pointcloud.vert", "shaders/pointcloud.frag"));
    
    // Test uploading empty point data
    std::vector<float> emptyPoints;
    
    bool result = renderer->uploadPointData(emptyPoints);
    EXPECT_FALSE(result);
    EXPECT_EQ(renderer->getPointCount(), 0);
}

TEST_F(OpenGLRendererTest, RenderingTest)
{
    ASSERT_TRUE(renderer->initialize());
    ASSERT_TRUE(renderer->loadShaders("shaders/pointcloud.vert", "shaders/pointcloud.frag"));
    
    // Upload test data
    std::vector<float> points = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };
    ASSERT_TRUE(renderer->uploadPointData(points));
    
    // Test rendering (should not crash)
    QMatrix4x4 mvpMatrix;
    mvpMatrix.setToIdentity();
    QVector3D color(1.0f, 1.0f, 1.0f);
    float pointSize = 2.0f;
    
    // This should not crash or produce OpenGL errors
    renderer->render(mvpMatrix, color, pointSize);
    
    // Check for OpenGL errors
    QOpenGLFunctions* gl = context->functions();
    GLenum error = gl->glGetError();
    EXPECT_EQ(error, GL_NO_ERROR) << "OpenGL error during rendering: " << error;
}

TEST_F(OpenGLRendererTest, ClearDataTest)
{
    ASSERT_TRUE(renderer->initialize());
    ASSERT_TRUE(renderer->loadShaders("shaders/pointcloud.vert", "shaders/pointcloud.frag"));
    
    // Upload test data
    std::vector<float> points = {0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
    ASSERT_TRUE(renderer->uploadPointData(points));
    EXPECT_EQ(renderer->getPointCount(), 2);
    
    // Clear data
    renderer->clearData();
    EXPECT_EQ(renderer->getPointCount(), 0);
}

TEST_F(OpenGLRendererTest, LargeDatasetTest)
{
    ASSERT_TRUE(renderer->initialize());
    ASSERT_TRUE(renderer->loadShaders("shaders/pointcloud.vert", "shaders/pointcloud.frag"));
    
    // Create a larger dataset (10,000 points)
    std::vector<float> points;
    points.reserve(30000); // 10,000 points * 3 coordinates
    
    for (int i = 0; i < 10000; ++i) {
        points.push_back(static_cast<float>(i % 100));
        points.push_back(static_cast<float>((i / 100) % 100));
        points.push_back(static_cast<float>(i / 10000));
    }
    
    bool result = renderer->uploadPointData(points);
    EXPECT_TRUE(result);
    EXPECT_EQ(renderer->getPointCount(), 10000);
    
    // Test rendering large dataset
    QMatrix4x4 mvpMatrix;
    mvpMatrix.setToIdentity();
    QVector3D color(1.0f, 1.0f, 1.0f);
    float pointSize = 1.0f;
    
    renderer->render(mvpMatrix, color, pointSize);
    
    // Check for OpenGL errors
    QOpenGLFunctions* gl = context->functions();
    GLenum error = gl->glGetError();
    EXPECT_EQ(error, GL_NO_ERROR) << "OpenGL error during large dataset rendering: " << error;
}

// Performance benchmark test
TEST_F(OpenGLRendererTest, PerformanceBenchmark)
{
    ASSERT_TRUE(renderer->initialize());
    ASSERT_TRUE(renderer->loadShaders("shaders/pointcloud.vert", "shaders/pointcloud.frag"));
    
    // Create 1 million points
    const int numPoints = 1000000;
    std::vector<float> points;
    points.reserve(numPoints * 3);
    
    for (int i = 0; i < numPoints; ++i) {
        points.push_back(static_cast<float>(rand()) / RAND_MAX * 100.0f);
        points.push_back(static_cast<float>(rand()) / RAND_MAX * 100.0f);
        points.push_back(static_cast<float>(rand()) / RAND_MAX * 100.0f);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    bool result = renderer->uploadPointData(points);
    auto end = std::chrono::high_resolution_clock::now();
    
    EXPECT_TRUE(result);
    EXPECT_EQ(renderer->getPointCount(), numPoints);
    
    auto uploadTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    qDebug() << "Upload time for" << numPoints << "points:" << uploadTime.count() << "ms";
    
    // Upload should complete within reasonable time (less than 5 seconds)
    EXPECT_LT(uploadTime.count(), 5000);
}
