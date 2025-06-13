#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QVector3D>

#include <memory>
#include <vector>

/**
 * @brief OpenGLRenderer - Core OpenGL rendering engine for point clouds
 *
 * This class encapsulates the core OpenGL rendering logic, abstracting away
 * the low-level API calls from the main viewer widget. It is responsible for
 * managing the lifecycle of OpenGL resources.
 *
 * Sprint 1 Requirements:
 * - Shader management and compilation
 * - VAO/VBO handling and lifecycle management
 * - Rendering pipeline coordination
 * - MVP matrix handling
 * - Point cloud rendering with configurable parameters
 */
class OpenGLRenderer : protected QOpenGLFunctions
{
public:
    /**
     * @brief Constructor
     */
    OpenGLRenderer();

    /**
     * @brief Destructor - cleans up OpenGL resources
     */
    ~OpenGLRenderer();

    /**
     * @brief Initialize the OpenGL renderer
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Load and compile shaders from files
     * @param vertexShaderPath Path to vertex shader file
     * @param fragmentShaderPath Path to fragment shader file
     * @return true if shaders compiled and linked successfully
     */
    bool loadShaders(const QString& vertexShaderPath, const QString& fragmentShaderPath);

    /**
     * @brief Upload point cloud data to GPU
     * @param points Vector of point data (x,y,z coordinates)
     * @return true if upload successful
     */
    bool uploadPointData(const std::vector<float>& points);

    /**
     * @brief Render the point cloud
     * @param mvpMatrix Model-View-Projection matrix
     * @param pointColor Color for rendering points
     * @param pointSize Size of rendered points
     */
    void render(const QMatrix4x4& mvpMatrix, const QVector3D& pointColor, float pointSize);

    /**
     * @brief Clear all point cloud data
     */
    void clearData();

    /**
     * @brief Get the number of points currently loaded
     * @return Number of points
     */
    int getPointCount() const
    {
        return m_pointCount;
    }

    /**
     * @brief Check if renderer is properly initialized
     * @return true if initialized
     */
    bool isInitialized() const
    {
        return m_initialized;
    }

    /**
     * @brief Check if shaders are compiled and ready
     * @return true if shaders are ready
     */
    bool areShadersReady() const
    {
        return m_shadersReady;
    }

    /**
     * @brief Get OpenGL error information
     * @return Error string, empty if no error
     */
    QString getLastError() const
    {
        return m_lastError;
    }

private:
    // OpenGL resources
    std::unique_ptr<QOpenGLShaderProgram> m_shaderProgram;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLVertexArrayObject m_vertexArrayObject;

    // Uniform locations
    int m_mvpMatrixLocation;
    int m_colorLocation;
    int m_pointSizeLocation;

    // State
    bool m_initialized;
    bool m_shadersReady;
    int m_pointCount;
    QString m_lastError;

    // Helper methods
    bool compileShaderFromFile(QOpenGLShader::ShaderType type, const QString& filePath);
    void setupVertexArrayObject();
    void logOpenGLError(const QString& operation);
    QString readShaderFile(const QString& filePath);
};

#endif  // OPENGLRENDERER_H
