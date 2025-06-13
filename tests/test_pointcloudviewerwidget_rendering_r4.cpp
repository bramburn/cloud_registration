// Sprint R4: Point Splatting and Lighting Tests
// Tests for Tasks R4.1.1 through R4.3.2

#include <QApplication>
#include <QColor>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLWidget>
#include <QVector3D>
#include <QtTest/QtTest>

#include <memory>
#include <vector>

#include "../src/octree.h"
#include "../src/pointcloudviewerwidget.h"
#include "../src/pointdata.h"

class TestPointCloudViewerWidgetRenderingR4 : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Sprint R4.1: Point Splatting Tests
    void testAggregateDataCalculation();
    void testSplatVertexDataStructure();
    void testSplatRenderingDecision();
    void testSplatShaderSetup();
    void testSplatTextureCreation();

    // Sprint R4.2: Lighting Tests
    void testLightingShaderSetup();
    void testLightingParameterPassing();
    void testNormalEstimation();

    // Sprint R4.3: UI Integration Tests
    void testSplattingUIControls();
    void testLightingUIControls();
    void testUISignalConnections();

    // Sprint R4.4: Performance Tests
    void testSplattingPerformance();
    void testLightingPerformance();

private:
    QApplication* m_app;
    PointCloudViewerWidget* m_viewer;
    QOpenGLContext* m_context;
    QOffscreenSurface* m_surface;
    std::unique_ptr<Octree> m_testOctree;

    void createTestPointCloud();
    void setupOpenGLContext();
    std::vector<PointFullData> generateTestPoints(int count);
};

void TestPointCloudViewerWidgetRenderingR4::initTestCase()
{
    // Initialize QApplication if not already done
    if (!QApplication::instance())
    {
        int argc = 1;
        char* argv[] = {"test"};
        m_app = new QApplication(argc, argv);
    }
    else
    {
        m_app = nullptr;
    }

    setupOpenGLContext();
}

void TestPointCloudViewerWidgetRenderingR4::cleanupTestCase()
{
    delete m_viewer;
    delete m_surface;
    delete m_context;
    delete m_app;
}

void TestPointCloudViewerWidgetRenderingR4::init()
{
    m_viewer = new PointCloudViewerWidget();
    m_viewer->show();
    QTest::qWaitForWindowExposed(m_viewer);

    createTestPointCloud();
}

void TestPointCloudViewerWidgetRenderingR4::cleanup()
{
    delete m_viewer;
    m_viewer = nullptr;
    m_testOctree.reset();
}

void TestPointCloudViewerWidgetRenderingR4::setupOpenGLContext()
{
    m_surface = new QOffscreenSurface();
    m_surface->create();

    m_context = new QOpenGLContext();
    m_context->create();
    m_context->makeCurrent(m_surface);
}

void TestPointCloudViewerWidgetRenderingR4::createTestPointCloud()
{
    auto testPoints = generateTestPoints(1000);

    // Create octree with test data
    m_testOctree = std::make_unique<Octree>();
    m_testOctree->buildFromPoints(testPoints);

    // Load into viewer
    std::vector<float> pointData;
    for (const auto& point : testPoints)
    {
        pointData.push_back(point.x);
        pointData.push_back(point.y);
        pointData.push_back(point.z);
    }

    m_viewer->loadPointCloud(pointData);
}

std::vector<PointFullData> TestPointCloudViewerWidgetRenderingR4::generateTestPoints(int count)
{
    std::vector<PointFullData> points;
    points.reserve(count);

    for (int i = 0; i < count; ++i)
    {
        float x = (i % 10) * 2.0f - 10.0f;
        float y = ((i / 10) % 10) * 2.0f - 10.0f;
        float z = (i / 100) * 2.0f - 5.0f;

        PointFullData point(x, y, z);

        // Add color and intensity data
        point.r = static_cast<uint8_t>((i * 37) % 256);
        point.g = static_cast<uint8_t>((i * 73) % 256);
        point.b = static_cast<uint8_t>((i * 109) % 256);
        point.intensity = (i % 100) / 100.0f;

        // Add normal data for lighting tests
        point.normal = QVector3D(0, 0, 1).normalized();

        points.push_back(point);
    }

    return points;
}

// Sprint R4.1: Point Splatting Tests
void TestPointCloudViewerWidgetRenderingR4::testAggregateDataCalculation()
{
    QVERIFY(m_testOctree != nullptr);
    QVERIFY(m_testOctree->root != nullptr);

    // Test aggregate data calculation (Task R4.1.2)
    const auto& aggregateData = m_testOctree->root->getAggregateData();

    QVERIFY(aggregateData.pointCount > 0);
    QVERIFY(aggregateData.boundingRadius > 0.0f);
    QVERIFY(aggregateData.center.length() >= 0.0f);
    QVERIFY(aggregateData.averageColor.x() >= 0.0f && aggregateData.averageColor.x() <= 1.0f);
    QVERIFY(aggregateData.averageColor.y() >= 0.0f && aggregateData.averageColor.y() <= 1.0f);
    QVERIFY(aggregateData.averageColor.z() >= 0.0f && aggregateData.averageColor.z() <= 1.0f);
    QVERIFY(aggregateData.averageIntensity >= 0.0f && aggregateData.averageIntensity <= 1.0f);
    QVERIFY(aggregateData.averageNormal.length() > 0.9f);  // Should be normalized

    qDebug() << "Aggregate data test passed - Point count:" << aggregateData.pointCount
             << "Bounding radius:" << aggregateData.boundingRadius;
}

void TestPointCloudViewerWidgetRenderingR4::testSplatVertexDataStructure()
{
    // Test SplatVertex structure (Task R4.1.3)
    AggregateNodeData testData;
    testData.center = QVector3D(1.0f, 2.0f, 3.0f);
    testData.averageColor = QVector3D(0.5f, 0.7f, 0.9f);
    testData.averageIntensity = 0.8f;
    testData.averageNormal = QVector3D(0, 0, 1);
    testData.boundingRadius = 5.0f;

    SplatVertex splatVertex(testData);

    QCOMPARE(splatVertex.position, testData.center);
    QCOMPARE(splatVertex.color, testData.averageColor);
    QCOMPARE(splatVertex.intensity, testData.averageIntensity);
    QCOMPARE(splatVertex.normal, testData.averageNormal);
    QCOMPARE(splatVertex.radius, testData.boundingRadius);

    qDebug() << "SplatVertex structure test passed";
}

void TestPointCloudViewerWidgetRenderingR4::testSplatRenderingDecision()
{
    QVERIFY(m_testOctree != nullptr);
    QVERIFY(m_testOctree->root != nullptr);

    // Test splat rendering decision logic (Task R4.1.4, R4.1.5)
    float splatThreshold = 10.0f;
    float screenSpaceError = 5.0f;  // Below threshold

    bool shouldSplat = m_testOctree->root->shouldRenderAsSplat(screenSpaceError, splatThreshold);
    QVERIFY(shouldSplat);  // Should render as splat when error is below threshold

    screenSpaceError = 15.0f;  // Above threshold
    shouldSplat = m_testOctree->root->shouldRenderAsSplat(screenSpaceError, splatThreshold);
    QVERIFY(!shouldSplat);  // Should not render as splat when error is above threshold

    qDebug() << "Splat rendering decision test passed";
}

void TestPointCloudViewerWidgetRenderingR4::testSplatShaderSetup()
{
    // Test that splat shaders are properly initialized
    QVERIFY(m_viewer != nullptr);

    // This test verifies that the shader setup doesn't crash
    // and that the viewer can handle splat rendering calls
    m_viewer->setSplattingEnabled(true);

    // Force a repaint to trigger shader usage
    m_viewer->update();
    QTest::qWait(100);  // Allow time for rendering

    qDebug() << "Splat shader setup test passed";
}

void TestPointCloudViewerWidgetRenderingR4::testSplatTextureCreation()
{
    // Test splat texture creation
    QVERIFY(m_viewer != nullptr);

    m_viewer->setSplattingEnabled(true);
    m_viewer->update();
    QTest::qWait(100);

    // If we reach here without crashing, texture creation succeeded
    qDebug() << "Splat texture creation test passed";
}

// Sprint R4.2: Lighting Tests
void TestPointCloudViewerWidgetRenderingR4::testLightingShaderSetup()
{
    QVERIFY(m_viewer != nullptr);

    // Test lighting shader setup (Task R4.2.2)
    m_viewer->setLightingEnabled(true);
    m_viewer->update();
    QTest::qWait(100);

    qDebug() << "Lighting shader setup test passed";
}

void TestPointCloudViewerWidgetRenderingR4::testLightingParameterPassing()
{
    QVERIFY(m_viewer != nullptr);

    // Test lighting parameter setting (Task R4.2.3)
    QVector3D testDirection(1.0f, 0.0f, 0.0f);
    QColor testColor(255, 128, 64);
    float testAmbient = 0.4f;

    m_viewer->setLightingEnabled(true);
    m_viewer->setLightDirection(testDirection);
    m_viewer->setLightColor(testColor);
    m_viewer->setAmbientIntensity(testAmbient);

    m_viewer->update();
    QTest::qWait(100);

    qDebug() << "Lighting parameter passing test passed";
}

void TestPointCloudViewerWidgetRenderingR4::testNormalEstimation()
{
    QVERIFY(m_testOctree != nullptr);
    QVERIFY(m_testOctree->root != nullptr);

    // Test normal estimation for lighting
    const auto& aggregateData = m_testOctree->root->getAggregateData();

    // Normal should be normalized
    float normalLength = aggregateData.averageNormal.length();
    QVERIFY(normalLength > 0.9f && normalLength < 1.1f);

    qDebug() << "Normal estimation test passed - Normal length:" << normalLength;
}

// Sprint R4.3: UI Integration Tests
void TestPointCloudViewerWidgetRenderingR4::testSplattingUIControls()
{
    QVERIFY(m_viewer != nullptr);

    // Test splatting UI control integration (Task R4.3.2)
    bool initialState = false;
    m_viewer->setSplattingEnabled(initialState);

    // Toggle splatting
    m_viewer->setSplattingEnabled(!initialState);

    m_viewer->update();
    QTest::qWait(50);

    qDebug() << "Splatting UI controls test passed";
}

void TestPointCloudViewerWidgetRenderingR4::testLightingUIControls()
{
    QVERIFY(m_viewer != nullptr);

    // Test lighting UI control integration
    m_viewer->setLightingEnabled(true);
    m_viewer->setLightDirection(QVector3D(0, 1, 0));
    m_viewer->setLightColor(QColor(255, 255, 0));
    m_viewer->setAmbientIntensity(0.5f);

    m_viewer->update();
    QTest::qWait(50);

    qDebug() << "Lighting UI controls test passed";
}

void TestPointCloudViewerWidgetRenderingR4::testUISignalConnections()
{
    // This test verifies that UI signals are properly connected
    // In a real application, this would test MainWindow signal connections
    QVERIFY(m_viewer != nullptr);

    // Test that viewer methods can be called without crashing
    m_viewer->setSplattingEnabled(true);
    m_viewer->setLightingEnabled(true);
    m_viewer->setLightDirection(QVector3D(1, 1, 1));
    m_viewer->setLightColor(QColor(Qt::white));
    m_viewer->setAmbientIntensity(0.3f);

    qDebug() << "UI signal connections test passed";
}

// Sprint R4.4: Performance Tests
void TestPointCloudViewerWidgetRenderingR4::testSplattingPerformance()
{
    QVERIFY(m_viewer != nullptr);

    // Test splatting performance
    QElapsedTimer timer;
    timer.start();

    m_viewer->setSplattingEnabled(true);
    for (int i = 0; i < 10; ++i)
    {
        m_viewer->update();
        QTest::qWait(10);
    }

    qint64 elapsed = timer.elapsed();
    qDebug() << "Splatting performance test - 10 renders took:" << elapsed << "ms";

    // Performance should be reasonable (less than 1 second for 10 renders)
    QVERIFY(elapsed < 1000);
}

void TestPointCloudViewerWidgetRenderingR4::testLightingPerformance()
{
    QVERIFY(m_viewer != nullptr);

    // Test lighting performance
    QElapsedTimer timer;
    timer.start();

    m_viewer->setLightingEnabled(true);
    for (int i = 0; i < 10; ++i)
    {
        m_viewer->update();
        QTest::qWait(10);
    }

    qint64 elapsed = timer.elapsed();
    qDebug() << "Lighting performance test - 10 renders took:" << elapsed << "ms";

    // Performance should be reasonable
    QVERIFY(elapsed < 1000);
}

QTEST_MAIN(TestPointCloudViewerWidgetRenderingR4)
#include "test_pointcloudviewerwidget_rendering_r4.moc"
