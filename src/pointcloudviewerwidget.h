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
#include <vector>

class PointCloudViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit PointCloudViewerWidget(QWidget *parent = nullptr);
    ~PointCloudViewerWidget();

    // Public interface
    void loadPointCloud(const std::vector<float>& points);
    void clearPointCloud();

    // Coordinate transformation access (User Story 3)
    QVector3D getGlobalOffset() const { return m_globalOffset; }

public slots:
    // View control slots
    void setTopView();
    void setLeftView();
    void setRightView();
    void setBottomView();

protected:
    // OpenGL overrides
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

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
};

#endif // POINTCLOUDVIEWERWIDGET_H
