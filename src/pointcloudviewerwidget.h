#ifndef POINTCLOUDVIEWERWIDGET_H
#define POINTCLOUDVIEWERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QVector3D>
#include <QColor>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QFont>
#include <vector>
#include <memory>
#include <chrono>
#include "octree.h"
#include "screenspaceerror.h"
#include "pointdata.h"
#include "IPointCloudViewer.h"
#include "rendering/GpuCuller.h"

class PointCloudViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions, public IPointCloudViewer
{
    Q_OBJECT

public:
    explicit PointCloudViewerWidget(QWidget *parent = nullptr);
    ~PointCloudViewerWidget();

    // IPointCloudViewer interface implementation
    void loadPointCloud(const std::vector<float>& points) override;
    void clearPointCloud() override;
    void setState(ViewerState state, const QString &message = "") override;
    void setTopView() override;
    void setLeftView() override;
    void setRightView() override;
    void setBottomView() override;
    void setLODEnabled(bool enabled) override;
    bool isLODEnabled() const override { return m_lodEnabled; }
    void setRenderWithColor(bool enabled) override;
    void setRenderWithIntensity(bool enabled) override;
    void setPointSizeAttenuationEnabled(bool enabled) override;
    void setPointSizeAttenuationParams(float minSize, float maxSize, float factor) override;
    ViewerState getViewerState() const override { return m_currentState; }
    bool hasPointCloudData() const override { return m_hasData; }
    bool hasData() const override { return m_hasData; }
    size_t getPointCount() const override { return static_cast<size_t>(m_pointCount); }
    QVector3D getGlobalOffset() const override { return m_globalOffset; }
    float getCurrentFPS() const override { return m_fps; }
    size_t getVisiblePointCount() const override { return m_visiblePointCount; }
    void resetCamera() override;
    void setFrontView() override;
    void setPointSize(float size) override;
    void setBackgroundColor(const QColor& color) override;
    void setSplattingEnabled(bool enabled) override;
    void setLightingEnabled(bool enabled) override;
    void setLightDirection(const QVector3D& direction) override;
    void setLightColor(const QColor& color) override;
    void setAmbientIntensity(float intensity) override;

    // Additional public methods specific to PointCloudViewerWidget
    float getCameraYaw() const { return m_cameraYaw; }
    float getCameraPitch() const { return m_cameraPitch; }
    QVector3D getCameraTarget() const { return m_cameraTarget; }
    float getCameraDistance() const { return m_cameraDistance; }

    // Test simulation methods
    void simulateOrbitCamera(const QPoint &start, const QPoint &end);
    void simulatePanCamera(const QPoint &start, const QPoint &end);
    void simulateZoomCamera(float factor);

    // Additional LOD methods not in interface
    void setLODDistances(float distance1, float distance2);
    void getLODDistances(float& distance1, float& distance2) const;
    size_t getOctreeNodeCount() const;

public slots:
    // IPointCloudViewer interface slots implementation
    void onLoadingStarted() override;
    void onLoadingProgress(int percentage, const QString &stage) override;
    void onLoadingFinished(bool success, const QString &message,
                          const std::vector<float> &points) override;
    void toggleLOD(bool enabled) override;
    void setLODSubsampleRate(float rate) override;
    void setScreenSpaceErrorThreshold(float threshold) override;
    void setPrimaryScreenSpaceErrorThreshold(float threshold) override;
    void setCullScreenSpaceErrorThreshold(float threshold) override;

    // Sprint R4: Splatting and lighting slots (Task R4.3.2)
    // Note: These methods are now declared in the interface implementation above

    // Sprint 6: Multi-scan visualization support
    void loadMultipleScans(const QStringList& scanIds);
    void unloadScan(const QString& scanId);
    void clearAllScans();
    void setScanColor(const QString& scanId, const QColor& color);
    QStringList getLoadedScans() const;

    // Sprint 6: GPU culling support
    void setGpuCullingEnabled(bool enabled);
    bool isGpuCullingEnabled() const;
    void setGpuCullingThreshold(float threshold);
    float getGpuCullingPerformance() const;

signals:
    // Sprint 2.2: Performance monitoring signals
    void statsUpdated(float fps, int visiblePoints);
    void renderingError(const QString& error);
    void pointCloudLoaded(const std::vector<float>& points);
    void pointCloudLoadFailed(const QString& error);
    void pointCloudCleared();
    void stateChanged(ViewerState newState, const QString& message);

protected:
    // OpenGL overrides
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void paintOverlayGL();  // For text overlays

    // Mouse and keyboard events
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void setupShaders();
    void setupBuffers();
    void updateCamera();
    void calculateBoundingBox();
    void fitCameraToPointCloud();

    // UCS methods
    void setupUCSShaders();
    void setupUCSBuffers();
    void drawUCS();

    // Sprint 1.3: Error state rendering for Task 1.3.3.2
    void renderErrorState();

    // Sprint 2.3: Visual state rendering methods
    void drawLoadingState(QPainter &painter);
    void drawLoadFailedState(QPainter &painter);
    void drawIdleState(QPainter &painter);

    // Sprint R1: LOD rendering methods
    void renderOctree();
    void updateFPS();
    std::array<QVector4D, 6> extractFrustumPlanes(const QMatrix4x4& viewProjection) const;

    // Sprint R2: Enhanced rendering methods
    void renderWithScreenSpaceErrorLOD();
    void updateViewportInfo();
    void logLODStatistics(const std::vector<PointFullData>& visiblePoints);

    // Sprint R3: Enhanced rendering methods (as per backlog Tasks R3.1.4, R3.1.5)
    void renderWithAttributes();
    void prepareVertexData(const std::vector<PointFullData>& points);
    void setupEnhancedShaders();
    void setupEnhancedVertexArrayObject();

    // Sprint R4: Splatting and lighting rendering methods (Task R4.1.3, R4.1.4, R4.1.5)
    void renderScene();
    void renderPoints(const std::vector<PointFullData>& points);
    void renderSplats(const std::vector<AggregateNodeData>& splats);
    void prepareSplatVertexData(const std::vector<AggregateNodeData>& splats);
    void setupSplatShaders();
    void setupSplatTexture();
    void setupSplatVertexArrayObject();

    // Sprint 6: GPU culling methods
    void initializeGpuCuller();
    void performGpuCulling();
    void renderWithGpuCulling();

    // Sprint 6: Multi-scan rendering methods
    void renderMultipleScans();
    void updateScanOctrees();
    QColor generateScanColor(int scanIndex);

private slots:
    void updateLoadingAnimation();
    void emitPerformanceStats(); // Sprint 2.2: Performance monitoring

private:
    // OpenGL objects
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLVertexArrayObject m_vertexArrayObject;
    QOpenGLShaderProgram *m_shaderProgram;

    // UCS OpenGL objects
    QOpenGLBuffer m_ucsVertexBuffer;
    QOpenGLVertexArrayObject m_ucsVertexArrayObject;
    QOpenGLShaderProgram *m_ucsShaderProgram;

    // Shader uniform locations
    int m_mvpMatrixLocation;
    int m_colorLocation;
    int m_pointSizeLocation;

    // UCS uniform locations
    int m_ucsMvpMatrixLocation;

    // Camera parameters
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;
    QMatrix4x4 m_modelMatrix;

    // Camera control
    QVector3D m_cameraPosition;
    QVector3D m_cameraTarget;
    QVector3D m_cameraUp;
    float m_cameraDistance;
    float m_cameraYaw;
    float m_cameraPitch;

    // Mouse interaction
    QPoint m_lastMousePosition;
    bool m_mousePressed;
    Qt::MouseButton m_pressedButton;

    // Point cloud data
    std::vector<float> m_pointData;
    int m_pointCount;

    // Coordinate transformation (User Story 3)
    QVector3D m_globalOffset;  // Original global center offset

    // Bounding box for auto-centering
    QVector3D m_boundingBoxMin;
    QVector3D m_boundingBoxMax;
    QVector3D m_boundingBoxCenter;
    float m_boundingBoxSize;

    // Rendering settings
    QVector3D m_pointColor;
    float m_pointSize;

    // State
    bool m_hasData;
    bool m_shadersInitialized;

    // Sprint 1.3: Error state management for Task 1.3.3.2
    bool m_showErrorState;
    QString m_errorMessage;

    // Sprint 2.3: State management and visual feedback
    ViewerState m_currentState;
    QString m_stateMessage;
    int m_loadingProgress;
    QString m_loadingStage;

    // Loading animation
    QTimer *m_loadingTimer;
    int m_loadingAngle;

    // Sprint 2.2: Performance monitoring timer
    QTimer *m_statsTimer;

    // Fonts for overlay text
    QFont m_overlayFont;
    QFont m_detailFont;

    // Sprint 3.4: LOD state (legacy)
    bool m_lodEnabled;
    float m_lodSubsampleRate;

    // Sprint R1: Advanced LOD system
    std::unique_ptr<Octree> m_octree;
    float m_lodDistance1;
    float m_lodDistance2;
    std::vector<PointFullData> m_visiblePoints;

    // Sprint R2: Screen-space error LOD system
    float m_primaryScreenSpaceErrorThreshold;  // Stop recursion threshold (pixels)
    float m_cullScreenSpaceErrorThreshold;     // Cull completely threshold (pixels)
    ViewportInfo m_viewportInfo;

    // Sprint R3: Attribute rendering and point size attenuation (as per backlog member variables)
    bool m_renderWithColor;
    bool m_renderWithIntensity;
    bool m_pointSizeAttenuationEnabled;
    float m_minPointSize;
    float m_maxPointSize;
    float m_attenuationFactor;
    std::vector<VertexData> m_vertexData;

    // Sprint R4: Splatting and lighting state (Task R4.3.2)
    bool m_splattingEnabled;
    bool m_lightingEnabled;
    QVector3D m_lightDirection;
    QColor m_lightColor;
    float m_ambientIntensity;
    float m_splatThreshold;

    // Sprint R4: OpenGL resources for splatting
    QOpenGLShaderProgram* m_pointShaderProgram;
    QOpenGLShaderProgram* m_splatShaderProgram;
    QOpenGLBuffer m_pointVertexBuffer;
    QOpenGLBuffer m_splatVertexBuffer;
    QOpenGLVertexArrayObject m_pointVAO;
    QOpenGLVertexArrayObject m_splatVAO;
    QOpenGLTexture* m_splatTexture;

    // Sprint R4: Rendering data
    std::vector<AggregateNodeData> m_visibleSplats;
    std::vector<VertexData> m_pointVertexData;
    std::vector<SplatVertex> m_splatVertexData;

    // Performance monitoring
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    float m_fps;
    int m_frameCount;
    size_t m_visiblePointCount;

    // Sprint 6: GPU culling support
    std::unique_ptr<GpuCuller> m_gpuCuller;
    bool m_gpuCullingEnabled;
    float m_gpuCullingThreshold;

    // Sprint 6: Multi-scan visualization support
    struct ScanData {
        QString scanId;
        std::vector<float> pointData;
        QColor color;
        bool isLoaded;
        std::unique_ptr<Octree> octree;
    };
    std::vector<ScanData> m_loadedScans;
    QStringList m_activeScanIds;
};

#endif // POINTCLOUDVIEWERWIDGET_H
