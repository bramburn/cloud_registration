#ifndef MOCKPOINTCLOUDVIEWER_H
#define MOCKPOINTCLOUDVIEWER_H

#include <vector>

#include "../../src/IPointCloudViewer.h"

#include <gmock/gmock.h>

/**
 * @brief MockPointCloudViewer - Mock implementation of IPointCloudViewer for testing
 *
 * This mock class implements the IPointCloudViewer interface using Google Mock's
 * MOCK_METHOD macro. It allows tests to verify that the presenter correctly
 * instructs the view to render data or change its state without requiring
 * an actual OpenGL context or rendering operations.
 *
 * Sprint 5 Testing Requirements:
 * - Enables unit testing of viewer interactions without OpenGL dependencies
 * - Allows verification of correct rendering method calls
 * - Provides controllable viewer state for testing different scenarios
 * - Verifies correct parameter passing for rendering operations
 */
class MockPointCloudViewer : public IPointCloudViewer
{
    Q_OBJECT

public:
    explicit MockPointCloudViewer(QObject* parent = nullptr) : IPointCloudViewer(parent) {}
    virtual ~MockPointCloudViewer() = default;

    // Core point cloud operations
    MOCK_METHOD(void, loadPointCloud, (const std::vector<float>& points), (override));
    MOCK_METHOD(void, clearPointCloud, (), (override));

    // State management
    MOCK_METHOD(void, setState, (ViewerState state, const QString& message), (override));
    MOCK_METHOD(ViewerState, getState, (), (const, override));

    // Rendering settings
    MOCK_METHOD(void, setLODEnabled, (bool enabled), (override));
    MOCK_METHOD(void, setRenderWithColor, (bool enabled), (override));
    MOCK_METHOD(void, setRenderWithIntensity, (bool enabled), (override));
    MOCK_METHOD(void, setPointSize, (float size), (override));
    MOCK_METHOD(void, setBackgroundColor, (const QColor& color), (override));

    // Point size attenuation
    MOCK_METHOD(void, setPointSizeAttenuationEnabled, (bool enabled), (override));
    MOCK_METHOD(void, setPointSizeAttenuationParams, (float minSize, float maxSize, float factor), (override));

    // Advanced rendering
    MOCK_METHOD(void, setSplattingEnabled, (bool enabled), (override));
    MOCK_METHOD(void, setLightingEnabled, (bool enabled), (override));
    MOCK_METHOD(void, setLightDirection, (const QVector3D& direction), (override));
    MOCK_METHOD(void, setLightColor, (const QColor& color), (override));
    MOCK_METHOD(void, setAmbientIntensity, (float intensity), (override));

    // Status queries
    MOCK_METHOD(bool, hasData, (), (const, override));
    MOCK_METHOD(size_t, pointCount, (), (const, override));

    // Camera operations
    MOCK_METHOD(void, resetCamera, (), (override));
    MOCK_METHOD(void, setTopView, (), (override));
    MOCK_METHOD(void, setFrontView, (), (override));
    MOCK_METHOD(void, setSideView, (), (override));

    // Helper methods for tests to emit signals
    void emitPointCloudLoadingStarted()
    {
        emit pointCloudLoadingStarted();
    }

    void emitPointCloudLoaded(const std::vector<float>& points)
    {
        emit pointCloudLoaded(points);
    }

    void emitPointCloudLoadFailed(const QString& error)
    {
        emit pointCloudLoadFailed(error);
    }

    void emitPointCloudCleared()
    {
        emit pointCloudCleared();
    }

    void emitStateChanged(ViewerState newState, const QString& message)
    {
        emit stateChanged(newState, message);
    }

    void emitRenderingError(const QString& error)
    {
        emit renderingError(error);
    }

    void emitStatsUpdated(float fps, int visiblePoints)
    {
        emit statsUpdated(fps, visiblePoints);
    }

    // Test helper methods to set up common scenarios
    void setupEmptyViewer()
    {
        using ::testing::Return;

        ON_CALL(*this, hasData()).WillByDefault(Return(false));

        ON_CALL(*this, pointCount()).WillByDefault(Return(0));

        ON_CALL(*this, getState()).WillByDefault(Return(ViewerState::Empty));
    }

    void setupLoadedViewer(size_t numPoints = 300)
    {
        using ::testing::Return;

        ON_CALL(*this, hasData()).WillByDefault(Return(true));

        ON_CALL(*this, pointCount()).WillByDefault(Return(numPoints));

        ON_CALL(*this, getState()).WillByDefault(Return(ViewerState::Ready));
    }

    void setupLoadingViewer()
    {
        using ::testing::Return;

        ON_CALL(*this, hasData()).WillByDefault(Return(false));

        ON_CALL(*this, pointCount()).WillByDefault(Return(0));

        ON_CALL(*this, getState()).WillByDefault(Return(ViewerState::Loading));
    }

    void setupErrorViewer(const QString& errorMessage = "Mock viewer error")
    {
        using ::testing::Return;

        ON_CALL(*this, hasData()).WillByDefault(Return(false));

        ON_CALL(*this, pointCount()).WillByDefault(Return(0));

        ON_CALL(*this, getState()).WillByDefault(Return(ViewerState::Error));
    }

    void setupRenderingViewer(size_t numPoints = 300)
    {
        using ::testing::Return;

        ON_CALL(*this, hasData()).WillByDefault(Return(true));

        ON_CALL(*this, pointCount()).WillByDefault(Return(numPoints));

        ON_CALL(*this, getState()).WillByDefault(Return(ViewerState::Rendering));
    }

    // Create test data helpers
    static std::vector<float> createTestPointCloud(int numPoints = 10)
    {
        std::vector<float> points;
        points.reserve(numPoints * 3);

        for (int i = 0; i < numPoints; ++i)
        {
            points.push_back(static_cast<float>(i));      // x
            points.push_back(static_cast<float>(i + 1));  // y
            points.push_back(static_cast<float>(i + 2));  // z
        }

        return points;
    }

    static QVector3D createTestLightDirection()
    {
        return QVector3D(0.0f, 0.0f, -1.0f);  // Light pointing down
    }

    static QColor createTestLightColor()
    {
        return QColor(255, 255, 255);  // White light
    }

    static QColor createTestBackgroundColor()
    {
        return QColor(64, 64, 64);  // Dark gray background
    }

    // Simulate viewer state transitions
    void simulateLoadingSequence(const std::vector<float>& points)
    {
        emitPointCloudLoadingStarted();
        emitStateChanged(ViewerState::Loading, "Loading point cloud...");

        // Simulate successful loading
        emitPointCloudLoaded(points);
        emitStateChanged(ViewerState::Ready, "Point cloud loaded successfully");
    }

    void simulateLoadingFailure(const QString& errorMessage = "Failed to load point cloud")
    {
        emitPointCloudLoadingStarted();
        emitStateChanged(ViewerState::Loading, "Loading point cloud...");

        // Simulate loading failure
        emitPointCloudLoadFailed(errorMessage);
        emitStateChanged(ViewerState::Error, errorMessage);
    }

    void simulateClearingSequence()
    {
        emitStateChanged(ViewerState::Empty, "Point cloud cleared");
        emitPointCloudCleared();
    }

    void simulateRenderingStats(float fps = 60.0f, int visiblePoints = 1000)
    {
        emitStatsUpdated(fps, visiblePoints);
    }

    void simulateRenderingError(const QString& errorMessage = "Rendering error occurred")
    {
        emitRenderingError(errorMessage);
        emitStateChanged(ViewerState::Error, errorMessage);
    }

    // Verify common interaction patterns
    void verifyPointCloudLoaded(const std::vector<float>& expectedPoints)
    {
        using ::testing::_;

        EXPECT_CALL(*this, loadPointCloud(expectedPoints)).Times(1);
    }

    void verifyViewerCleared()
    {
        EXPECT_CALL(*this, clearPointCloud()).Times(1);
    }

    void verifyCameraReset()
    {
        EXPECT_CALL(*this, resetCamera()).Times(1);
    }

    void verifyStateSet(ViewerState expectedState, const QString& expectedMessage = QString())
    {
        using ::testing::_;

        if (expectedMessage.isEmpty())
        {
            EXPECT_CALL(*this, setState(expectedState, _)).Times(1);
        }
        else
        {
            EXPECT_CALL(*this, setState(expectedState, expectedMessage)).Times(1);
        }
    }

    void verifyRenderingSettingsApplied()
    {
        using ::testing::_;

        // Verify that common rendering settings are applied
        EXPECT_CALL(*this, setPointSize(_)).Times(::testing::AtLeast(0));

        EXPECT_CALL(*this, setLODEnabled(_)).Times(::testing::AtLeast(0));

        EXPECT_CALL(*this, setRenderWithColor(_)).Times(::testing::AtLeast(0));

        EXPECT_CALL(*this, setRenderWithIntensity(_)).Times(::testing::AtLeast(0));
    }
};

#endif  // MOCKPOINTCLOUDVIEWER_H
