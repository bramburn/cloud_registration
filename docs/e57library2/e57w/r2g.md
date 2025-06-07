Based on the Sprint R2 backlog for implementing advanced LOD enhancements with screen-space error metrics and refined point selection, I'll provide comprehensive implementation guidance for your Qt6 C++ FARO scene registration software.

## Screen-Space Error Calculation Implementation

The core of Sprint R2 is implementing a screen-space error metric for more accurate LOD selection. Here's the implementation:

```cpp
// src/screenspaceerror.h
#ifndef SCREENSPACEERROR_H
#define SCREENSPACEERROR_H

#include 
#include 
#include 
#include 

struct ViewportInfo {
    int width;
    int height;
    float nearPlane;
    float farPlane;
};

class ScreenSpaceErrorCalculator {
public:
    static float calculateAABBScreenSpaceError(
        const AxisAlignedBoundingBox& aabb,
        const QMatrix4x4& mvpMatrix,
        const ViewportInfo& viewport
    );
    
    static float calculateNodeScreenSpaceExtent(
        const AxisAlignedBoundingBox& aabb,
        const QMatrix4x4& mvpMatrix,
        const ViewportInfo& viewport
    );
    
    static bool shouldCullNode(float screenSpaceError, float threshold);
    static bool shouldStopRecursion(float screenSpaceError, float primaryThreshold);

private:
    static std::array getAABBCorners(const AxisAlignedBoundingBox& aabb);
    static QVector3D projectToScreen(const QVector3D& worldPos, 
                                   const QMatrix4x4& mvpMatrix, 
                                   const ViewportInfo& viewport);
};

#endif // SCREENSPACEERROR_H
```

```cpp
// src/screenspaceerror.cpp
#include "screenspaceerror.h"
#include 
#include 

float ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
    const AxisAlignedBoundingBox& aabb,
    const QMatrix4x4& mvpMatrix,
    const ViewportInfo& viewport) {
    
    // Get all 8 corners of the AABB
    auto corners = getAABBCorners(aabb);
    
    // Project all corners to screen space
    std::vector screenCorners;
    screenCorners.reserve(8);
    
    bool allBehindCamera = true;
    for (const auto& corner : corners) {
        QVector4D clipSpace = mvpMatrix * QVector4D(corner, 1.0f);
        
        // Check if point is in front of camera
        if (clipSpace.w() > 0.001f) {
            allBehindCamera = false;
            QVector3D screenPos = projectToScreen(corner, mvpMatrix, viewport);
            screenCorners.push_back(screenPos);
        }
    }
    
    // If all corners are behind camera, return maximum error to cull
    if (allBehindCamera || screenCorners.empty()) {
        return 0.0f; // Will be culled
    }
    
    // Find screen-space bounding box
    float minX = screenCorners[0].x();
    float maxX = screenCorners[0].x();
    float minY = screenCorners[0].y();
    float maxY = screenCorners[0].y();
    
    for (const auto& screenPos : screenCorners) {
        minX = std::min(minX, screenPos.x());
        maxX = std::max(maxX, screenPos.x());
        minY = std::min(minY, screenPos.y());
        maxY = std::max(maxY, screenPos.y());
    }
    
    // Calculate screen-space extent (diagonal in pixels)
    float width = maxX - minX;
    float height = maxY - minY;
    return std::sqrt(width * width + height * height);
}

float ScreenSpaceErrorCalculator::calculateNodeScreenSpaceExtent(
    const AxisAlignedBoundingBox& aabb,
    const QMatrix4x4& mvpMatrix,
    const ViewportInfo& viewport) {
    
    return calculateAABBScreenSpaceError(aabb, mvpMatrix, viewport);
}

bool ScreenSpaceErrorCalculator::shouldCullNode(float screenSpaceError, float threshold) {
    return screenSpaceError  ScreenSpaceErrorCalculator::getAABBCorners(const AxisAlignedBoundingBox& aabb) {
    return {{
        QVector3D(aabb.min.x(), aabb.min.y(), aabb.min.z()),
        QVector3D(aabb.max.x(), aabb.min.y(), aabb.min.z()),
        QVector3D(aabb.min.x(), aabb.max.y(), aabb.min.z()),
        QVector3D(aabb.max.x(), aabb.max.y(), aabb.min.z()),
        QVector3D(aabb.min.x(), aabb.min.y(), aabb.max.z()),
        QVector3D(aabb.max.x(), aabb.min.y(), aabb.max.z()),
        QVector3D(aabb.min.x(), aabb.max.y(), aabb.max.z()),
        QVector3D(aabb.max.x(), aabb.max.y(), aabb.max.z())
    }};
}

QVector3D ScreenSpaceErrorCalculator::projectToScreen(
    const QVector3D& worldPos, 
    const QMatrix4x4& mvpMatrix, 
    const ViewportInfo& viewport) {
    
    QVector4D clipSpace = mvpMatrix * QVector4D(worldPos, 1.0f);
    
    if (std::abs(clipSpace.w())  getSampledPoints(int maxPoints) const;
    std::vector getSampledPointsByPercentage(float percentage) const;
    std::vector getRepresentativePoints() const;
    
    // Screen-space error based traversal
    void collectVisiblePointsWithScreenSpaceError(
        const std::array& frustumPlanes,
        const QMatrix4x4& mvpMatrix,
        const ViewportInfo& viewport,
        float primaryThreshold,
        float cullThreshold,
        std::vector& visiblePoints
    ) const;

private:
    // Pre-calculated representative points for coarse LOD
    mutable std::vector m_representativePoints;
    mutable bool m_representativePointsCalculated = false;
    
    void calculateRepresentativePoints() const;
};
```

```cpp
// src/octree.cpp (additions)
std::vector OctreeNode::getSampledPoints(int maxPoints) const {
    if (points.empty()) {
        return {};
    }
    
    if (static_cast(points.size())  sampledPoints;
    sampledPoints.reserve(maxPoints);
    
    // Use deterministic sampling based on point index for consistency
    int step = static_cast(points.size()) / maxPoints;
    for (int i = 0; i (points.size()); ++i) {
        sampledPoints.push_back(points[i * step]);
    }
    
    return sampledPoints;
}

std::vector OctreeNode::getSampledPointsByPercentage(float percentage) const {
    int maxPoints = static_cast(points.size() * percentage);
    return getSampledPoints(maxPoints);
}

std::vector OctreeNode::getRepresentativePoints() const {
    if (!m_representativePointsCalculated) {
        calculateRepresentativePoints();
        m_representativePointsCalculated = true;
    }
    return m_representativePoints;
}

void OctreeNode::calculateRepresentativePoints() const {
    if (isLeaf) {
        // For leaf nodes, use a subset of points
        m_representativePoints = getSampledPoints(std::min(100, static_cast(points.size())));
    } else {
        // For internal nodes, collect representative points from children
        m_representativePoints.clear();
        for (const auto& child : children) {
            if (child) {
                auto childRepPoints = child->getRepresentativePoints();
                m_representativePoints.insert(m_representativePoints.end(), 
                                            childRepPoints.begin(), childRepPoints.end());
            }
        }
        
        // Limit total representative points for internal nodes
        if (m_representativePoints.size() > 200) {
            m_representativePoints.resize(200);
        }
    }
}

void OctreeNode::collectVisiblePointsWithScreenSpaceError(
    const std::array& frustumPlanes,
    const QMatrix4x4& mvpMatrix,
    const ViewportInfo& viewport,
    float primaryThreshold,
    float cullThreshold,
    std::vector& visiblePoints) const {
    
    // Check frustum culling first
    if (!intersectsFrustum(frustumPlanes)) {
        return;
    }
    
    // Calculate screen-space error
    float screenSpaceError = ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
        bounds, mvpMatrix, viewport);
    
    // Cull if error is too small
    if (ScreenSpaceErrorCalculator::shouldCullNode(screenSpaceError, cullThreshold)) {
        return;
    }
    
    // Stop recursion if error is below primary threshold
    if (ScreenSpaceErrorCalculator::shouldStopRecursion(screenSpaceError, primaryThreshold)) {
        // Render representative points for this coarse LOD level
        auto representativePoints = getRepresentativePoints();
        visiblePoints.insert(visiblePoints.end(), 
                           representativePoints.begin(), representativePoints.end());
        return;
    }
    
    if (isLeaf) {
        // Leaf node: add all points
        visiblePoints.insert(visiblePoints.end(), points.begin(), points.end());
    } else {
        // Internal node: recurse to children
        for (const auto& child : children) {
            if (child) {
                child->collectVisiblePointsWithScreenSpaceError(
                    frustumPlanes, mvpMatrix, viewport, 
                    primaryThreshold, cullThreshold, visiblePoints);
            }
        }
    }
}
```

## Enhanced PointCloudViewerWidget with UI Controls

Update the viewer widget to support the new LOD system:

```cpp
// src/pointcloudviewerwidget.h (additions)
class PointCloudViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    // ... existing methods ...

public slots:
    void setLODEnabled(bool enabled);
    void setScreenSpaceErrorThreshold(float threshold);
    void setPrimaryScreenSpaceErrorThreshold(float threshold);
    void setCullScreenSpaceErrorThreshold(float threshold);

private:
    // LOD system parameters
    bool m_lodEnabled;
    float m_primaryScreenSpaceErrorThreshold;  // Stop recursion threshold (pixels)
    float m_cullScreenSpaceErrorThreshold;     // Cull completely threshold (pixels)
    
    // Viewport information
    ViewportInfo m_viewportInfo;
    
    // Enhanced rendering methods
    void renderWithScreenSpaceErrorLOD();
    void updateViewportInfo();
    
    // Performance monitoring
    void logLODStatistics(const std::vector& visiblePoints);
};
```

```cpp
// src/pointcloudviewerwidget.cpp (key methods)
PointCloudViewerWidget::PointCloudViewerWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    // ... existing initialization ...
    , m_lodEnabled(true)
    , m_primaryScreenSpaceErrorThreshold(50.0f)  // 50 pixels
    , m_cullScreenSpaceErrorThreshold(2.0f)      // 2 pixels
{
    // ... existing constructor code ...
}

void PointCloudViewerWidget::setLODEnabled(bool enabled) {
    m_lodEnabled = enabled;
    update(); // Trigger repaint
}

void PointCloudViewerWidget::setScreenSpaceErrorThreshold(float threshold) {
    m_primaryScreenSpaceErrorThreshold = threshold;
    update();
}

void PointCloudViewerWidget::setPrimaryScreenSpaceErrorThreshold(float threshold) {
    m_primaryScreenSpaceErrorThreshold = threshold;
    update();
}

void PointCloudViewerWidget::setCullScreenSpaceErrorThreshold(float threshold) {
    m_cullScreenSpaceErrorThreshold = threshold;
    update();
}

void PointCloudViewerWidget::paintGL() {
    updateFPS();
    updateViewportInfo();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (!m_octree->root) return;
    
    updateCamera();
    
    if (m_lodEnabled) {
        renderWithScreenSpaceErrorLOD();
    } else {
        // Fallback: render with basic frustum culling only
        renderOctree(); // From Sprint R1
    }
}

void PointCloudViewerWidget::renderWithScreenSpaceErrorLOD() {
    // Extract frustum planes
    QMatrix4x4 viewProjection = m_projectionMatrix * m_viewMatrix;
    auto frustumPlanes = extractFrustumPlanes(viewProjection);
    
    // Clear previous visible points
    m_visiblePoints.clear();
    
    // Collect visible points using screen-space error LOD
    m_octree->root->collectVisiblePointsWithScreenSpaceError(
        frustumPlanes, viewProjection, m_viewportInfo,
        m_primaryScreenSpaceErrorThreshold,
        m_cullScreenSpaceErrorThreshold,
        m_visiblePoints
    );
    
    if (m_visiblePoints.empty()) return;
    
    // Log statistics for debugging
    logLODStatistics(m_visiblePoints);
    
    // Update VBO and render
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(m_visiblePoints.data(), 
                           static_cast(m_visiblePoints.size() * sizeof(PointFullData)));
    
    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue("mvpMatrix", viewProjection);
    
    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, static_cast(m_visiblePoints.size()));
    glBindVertexArray(0);
    
    m_shaderProgram->release();
    m_vertexBuffer.release();
}

void PointCloudViewerWidget::updateViewportInfo() {
    m_viewportInfo.width = width();
    m_viewportInfo.height = height();
    m_viewportInfo.nearPlane = 0.1f;  // Should match your projection matrix
    m_viewportInfo.farPlane = 1000.0f; // Should match your projection matrix
}

void PointCloudViewerWidget::logLODStatistics(const std::vector& visiblePoints) {
    static int frameCount = 0;
    frameCount++;
    
    if (frameCount % 60 == 0) { // Log every 60 frames
        qDebug() setChecked(true);
    lodLayout->addWidget(m_lodEnableCheckbox);
    
    // LOD Quality Slider (inverse of primary threshold)
    QLabel* qualityLabel = new QLabel("LOD Quality (Higher = More Detail):", this);
    m_lodQualitySlider = new QSlider(Qt::Horizontal, this);
    m_lodQualitySlider->setRange(1, 100);
    m_lodQualitySlider->setValue(50); // Default to medium quality
    m_lodQualitySlider->setTickPosition(QSlider::TicksBelow);
    m_lodQualitySlider->setTickInterval(10);
    
    QHBoxLayout* qualityLayout = new QHBoxLayout();
    qualityLayout->addWidget(qualityLabel);
    qualityLayout->addWidget(m_lodQualitySlider);
    lodLayout->addLayout(qualityLayout);
    
    // Primary threshold control
    QLabel* primaryLabel = new QLabel("Primary Threshold (pixels):", this);
    m_primaryThresholdSpinBox = new QSpinBox(this);
    m_primaryThresholdSpinBox->setRange(1, 200);
    m_primaryThresholdSpinBox->setValue(50);
    
    QHBoxLayout* primaryLayout = new QHBoxLayout();
    primaryLayout->addWidget(primaryLabel);
    primaryLayout->addWidget(m_primaryThresholdSpinBox);
    lodLayout->addLayout(primaryLayout);
    
    // Cull threshold control
    QLabel* cullLabel = new QLabel("Cull Threshold (pixels):", this);
    m_cullThresholdSpinBox = new QSpinBox(this);
    m_cullThresholdSpinBox->setRange(1, 10);
    m_cullThresholdSpinBox->setValue(2);
    
    QHBoxLayout* cullLayout = new QHBoxLayout();
    cullLayout->addWidget(cullLabel);
    cullLayout->addWidget(m_cullThresholdSpinBox);
    lodLayout->addLayout(cullLayout);
    
    // Statistics display
    m_lodStatsLabel = new QLabel("LOD Statistics will appear here", this);
    m_lodStatsLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 5px; }");
    lodLayout->addWidget(m_lodStatsLabel);
    
    // Add to main layout (adjust based on your existing UI structure)
    QWidget* centralWidget = this->centralWidget();
    if (centralWidget && centralWidget->layout()) {
        centralWidget->layout()->addWidget(lodGroupBox);
    }
}

void MainWindow::connectLODSignals() {
    connect(m_lodEnableCheckbox, &QCheckBox::toggled, 
            this, &MainWindow::onLODEnabledChanged);
    connect(m_lodQualitySlider, &QSlider::valueChanged,
            this, &MainWindow::onLODQualityChanged);
    connect(m_primaryThresholdSpinBox, QOverload::of(&QSpinBox::valueChanged),
            this, &MainWindow::onPrimaryThresholdChanged);
    connect(m_cullThresholdSpinBox, QOverload::of(&QSpinBox::valueChanged),
            this, &MainWindow::onCullThresholdChanged);
    
    // Connect to point cloud viewer
    connect(m_lodEnableCheckbox, &QCheckBox::toggled,
            m_pointCloudViewer, &PointCloudViewerWidget::setLODEnabled);
    connect(m_primaryThresholdSpinBox, QOverload::of(&QSpinBox::valueChanged),
            m_pointCloudViewer, &PointCloudViewerWidget::setPrimaryScreenSpaceErrorThreshold);
    connect(m_cullThresholdSpinBox, QOverload::of(&QSpinBox::valueChanged),
            m_pointCloudViewer, &PointCloudViewerWidget::setCullScreenSpaceErrorThreshold);
}

void MainWindow::onLODEnabledChanged(bool enabled) {
    m_lodQualitySlider->setEnabled(enabled);
    m_primaryThresholdSpinBox->setEnabled(enabled);
    m_cullThresholdSpinBox->setEnabled(enabled);
}

void MainWindow::onLODQualityChanged(int value) {
    // Convert quality slider (1-100) to threshold (100-1 pixels)
    float threshold = 101.0f - value;
    m_primaryThresholdSpinBox->setValue(static_cast(threshold));
}

void MainWindow::onPrimaryThresholdChanged(int value) {
    // Update quality slider to reflect threshold change
    int qualityValue = 101 - value;
    m_lodQualitySlider->setValue(qualityValue);
}

void MainWindow::onCullThresholdChanged(int value) {
    // This is handled automatically by the signal connection
    Q_UNUSED(value)
}
```

## Google Test Implementation for Sprint R2

Create comprehensive tests for the new functionality:

```cpp
// tests/test_pointcloudviewerwidget_lod_r2.cpp
#include 
#include "../src/screenspaceerror.h"
#include "../src/octree.h"
#include 

class ScreenSpaceErrorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup test viewport
        viewport.width = 1920;
        viewport.height = 1080;
        viewport.nearPlane = 0.1f;
        viewport.farPlane = 1000.0f;
        
        // Setup test AABB
        testAABB = AxisAlignedBoundingBox(
            QVector3D(-1, -1, -1), 
            QVector3D(1, 1, 1)
        );
        
        // Setup test MVP matrix (identity for simplicity)
        mvpMatrix.setToIdentity();
        mvpMatrix.perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
        mvpMatrix.lookAt(QVector3D(0, 0, 5), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    }
    
    ViewportInfo viewport;
    AxisAlignedBoundingBox testAABB;
    QMatrix4x4 mvpMatrix;
};

TEST_F(ScreenSpaceErrorTest, CalculateScreenSpaceErrorBasic) {
    float error = ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
        testAABB, mvpMatrix, viewport);
    
    EXPECT_GT(error, 0.0f);
    EXPECT_LT(error, viewport.width); // Should be reasonable
}

TEST_F(ScreenSpaceErrorTest, DistantObjectHasSmallerError) {
    // Close AABB
    QMatrix4x4 closeMVP;
    closeMVP.setToIdentity();
    closeMVP.perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    closeMVP.lookAt(QVector3D(0, 0, 3), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    
    // Distant AABB
    QMatrix4x4 distantMVP;
    distantMVP.setToIdentity();
    distantMVP.perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    distantMVP.lookAt(QVector3D(0, 0, 20), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    
    float closeError = ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
        testAABB, closeMVP, viewport);
    float distantError = ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
        testAABB, distantMVP, viewport);
    
    EXPECT_GT(closeError, distantError);
}

TEST_F(ScreenSpaceErrorTest, CullingThresholds) {
    float error = 10.0f;
    
    EXPECT_TRUE(ScreenSpaceErrorCalculator::shouldCullNode(error, 15.0f));
    EXPECT_FALSE(ScreenSpaceErrorCalculator::shouldCullNode(error, 5.0f));
    
    EXPECT_TRUE(ScreenSpaceErrorCalculator::shouldStopRecursion(error, 15.0f));
    EXPECT_FALSE(ScreenSpaceErrorCalculator::shouldStopRecursion(error, 5.0f));
}

class RefinedPointSelectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test octree node with known points
        AxisAlignedBoundingBox bounds(QVector3D(0, 0, 0), QVector3D(10, 10, 10));
        testNode = std::make_unique(bounds);
        
        // Add test points
        for (int i = 0; i points.emplace_back(
                static_cast(i % 10),
                static_cast((i / 10) % 10),
                static_cast(i / 100),
                1.0f, 1.0f, 1.0f, 1.0f
            );
        }
    }
    
    std::unique_ptr testNode;
};

TEST_F(RefinedPointSelectionTest, SampledPointsRespectMaxCount) {
    auto sampledPoints = testNode->getSampledPoints(100);
    
    EXPECT_EQ(sampledPoints.size(), 100);
}

TEST_F(RefinedPointSelectionTest, SampledPointsByPercentage) {
    auto sampledPoints = testNode->getSampledPointsByPercentage(0.1f); // 10%
    
    EXPECT_EQ(sampledPoints.size(), 100); // 10% of 1000
}

TEST_F(RefinedPointSelectionTest, RepresentativePointsConsistent) {
    auto rep1 = testNode->getRepresentativePoints();
    auto rep2 = testNode->getRepresentativePoints();
    
    EXPECT_EQ(rep1.size(), rep2.size());
    // Should be identical due to caching
    for (size_t i = 0; i (x),
                        static_cast(y),
                        static_cast(z),
                        1.0f, 1.0f, 1.0f, 1.0f
                    );
                }
            }
        }
        
        octree.build(testPoints, 6, 100);
        
        // Setup viewport and matrices
        viewport.width = 1920;
        viewport.height = 1080;
        viewport.nearPlane = 0.1f;
        viewport.farPlane = 1000.0f;
        
        mvpMatrix.setToIdentity();
        mvpMatrix.perspective(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
        mvpMatrix.lookAt(QVector3D(25, 25, 50), QVector3D(25, 25, 5), QVector3D(0, 1, 0));
        
        // Setup frustum
        frustumPlanes = extractFrustumPlanesFromMatrix(mvpMatrix);
    }
    
    std::vector testPoints;
    Octree octree;
    ViewportInfo viewport;
    QMatrix4x4 mvpMatrix;
    std::array frustumPlanes;
    
    std::array extractFrustumPlanesFromMatrix(const QMatrix4x4& matrix) {
        // Implementation similar to PointCloudViewerWidget::extractFrustumPlanes
        std::array planes;
        // ... (implementation details)
        return planes;
    }
};

TEST_F(IntegrationTest, ScreenSpaceErrorLODReducesPoints) {
    std::vector visiblePoints;
    
    // High threshold - should get fewer points
    octree.root->collectVisiblePointsWithScreenSpaceError(
        frustumPlanes, mvpMatrix, viewport,
        100.0f, 5.0f, visiblePoints);
    
    size_t highThresholdCount = visiblePoints.size();
    visiblePoints.clear();
    
    // Low threshold - should get more points
    octree.root->collectVisiblePointsWithScreenSpaceError(
        frustumPlanes, mvpMatrix, viewport,
        10.0f, 1.0f, visiblePoints);
    
    size_t lowThresholdCount = visiblePoints.size();
    
    EXPECT_LT(highThresholdCount, lowThresholdCount);
    EXPECT_GT(highThresholdCount, 0);
}
```

## Performance Optimization and External Libraries

For enhanced performance, consider integrating these external libraries via vcpkg:

```json
// vcpkg.json additions
{
  "dependencies": [
    "qt6-base",
    "qt6-opengl",
    "gtest",
    "eigen3",
    "tbb",
    "benchmark"
  ]
}
```

**Intel TBB for Parallel Processing:**

```cpp
// For parallel octree operations
#include 
#include 

void OctreeNode::calculateRepresentativePointsParallel() const {
    if (!isLeaf && !children.empty()) {
        tbb::parallel_for(tbb::blocked_range(0, children.size()),
            [this](const tbb::blocked_range& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    if (children[i]) {
                        children[i]->getRepresentativePoints();
                    }
                }
            });
    }
}
```

## CMakeLists.txt Updates

```cmake
# CMakeLists.txt additions for Sprint R2
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL OpenGLWidgets)
find_package(GTest REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(TBB REQUIRED)

# Add new source files
set(SPRINT_R2_SOURCES
    src/screenspaceerror.cpp
    src/screenspaceerror.h
    # Updated octree files from R1
    src/octree.cpp
    src/octree.h
)

# Link additional libraries
target_link_libraries(your_target 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::OpenGL 
    Qt6::OpenGLWidgets
    Eigen3::Eigen
    TBB::tbb
)

# Test executable for R2
add_executable(sprint_r2_tests
    tests/test_pointcloudviewerwidget_lod_r2.cpp
    ${SPRINT_R2_SOURCES}
)

target_link_libraries(sprint_r2_tests 
    GTest::gtest_main
    Qt6::Core
    Eigen3::Eigen
)
```

This implementation provides a comprehensive solution for Sprint R2, incorporating screen-space error metrics, refined point selection, and user-controllable LOD parameters. The system should demonstrate significant performance improvements while maintaining visual quality, especially for complex scenes with varying depth.[1][2][3][4][5]

[1] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/35208055/cf9b38e0-cca7-401e-bc93-2d42dae186b4/paste.txt
[2] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/collection_59dddd7d-a43e-4981-8f37-8f1d5f4c9bdc/39178ad2-15a6-4478-bf2f-d68b85feb663/paste.txt
[3] https://www.cg.tuwien.ac.at/research/publications/2024/SCHUETZ-2024-SIMLOD/SCHUETZ-2024-SIMLOD-paper.pdf
[4] https://github.com/m-schuetz/CudaLOD
[5] https://gamedev.stackexchange.com/questions/211056/how-to-compute-screen-space-error-for-lod-selection
[6] https://www.semanticscholar.org/paper/eafe5df41bc2772a70de00d897c8737a6fdbec82
[7] https://onlinelibrary.wiley.com/doi/10.1111/j.1467-8659.2009.01543.x
[8] https://www.semanticscholar.org/paper/06b2a42af817c5692c1f6d51b88ddf70ab863526
[9] https://www.semanticscholar.org/paper/d07ecd38524a2fe9b5f544b8137f98bd934cdc5e
[10] https://dl.acm.org/doi/10.1145/1503454.1503462
[11] https://www.semanticscholar.org/paper/38d814b2a86084cfdcd73b23ba3934adc51c81d0
[12] https://doc.qt.io/qt-6/qwidget.html
[13] https://doc.qt.io/qt-6/whatsnew62.html
[14] https://stackoverflow.com/questions/28216001/how-to-render-text-with-qopenglwidget
[15] https://github.com/mikeroyal/Qt-Guide
[16] https://onlinelibrary.wiley.com/doi/10.1111/cgf.13993
[17] https://ciss-journal.org/article/view/9384
[18] http://diglib.eg.org/handle/10.2312/SGP.SGP03.094-101
[19] http://koreascience.or.kr/journal/view.jsp?kj=JBCREI&py=2002&vnc=v9An4&sp=581
[20] https://community.cesium.com/t/city-scale-3d-buildings-performance-problems/7122
[21] https://www.cg.tuwien.ac.at/research/publications/2011/scheiblauer-2011-cag/scheiblauer-2011-cag-paper.pdf
[22] https://stackoverflow.com/questions/42652738/how-to-automatically-increase-decrease-text-size-in-label-in-qt