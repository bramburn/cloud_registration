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

class PointCloudViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    enum class ViewerState {
        Idle,
        Loading,
        DisplayingData,
        LoadFailed
    };

    explicit PointCloudViewerWidget(QWidget *parent = nullptr);
    ~PointCloudViewerWidget();

    // Public interface
    void loadPointCloud(const std::vector<float>& points);
    void clearPointCloud();
    void setState(ViewerState state, const QString &message = "");

    // Coordinate transformation access (User Story 3)
    QVector3D getGlobalOffset() const { return m_globalOffset; }

    // Sprint 3.2: Test helper methods
    ViewerState getViewerState() const { return m_currentState; }
    bool hasPointCloudData() const { return m_hasData; }
    size_t getPointCount() const { return static_cast<size_t>(m_pointCount); }
    float getCameraYaw() const { return m_cameraYaw; }
    float getCameraPitch() const { return m_cameraPitch; }
    QVector3D getCameraTarget() const { return m_cameraTarget; }
    float getCameraDistance() const { return m_cameraDistance; }

    // Test simulation methods
    void simulateOrbitCamera(const QPoint &start, const QPoint &end);
    void simulatePanCamera(const QPoint &start, const QPoint &end);
    void simulateZoomCamera(float factor);

    // Sprint R1: LOD system controls
    void setLODEnabled(bool enabled);
    bool isLODEnabled() const { return m_lodEnabled; }
    void setLODDistances(float distance1, float distance2);
    void getLODDistances(float& distance1, float& distance2) const;

    // Performance monitoring
    float getCurrentFPS() const { return m_fps; }
    size_t getVisiblePointCount() const { return m_visiblePointCount; }
    size_t getOctreeNodeCount() const;

public slots:
    // View control slots
    void setTopView();
    void setLeftView();
    void setRightView();
    void setBottomView();

    // Sprint 2.3: Loading feedback slots
    void onLoadingStarted();
    void onLoadingProgress(int percentage, const QString &stage);
    void onLoadingFinished(bool success, const QString &message,
                          const std::vector<float> &points);

    // Sprint 3.4: LOD control slots
    void toggleLOD(bool enabled);
    void setLODSubsampleRate(float rate);

    // Sprint R2: Screen-space error LOD control slots
    void setScreenSpaceErrorThreshold(float threshold);
    void setPrimaryScreenSpaceErrorThreshold(float threshold);
    void setCullScreenSpaceErrorThreshold(float threshold);

    // Sprint R3: Attribute rendering and point size attenuation slots (as per backlog Tasks R3.1.6, R3.2.5, R3.3.3)
    void setRenderWithColor(bool enabled);
    void setRenderWithIntensity(bool enabled);
    void setPointSizeAttenuationEnabled(bool enabled);
    void setPointSizeAttenuationParams(float minSize, float maxSize, float factor);

    // Sprint R4: Splatting and lighting slots (Task R4.3.2)
    void setSplattingEnabled(bool enabled);
    void setLightingEnabled(bool enabled);
    void setLightDirection(const QVector3D& direction);
    void setLightColor(const QColor& color);
    void setAmbientIntensity(float intensity);

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

private slots:
    void updateLoadingAnimation();

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
};

#endif // POINTCLOUDVIEWERWIDGET_H
