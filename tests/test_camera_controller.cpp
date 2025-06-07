#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <cmath>
#include "../src/camera/CameraController.h"

class CameraControllerTest : public ::testing::Test
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

        camera = new CameraController();
    }

    void TearDown() override
    {
        delete camera;
        // Note: Don't delete app as it might be shared
    }

    QApplication* app = nullptr;
    CameraController* camera = nullptr;
};

TEST_F(CameraControllerTest, InitializationTest)
{
    // Test default initialization values
    EXPECT_EQ(camera->getCameraPosition(), QVector3D(0.0f, 0.0f, 5.0f));
    EXPECT_EQ(camera->getCameraTarget(), QVector3D(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(camera->getCameraUp(), QVector3D(0.0f, 1.0f, 0.0f));
    EXPECT_FLOAT_EQ(camera->getFieldOfView(), 45.0f);
    EXPECT_FLOAT_EQ(camera->getDistance(), 5.0f);
}

TEST_F(CameraControllerTest, ViewMatrixTest)
{
    QMatrix4x4 viewMatrix = camera->getViewMatrix();
    
    // View matrix should not be identity (since camera is not at origin looking down -Z)
    EXPECT_NE(viewMatrix, QMatrix4x4());
    
    // View matrix should be invertible
    bool invertible = false;
    QMatrix4x4 inverse = viewMatrix.inverted(&invertible);
    EXPECT_TRUE(invertible);
}

TEST_F(CameraControllerTest, ProjectionMatrixTest)
{
    float aspectRatio = 16.0f / 9.0f;
    QMatrix4x4 projMatrix = camera->getProjectionMatrix(aspectRatio);
    
    // Projection matrix should not be identity
    EXPECT_NE(projMatrix, QMatrix4x4());
    
    // Projection matrix should be invertible
    bool invertible = false;
    QMatrix4x4 inverse = projMatrix.inverted(&invertible);
    EXPECT_TRUE(invertible);
}

TEST_F(CameraControllerTest, OrbitTest)
{
    QSignalSpy spy(camera, &CameraController::cameraChanged);
    
    QVector3D initialPosition = camera->getCameraPosition();
    
    // Perform orbit operation
    camera->orbit(45.0f, 30.0f);
    
    // Camera position should have changed
    QVector3D newPosition = camera->getCameraPosition();
    EXPECT_NE(initialPosition, newPosition);
    
    // Signal should have been emitted
    EXPECT_EQ(spy.count(), 1);
    
    // Target should remain the same
    EXPECT_EQ(camera->getCameraTarget(), QVector3D(0.0f, 0.0f, 0.0f));
}

TEST_F(CameraControllerTest, PanTest)
{
    QSignalSpy spy(camera, &CameraController::cameraChanged);
    
    QVector3D initialPosition = camera->getCameraPosition();
    QVector3D initialTarget = camera->getCameraTarget();
    
    // Perform pan operation
    camera->pan(1.0f, 1.0f);
    
    // Both position and target should have changed
    EXPECT_NE(camera->getCameraPosition(), initialPosition);
    EXPECT_NE(camera->getCameraTarget(), initialTarget);
    
    // Signal should have been emitted
    EXPECT_EQ(spy.count(), 1);
    
    // Distance between camera and target should remain the same
    float initialDistance = (initialTarget - initialPosition).length();
    float newDistance = (camera->getCameraTarget() - camera->getCameraPosition()).length();
    EXPECT_NEAR(initialDistance, newDistance, 0.001f);
}

TEST_F(CameraControllerTest, ZoomTest)
{
    QSignalSpy spy(camera, &CameraController::cameraChanged);
    
    float initialDistance = camera->getDistance();
    
    // Zoom in
    camera->zoom(1.0f);
    
    // Distance should have decreased
    EXPECT_LT(camera->getDistance(), initialDistance);
    
    // Signal should have been emitted
    EXPECT_EQ(spy.count(), 1);
    
    // Target should remain the same
    EXPECT_EQ(camera->getCameraTarget(), QVector3D(0.0f, 0.0f, 0.0f));
}

TEST_F(CameraControllerTest, ZoomConstraintsTest)
{
    // Set zoom constraints
    camera->setZoomConstraints(1.0f, 10.0f);
    
    // Try to zoom beyond minimum
    camera->setDistance(0.5f);
    EXPECT_GE(camera->getDistance(), 1.0f);
    
    // Try to zoom beyond maximum
    camera->setDistance(15.0f);
    EXPECT_LE(camera->getDistance(), 10.0f);
}

TEST_F(CameraControllerTest, FitToViewTest)
{
    QSignalSpy spy(camera, &CameraController::cameraChanged);
    
    QVector3D minBounds(-10.0f, -5.0f, -8.0f);
    QVector3D maxBounds(10.0f, 5.0f, 8.0f);
    
    camera->fitToView(minBounds, maxBounds);
    
    // Target should be at the center of the bounding box
    QVector3D expectedCenter = (minBounds + maxBounds) * 0.5f;
    EXPECT_EQ(camera->getCameraTarget(), expectedCenter);
    
    // Distance should be appropriate for the bounding box size
    EXPECT_GT(camera->getDistance(), 0.0f);
    
    // Signal should have been emitted
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(CameraControllerTest, ViewPresetsTest)
{
    QSignalSpy spy(camera, &CameraController::cameraChanged);
    
    // Test top view
    camera->setTopView();
    EXPECT_EQ(spy.count(), 1);
    
    // Test front view
    camera->setFrontView();
    EXPECT_EQ(spy.count(), 2);
    
    // Test side view
    camera->setSideView();
    EXPECT_EQ(spy.count(), 3);
    
    // Test isometric view
    camera->setIsometricView();
    EXPECT_EQ(spy.count(), 4);
    
    // Each view should produce different camera positions
    // (We don't test exact positions as they depend on internal implementation)
}

TEST_F(CameraControllerTest, ResetTest)
{
    QSignalSpy spy(camera, &CameraController::cameraChanged);
    
    // Modify camera state
    camera->orbit(45.0f, 30.0f);
    camera->pan(2.0f, 2.0f);
    camera->zoom(2.0f);
    
    // Reset camera
    camera->reset();
    
    // Camera should be back to initial state
    EXPECT_EQ(camera->getCameraPosition(), QVector3D(0.0f, 0.0f, 5.0f));
    EXPECT_EQ(camera->getCameraTarget(), QVector3D(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(camera->getCameraUp(), QVector3D(0.0f, 1.0f, 0.0f));
    EXPECT_FLOAT_EQ(camera->getDistance(), 5.0f);
    
    // Signal should have been emitted
    EXPECT_GT(spy.count(), 0);
}

TEST_F(CameraControllerTest, FieldOfViewTest)
{
    QSignalSpy spy(camera, &CameraController::cameraChanged);
    
    // Test setting valid FOV
    camera->setFieldOfView(60.0f);
    EXPECT_FLOAT_EQ(camera->getFieldOfView(), 60.0f);
    EXPECT_EQ(spy.count(), 1);
    
    // Test FOV constraints (should be clamped to valid range)
    camera->setFieldOfView(5.0f); // Too small
    EXPECT_GE(camera->getFieldOfView(), 10.0f);
    
    camera->setFieldOfView(150.0f); // Too large
    EXPECT_LE(camera->getFieldOfView(), 120.0f);
}

TEST_F(CameraControllerTest, SensitivityTest)
{
    // Test setting sensitivity values
    camera->setPanSensitivity(0.02f);
    camera->setOrbitSensitivity(1.0f);
    camera->setZoomSensitivity(0.2f);
    
    // Perform operations and verify they work with new sensitivity
    QVector3D initialPos = camera->getCameraPosition();
    camera->orbit(10.0f, 10.0f);
    EXPECT_NE(camera->getCameraPosition(), initialPos);
    
    QVector3D initialTarget = camera->getCameraTarget();
    camera->pan(1.0f, 1.0f);
    EXPECT_NE(camera->getCameraTarget(), initialTarget);
    
    float initialDistance = camera->getDistance();
    camera->zoom(1.0f);
    EXPECT_NE(camera->getDistance(), initialDistance);
}

TEST_F(CameraControllerTest, MatrixConsistencyTest)
{
    // Test that view matrix is consistent with camera parameters
    camera->orbit(30.0f, 45.0f);
    
    QMatrix4x4 viewMatrix = camera->getViewMatrix();
    
    // The view matrix should transform the camera position to origin
    QVector3D cameraPos = camera->getCameraPosition();
    QVector3D target = camera->getCameraTarget();
    QVector3D up = camera->getCameraUp();
    
    // Create expected view matrix manually
    QMatrix4x4 expectedView;
    expectedView.lookAt(cameraPos, target, up);
    
    // Matrices should be very close (allowing for floating point precision)
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            EXPECT_NEAR(viewMatrix(i, j), expectedView(i, j), 0.0001f);
        }
    }
}

TEST_F(CameraControllerTest, ContinuousOrbitTest)
{
    // Test that continuous orbit operations work correctly
    for (int i = 0; i < 36; ++i) {
        camera->orbit(10.0f, 0.0f); // 360 degrees total
    }
    
    // Camera should still be functional
    QMatrix4x4 viewMatrix = camera->getViewMatrix();
    bool invertible = false;
    viewMatrix.inverted(&invertible);
    EXPECT_TRUE(invertible);
    
    // Distance should remain constant
    EXPECT_NEAR(camera->getDistance(), 5.0f, 0.1f);
}
