#include "pointcloudviewerwidget.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <cmath>

PointCloudViewerWidget::PointCloudViewerWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , m_shaderProgram(nullptr)
    , m_ucsVertexBuffer(QOpenGLBuffer::VertexBuffer)
    , m_ucsShaderProgram(nullptr)
    , m_mvpMatrixLocation(-1)
    , m_colorLocation(-1)
    , m_pointSizeLocation(-1)
    , m_ucsMvpMatrixLocation(-1)
    , m_cameraPosition(0.0f, 0.0f, 5.0f)
    , m_cameraTarget(0.0f, 0.0f, 0.0f)
    , m_cameraUp(0.0f, 1.0f, 0.0f)
    , m_cameraDistance(5.0f)
    , m_cameraYaw(0.0f)
    , m_cameraPitch(0.0f)
    , m_mousePressed(false)
    , m_pressedButton(Qt::NoButton)
    , m_pointCount(0)
    , m_boundingBoxMin(0.0f, 0.0f, 0.0f)
    , m_boundingBoxMax(0.0f, 0.0f, 0.0f)
    , m_boundingBoxCenter(0.0f, 0.0f, 0.0f)
    , m_boundingBoxSize(1.0f)
    , m_pointColor(1.0f, 1.0f, 1.0f)
    , m_pointSize(2.0f)
    , m_hasData(false)
    , m_shadersInitialized(false)
{
    qDebug() << "PointCloudViewerWidget constructor started";
    setFocusPolicy(Qt::StrongFocus);

    // Initialize matrices
    m_modelMatrix.setToIdentity();
    m_viewMatrix.setToIdentity();
    m_projectionMatrix.setToIdentity();

    qDebug() << "PointCloudViewerWidget constructor completed";
}

PointCloudViewerWidget::~PointCloudViewerWidget()
{
    makeCurrent();

    if (m_shaderProgram) {
        delete m_shaderProgram;
    }

    if (m_ucsShaderProgram) {
        delete m_ucsShaderProgram;
    }

    doneCurrent();
}

void PointCloudViewerWidget::initializeGL()
{
    qDebug() << "PointCloudViewerWidget::initializeGL() started";

    try {
        qDebug() << "Initializing OpenGL functions...";
        initializeOpenGLFunctions();
        qDebug() << "OpenGL functions initialized";

        // Log OpenGL information
        qDebug() << "OpenGL Version:" << reinterpret_cast<const char*>(glGetString(GL_VERSION));
        qDebug() << "OpenGL Vendor:" << reinterpret_cast<const char*>(glGetString(GL_VENDOR));
        qDebug() << "OpenGL Renderer:" << reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        qDebug() << "GLSL Version:" << reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));

        // Set clear color to dark gray with error checking (User Story 2)
        qDebug() << "Setting OpenGL state...";
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            qCritical() << "OpenGL Error after glClearColor:" << QString("0x%1").arg(error, 0, 16);
        }

        // Enable depth testing with error checking
        glEnable(GL_DEPTH_TEST);
        error = glGetError();
        if (error != GL_NO_ERROR) {
            qCritical() << "OpenGL Error after glEnable(GL_DEPTH_TEST):" << QString("0x%1").arg(error, 0, 16);
        }

        // Enable point size control from vertex shader with error checking
        glEnable(GL_PROGRAM_POINT_SIZE);
        error = glGetError();
        if (error != GL_NO_ERROR) {
            qCritical() << "OpenGL Error after glEnable(GL_PROGRAM_POINT_SIZE):" << QString("0x%1").arg(error, 0, 16);
        }
        qDebug() << "OpenGL state configured";

        // Setup shaders
        qDebug() << "Setting up main shaders...";
        setupShaders();
        qDebug() << "Main shaders setup completed";

        qDebug() << "Setting up UCS shaders...";
        setupUCSShaders();
        qDebug() << "UCS shaders setup completed";

        // Setup buffers
        qDebug() << "Setting up main buffers...";
        setupBuffers();
        qDebug() << "Main buffers setup completed";

        qDebug() << "Setting up UCS buffers...";
        setupUCSBuffers();
        qDebug() << "UCS buffers setup completed";

        qDebug() << "OpenGL initialized successfully";
    } catch (const std::exception& e) {
        qCritical() << "Exception in initializeGL:" << e.what();
        throw;
    } catch (...) {
        qCritical() << "Unknown exception in initializeGL";
        throw;
    }
}

void PointCloudViewerWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);

    // Update projection matrix
    m_projectionMatrix.setToIdentity();
    float aspect = float(w) / float(h ? h : 1);
    m_projectionMatrix.perspective(45.0f, aspect, 0.1f, 1000.0f);

    updateCamera();
}

void PointCloudViewerWidget::paintGL()
{
    // Clear buffers with error checking (User Story 2)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        qCritical() << "OpenGL Error after glClear:" << QString("0x%1").arg(error, 0, 16);
    }

    // Debug logging for rendering state (User Story 2)
    if (!m_hasData) {
        qDebug() << "paintGL: No data to render (m_hasData = false)";
        return;
    }
    if (!m_shadersInitialized) {
        qDebug() << "paintGL: Shaders not initialized (m_shadersInitialized = false)";
        return;
    }

    qDebug() << "paintGL: Rendering" << m_pointCount << "points";

    // Use shader program with error checking
    if (!m_shaderProgram->bind()) {
        qWarning() << "Failed to bind shader program";
        return;
    }
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qCritical() << "OpenGL Error after shader bind:" << QString("0x%1").arg(error, 0, 16);
    }

    // Calculate MVP matrix
    QMatrix4x4 mvpMatrix = m_projectionMatrix * m_viewMatrix * m_modelMatrix;

    // Set uniforms with error checking
    m_shaderProgram->setUniformValue(m_mvpMatrixLocation, mvpMatrix);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qCritical() << "OpenGL Error after setting MVP matrix uniform:" << QString("0x%1").arg(error, 0, 16);
    }

    m_shaderProgram->setUniformValue(m_colorLocation, m_pointColor);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qCritical() << "OpenGL Error after setting color uniform:" << QString("0x%1").arg(error, 0, 16);
    }

    m_shaderProgram->setUniformValue(m_pointSizeLocation, m_pointSize);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qCritical() << "OpenGL Error after setting point size uniform:" << QString("0x%1").arg(error, 0, 16);
    }

    qDebug() << "paintGL: Point size set to:" << m_pointSize;

    // Bind VAO and draw points with error checking
    m_vertexArrayObject.bind();
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qCritical() << "OpenGL Error after VAO bind:" << QString("0x%1").arg(error, 0, 16);
    }

    qDebug() << "paintGL: Drawing" << m_pointCount << "points with glDrawArrays(GL_POINTS, 0," << m_pointCount << ")";
    glDrawArrays(GL_POINTS, 0, m_pointCount);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qCritical() << "OpenGL Error after glDrawArrays:" << QString("0x%1").arg(error, 0, 16);
    }

    m_vertexArrayObject.release();
    m_shaderProgram->release();

    // Draw UCS indicator
    drawUCS();
}

void PointCloudViewerWidget::setupShaders()
{
    m_shaderProgram = new QOpenGLShaderProgram(this);

    // Vertex shader source (embedded for simplicity)
    const char* vertexShaderSource = R"(
        #version 330 core

        layout (location = 0) in vec3 position;

        uniform mat4 mvpMatrix;
        uniform float pointSize;

        void main()
        {
            gl_Position = mvpMatrix * vec4(position, 1.0);
            gl_PointSize = pointSize;
        }
    )";

    // Fragment shader source (embedded for simplicity)
    const char* fragmentShaderSource = R"(
        #version 330 core

        uniform vec3 color;
        out vec4 fragColor;

        void main()
        {
            fragColor = vec4(color, 1.0);
        }
    )";

    // Compile shaders
    if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource)) {
        qCritical() << "Failed to compile vertex shader:" << m_shaderProgram->log();
        return;
    }

    if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource)) {
        qCritical() << "Failed to compile fragment shader:" << m_shaderProgram->log();
        return;
    }

    // Link shader program
    if (!m_shaderProgram->link()) {
        qCritical() << "Failed to link shader program:" << m_shaderProgram->log();
        return;
    }

    // Get uniform locations with detailed checking (User Story 2)
    m_mvpMatrixLocation = m_shaderProgram->uniformLocation("mvpMatrix");
    m_colorLocation = m_shaderProgram->uniformLocation("color");
    m_pointSizeLocation = m_shaderProgram->uniformLocation("pointSize");

    qDebug() << "Uniform locations:";
    qDebug() << "  mvpMatrix:" << m_mvpMatrixLocation;
    qDebug() << "  color:" << m_colorLocation;
    qDebug() << "  pointSize:" << m_pointSizeLocation;

    if (m_mvpMatrixLocation == -1) {
        qCritical() << "Failed to get mvpMatrix uniform location - shader may have optimized it out or name is incorrect";
    }
    if (m_colorLocation == -1) {
        qCritical() << "Failed to get color uniform location - shader may have optimized it out or name is incorrect";
    }
    if (m_pointSizeLocation == -1) {
        qCritical() << "Failed to get pointSize uniform location - shader may have optimized it out or name is incorrect";
    }

    // Only set initialized flag if all uniforms are found
    if (m_mvpMatrixLocation != -1 && m_colorLocation != -1 && m_pointSizeLocation != -1) {
        m_shadersInitialized = true;
        qDebug() << "Shaders compiled and linked successfully - all uniforms found";
    } else {
        m_shadersInitialized = false;
        qCritical() << "Shader setup failed - one or more uniform locations not found";
    }
}

void PointCloudViewerWidget::setupBuffers()
{
    // Create VAO
    if (!m_vertexArrayObject.create()) {
        qCritical() << "Failed to create VAO";
        return;
    }

    // Create VBO
    if (!m_vertexBuffer.create()) {
        qCritical() << "Failed to create VBO";
        return;
    }

    qDebug() << "OpenGL buffers created successfully";
}

void PointCloudViewerWidget::loadPointCloud(const std::vector<float>& points)
{
    // Debug logging for data reception (User Story 1)
    qDebug() << "=== PointCloudViewerWidget::loadPointCloud ===";
    qDebug() << "Received points vector size:" << points.size();
    qDebug() << "Number of points:" << (points.size() / 3);

    if (points.empty() || points.size() % 3 != 0) {
        qWarning() << "Invalid point cloud data - empty or not divisible by 3";
        return;
    }

    makeCurrent();

    m_pointData = points;
    m_pointCount = static_cast<int>(points.size() / 3);
    qDebug() << "Point count set to:" << m_pointCount;

    // Calculate bounding box
    calculateBoundingBox();

    // Debug logging after bounding box calculation (User Story 1)
    qDebug() << "Bounding box calculated:";
    qDebug() << "  Min:" << m_boundingBoxMin;
    qDebug() << "  Max:" << m_boundingBoxMax;
    qDebug() << "  Center:" << m_boundingBoxCenter;
    qDebug() << "  Size:" << m_boundingBoxSize;

    // Update camera to fit the point cloud using proper field-of-view calculation
    fitCameraToPointCloud();

    // Debug logging after camera fitting (User Story 1)
    qDebug() << "Camera fitted:";
    qDebug() << "  Distance:" << m_cameraDistance;

    updateCamera();

    // Debug logging after camera update (User Story 1)
    qDebug() << "Camera updated:";
    qDebug() << "  Position:" << m_cameraPosition;
    qDebug() << "  Target:" << m_cameraTarget;

    // Upload data to GPU with OpenGL error checking (User Story 2)
    m_vertexArrayObject.bind();
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        qCritical() << "OpenGL Error after VAO bind:" << QString("0x%1").arg(error, 0, 16);
    }

    m_vertexBuffer.bind();
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qCritical() << "OpenGL Error after VBO bind:" << QString("0x%1").arg(error, 0, 16);
    }

    m_vertexBuffer.allocate(points.data(), static_cast<int>(points.size() * sizeof(float)));
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qCritical() << "OpenGL Error after VBO allocate:" << QString("0x%1").arg(error, 0, 16);
    }

    // Set vertex attribute
    glEnableVertexAttribArray(0);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qCritical() << "OpenGL Error after glEnableVertexAttribArray:" << QString("0x%1").arg(error, 0, 16);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    error = glGetError();
    if (error != GL_NO_ERROR) {
        qCritical() << "OpenGL Error after glVertexAttribPointer:" << QString("0x%1").arg(error, 0, 16);
    }

    m_vertexBuffer.release();
    m_vertexArrayObject.release();

    m_hasData = true;
    qDebug() << "m_hasData set to true";

    doneCurrent();
    update(); // Trigger repaint

    qDebug() << "Point cloud loading completed successfully";
}

void PointCloudViewerWidget::clearPointCloud()
{
    makeCurrent();

    m_pointData.clear();
    m_pointCount = 0;
    m_hasData = false;

    doneCurrent();
    update();
}

void PointCloudViewerWidget::calculateBoundingBox()
{
    if (m_pointData.empty()) {
        return;
    }

    // Initialize with first point
    m_boundingBoxMin = QVector3D(m_pointData[0], m_pointData[1], m_pointData[2]);
    m_boundingBoxMax = m_boundingBoxMin;

    // Find min/max for each axis
    for (size_t i = 0; i < m_pointData.size(); i += 3) {
        QVector3D point(m_pointData[i], m_pointData[i + 1], m_pointData[i + 2]);

        m_boundingBoxMin.setX(std::min(m_boundingBoxMin.x(), point.x()));
        m_boundingBoxMin.setY(std::min(m_boundingBoxMin.y(), point.y()));
        m_boundingBoxMin.setZ(std::min(m_boundingBoxMin.z(), point.z()));

        m_boundingBoxMax.setX(std::max(m_boundingBoxMax.x(), point.x()));
        m_boundingBoxMax.setY(std::max(m_boundingBoxMax.y(), point.y()));
        m_boundingBoxMax.setZ(std::max(m_boundingBoxMax.z(), point.z()));
    }

    // Calculate center and size
    m_boundingBoxCenter = (m_boundingBoxMin + m_boundingBoxMax) * 0.5f;
    QVector3D size = m_boundingBoxMax - m_boundingBoxMin;
    m_boundingBoxSize = std::max({size.x(), size.y(), size.z()});

    if (m_boundingBoxSize < 0.001f) {
        m_boundingBoxSize = 1.0f; // Prevent division by zero
    }
}

void PointCloudViewerWidget::fitCameraToPointCloud()
{
    if (m_boundingBoxSize < 0.001f) {
        return; // No valid bounding box
    }

    // Set camera target to bounding box center
    m_cameraTarget = m_boundingBoxCenter;

    // Calculate optimal camera distance using field of view
    const float fov = 45.0f; // Field of view in degrees (matches projection matrix)
    const float aspect = static_cast<float>(width()) / static_cast<float>(height() ? height() : 1);

    // Calculate the maximum extent in any direction
    QVector3D size = m_boundingBoxMax - m_boundingBoxMin;
    float maxExtent = std::max({size.x(), size.y(), size.z()});

    // Add some padding (20% extra space around the object)
    maxExtent *= 1.2f;

    // Calculate distance needed to fit the object in view
    // For a perspective projection, distance = (extent/2) / tan(fov/2)
    float fovRadians = qDegreesToRadians(fov / 2.0f);
    float distance = (maxExtent / 2.0f) / std::tan(fovRadians);

    // Adjust for aspect ratio - use the smaller dimension to ensure everything fits
    if (aspect < 1.0f) {
        distance /= aspect;
    }

    // Set minimum distance to prevent getting too close
    distance = std::max(distance, maxExtent * 0.5f);

    m_cameraDistance = distance;

    // Reset camera angles for a good initial view
    m_cameraYaw = 0.0f;
    m_cameraPitch = 0.0f;

    qDebug() << "Camera fitted - Distance:" << m_cameraDistance
             << "Target:" << m_cameraTarget
             << "Max extent:" << maxExtent;
}

void PointCloudViewerWidget::updateCamera()
{
    // Calculate camera position based on spherical coordinates
    float x = m_cameraDistance * cos(m_cameraPitch) * cos(m_cameraYaw);
    float y = m_cameraDistance * sin(m_cameraPitch);
    float z = m_cameraDistance * cos(m_cameraPitch) * sin(m_cameraYaw);

    m_cameraPosition = m_cameraTarget + QVector3D(x, y, z);

    // Update view matrix
    m_viewMatrix.setToIdentity();
    m_viewMatrix.lookAt(m_cameraPosition, m_cameraTarget, m_cameraUp);

    update();
}

void PointCloudViewerWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePosition = event->pos();
    m_mousePressed = true;
    m_pressedButton = event->button();
}

void PointCloudViewerWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_mousePressed) {
        return;
    }

    QPoint delta = event->pos() - m_lastMousePosition;
    m_lastMousePosition = event->pos();

    const float sensitivity = 0.01f;

    if (m_pressedButton == Qt::LeftButton) {
        // Orbit camera
        m_cameraYaw += delta.x() * sensitivity;
        m_cameraPitch -= delta.y() * sensitivity;

        // Clamp pitch to prevent flipping
        m_cameraPitch = std::max(-static_cast<float>(M_PI)/2.0f + 0.1f, std::min(static_cast<float>(M_PI)/2.0f - 0.1f, m_cameraPitch));

        updateCamera();
    } else if (m_pressedButton == Qt::RightButton) {
        // Pan camera
        QVector3D right = QVector3D::crossProduct(m_cameraTarget - m_cameraPosition, m_cameraUp).normalized();
        QVector3D up = QVector3D::crossProduct(right, m_cameraTarget - m_cameraPosition).normalized();

        float panSpeed = m_boundingBoxSize * 0.001f;
        QVector3D panOffset = (right * -delta.x() + up * delta.y()) * panSpeed;

        m_cameraTarget += panOffset;
        updateCamera();
    }
}

void PointCloudViewerWidget::wheelEvent(QWheelEvent *event)
{
    const float zoomSpeed = 0.1f;
    float zoomFactor = 1.0f + (event->angleDelta().y() / 120.0f) * zoomSpeed;

    m_cameraDistance *= zoomFactor;
    m_cameraDistance = std::max(0.1f, std::min(m_boundingBoxSize * 10.0f, m_cameraDistance));

    updateCamera();
}

// View control methods
void PointCloudViewerWidget::setTopView()
{
    // Top view: Camera directly above target, looking down
    m_cameraYaw = 0.0f;
    m_cameraPitch = static_cast<float>(M_PI) / 2.0f - 0.1f; // Almost 90 degrees, avoid singularity
    m_cameraUp = QVector3D(0.0f, 0.0f, -1.0f); // Z-axis points forward in top view
    updateCamera();
}

void PointCloudViewerWidget::setLeftView()
{
    // Left view: Camera to the left of target, looking right
    m_cameraYaw = -static_cast<float>(M_PI) / 2.0f; // -90 degrees
    m_cameraPitch = 0.0f;
    m_cameraUp = QVector3D(0.0f, 1.0f, 0.0f); // Y-axis points up
    updateCamera();
}

void PointCloudViewerWidget::setRightView()
{
    // Right view: Camera to the right of target, looking left
    m_cameraYaw = static_cast<float>(M_PI) / 2.0f; // 90 degrees
    m_cameraPitch = 0.0f;
    m_cameraUp = QVector3D(0.0f, 1.0f, 0.0f); // Y-axis points up
    updateCamera();
}

void PointCloudViewerWidget::setBottomView()
{
    // Bottom view: Camera directly below target, looking up
    m_cameraYaw = 0.0f;
    m_cameraPitch = -static_cast<float>(M_PI) / 2.0f + 0.1f; // Almost -90 degrees, avoid singularity
    m_cameraUp = QVector3D(0.0f, 0.0f, 1.0f); // Z-axis points forward in bottom view
    updateCamera();
}

// UCS implementation
void PointCloudViewerWidget::setupUCSShaders()
{
    m_ucsShaderProgram = new QOpenGLShaderProgram(this);

    // UCS Vertex shader - simple line rendering
    const char* ucsVertexShaderSource = R"(
        #version 330 core

        layout (location = 0) in vec3 position;
        layout (location = 1) in vec3 color;

        uniform mat4 mvpMatrix;

        out vec3 vertexColor;

        void main()
        {
            gl_Position = mvpMatrix * vec4(position, 1.0);
            vertexColor = color;
        }
    )";

    // UCS Fragment shader
    const char* ucsFragmentShaderSource = R"(
        #version 330 core

        in vec3 vertexColor;
        out vec4 fragColor;

        void main()
        {
            fragColor = vec4(vertexColor, 1.0);
        }
    )";

    // Compile and link UCS shaders
    if (!m_ucsShaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, ucsVertexShaderSource)) {
        qCritical() << "Failed to compile UCS vertex shader:" << m_ucsShaderProgram->log();
        return;
    }

    if (!m_ucsShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, ucsFragmentShaderSource)) {
        qCritical() << "Failed to compile UCS fragment shader:" << m_ucsShaderProgram->log();
        return;
    }

    if (!m_ucsShaderProgram->link()) {
        qCritical() << "Failed to link UCS shader program:" << m_ucsShaderProgram->log();
        return;
    }

    // Get UCS uniform locations
    m_ucsMvpMatrixLocation = m_ucsShaderProgram->uniformLocation("mvpMatrix");

    if (m_ucsMvpMatrixLocation == -1) {
        qWarning() << "Failed to get UCS uniform locations";
    }

    qDebug() << "UCS shaders compiled and linked successfully";
}

void PointCloudViewerWidget::setupUCSBuffers()
{
    // Create UCS VAO
    if (!m_ucsVertexArrayObject.create()) {
        qCritical() << "Failed to create UCS VAO";
        return;
    }

    // Create UCS VBO
    if (!m_ucsVertexBuffer.create()) {
        qCritical() << "Failed to create UCS VBO";
        return;
    }

    // Define UCS axes data (position + color)
    // Each axis: origin to endpoint, with color
    // X-axis: Red (1,0,0), Y-axis: Green (0,1,0), Z-axis: Blue (0,0,1)
    float ucsVertices[] = {
        // X-axis (Red)
        0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Origin, Red
        1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // X endpoint, Red

        // Y-axis (Green)
        0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Origin, Green
        0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Y endpoint, Green

        // Z-axis (Blue)
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Origin, Blue
        0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f   // Z endpoint, Blue
    };

    // Upload UCS data to GPU
    m_ucsVertexArrayObject.bind();
    m_ucsVertexBuffer.bind();
    m_ucsVertexBuffer.allocate(ucsVertices, sizeof(ucsVertices));

    // Set vertex attributes for UCS
    // Position attribute (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), nullptr);

    // Color attribute (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    m_ucsVertexBuffer.release();
    m_ucsVertexArrayObject.release();

    qDebug() << "UCS buffers created successfully";
}

void PointCloudViewerWidget::drawUCS()
{
    if (!m_ucsShaderProgram || m_ucsMvpMatrixLocation == -1) {
        return;
    }

    // Save current OpenGL state
    GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    GLfloat lineWidth;
    glGetFloatv(GL_LINE_WIDTH, &lineWidth);

    // Configure OpenGL for UCS rendering
    glDisable(GL_DEPTH_TEST); // UCS should always be visible
    glLineWidth(3.0f); // Make UCS lines thicker

    // Use UCS shader program
    if (!m_ucsShaderProgram->bind()) {
        qWarning() << "Failed to bind UCS shader program";
        return;
    }

    // Calculate UCS transformation matrix
    // Position UCS in top-right corner of screen
    QMatrix4x4 ucsProjectionMatrix;
    QMatrix4x4 ucsViewMatrix;
    QMatrix4x4 ucsModelMatrix;

    // Create orthographic projection for screen-space positioning
    float aspectRatio = static_cast<float>(width()) / static_cast<float>(height() ? height() : 1);
    ucsProjectionMatrix.ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -10.0f, 10.0f);

    // Extract rotation from current view matrix (remove translation)
    QMatrix4x4 rotationMatrix = m_viewMatrix;
    rotationMatrix.setColumn(3, QVector4D(0, 0, 0, 1)); // Remove translation

    // Position UCS in top-right corner
    ucsModelMatrix.translate(aspectRatio * 0.7f, 0.7f, 0.0f); // Top-right corner
    ucsModelMatrix.scale(0.15f); // Scale down the UCS

    // Apply only rotation from camera (not translation)
    ucsViewMatrix = rotationMatrix;

    // Calculate final MVP matrix for UCS
    QMatrix4x4 ucsMvpMatrix = ucsProjectionMatrix * ucsViewMatrix * ucsModelMatrix;

    // Set UCS uniforms
    m_ucsShaderProgram->setUniformValue(m_ucsMvpMatrixLocation, ucsMvpMatrix);

    // Bind UCS VAO and draw lines
    m_ucsVertexArrayObject.bind();
    glDrawArrays(GL_LINES, 0, 6); // 6 vertices (3 lines, 2 vertices each)
    m_ucsVertexArrayObject.release();

    m_ucsShaderProgram->release();

    // Restore OpenGL state
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    glLineWidth(lineWidth);
}
