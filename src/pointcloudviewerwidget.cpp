#include "pointcloudviewerwidget.h"
#include "performance_profiler.h"
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <QPainter>
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
    , m_globalOffset(0.0f, 0.0f, 0.0f)
    , m_boundingBoxMin(0.0f, 0.0f, 0.0f)
    , m_boundingBoxMax(0.0f, 0.0f, 0.0f)
    , m_boundingBoxCenter(0.0f, 0.0f, 0.0f)
    , m_boundingBoxSize(1.0f)
    , m_pointColor(1.0f, 1.0f, 1.0f)
    , m_pointSize(2.0f)
    , m_hasData(false)
    , m_shadersInitialized(false)
    , m_showErrorState(true)  // Sprint 1.3: Start in error state
    , m_errorMessage("No point cloud data loaded")
    , m_currentState(ViewerState::Idle)  // Sprint 2.3: Initialize state
    , m_loadingProgress(0)
    , m_loadingAngle(0)
    , m_lodEnabled(false)  // Sprint 3.4: Initialize LOD state
    , m_lodSubsampleRate(0.5f)
    , m_octree(std::make_unique<Octree>())  // Sprint R1: Initialize octree
    , m_lodDistance1(50.0f)  // Sprint R1: Close LOD distance
    , m_lodDistance2(200.0f)  // Sprint R1: Far LOD distance
    , m_primaryScreenSpaceErrorThreshold(50.0f)  // Sprint R2: Primary threshold (pixels)
    , m_cullScreenSpaceErrorThreshold(2.0f)      // Sprint R2: Cull threshold (pixels)
    , m_renderWithColor(false)                   // Sprint R3: Initialize attribute rendering
    , m_renderWithIntensity(false)
    , m_pointSizeAttenuationEnabled(false)
    , m_minPointSize(1.0f)
    , m_maxPointSize(10.0f)
    , m_attenuationFactor(0.1f)
    , m_fps(0.0f)
    , m_frameCount(0)
    , m_visiblePointCount(0)
{
    qDebug() << "PointCloudViewerWidget constructor started";
    setFocusPolicy(Qt::StrongFocus);

    // Initialize performance monitoring
    m_lastFrameTime = std::chrono::high_resolution_clock::now();

    // Initialize matrices
    m_modelMatrix.setToIdentity();
    m_viewMatrix.setToIdentity();
    m_projectionMatrix.setToIdentity();

    // Sprint 2.3: Setup loading animation timer
    m_loadingTimer = new QTimer(this);
    m_loadingTimer->setInterval(50); // 20 FPS animation
    connect(m_loadingTimer, &QTimer::timeout,
            this, &PointCloudViewerWidget::updateLoadingAnimation);

    // Setup fonts for overlay text
    m_overlayFont.setFamily("Arial");
    m_overlayFont.setPointSize(16);
    m_overlayFont.setBold(true);

    m_detailFont.setFamily("Arial");
    m_detailFont.setPointSize(12);

    // Initialize in idle state
    setState(ViewerState::Idle, "Ready to load point cloud files");

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

    // Sprint 2.3: Handle state-based rendering
    if (m_currentState == ViewerState::DisplayingData && m_hasData && m_shadersInitialized) {
        // Sprint R3: Use enhanced attribute rendering if color/intensity features are enabled
        if (m_renderWithColor || m_renderWithIntensity || m_pointSizeAttenuationEnabled) {
            renderWithAttributes();
        }
        // Sprint R2: Use screen-space error LOD if enabled, otherwise fall back to R1 or traditional
        else if (m_lodEnabled && m_octree && m_octree->root) {
            renderWithScreenSpaceErrorLOD();
        } else {
            // Fallback: traditional rendering
            qDebug() << "paintGL: Rendering" << m_pointCount << "points (traditional)";

            // Use shader program with error checking
            if (!m_shaderProgram->bind()) {
                qWarning() << "Failed to bind shader program";
                paintOverlayGL(); // Still show overlay even if shader fails
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

            // Bind VAO and draw points with error checking
            m_vertexArrayObject.bind();
            error = glGetError();
            if (error != GL_NO_ERROR) {
                qCritical() << "OpenGL Error after VAO bind:" << QString("0x%1").arg(error, 0, 16);
            }

            glDrawArrays(GL_POINTS, 0, m_pointCount);
            error = glGetError();
            if (error != GL_NO_ERROR) {
                qCritical() << "OpenGL Error after glDrawArrays:" << QString("0x%1").arg(error, 0, 16);
            }

            m_vertexArrayObject.release();
            m_shaderProgram->release();
        }

        // Draw UCS indicator
        drawUCS();

        // Update FPS counter
        updateFPS();
    } else if (m_showErrorState || !m_hasData) {
        // Sprint 1.3: Handle legacy error state rendering (Task 1.3.3.2)
        renderErrorState();
    }

    // Sprint 2.3: Always draw overlay for state feedback
    paintOverlayGL();
}

void PointCloudViewerWidget::setupShaders()
{
    m_shaderProgram = new QOpenGLShaderProgram(this);

    // Sprint R3: Enhanced vertex shader with attribute support (Tasks R3.1.2, R3.2.2, R3.3.1)
    const char* vertexShaderSource = R"(
        #version 330 core

        // Vertex attributes as specified in backlog Task R3.1.2, R3.2.2
        layout (location = 0) in vec3 position;
        layout (location = 1) in vec3 vertexColor;
        layout (location = 2) in float vertexIntensity;

        // Uniforms for transformation
        uniform mat4 mvpMatrix;

        // Uniforms for point size attenuation as specified in Task R3.3.1
        uniform vec3 cameraPosition_worldSpace;
        uniform float minPointSize;
        uniform float maxPointSize;
        uniform float attenuationFactor;
        uniform bool pointSizeAttenuationEnabled;
        uniform float basePointSize;

        // Outputs to fragment shader
        out vec3 fragVertexColor;
        out float fragVertexIntensity;

        void main() {
            gl_Position = mvpMatrix * vec4(position, 1.0);

            // Pass attributes to fragment shader
            fragVertexColor = vertexColor;
            fragVertexIntensity = vertexIntensity;

            // Point size attenuation calculation as specified in Task R3.3.1
            if (pointSizeAttenuationEnabled) {
                float distance = length(cameraPosition_worldSpace - position);
                float attenuatedSize = basePointSize / (1.0 + distance * attenuationFactor);
                gl_PointSize = clamp(attenuatedSize, minPointSize, maxPointSize);
            } else {
                gl_PointSize = basePointSize;
            }
        }
    )";

    // Sprint R3: Enhanced fragment shader with attribute rendering (Tasks R3.1.3, R3.2.3)
    const char* fragmentShaderSource = R"(
        #version 330 core

        // Inputs from vertex shader
        in vec3 fragVertexColor;
        in float fragVertexIntensity;

        // Uniforms for rendering control
        uniform bool renderWithColor;
        uniform bool renderWithIntensity;
        uniform vec3 uniformColor;

        out vec4 fragColor;

        void main() {
            vec3 finalColor = uniformColor;

            // Color rendering logic as specified in Task R3.1.3
            if (renderWithColor) {
                finalColor = fragVertexColor;
            }

            // Intensity rendering logic as specified in Task R3.2.3
            if (renderWithIntensity) {
                if (renderWithColor) {
                    // Modulate color with intensity
                    finalColor = fragVertexColor * fragVertexIntensity;
                } else {
                    // Grayscale intensity mapping
                    finalColor = vec3(fragVertexIntensity);
                }
            }

            // Create circular point shape with smooth edges
            vec2 coord = gl_PointCoord - vec2(0.5);
            float distance = length(coord);
            if (distance > 0.5) {
                discard;
            }

            float alpha = 1.0 - smoothstep(0.3, 0.5, distance);
            fragColor = vec4(finalColor, alpha);
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

    // Sprint R3: Setup enhanced vertex array object for attribute rendering (Task R3.1.4)
    setupEnhancedVertexArrayObject();

    qDebug() << "OpenGL buffers created successfully";
}

// Sprint R3: Enhanced vertex array object setup (as per backlog Task R3.1.4)
void PointCloudViewerWidget::setupEnhancedVertexArrayObject()
{
    m_vertexArrayObject.bind();
    m_vertexBuffer.bind();

    if (m_shaderProgram) {
        m_shaderProgram->bind();

        // Position attribute (location 0) - XYZ
        m_shaderProgram->enableAttributeArray(0);
        m_shaderProgram->setAttributeBuffer(0, GL_FLOAT,
            offsetof(VertexData, position), 3, sizeof(VertexData));

        // Color attribute (location 1) - RGB
        m_shaderProgram->enableAttributeArray(1);
        m_shaderProgram->setAttributeBuffer(1, GL_FLOAT,
            offsetof(VertexData, color), 3, sizeof(VertexData));

        // Intensity attribute (location 2) - I
        m_shaderProgram->enableAttributeArray(2);
        m_shaderProgram->setAttributeBuffer(2, GL_FLOAT,
            offsetof(VertexData, intensity), 1, sizeof(VertexData));

        m_shaderProgram->release();
    }

    m_vertexBuffer.release();
    m_vertexArrayObject.release();
}

void PointCloudViewerWidget::loadPointCloud(const std::vector<float>& points)
{
    PROFILE_FUNCTION();

    // Debug logging for data reception (User Story 1)
    qDebug() << "=== PointCloudViewerWidget::loadPointCloud ===";
    qDebug() << "Received points vector size:" << points.size();
    qDebug() << "Number of points:" << (points.size() / 3);

    if (points.empty() || points.size() % 3 != 0) {
        qWarning() << "Invalid point cloud data - empty or not divisible by 3";
        return;
    }

    makeCurrent();

    // First, calculate the global offset from the original data (User Story 3)
    QVector3D originalMin, originalMax;
    if (!points.empty()) {
        originalMin = QVector3D(points[0], points[1], points[2]);
        originalMax = originalMin;

        for (size_t i = 0; i < points.size(); i += 3) {
            if (i + 2 < points.size()) {
                QVector3D point(points[i], points[i + 1], points[i + 2]);
                originalMin.setX(std::min(originalMin.x(), point.x()));
                originalMin.setY(std::min(originalMin.y(), point.y()));
                originalMin.setZ(std::min(originalMin.z(), point.z()));
                originalMax.setX(std::max(originalMax.x(), point.x()));
                originalMax.setY(std::max(originalMax.y(), point.y()));
                originalMax.setZ(std::max(originalMax.z(), point.z()));
            }
        }

        // Calculate global offset as the center of the original bounding box
        m_globalOffset = (originalMin + originalMax) * 0.5f;

        qDebug() << "Original bounding box - Min:" << originalMin << "Max:" << originalMax;
        qDebug() << "Global offset calculated:" << m_globalOffset;

        // Apply coordinate transformation - center points around origin
        m_pointData = points; // Copy the data first
        for (size_t i = 0; i < m_pointData.size(); i += 3) {
            if (i + 2 < m_pointData.size()) {
                m_pointData[i] -= m_globalOffset.x();
                m_pointData[i + 1] -= m_globalOffset.y();
                m_pointData[i + 2] -= m_globalOffset.z();
            }
        }

        qDebug() << "Applied coordinate transformation - points centered around origin";
    } else {
        m_pointData = points;
        m_globalOffset = QVector3D(0, 0, 0);
    }

    m_pointCount = static_cast<int>(m_pointData.size() / 3);
    qDebug() << "Point count set to:" << m_pointCount;

    // Calculate bounding box (now on transformed coordinates)
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
    {
        PROFILE_SECTION("GPU::DataUpload");
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

        m_vertexBuffer.allocate(m_pointData.data(), static_cast<int>(m_pointData.size() * sizeof(float)));
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
    }

    m_hasData = true;
    qDebug() << "m_hasData set to true";

    // Sprint R1: Build octree for LOD system
    if (m_lodEnabled) {
        qDebug() << "Building octree for LOD system...";
        m_octree->buildFromFloatArray(m_pointData, 8, 100);
        qDebug() << "Octree built - Total points:" << m_octree->getTotalPointCount()
                 << "Max depth:" << m_octree->getMaxDepth()
                 << "Node count:" << m_octree->getNodeCount();
    }

    // Sprint 1.3: Clear error state when data is successfully loaded
    m_showErrorState = false;
    m_errorMessage.clear();

    doneCurrent();
    update(); // Trigger repaint

    qDebug() << "Point cloud loading completed successfully";
}

void PointCloudViewerWidget::clearPointCloud()
{
    // Task 1.3.3.2: Enhanced data clearing for Sprint 1.3
    qDebug() << "PointCloudViewerWidget::clearPointCloud() - Clearing all point cloud data";

    makeCurrent();

    // Clear all point data
    m_pointData.clear();
    m_pointCount = 0;
    m_hasData = false;

    // Reset bounding box
    m_boundingBoxMin = QVector3D(0.0f, 0.0f, 0.0f);
    m_boundingBoxMax = QVector3D(0.0f, 0.0f, 0.0f);
    m_boundingBoxCenter = QVector3D(0.0f, 0.0f, 0.0f);
    m_boundingBoxSize = 1.0f;

    // Reset global offset
    m_globalOffset = QVector3D(0.0f, 0.0f, 0.0f);

    // Set error state display
    m_showErrorState = true;
    m_errorMessage = "No point cloud data loaded";

    doneCurrent();
    update(); // Trigger repaint to show cleared state

    qDebug() << "PointCloudViewerWidget::clearPointCloud() - Data cleared, error state set";
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

// Sprint 1.3: Error state rendering implementation for Task 1.3.3.2
void PointCloudViewerWidget::renderErrorState()
{
    // Render centered text indicating no data loaded or error state
    // Since we're in an OpenGL context, we need to use QPainter overlay

    // Create a simple text rendering using OpenGL-compatible method
    // For now, just clear the screen with a different color to indicate error state
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Darker background for error state
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Note: For proper text rendering in OpenGL context, we would need to:
    // 1. Use texture-based text rendering, or
    // 2. Use QPainter with paintEvent override, or
    // 3. Use a text rendering library like FreeType
    // For now, the darker background serves as a visual indicator

    qDebug() << "renderErrorState: Displaying error state -" << m_errorMessage;
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

// Sprint 2.3: State management and visual feedback methods
void PointCloudViewerWidget::setState(ViewerState state, const QString &message)
{
    if (m_currentState != state) {
        m_currentState = state;
        m_stateMessage = message;

        switch (state) {
            case ViewerState::Loading:
                m_loadingProgress = 0;
                m_loadingStage = "Initializing...";
                m_loadingTimer->start();
                break;

            case ViewerState::DisplayingData:
                m_loadingTimer->stop();
                break;

            case ViewerState::LoadFailed:
                m_loadingTimer->stop();
                break;

            case ViewerState::Idle:
                m_loadingTimer->stop();
                break;
        }

        update(); // Trigger repaint
    }
}

void PointCloudViewerWidget::onLoadingStarted()
{
    setState(ViewerState::Loading, "Loading point cloud...");
}

void PointCloudViewerWidget::onLoadingProgress(int percentage, const QString &stage)
{
    m_loadingProgress = percentage;
    m_loadingStage = stage;
    update(); // Trigger repaint to update progress display
}

void PointCloudViewerWidget::onLoadingFinished(bool success, const QString &message,
                                              const std::vector<float> &points)
{
    if (success && !points.empty()) {
        setState(ViewerState::DisplayingData, message);
        loadPointCloud(points);
    } else {
        setState(ViewerState::LoadFailed, message);
    }
}

void PointCloudViewerWidget::updateLoadingAnimation()
{
    m_loadingAngle = (m_loadingAngle + 10) % 360;
    update(); // Trigger repaint for animation
}

void PointCloudViewerWidget::paintOverlayGL()
{
    // Use QPainter for text overlays on top of OpenGL content
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    switch (m_currentState) {
        case ViewerState::Loading:
            drawLoadingState(painter);
            break;

        case ViewerState::LoadFailed:
            drawLoadFailedState(painter);
            break;

        case ViewerState::Idle:
            drawIdleState(painter);
            break;

        case ViewerState::DisplayingData:
            // No overlay needed when displaying data
            break;
    }
}

void PointCloudViewerWidget::drawLoadingState(QPainter &painter)
{
    QRect rect = this->rect();
    QPoint center = rect.center();

    // Semi-transparent background
    painter.fillRect(rect, QColor(0, 0, 0, 100));

    // Draw loading spinner
    painter.setPen(QPen(QColor(100, 150, 255), 3));
    painter.setFont(m_overlayFont);

    const int spinnerRadius = 30;

    // Draw spinning arc
    QRect spinnerRect(center.x() - spinnerRadius, center.y() - spinnerRadius - 40,
                      spinnerRadius * 2, spinnerRadius * 2);

    painter.drawArc(spinnerRect, m_loadingAngle * 16, 120 * 16); // 120 degree arc

    // Draw main loading text
    painter.setPen(QColor(255, 255, 255));
    QRect textRect = rect;
    textRect.setTop(center.y() + 10);
    textRect.setHeight(30);

    painter.drawText(textRect, Qt::AlignCenter, "Loading Point Cloud...");

    // Draw progress information
    painter.setFont(m_detailFont);
    QRect progressRect = rect;
    progressRect.setTop(center.y() + 50);
    progressRect.setHeight(20);

    QString progressText = QString("%1% - %2").arg(m_loadingProgress).arg(m_loadingStage);
    painter.drawText(progressRect, Qt::AlignCenter, progressText);

    // Draw progress bar
    const int progressBarWidth = 300;
    const int progressBarHeight = 6;
    QRect progressBarRect(center.x() - progressBarWidth/2, center.y() + 80,
                         progressBarWidth, progressBarHeight);

    // Background
    painter.fillRect(progressBarRect, QColor(70, 70, 70));

    // Progress fill
    QRect fillRect = progressBarRect;
    fillRect.setWidth((progressBarWidth * m_loadingProgress) / 100);
    painter.fillRect(fillRect, QColor(100, 150, 255));

    // Progress bar border
    painter.setPen(QColor(150, 150, 150));
    painter.drawRect(progressBarRect);
}

void PointCloudViewerWidget::drawLoadFailedState(QPainter &painter)
{
    QRect rect = this->rect();
    QPoint center = rect.center();

    // Semi-transparent red background
    painter.fillRect(rect, QColor(100, 0, 0, 80));

    // Draw error icon (simple X)
    painter.setPen(QPen(QColor(255, 100, 100), 4));
    const int iconSize = 40;
    QRect iconRect(center.x() - iconSize/2, center.y() - iconSize/2 - 40,
                   iconSize, iconSize);

    painter.drawLine(iconRect.topLeft(), iconRect.bottomRight());
    painter.drawLine(iconRect.topRight(), iconRect.bottomLeft());

    // Draw main error text
    painter.setPen(QColor(255, 255, 255));
    painter.setFont(m_overlayFont);

    QRect textRect = rect;
    textRect.setTop(center.y() + 10);
    textRect.setHeight(30);

    painter.drawText(textRect, Qt::AlignCenter, "Failed to Load File");

    // Draw error details
    painter.setFont(m_detailFont);
    QRect detailRect = rect;
    detailRect.setTop(center.y() + 50);
    detailRect.setHeight(60);
    detailRect.adjust(20, 0, -20, 0); // Add margins

    painter.drawText(detailRect, Qt::AlignCenter | Qt::TextWordWrap, m_stateMessage);
}

void PointCloudViewerWidget::drawIdleState(QPainter &painter)
{
    QRect rect = this->rect();
    QPoint center = rect.center();

    // Light background
    painter.fillRect(rect, QColor(50, 50, 50, 50));

    // Draw file icon (simple representation)
    painter.setPen(QPen(QColor(150, 150, 150), 2));
    painter.setBrush(QBrush(QColor(200, 200, 200, 100)));

    const int iconWidth = 60;
    const int iconHeight = 80;
    QRect iconRect(center.x() - iconWidth/2, center.y() - iconHeight/2 - 20,
                   iconWidth, iconHeight);

    painter.drawRoundedRect(iconRect, 5, 5);

    // Draw some lines to represent file content
    painter.setPen(QColor(150, 150, 150));
    for (int i = 0; i < 4; ++i) {
        int lineY = iconRect.top() + 20 + i * 12;
        int lineWidth = (i == 3) ? iconWidth / 2 : iconWidth - 20;
        painter.drawLine(iconRect.left() + 10, lineY,
                        iconRect.left() + 10 + lineWidth, lineY);
    }

    // Draw main text
    painter.setPen(QColor(200, 200, 200));
    painter.setFont(m_overlayFont);

    QRect textRect = rect;
    textRect.setTop(center.y() + 50);
    textRect.setHeight(30);

    painter.drawText(textRect, Qt::AlignCenter, "Ready to Load Point Cloud");

    // Draw instruction text
    painter.setFont(m_detailFont);
    QRect instructionRect = rect;
    instructionRect.setTop(center.y() + 90);
    instructionRect.setHeight(40);

    painter.drawText(instructionRect, Qt::AlignCenter,
                    "Click 'Open File' to load E57 or LAS files");
}

// Sprint 3.2: Test simulation methods implementation
void PointCloudViewerWidget::simulateOrbitCamera(const QPoint &start, const QPoint &end)
{
    QPoint delta = end - start;
    const float sensitivity = 0.01f;

    m_cameraYaw += delta.x() * sensitivity;
    m_cameraPitch -= delta.y() * sensitivity;

    // Clamp pitch to prevent flipping
    m_cameraPitch = std::max(-static_cast<float>(M_PI)/2.0f + 0.1f,
                            std::min(static_cast<float>(M_PI)/2.0f - 0.1f, m_cameraPitch));

    updateCamera();
}

void PointCloudViewerWidget::simulatePanCamera(const QPoint &start, const QPoint &end)
{
    QPoint delta = end - start;

    // Calculate camera right and up vectors
    QVector3D right = QVector3D::crossProduct(m_cameraTarget - m_cameraPosition, m_cameraUp).normalized();
    QVector3D up = QVector3D::crossProduct(right, m_cameraTarget - m_cameraPosition).normalized();

    float panSpeed = m_boundingBoxSize * 0.001f;
    QVector3D panOffset = (right * -delta.x() + up * delta.y()) * panSpeed;

    m_cameraTarget += panOffset;
    updateCamera();
}

void PointCloudViewerWidget::simulateZoomCamera(float factor)
{
    m_cameraDistance *= factor;
    m_cameraDistance = std::max(0.1f, std::min(m_boundingBoxSize * 10.0f, m_cameraDistance));

    updateCamera();
}

// Sprint 3.4: LOD control slot implementations
void PointCloudViewerWidget::toggleLOD(bool enabled)
{
    m_lodEnabled = enabled;
    qDebug() << "LOD toggled:" << (enabled ? "enabled" : "disabled");

    // Trigger repaint to show LOD changes
    update();
}

void PointCloudViewerWidget::setLODSubsampleRate(float rate)
{
    m_lodSubsampleRate = qBound(0.1f, rate, 1.0f);
    qDebug() << "LOD subsample rate set to:" << m_lodSubsampleRate;

    // If LOD is currently enabled, trigger repaint
    if (m_lodEnabled) {
        update();
    }
}

// Sprint R1: Advanced LOD system implementation
void PointCloudViewerWidget::setLODEnabled(bool enabled)
{
    m_lodEnabled = enabled;
    qDebug() << "Advanced LOD system:" << (enabled ? "enabled" : "disabled");

    // Sprint R1: Build octree when LOD is enabled
    if (enabled && m_hasData && !m_pointData.empty()) {
        qDebug() << "Building octree for LOD system...";
        m_octree->buildFromFloatArray(m_pointData, 8, 100);
        qDebug() << "Octree built - Total points:" << m_octree->getTotalPointCount()
                 << "Max depth:" << m_octree->getMaxDepth()
                 << "Node count:" << m_octree->getNodeCount();
    }

    update(); // Trigger repaint
}

void PointCloudViewerWidget::setLODDistances(float distance1, float distance2)
{
    m_lodDistance1 = distance1;
    m_lodDistance2 = distance2;
    qDebug() << "LOD distances set - Close:" << distance1 << "Far:" << distance2;
    update();
}

void PointCloudViewerWidget::getLODDistances(float& distance1, float& distance2) const
{
    distance1 = m_lodDistance1;
    distance2 = m_lodDistance2;
}

size_t PointCloudViewerWidget::getOctreeNodeCount() const
{
    return m_octree ? m_octree->getNodeCount() : 0;
}

// Sprint R2: Screen-space error LOD control slot implementations
void PointCloudViewerWidget::setScreenSpaceErrorThreshold(float threshold)
{
    m_primaryScreenSpaceErrorThreshold = threshold;
    qDebug() << "Screen-space error threshold set to:" << threshold;
    update();
}

void PointCloudViewerWidget::setPrimaryScreenSpaceErrorThreshold(float threshold)
{
    m_primaryScreenSpaceErrorThreshold = threshold;
    qDebug() << "Primary screen-space error threshold set to:" << threshold;
    update();
}

void PointCloudViewerWidget::setCullScreenSpaceErrorThreshold(float threshold)
{
    m_cullScreenSpaceErrorThreshold = threshold;
    qDebug() << "Cull screen-space error threshold set to:" << threshold;
    update();
}

void PointCloudViewerWidget::renderOctree()
{
    if (!m_octree || !m_octree->root) {
        return;
    }

    // Extract frustum planes from view-projection matrix
    QMatrix4x4 viewProjection = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
    auto frustumPlanes = extractFrustumPlanes(viewProjection);

    // Clear previous visible points
    m_visiblePoints.clear();

    // Collect visible points using octree traversal
    m_octree->getVisiblePoints(frustumPlanes, m_cameraPosition,
                              m_lodDistance1, m_lodDistance2, m_visiblePoints);

    m_visiblePointCount = m_visiblePoints.size();

    if (m_visiblePoints.empty()) {
        return;
    }

    qDebug() << "Octree rendering - Visible points:" << m_visiblePointCount
             << "out of" << m_octree->getTotalPointCount();

    // Convert PointFullData to flat float array for OpenGL
    std::vector<float> renderData;
    renderData.reserve(m_visiblePoints.size() * 3);

    for (const auto& point : m_visiblePoints) {
        renderData.push_back(point.x);
        renderData.push_back(point.y);
        renderData.push_back(point.z);
    }

    // Use shader program with error checking
    if (!m_shaderProgram->bind()) {
        qWarning() << "Failed to bind shader program in octree rendering";
        return;
    }

    // Set uniforms
    m_shaderProgram->setUniformValue(m_mvpMatrixLocation, viewProjection);
    m_shaderProgram->setUniformValue(m_colorLocation, m_pointColor);
    m_shaderProgram->setUniformValue(m_pointSizeLocation, m_pointSize);

    // Create temporary VBO for visible points
    QOpenGLBuffer tempBuffer(QOpenGLBuffer::VertexBuffer);
    if (!tempBuffer.create()) {
        qWarning() << "Failed to create temporary VBO for octree rendering";
        m_shaderProgram->release();
        return;
    }

    tempBuffer.bind();
    tempBuffer.allocate(renderData.data(), static_cast<int>(renderData.size() * sizeof(float)));

    // Set vertex attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    // Draw points
    glDrawArrays(GL_POINTS, 0, static_cast<int>(m_visiblePoints.size()));

    // Cleanup
    glDisableVertexAttribArray(0);
    tempBuffer.release();
    m_shaderProgram->release();
}

std::array<QVector4D, 6> PointCloudViewerWidget::extractFrustumPlanes(const QMatrix4x4& viewProjection) const
{
    return FrustumUtils::extractFrustumPlanes(viewProjection);
}

void PointCloudViewerWidget::updateFPS()
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_lastFrameTime);

    m_frameCount++;
    if (deltaTime.count() >= 1000000) { // Update every second
        m_fps = m_frameCount * 1000000.0f / deltaTime.count();
        m_frameCount = 0;
        m_lastFrameTime = currentTime;

        // Debug output for performance monitoring
        if (m_lodEnabled && m_octree && m_octree->root) {
            qDebug() << "FPS:" << QString::number(m_fps, 'f', 1)
                     << "Visible points:" << m_visiblePointCount
                     << "Total points:" << m_octree->getTotalPointCount();
        }
    }
}

// Sprint R2: Enhanced rendering methods implementation
void PointCloudViewerWidget::renderWithScreenSpaceErrorLOD()
{
    if (!m_octree || !m_octree->root) {
        return;
    }

    updateViewportInfo();

    // Extract frustum planes from view-projection matrix
    QMatrix4x4 viewProjection = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
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

    m_visiblePointCount = m_visiblePoints.size();

    if (m_visiblePoints.empty()) {
        return;
    }

    // Log statistics for debugging
    logLODStatistics(m_visiblePoints);

    // Convert PointFullData to flat float array for OpenGL
    std::vector<float> renderData;
    renderData.reserve(m_visiblePoints.size() * 3);

    for (const auto& point : m_visiblePoints) {
        renderData.push_back(point.x);
        renderData.push_back(point.y);
        renderData.push_back(point.z);
    }

    // Use shader program with error checking
    if (!m_shaderProgram->bind()) {
        qWarning() << "Failed to bind shader program in screen-space error LOD rendering";
        return;
    }

    // Set uniforms
    m_shaderProgram->setUniformValue(m_mvpMatrixLocation, viewProjection);
    m_shaderProgram->setUniformValue(m_colorLocation, m_pointColor);
    m_shaderProgram->setUniformValue(m_pointSizeLocation, m_pointSize);

    // Create temporary VBO for visible points
    QOpenGLBuffer tempBuffer(QOpenGLBuffer::VertexBuffer);
    if (!tempBuffer.create()) {
        qWarning() << "Failed to create temporary VBO for screen-space error LOD rendering";
        m_shaderProgram->release();
        return;
    }

    tempBuffer.bind();
    tempBuffer.allocate(renderData.data(), static_cast<int>(renderData.size() * sizeof(float)));

    // Set vertex attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    // Draw points
    glDrawArrays(GL_POINTS, 0, static_cast<int>(m_visiblePoints.size()));

    // Cleanup
    glDisableVertexAttribArray(0);
    tempBuffer.release();
    m_shaderProgram->release();
}

void PointCloudViewerWidget::updateViewportInfo()
{
    m_viewportInfo.width = width();
    m_viewportInfo.height = height();
    m_viewportInfo.nearPlane = 0.1f;  // Should match your projection matrix
    m_viewportInfo.farPlane = 1000.0f; // Should match your projection matrix
}

void PointCloudViewerWidget::logLODStatistics(const std::vector<PointFullData>& visiblePoints)
{
    static int frameCount = 0;
    frameCount++;

    if (frameCount % 60 == 0) { // Log every 60 frames
        qDebug() << "Sprint R2 LOD Statistics:"
                 << "Visible points:" << visiblePoints.size()
                 << "Total points:" << (m_octree ? m_octree->getTotalPointCount() : 0)
                 << "Primary threshold:" << m_primaryScreenSpaceErrorThreshold
                 << "Cull threshold:" << m_cullScreenSpaceErrorThreshold
                 << "FPS:" << QString::number(m_fps, 'f', 1);
    }
}

// Sprint R3: Enhanced rendering methods implementation (as per backlog Tasks R3.1.4, R3.1.5)
void PointCloudViewerWidget::renderWithAttributes()
{
    if (!m_shaderProgram || m_vertexData.empty()) return;

    // Collect visible points using LOD system (integrating with R1/R2 as specified in backlog)
    QMatrix4x4 viewProjection = m_projectionMatrix * m_viewMatrix * m_modelMatrix;
    auto frustumPlanes = extractFrustumPlanes(viewProjection);

    m_visiblePoints.clear();
    if (m_octree && m_octree->root) {
        m_octree->getVisiblePoints(frustumPlanes, m_cameraPosition,
                                  m_lodDistance1, m_lodDistance2, m_visiblePoints);
    }

    if (m_visiblePoints.empty()) return;

    // Prepare vertex data for visible points
    prepareVertexData(m_visiblePoints);

    // Bind shader and set uniforms as specified in Task R3.1.5, R3.3.2
    if (!m_shaderProgram->bind()) {
        qWarning() << "Failed to bind enhanced shader program";
        return;
    }

    // Set transformation uniforms
    m_shaderProgram->setUniformValue("mvpMatrix", viewProjection);

    // Set camera position for point size attenuation (Task R3.3.2)
    m_shaderProgram->setUniformValue("cameraPosition_worldSpace", m_cameraPosition);

    // Set rendering mode flags (Tasks R3.1.6, R3.2.5)
    m_shaderProgram->setUniformValue("renderWithColor", m_renderWithColor);
    m_shaderProgram->setUniformValue("renderWithIntensity", m_renderWithIntensity);

    // Set point size attenuation parameters (Task R3.3.3)
    m_shaderProgram->setUniformValue("pointSizeAttenuationEnabled", m_pointSizeAttenuationEnabled);
    m_shaderProgram->setUniformValue("basePointSize", m_pointSize);
    m_shaderProgram->setUniformValue("minPointSize", m_minPointSize);
    m_shaderProgram->setUniformValue("maxPointSize", m_maxPointSize);
    m_shaderProgram->setUniformValue("attenuationFactor", m_attenuationFactor);

    // Set uniform color for fallback
    m_shaderProgram->setUniformValue("uniformColor", m_pointColor);

    // Render points
    m_vertexArrayObject.bind();
    glDrawArrays(GL_POINTS, 0, static_cast<int>(m_vertexData.size()));
    m_vertexArrayObject.release();

    m_shaderProgram->release();
}

void PointCloudViewerWidget::prepareVertexData(const std::vector<PointFullData>& points)
{
    m_vertexData.clear();
    m_vertexData.reserve(points.size());

    for (const auto& point : points) {
        m_vertexData.emplace_back(point);
    }

    // Update VBO with new vertex data (interleaved X,Y,Z,R,G,B,I as per backlog Task R3.1.4)
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(m_vertexData.data(),
                           static_cast<int>(m_vertexData.size() * sizeof(VertexData)));
    m_vertexBuffer.release();
}

// Sprint R3: Attribute rendering and point size attenuation slot implementations (as per backlog)
void PointCloudViewerWidget::setRenderWithColor(bool enabled)
{
    m_renderWithColor = enabled;
    qDebug() << "Color rendering:" << (enabled ? "enabled" : "disabled");
    update();
}

void PointCloudViewerWidget::setRenderWithIntensity(bool enabled)
{
    m_renderWithIntensity = enabled;
    qDebug() << "Intensity rendering:" << (enabled ? "enabled" : "disabled");
    update();
}

void PointCloudViewerWidget::setPointSizeAttenuationEnabled(bool enabled)
{
    m_pointSizeAttenuationEnabled = enabled;
    qDebug() << "Point size attenuation:" << (enabled ? "enabled" : "disabled");
    update();
}

void PointCloudViewerWidget::setPointSizeAttenuationParams(float minSize, float maxSize, float factor)
{
    m_minPointSize = minSize;
    m_maxPointSize = maxSize;
    m_attenuationFactor = factor;
    qDebug() << "Point size attenuation params - Min:" << minSize
             << "Max:" << maxSize << "Factor:" << factor;
    update();
}
