#include "OpenGLRenderer.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QOpenGLShader>

OpenGLRenderer::OpenGLRenderer()
    : m_vertexBuffer(QOpenGLBuffer::VertexBuffer)
    , m_mvpMatrixLocation(-1)
    , m_colorLocation(-1)
    , m_pointSizeLocation(-1)
    , m_initialized(false)
    , m_shadersReady(false)
    , m_pointCount(0)
{
}

OpenGLRenderer::~OpenGLRenderer()
{
    clearData();
}

bool OpenGLRenderer::initialize()
{
    if (m_initialized) {
        return true;
    }

    if (!initializeOpenGLFunctions()) {
        m_lastError = "Failed to initialize OpenGL functions";
        qCritical() << m_lastError;
        return false;
    }

    // Create vertex buffer
    if (!m_vertexBuffer.create()) {
        m_lastError = "Failed to create vertex buffer";
        qCritical() << m_lastError;
        return false;
    }

    // Create vertex array object
    if (!m_vertexArrayObject.create()) {
        m_lastError = "Failed to create vertex array object";
        qCritical() << m_lastError;
        return false;
    }

    m_initialized = true;
    qDebug() << "OpenGLRenderer initialized successfully";
    return true;
}

bool OpenGLRenderer::loadShaders(const QString& vertexShaderPath, const QString& fragmentShaderPath)
{
    if (!m_initialized) {
        m_lastError = "Renderer not initialized";
        return false;
    }

    // Create shader program
    m_shaderProgram = std::make_unique<QOpenGLShaderProgram>();

    // Compile vertex shader
    if (!compileShaderFromFile(QOpenGLShader::Vertex, vertexShaderPath)) {
        return false;
    }

    // Compile fragment shader
    if (!compileShaderFromFile(QOpenGLShader::Fragment, fragmentShaderPath)) {
        return false;
    }

    // Link shader program
    if (!m_shaderProgram->link()) {
        m_lastError = QString("Failed to link shader program: %1").arg(m_shaderProgram->log());
        qCritical() << m_lastError;
        return false;
    }

    // Get uniform locations
    m_mvpMatrixLocation = m_shaderProgram->uniformLocation("mvpMatrix");
    m_colorLocation = m_shaderProgram->uniformLocation("color");
    m_pointSizeLocation = m_shaderProgram->uniformLocation("pointSize");

    if (m_mvpMatrixLocation == -1 || m_colorLocation == -1 || m_pointSizeLocation == -1) {
        m_lastError = "Failed to get uniform locations";
        qWarning() << m_lastError;
        qDebug() << "MVP location:" << m_mvpMatrixLocation;
        qDebug() << "Color location:" << m_colorLocation;
        qDebug() << "Point size location:" << m_pointSizeLocation;
    }

    setupVertexArrayObject();
    m_shadersReady = true;
    qDebug() << "Shaders loaded and linked successfully";
    return true;
}

bool OpenGLRenderer::uploadPointData(const std::vector<float>& points)
{
    if (!m_initialized || !m_shadersReady) {
        m_lastError = "Renderer not ready for data upload";
        return false;
    }

    if (points.empty()) {
        m_lastError = "No point data provided";
        return false;
    }

    if (points.size() % 3 != 0) {
        m_lastError = "Point data size must be multiple of 3 (x,y,z coordinates)";
        return false;
    }

    m_pointCount = static_cast<int>(points.size() / 3);

    // Bind vertex buffer and upload data
    m_vertexBuffer.bind();
    m_vertexBuffer.allocate(points.data(), static_cast<int>(points.size() * sizeof(float)));
    m_vertexBuffer.release();

    qDebug() << "Uploaded" << m_pointCount << "points to GPU";
    return true;
}

void OpenGLRenderer::render(const QMatrix4x4& mvpMatrix, const QVector3D& pointColor, float pointSize)
{
    if (!m_initialized || !m_shadersReady || m_pointCount == 0) {
        return;
    }

    // Bind shader program
    if (!m_shaderProgram->bind()) {
        qWarning() << "Failed to bind shader program for rendering";
        return;
    }

    // Set uniforms
    m_shaderProgram->setUniformValue(m_mvpMatrixLocation, mvpMatrix);
    m_shaderProgram->setUniformValue(m_colorLocation, pointColor);
    m_shaderProgram->setUniformValue(m_pointSizeLocation, pointSize);

    // Bind VAO and render
    m_vertexArrayObject.bind();
    glDrawArrays(GL_POINTS, 0, m_pointCount);
    logOpenGLError("glDrawArrays");
    
    m_vertexArrayObject.release();
    m_shaderProgram->release();
}

void OpenGLRenderer::clearData()
{
    m_pointCount = 0;
    if (m_vertexBuffer.isCreated()) {
        m_vertexBuffer.release();
    }
}

bool OpenGLRenderer::compileShaderFromFile(QOpenGLShader::ShaderType type, const QString& filePath)
{
    QString shaderSource = readShaderFile(filePath);
    if (shaderSource.isEmpty()) {
        m_lastError = QString("Failed to read shader file: %1").arg(filePath);
        return false;
    }

    if (!m_shaderProgram->addShaderFromSourceCode(type, shaderSource)) {
        m_lastError = QString("Failed to compile shader %1: %2").arg(filePath, m_shaderProgram->log());
        qCritical() << m_lastError;
        return false;
    }

    qDebug() << "Compiled shader:" << filePath;
    return true;
}

void OpenGLRenderer::setupVertexArrayObject()
{
    m_vertexArrayObject.bind();
    m_vertexBuffer.bind();

    // Setup vertex attributes (position only for now)
    m_shaderProgram->enableAttributeArray(0);
    m_shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3);

    m_vertexArrayObject.release();
    m_vertexBuffer.release();
}

void OpenGLRenderer::logOpenGLError(const QString& operation)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        QString errorMsg = QString("OpenGL error in %1: 0x%2").arg(operation).arg(error, 0, 16);
        qWarning() << errorMsg;
        m_lastError = errorMsg;
    }
}

QString OpenGLRenderer::readShaderFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Cannot open shader file:" << filePath;
        return QString();
    }

    QTextStream stream(&file);
    return stream.readAll();
}
