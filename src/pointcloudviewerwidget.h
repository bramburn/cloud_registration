#ifndef POINTCLOUDVIEWERWIDGET_H
#define POINTCLOUDVIEWERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QFont>
#include <vector>

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

    // Sprint 3.4: LOD state
    bool m_lodEnabled;
    float m_lodSubsampleRate;
};

#endif // POINTCLOUDVIEWERWIDGET_H
