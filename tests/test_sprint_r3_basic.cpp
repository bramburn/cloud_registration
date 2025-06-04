#include <gtest/gtest.h>
#include <QApplication>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include "../src/pointcloudviewerwidget.h"
#include "../src/pointdata.h"

class SprintR3BasicTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create minimal Qt application for OpenGL context
        if (!QApplication::instance()) {
            int argc = 0;
            char* argv[] = {nullptr};
            app = new QApplication(argc, argv);
        }
        
        // Create OpenGL context for testing
        surface = std::make_unique<QOffscreenSurface>();
        surface->create();
        
        context = std::make_unique<QOpenGLContext>();
        context->create();
        context->makeCurrent(surface.get());
        
        // Create test data
        createTestPointClouds();
    }
    
    void TearDown() override {
        context->doneCurrent();
    }
    
    void createTestPointClouds() {
        // Point cloud with colors only
        for (int i = 0; i < 100; ++i) {
            float x = (i % 10) * 0.1f;
            float y = (i / 10) * 0.1f;
            float z = 0.0f;
            uint8_t r = static_cast<uint8_t>((i * 255) / 100);
            uint8_t g = static_cast<uint8_t>(128);
            uint8_t b = static_cast<uint8_t>(255 - (i * 255) / 100);
            
            colorPoints.emplace_back(x, y, z, r, g, b);
        }
        
        // Point cloud with intensity only
        for (int i = 0; i < 100; ++i) {
            float x = (i % 10) * 0.1f;
            float y = (i / 10) * 0.1f;
            float z = 0.1f;
            float intensity = static_cast<float>(i) / 100.0f;
            
            intensityPoints.emplace_back(x, y, z, intensity);
        }
        
        // Point cloud with both color and intensity
        for (int i = 0; i < 100; ++i) {
            float x = (i % 10) * 0.1f;
            float y = (i / 10) * 0.1f;
            float z = 0.2f;
            uint8_t r = static_cast<uint8_t>((i * 255) / 100);
            uint8_t g = static_cast<uint8_t>(128);
            uint8_t b = static_cast<uint8_t>(255 - (i * 255) / 100);
            float intensity = static_cast<float>(i) / 100.0f;
            
            fullAttributePoints.emplace_back(x, y, z, r, g, b, intensity);
        }
        
        // Point cloud with XYZ only
        for (int i = 0; i < 100; ++i) {
            float x = (i % 10) * 0.1f;
            float y = (i / 10) * 0.1f;
            float z = 0.3f;
            
            xyzOnlyPoints.emplace_back(x, y, z);
        }
    }
    
    QApplication* app = nullptr;
    std::unique_ptr<QOffscreenSurface> surface;
    std::unique_ptr<QOpenGLContext> context;
    
    std::vector<PointFullData> colorPoints;
    std::vector<PointFullData> intensityPoints;
    std::vector<PointFullData> fullAttributePoints;
    std::vector<PointFullData> xyzOnlyPoints;
};

TEST_F(SprintR3BasicTest, PointDataStructure) {
    // Test point data structure functionality
    PointFullData colorPoint(1.0f, 2.0f, 3.0f, 255, 128, 64);
    EXPECT_TRUE(colorPoint.hasColor());
    EXPECT_FALSE(colorPoint.hasIntensity());
    
    PointFullData intensityPoint(1.0f, 2.0f, 3.0f, 0.75f);
    EXPECT_FALSE(intensityPoint.hasColor());
    EXPECT_TRUE(intensityPoint.hasIntensity());
    
    PointFullData fullPoint(1.0f, 2.0f, 3.0f, 255, 128, 64, 0.75f);
    EXPECT_TRUE(fullPoint.hasColor());
    EXPECT_TRUE(fullPoint.hasIntensity());
    
    // Test normalized color extraction
    float r, g, b;
    fullPoint.getNormalizedColor(r, g, b);
    EXPECT_FLOAT_EQ(r, 1.0f);
    EXPECT_FLOAT_EQ(g, 128.0f / 255.0f);
    EXPECT_FLOAT_EQ(b, 64.0f / 255.0f);
}

TEST_F(SprintR3BasicTest, VertexDataConversion) {
    PointFullData point(1.0f, 2.0f, 3.0f, 255, 128, 64, 0.75f);
    VertexData vertex(point);
    
    EXPECT_FLOAT_EQ(vertex.position[0], 1.0f);
    EXPECT_FLOAT_EQ(vertex.position[1], 2.0f);
    EXPECT_FLOAT_EQ(vertex.position[2], 3.0f);
    
    EXPECT_FLOAT_EQ(vertex.color[0], 1.0f);
    EXPECT_FLOAT_EQ(vertex.color[1], 128.0f / 255.0f);
    EXPECT_FLOAT_EQ(vertex.color[2], 64.0f / 255.0f);
    
    EXPECT_FLOAT_EQ(vertex.intensity, 0.75f);
}

TEST_F(SprintR3BasicTest, ViewerSlotFunctionality) {
    PointCloudViewerWidget viewer;
    
    // Test attribute rendering toggles
    viewer.setRenderWithColor(true);
    viewer.setRenderWithColor(false);
    
    viewer.setRenderWithIntensity(true);
    viewer.setRenderWithIntensity(false);
    
    // Test point size attenuation
    viewer.setPointSizeAttenuationEnabled(true);
    viewer.setPointSizeAttenuationParams(1.0f, 10.0f, 0.1f);
    viewer.setPointSizeAttenuationEnabled(false);
    
    // Verify no crashes occur during these operations
    SUCCEED();
}

TEST_F(SprintR3BasicTest, PointCloudDataTypes) {
    PointCloudViewerWidget viewer;
    
    // Test that different point cloud types can be handled
    // Note: These would normally require proper OpenGL setup for full testing
    
    // Test color points
    EXPECT_EQ(colorPoints.size(), 100);
    EXPECT_TRUE(colorPoints[0].hasColor());
    EXPECT_FALSE(colorPoints[0].hasIntensity());
    
    // Test intensity points
    EXPECT_EQ(intensityPoints.size(), 100);
    EXPECT_FALSE(intensityPoints[0].hasColor());
    EXPECT_TRUE(intensityPoints[0].hasIntensity());
    
    // Test full attribute points
    EXPECT_EQ(fullAttributePoints.size(), 100);
    EXPECT_TRUE(fullAttributePoints[0].hasColor());
    EXPECT_TRUE(fullAttributePoints[0].hasIntensity());
    
    // Test XYZ only points
    EXPECT_EQ(xyzOnlyPoints.size(), 100);
    EXPECT_FALSE(xyzOnlyPoints[0].hasColor());
    EXPECT_FALSE(xyzOnlyPoints[0].hasIntensity());
}
