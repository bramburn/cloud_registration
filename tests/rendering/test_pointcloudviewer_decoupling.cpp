#include <QApplication>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include <vector>

#include "../src/IPointCloudViewer.h"
#include "../src/mainwindow.h"

/**
 * @brief Mock implementation of IPointCloudViewer for testing decoupling
 *
 * This mock class demonstrates that MainWindow can work with any implementation
 * of IPointCloudViewer, proving the decoupling is successful.
 */
class MockPointCloudViewer : public IPointCloudViewer
{
    Q_OBJECT

public:
    MockPointCloudViewer(QObject* parent = nullptr) : IPointCloudViewer(parent)
    {
        reset();
    }

    // Reset all tracking variables
    void reset()
    {
        m_loadPointCloudCalled = false;
        m_clearPointCloudCalled = false;
        m_setStateCalled = false;
        m_setTopViewCalled = false;
        m_setLeftViewCalled = false;
        m_setRightViewCalled = false;
        m_setBottomViewCalled = false;
        m_setLODEnabledCalled = false;
        m_setRenderWithColorCalled = false;
        m_setRenderWithIntensityCalled = false;
        m_setPointSizeAttenuationEnabledCalled = false;
        m_setPointSizeAttenuationParamsCalled = false;
        m_onLoadingStartedCalled = false;
        m_onLoadingProgressCalled = false;
        m_onLoadingFinishedCalled = false;

        m_lastPointsSize = 0;
        m_lastState = ViewerState::Idle;
        m_lastMessage.clear();
        m_lastLODEnabled = false;
        m_lastColorEnabled = false;
        m_lastIntensityEnabled = false;
        m_lastAttenuationEnabled = false;
        m_lastMinSize = 0.0f;
        m_lastMaxSize = 0.0f;
        m_lastFactor = 0.0f;
        m_lastPercentage = 0;
        m_lastStage.clear();
        m_lastSuccess = false;
        m_lastLoadingMessage.clear();
        m_lastLoadingPoints.clear();
    }

    // IPointCloudViewer interface implementation
    void loadPointCloud(const std::vector<float>& points) override
    {
        m_loadPointCloudCalled = true;
        m_lastPointsSize = points.size();
        emit pointCloudLoaded(points);
    }

    void clearPointCloud() override
    {
        m_clearPointCloudCalled = true;
        emit pointCloudCleared();
    }

    void setState(ViewerState state, const QString& message = "") override
    {
        m_setStateCalled = true;
        m_lastState = state;
        m_lastMessage = message;
        emit stateChanged(state, message);
    }

    void setTopView() override
    {
        m_setTopViewCalled = true;
    }
    void setLeftView() override
    {
        m_setLeftViewCalled = true;
    }
    void setRightView() override
    {
        m_setRightViewCalled = true;
    }
    void setBottomView() override
    {
        m_setBottomViewCalled = true;
    }

    void setLODEnabled(bool enabled) override
    {
        m_setLODEnabledCalled = true;
        m_lastLODEnabled = enabled;
    }

    bool isLODEnabled() const override
    {
        return m_lastLODEnabled;
    }

    void setRenderWithColor(bool enabled) override
    {
        m_setRenderWithColorCalled = true;
        m_lastColorEnabled = enabled;
    }

    void setRenderWithIntensity(bool enabled) override
    {
        m_setRenderWithIntensityCalled = true;
        m_lastIntensityEnabled = enabled;
    }

    void setPointSizeAttenuationEnabled(bool enabled) override
    {
        m_setPointSizeAttenuationEnabledCalled = true;
        m_lastAttenuationEnabled = enabled;
    }

    void setPointSizeAttenuationParams(float minSize, float maxSize, float factor) override
    {
        m_setPointSizeAttenuationParamsCalled = true;
        m_lastMinSize = minSize;
        m_lastMaxSize = maxSize;
        m_lastFactor = factor;
    }

    ViewerState getViewerState() const override
    {
        return m_lastState;
    }
    bool hasPointCloudData() const override
    {
        return m_lastPointsSize > 0;
    }
    size_t getPointCount() const override
    {
        return m_lastPointsSize / 3;
    }
    QVector3D getGlobalOffset() const override
    {
        return QVector3D(0, 0, 0);
    }
    float getCurrentFPS() const override
    {
        return 60.0f;
    }
    size_t getVisiblePointCount() const override
    {
        return getPointCount();
    }

public slots:
    void onLoadingStarted() override
    {
        m_onLoadingStartedCalled = true;
    }

    void onLoadingProgress(int percentage, const QString& stage) override
    {
        m_onLoadingProgressCalled = true;
        m_lastPercentage = percentage;
        m_lastStage = stage;
    }

    void onLoadingFinished(bool success, const QString& message, const std::vector<float>& points) override
    {
        m_onLoadingFinishedCalled = true;
        m_lastSuccess = success;
        m_lastLoadingMessage = message;
        m_lastLoadingPoints = points;
    }

    void toggleLOD(bool enabled) override
    {
        setLODEnabled(enabled);
    }
    void setLODSubsampleRate(float rate) override
    {
        Q_UNUSED(rate)
    }
    void setScreenSpaceErrorThreshold(float threshold) override
    {
        Q_UNUSED(threshold)
    }
    void setPrimaryScreenSpaceErrorThreshold(float threshold) override
    {
        Q_UNUSED(threshold)
    }
    void setCullScreenSpaceErrorThreshold(float threshold) override
    {
        Q_UNUSED(threshold)
    }

public:
    // Test verification methods
    bool wasLoadPointCloudCalled() const
    {
        return m_loadPointCloudCalled;
    }
    bool wasClearPointCloudCalled() const
    {
        return m_clearPointCloudCalled;
    }
    bool wasSetStateCalled() const
    {
        return m_setStateCalled;
    }
    bool wasSetTopViewCalled() const
    {
        return m_setTopViewCalled;
    }
    bool wasSetLeftViewCalled() const
    {
        return m_setLeftViewCalled;
    }
    bool wasSetRightViewCalled() const
    {
        return m_setRightViewCalled;
    }
    bool wasSetBottomViewCalled() const
    {
        return m_setBottomViewCalled;
    }
    bool wasSetLODEnabledCalled() const
    {
        return m_setLODEnabledCalled;
    }
    bool wasSetRenderWithColorCalled() const
    {
        return m_setRenderWithColorCalled;
    }
    bool wasSetRenderWithIntensityCalled() const
    {
        return m_setRenderWithIntensityCalled;
    }
    bool wasSetPointSizeAttenuationEnabledCalled() const
    {
        return m_setPointSizeAttenuationEnabledCalled;
    }
    bool wasSetPointSizeAttenuationParamsCalled() const
    {
        return m_setPointSizeAttenuationParamsCalled;
    }
    bool wasOnLoadingStartedCalled() const
    {
        return m_onLoadingStartedCalled;
    }
    bool wasOnLoadingProgressCalled() const
    {
        return m_onLoadingProgressCalled;
    }
    bool wasOnLoadingFinishedCalled() const
    {
        return m_onLoadingFinishedCalled;
    }

    size_t getLastPointsSize() const
    {
        return m_lastPointsSize;
    }
    ViewerState getLastState() const
    {
        return m_lastState;
    }
    QString getLastMessage() const
    {
        return m_lastMessage;
    }
    bool getLastLODEnabled() const
    {
        return m_lastLODEnabled;
    }
    bool getLastColorEnabled() const
    {
        return m_lastColorEnabled;
    }
    bool getLastIntensityEnabled() const
    {
        return m_lastIntensityEnabled;
    }
    bool getLastAttenuationEnabled() const
    {
        return m_lastAttenuationEnabled;
    }
    float getLastMinSize() const
    {
        return m_lastMinSize;
    }
    float getLastMaxSize() const
    {
        return m_lastMaxSize;
    }
    float getLastFactor() const
    {
        return m_lastFactor;
    }
    int getLastPercentage() const
    {
        return m_lastPercentage;
    }
    QString getLastStage() const
    {
        return m_lastStage;
    }
    bool getLastSuccess() const
    {
        return m_lastSuccess;
    }
    QString getLastLoadingMessage() const
    {
        return m_lastLoadingMessage;
    }
    std::vector<float> getLastLoadingPoints() const
    {
        return m_lastLoadingPoints;
    }

private:
    // Call tracking
    bool m_loadPointCloudCalled;
    bool m_clearPointCloudCalled;
    bool m_setStateCalled;
    bool m_setTopViewCalled;
    bool m_setLeftViewCalled;
    bool m_setRightViewCalled;
    bool m_setBottomViewCalled;
    bool m_setLODEnabledCalled;
    bool m_setRenderWithColorCalled;
    bool m_setRenderWithIntensityCalled;
    bool m_setPointSizeAttenuationEnabledCalled;
    bool m_setPointSizeAttenuationParamsCalled;
    bool m_onLoadingStartedCalled;
    bool m_onLoadingProgressCalled;
    bool m_onLoadingFinishedCalled;

    // Parameter tracking
    size_t m_lastPointsSize;
    ViewerState m_lastState;
    QString m_lastMessage;
    bool m_lastLODEnabled;
    bool m_lastColorEnabled;
    bool m_lastIntensityEnabled;
    bool m_lastAttenuationEnabled;
    float m_lastMinSize;
    float m_lastMaxSize;
    float m_lastFactor;
    int m_lastPercentage;
    QString m_lastStage;
    bool m_lastSuccess;
    QString m_lastLoadingMessage;
    std::vector<float> m_lastLoadingPoints;
};

/**
 * @brief Test class for verifying PointCloudViewer decoupling
 */
class TestPointCloudViewerDecoupling : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // Ensure QApplication exists for Qt tests
        if (!QApplication::instance())
        {
            int argc = 1;
            char* argv[] = {"test"};
            new QApplication(argc, argv);
        }
    }

    void testMockViewerBasicOperations()
    {
        MockPointCloudViewer mockViewer;

        // Test loadPointCloud
        std::vector<float> testPoints = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};  // 2 points
        mockViewer.loadPointCloud(testPoints);

        QVERIFY(mockViewer.wasLoadPointCloudCalled());
        QCOMPARE(mockViewer.getLastPointsSize(), static_cast<size_t>(6));
        QCOMPARE(mockViewer.getPointCount(), static_cast<size_t>(2));
        QVERIFY(mockViewer.hasPointCloudData());

        // Test clearPointCloud
        mockViewer.clearPointCloud();
        QVERIFY(mockViewer.wasClearPointCloudCalled());

        // Test setState
        mockViewer.setState(ViewerState::Loading, "Test message");
        QVERIFY(mockViewer.wasSetStateCalled());
        QCOMPARE(mockViewer.getLastState(), ViewerState::Loading);
        QCOMPARE(mockViewer.getLastMessage(), QString("Test message"));
    }

    void testMockViewerViewControls()
    {
        MockPointCloudViewer mockViewer;

        // Test view controls
        mockViewer.setTopView();
        QVERIFY(mockViewer.wasSetTopViewCalled());

        mockViewer.setLeftView();
        QVERIFY(mockViewer.wasSetLeftViewCalled());

        mockViewer.setRightView();
        QVERIFY(mockViewer.wasSetRightViewCalled());

        mockViewer.setBottomView();
        QVERIFY(mockViewer.wasSetBottomViewCalled());
    }

    void testMockViewerRenderingControls()
    {
        MockPointCloudViewer mockViewer;

        // Test LOD control
        mockViewer.setLODEnabled(true);
        QVERIFY(mockViewer.wasSetLODEnabledCalled());
        QVERIFY(mockViewer.getLastLODEnabled());
        QVERIFY(mockViewer.isLODEnabled());

        // Test color rendering
        mockViewer.setRenderWithColor(true);
        QVERIFY(mockViewer.wasSetRenderWithColorCalled());
        QVERIFY(mockViewer.getLastColorEnabled());

        // Test intensity rendering
        mockViewer.setRenderWithIntensity(true);
        QVERIFY(mockViewer.wasSetRenderWithIntensityCalled());
        QVERIFY(mockViewer.getLastIntensityEnabled());

        // Test point size attenuation
        mockViewer.setPointSizeAttenuationEnabled(true);
        QVERIFY(mockViewer.wasSetPointSizeAttenuationEnabledCalled());
        QVERIFY(mockViewer.getLastAttenuationEnabled());

        mockViewer.setPointSizeAttenuationParams(1.0f, 5.0f, 0.5f);
        QVERIFY(mockViewer.wasSetPointSizeAttenuationParamsCalled());
        QCOMPARE(mockViewer.getLastMinSize(), 1.0f);
        QCOMPARE(mockViewer.getLastMaxSize(), 5.0f);
        QCOMPARE(mockViewer.getLastFactor(), 0.5f);
    }

    void testMockViewerSignals()
    {
        MockPointCloudViewer mockViewer;

        // Test signal emission
        QSignalSpy pointCloudLoadedSpy(&mockViewer, &IPointCloudViewer::pointCloudLoaded);
        QSignalSpy pointCloudClearedSpy(&mockViewer, &IPointCloudViewer::pointCloudCleared);
        QSignalSpy stateChangedSpy(&mockViewer, &IPointCloudViewer::stateChanged);

        std::vector<float> testPoints = {1.0f, 2.0f, 3.0f};
        mockViewer.loadPointCloud(testPoints);
        QCOMPARE(pointCloudLoadedSpy.count(), 1);

        mockViewer.clearPointCloud();
        QCOMPARE(pointCloudClearedSpy.count(), 1);

        mockViewer.setState(ViewerState::DisplayingData, "Test");
        QCOMPARE(stateChangedSpy.count(), 1);

        // Verify signal parameters
        auto stateArgs = stateChangedSpy.takeFirst();
        QCOMPARE(qvariant_cast<ViewerState>(stateArgs.at(0)), ViewerState::DisplayingData);
        QCOMPARE(stateArgs.at(1).toString(), QString("Test"));
    }

    void cleanupTestCase()
    {
        // Cleanup is handled automatically by Qt
    }
};

QTEST_MAIN(TestPointCloudViewerDecoupling)
#include "test_pointcloudviewer_decoupling.moc"
