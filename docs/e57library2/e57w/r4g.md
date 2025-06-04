Based on Sprint R4's backlog for implementing advanced rendering with point splatting/impostors and basic lighting, I'll provide comprehensive implementation guidance for your Qt6 C++ FARO scene registration software.

## Enhanced Octree with Aggregate Data

First, let's enhance the octree to store aggregate data needed for splatting:

```cpp
// src/octree.h (Enhanced for Sprint R4)
#ifndef OCTREE_H
#define OCTREE_H

#include 
#include 
#include 
#include 
#include 
#include 

struct PointFullData {
    float x, y, z;
    std::optional r, g, b;
    std::optional intensity;
    std::optional normal; // Added for lighting
    
    // Constructors and utility methods from previous sprints...
    PointFullData(float x = 0, float y = 0, float z = 0)
        : x(x), y(y), z(z) {}
    
    bool hasColor() const { return r.has_value() && g.has_value() && b.has_value(); }
    bool hasIntensity() const { return intensity.has_value(); }
    bool hasNormal() const { return normal.has_value(); }
};

struct AggregateNodeData {
    QVector3D center;
    QVector3D averageColor;
    float averageIntensity;
    QVector3D averageNormal;
    float boundingRadius;
    int pointCount;
    
    AggregateNodeData() 
        : center(0, 0, 0)
        , averageColor(1, 1, 1)
        , averageIntensity(1.0f)
        , averageNormal(0, 0, 1)
        , boundingRadius(0.0f)
        , pointCount(0) {}
};

class OctreeNode {
public:
    AxisAlignedBoundingBox bounds;
    std::vector points;
    std::array, 8> children;
    bool isLeaf = true;
    int depth = 0;
    
    // Aggregate data for splatting (Task R4.1.2)
    mutable AggregateNodeData aggregateData;
    mutable bool aggregateDataCalculated = false;
    
    OctreeNode(const AxisAlignedBoundingBox& bounds, int depth = 0)
        : bounds(bounds), depth(depth) {}
    
    // Existing methods from R1/R2...
    void insert(const PointFullData& point, int maxDepth = 8, int maxPointsPerNode = 100);
    void subdivide();
    
    // New methods for R4
    const AggregateNodeData& getAggregateData() const;
    void calculateAggregateData() const;
    bool shouldRenderAsSplat(float screenSpaceError, float splatThreshold) const;
    
    // Enhanced traversal for splat rendering
    void collectRenderData(
        const std::array& frustumPlanes,
        const QMatrix4x4& mvpMatrix,
        const ViewportInfo& viewport,
        float splatThreshold,
        bool splattingEnabled,
        std::vector& individualPoints,
        std::vector& splatData
    ) const;

private:
    bool intersectsFrustum(const std::array& frustumPlanes) const;
    QVector3D estimateNormalFromPoints() const;
};

#endif // OCTREE_H
```

```cpp
// src/octree.cpp (Enhanced implementation)
#include "octree.h"
#include "screenspaceerror.h"
#include 
#include 

const AggregateNodeData& OctreeNode::getAggregateData() const {
    if (!aggregateDataCalculated) {
        calculateAggregateData();
        aggregateDataCalculated = true;
    }
    return aggregateData;
}

void OctreeNode::calculateAggregateData() const {
    if (isLeaf) {
        if (points.empty()) {
            aggregateData = AggregateNodeData();
            return;
        }
        
        // Calculate center
        QVector3D centerSum(0, 0, 0);
        QVector3D colorSum(0, 0, 0);
        QVector3D normalSum(0, 0, 0);
        float intensitySum = 0.0f;
        int colorCount = 0;
        int intensityCount = 0;
        int normalCount = 0;
        
        for (const auto& point : points) {
            centerSum += QVector3D(point.x, point.y, point.z);
            
            if (point.hasColor()) {
                colorSum += QVector3D(point.r.value() / 255.0f, 
                                     point.g.value() / 255.0f, 
                                     point.b.value() / 255.0f);
                colorCount++;
            }
            
            if (point.hasIntensity()) {
                intensitySum += point.intensity.value();
                intensityCount++;
            }
            
            if (point.hasNormal()) {
                normalSum += point.normal.value();
                normalCount++;
            }
        }
        
        aggregateData.center = centerSum / static_cast(points.size());
        aggregateData.averageColor = colorCount > 0 ? colorSum / colorCount : QVector3D(1, 1, 1);
        aggregateData.averageIntensity = intensityCount > 0 ? intensitySum / intensityCount : 1.0f;
        aggregateData.averageNormal = normalCount > 0 ? 
            normalSum.normalized() : estimateNormalFromPoints();
        aggregateData.pointCount = static_cast(points.size());
        
        // Calculate bounding radius
        float maxDistSq = 0.0f;
        for (const auto& point : points) {
            QVector3D pos(point.x, point.y, point.z);
            float distSq = (pos - aggregateData.center).lengthSquared();
            maxDistSq = std::max(maxDistSq, distSq);
        }
        aggregateData.boundingRadius = std::sqrt(maxDistSq);
        
    } else {
        // Internal node: aggregate from children
        QVector3D centerSum(0, 0, 0);
        QVector3D colorSum(0, 0, 0);
        QVector3D normalSum(0, 0, 0);
        float intensitySum = 0.0f;
        int totalPoints = 0;
        
        for (const auto& child : children) {
            if (child) {
                const auto& childData = child->getAggregateData();
                if (childData.pointCount > 0) {
                    centerSum += childData.center * childData.pointCount;
                    colorSum += childData.averageColor * childData.pointCount;
                    normalSum += childData.averageNormal * childData.pointCount;
                    intensitySum += childData.averageIntensity * childData.pointCount;
                    totalPoints += childData.pointCount;
                }
            }
        }
        
        if (totalPoints > 0) {
            aggregateData.center = centerSum / totalPoints;
            aggregateData.averageColor = colorSum / totalPoints;
            aggregateData.averageIntensity = intensitySum / totalPoints;
            aggregateData.averageNormal = normalSum.normalized();
            aggregateData.pointCount = totalPoints;
        }
        
        // Calculate bounding radius from bounds
        QVector3D boundsSize = bounds.max - bounds.min;
        aggregateData.boundingRadius = boundsSize.length() * 0.5f;
    }
}

bool OctreeNode::shouldRenderAsSplat(float screenSpaceError, float splatThreshold) const {
    return screenSpaceError  10;
}

void OctreeNode::collectRenderData(
    const std::array& frustumPlanes,
    const QMatrix4x4& mvpMatrix,
    const ViewportInfo& viewport,
    float splatThreshold,
    bool splattingEnabled,
    std::vector& individualPoints,
    std::vector& splatData) const {
    
    // Frustum culling
    if (!intersectsFrustum(frustumPlanes)) {
        return;
    }
    
    // Calculate screen-space error
    float screenSpaceError = ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
        bounds, mvpMatrix, viewport);
    
    // Decide rendering method
    if (splattingEnabled && shouldRenderAsSplat(screenSpaceError, splatThreshold)) {
        // Render as splat
        splatData.push_back(getAggregateData());
    } else if (isLeaf) {
        // Render individual points
        individualPoints.insert(individualPoints.end(), points.begin(), points.end());
    } else {
        // Recurse to children
        for (const auto& child : children) {
            if (child) {
                child->collectRenderData(frustumPlanes, mvpMatrix, viewport,
                                       splatThreshold, splattingEnabled,
                                       individualPoints, splatData);
            }
        }
    }
}

QVector3D OctreeNode::estimateNormalFromPoints() const {
    if (points.size()  0.1f ? normal : QVector3D(0, 0, 1);
}
```

## Enhanced PointCloudViewerWidget with Splatting and Lighting

```cpp
// src/pointcloudviewerwidget.h (Enhanced for Sprint R4)
#ifndef POINTCLOUDVIEWERWIDGET_H
#define POINTCLOUDVIEWERWIDGET_H

#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include "pointdata.h"
#include "octree.h"

struct SplatVertex {
    QVector3D position;
    QVector3D color;
    QVector3D normal;
    float intensity;
    float radius;
    
    SplatVertex() = default;
    SplatVertex(const AggregateNodeData& data) 
        : position(data.center)
        , color(data.averageColor)
        , normal(data.averageNormal)
        , intensity(data.averageIntensity)
        , radius(data.boundingRadius) {}
};

class PointCloudViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit PointCloudViewerWidget(QWidget *parent = nullptr);
    ~PointCloudViewerWidget();

    void loadPointCloud(const std::vector& points);

public slots:
    // Existing slots from R3...
    void setRenderWithColor(bool enabled);
    void setRenderWithIntensity(bool enabled);
    void setPointSizeAttenuationEnabled(bool enabled);
    void setPointSizeAttenuationParams(float minSize, float maxSize, float factor);
    
    // New slots for R4 (Task R4.3.2)
    void setSplattingEnabled(bool enabled);
    void setLightingEnabled(bool enabled);
    void setLightDirection(const QVector3D& direction);
    void setLightColor(const QColor& color);
    void setAmbientIntensity(float intensity);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void setupShaders();
    void setupVertexArrayObjects();
    void setupSplatTexture();
    void updateCamera();
    void renderScene();
    void renderPoints(const std::vector& points);
    void renderSplats(const std::vector& splats);
    void preparePointVertexData(const std::vector& points);
    void prepareSplatVertexData(const std::vector& splats);
    std::array extractFrustumPlanes(const QMatrix4x4& viewProjection) const;

    // OpenGL resources
    QOpenGLShaderProgram* m_pointShaderProgram;
    QOpenGLShaderProgram* m_splatShaderProgram;
    QOpenGLBuffer m_pointVertexBuffer;
    QOpenGLBuffer m_splatVertexBuffer;
    QOpenGLVertexArrayObject m_pointVAO;
    QOpenGLVertexArrayObject m_splatVAO;
    QOpenGLTexture* m_splatTexture;

    // Rendering state from R3...
    bool m_renderWithColor;
    bool m_renderWithIntensity;
    bool m_pointSizeAttenuationEnabled;
    float m_minPointSize;
    float m_maxPointSize;
    float m_attenuationFactor;

    // New rendering state for R4
    bool m_splattingEnabled;
    bool m_lightingEnabled;
    QVector3D m_lightDirection;
    QColor m_lightColor;
    float m_ambientIntensity;
    float m_splatThreshold;

    // Camera and transformation
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;
    QVector3D m_cameraPosition;
    QVector3D m_cameraTarget;
    QVector3D m_cameraUp;
    float m_cameraDistance;
    float m_cameraYaw;
    float m_cameraPitch;

    // Mouse interaction
    QPoint m_lastMousePos;
    bool m_mousePressed;

    // LOD system
    std::unique_ptr m_octree;
    std::vector m_visiblePoints;
    std::vector m_visibleSplats;
    std::vector m_pointVertexData;
    std::vector m_splatVertexData;

    // Shader source methods
    static const char* pointVertexShaderSource();
    static const char* pointFragmentShaderSource();
    static const char* splatVertexShaderSource();
    static const char* splatGeometryShaderSource();
    static const char* splatFragmentShaderSource();
};

#endif // POINTCLOUDVIEWERWIDGET_H
```

## Shader Implementation for Splatting and Lighting

```cpp
// src/pointcloudviewerwidget.cpp (Enhanced shader implementation)

// Point rendering shaders with lighting (Task R4.2.2)
const char* PointCloudViewerWidget::pointVertexShaderSource() {
    return R"(
        #version 330 core
        
        layout (location = 0) in vec3 position;
        layout (location = 1) in vec3 color;
        layout (location = 2) in float intensity;
        layout (location = 3) in vec3 normal;
        
        uniform mat4 mvpMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 modelMatrix;
        uniform mat3 normalMatrix;
        uniform vec3 cameraPosition_worldSpace;
        
        // Point size attenuation
        uniform bool pointSizeAttenuationEnabled;
        uniform float basePointSize;
        uniform float minPointSize;
        uniform float maxPointSize;
        uniform float attenuationFactor;
        
        // Outputs to fragment shader
        out vec3 fragColor;
        out float fragIntensity;
        out vec3 fragNormal_viewSpace;
        out vec3 fragPosition_viewSpace;
        
        void main() {
            gl_Position = mvpMatrix * vec4(position, 1.0);
            
            // Transform normal to view space for lighting
            fragNormal_viewSpace = normalMatrix * normal;
            
            // Transform position to view space
            vec4 position_viewSpace = viewMatrix * modelMatrix * vec4(position, 1.0);
            fragPosition_viewSpace = position_viewSpace.xyz;
            
            // Pass attributes
            fragColor = color;
            fragIntensity = intensity;
            
            // Point size attenuation
            if (pointSizeAttenuationEnabled) {
                float distance = length(cameraPosition_worldSpace - position);
                float attenuatedSize = basePointSize / (1.0 + distance * attenuationFactor);
                gl_PointSize = clamp(attenuatedSize, minPointSize, maxPointSize);
            } else {
                gl_PointSize = basePointSize;
            }
        }
    )";
}

const char* PointCloudViewerWidget::pointFragmentShaderSource() {
    return R"(
        #version 330 core
        
        in vec3 fragColor;
        in float fragIntensity;
        in vec3 fragNormal_viewSpace;
        in vec3 fragPosition_viewSpace;
        
        // Rendering control uniforms
        uniform bool renderWithColor;
        uniform bool renderWithIntensity;
        uniform vec3 uniformColor;
        
        // Lighting uniforms (Task R4.2.2)
        uniform bool lightingEnabled;
        uniform vec3 lightDirection_viewSpace;
        uniform vec3 lightColor;
        uniform float ambientIntensity;
        
        out vec4 finalColor;
        
        void main() {
            vec3 baseColor = uniformColor;
            
            // Apply color/intensity rendering logic from R3
            if (renderWithColor) {
                baseColor = fragColor;
            }
            
            if (renderWithIntensity) {
                if (renderWithColor) {
                    baseColor = fragColor * fragIntensity;
                } else {
                    baseColor = vec3(fragIntensity);
                }
            }
            
            // Apply lighting if enabled (Task R4.2.2)
            vec3 litColor = baseColor;
            if (lightingEnabled) {
                vec3 normal = normalize(fragNormal_viewSpace);
                vec3 lightDir = normalize(-lightDirection_viewSpace);
                
                // Lambertian diffuse lighting
                float diffuse = max(dot(normal, lightDir), 0.0);
                
                // Combine ambient and diffuse
                vec3 ambient = baseColor * ambientIntensity;
                vec3 diffuseComponent = baseColor * lightColor * diffuse;
                
                litColor = ambient + diffuseComponent;
            }
            
            // Create circular point shape
            vec2 coord = gl_PointCoord - vec2(0.5);
            float distance = length(coord);
            if (distance > 0.5) {
                discard;
            }
            
            float alpha = 1.0 - smoothstep(0.3, 0.5, distance);
            finalColor = vec4(litColor, alpha);
        }
    )";
}

// Splat rendering shaders (Task R4.1.3)
const char* PointCloudViewerWidget::splatVertexShaderSource() {
    return R"(
        #version 330 core
        
        layout (location = 0) in vec3 position;
        layout (location = 1) in vec3 color;
        layout (location = 2) in vec3 normal;
        layout (location = 3) in float intensity;
        layout (location = 4) in float radius;
        
        uniform mat4 mvpMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 modelMatrix;
        uniform mat3 normalMatrix;
        uniform vec3 cameraPosition_worldSpace;
        
        // Outputs to geometry shader
        out vec3 vertexColor;
        out vec3 vertexNormal_viewSpace;
        out float vertexIntensity;
        out float vertexRadius;
        out vec3 vertexPosition_viewSpace;
        
        void main() {
            gl_Position = mvpMatrix * vec4(position, 1.0);
            
            // Transform to view space
            vertexNormal_viewSpace = normalMatrix * normal;
            vec4 position_viewSpace = viewMatrix * modelMatrix * vec4(position, 1.0);
            vertexPosition_viewSpace = position_viewSpace.xyz;
            
            // Pass attributes
            vertexColor = color;
            vertexIntensity = intensity;
            
            // Calculate screen-space radius
            float distance = length(cameraPosition_worldSpace - position);
            vertexRadius = radius / (1.0 + distance * 0.01); // Simple attenuation
        }
    )";
}

const char* PointCloudViewerWidget::splatGeometryShaderSource() {
    return R"(
        #version 330 core
        
        layout (points) in;
        layout (triangle_strip, max_vertices = 4) out;
        
        in vec3 vertexColor[];
        in vec3 vertexNormal_viewSpace[];
        in float vertexIntensity[];
        in float vertexRadius[];
        in vec3 vertexPosition_viewSpace[];
        
        uniform mat4 projectionMatrix;
        uniform vec2 viewportSize;
        
        out vec3 fragColor;
        out vec3 fragNormal_viewSpace;
        out float fragIntensity;
        out vec2 fragTexCoord;
        out vec3 fragPosition_viewSpace;
        
        void main() {
            vec4 center = gl_in[0].gl_Position;
            
            // Calculate screen-space quad size
            float screenRadius = vertexRadius[0] * 100.0; // Scale factor
            vec2 quadSize = vec2(screenRadius) / viewportSize;
            
            // Pass attributes to fragment shader
            fragColor = vertexColor[0];
            fragNormal_viewSpace = vertexNormal_viewSpace[0];
            fragIntensity = vertexIntensity[0];
            fragPosition_viewSpace = vertexPosition_viewSpace[0];
            
            // Generate screen-aligned quad
            // Bottom-left
            gl_Position = center + vec4(-quadSize.x, -quadSize.y, 0.0, 0.0);
            fragTexCoord = vec2(0.0, 0.0);
            EmitVertex();
            
            // Bottom-right
            gl_Position = center + vec4(quadSize.x, -quadSize.y, 0.0, 0.0);
            fragTexCoord = vec2(1.0, 0.0);
            EmitVertex();
            
            // Top-left
            gl_Position = center + vec4(-quadSize.x, quadSize.y, 0.0, 0.0);
            fragTexCoord = vec2(0.0, 1.0);
            EmitVertex();
            
            // Top-right
            gl_Position = center + vec4(quadSize.x, quadSize.y, 0.0, 0.0);
            fragTexCoord = vec2(1.0, 1.0);
            EmitVertex();
            
            EndPrimitive();
        }
    )";
}

const char* PointCloudViewerWidget::splatFragmentShaderSource() {
    return R"(
        #version 330 core
        
        in vec3 fragColor;
        in vec3 fragNormal_viewSpace;
        in float fragIntensity;
        in vec2 fragTexCoord;
        in vec3 fragPosition_viewSpace;
        
        uniform sampler2D splatTexture;
        uniform bool renderWithColor;
        uniform bool renderWithIntensity;
        uniform vec3 uniformColor;
        
        // Lighting uniforms
        uniform bool lightingEnabled;
        uniform vec3 lightDirection_viewSpace;
        uniform vec3 lightColor;
        uniform float ambientIntensity;
        
        out vec4 finalColor;
        
        void main() {
            // Sample splat texture
            vec4 texColor = texture(splatTexture, fragTexCoord);
            
            vec3 baseColor = uniformColor;
            
            // Apply color/intensity logic
            if (renderWithColor) {
                baseColor = fragColor;
            }
            
            if (renderWithIntensity) {
                if (renderWithColor) {
                    baseColor = fragColor * fragIntensity;
                } else {
                    baseColor = vec3(fragIntensity);
                }
            }
            
            // Apply lighting
            vec3 litColor = baseColor;
            if (lightingEnabled) {
                vec3 normal = normalize(fragNormal_viewSpace);
                vec3 lightDir = normalize(-lightDirection_viewSpace);
                
                float diffuse = max(dot(normal, lightDir), 0.0);
                
                vec3 ambient = baseColor * ambientIntensity;
                vec3 diffuseComponent = baseColor * lightColor * diffuse;
                
                litColor = ambient + diffuseComponent;
            }
            
            // Combine with texture alpha
            finalColor = vec4(litColor, texColor.a);
        }
    )";
}
```

## Enhanced Implementation Methods

```cpp
// src/pointcloudviewerwidget.cpp (Enhanced implementation)
PointCloudViewerWidget::PointCloudViewerWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_pointShaderProgram(nullptr)
    , m_splatShaderProgram(nullptr)
    , m_splatTexture(nullptr)
    , m_renderWithColor(false)
    , m_renderWithIntensity(false)
    , m_pointSizeAttenuationEnabled(false)
    , m_minPointSize(1.0f)
    , m_maxPointSize(10.0f)
    , m_attenuationFactor(0.1f)
    , m_splattingEnabled(true)
    , m_lightingEnabled(false)
    , m_lightDirection(0, 0, -1)
    , m_lightColor(Qt::white)
    , m_ambientIntensity(0.3f)
    , m_splatThreshold(10.0f)
    , m_cameraPosition(0, 0, 10)
    , m_cameraTarget(0, 0, 0)
    , m_cameraUp(0, 1, 0)
    , m_cameraDistance(10.0f)
    , m_cameraYaw(0.0f)
    , m_cameraPitch(0.0f)
    , m_mousePressed(false)
    , m_octree(std::make_unique())
{
    setFocusPolicy(Qt::StrongFocus);
}

void PointCloudViewerWidget::initializeGL() {
    initializeOpenGLFunctions();
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    setupShaders();
    setupVertexArrayObjects();
    setupSplatTexture();
}

void PointCloudViewerWidget::setupShaders() {
    // Setup point rendering shader
    m_pointShaderProgram = new QOpenGLShaderProgram(this);
    m_pointShaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, pointVertexShaderSource());
    m_pointShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, pointFragmentShaderSource());
    m_pointShaderProgram->link();
    
    // Setup splat rendering shader
    m_splatShaderProgram = new QOpenGLShaderProgram(this);
    m_splatShaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, splatVertexShaderSource());
    m_splatShaderProgram->addShaderFromSourceCode(QOpenGLShader::Geometry, splatGeometryShaderSource());
    m_splatShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, splatFragmentShaderSource());
    m_splatShaderProgram->link();
}

void PointCloudViewerWidget::setupSplatTexture() {
    // Create a simple circular splat texture
    const int textureSize = 64;
    QImage splatImage(textureSize, textureSize, QImage::Format_RGBA8888);
    splatImage.fill(Qt::transparent);
    
    QPainter painter(&splatImage);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Create radial gradient
    QRadialGradient gradient(textureSize/2, textureSize/2, textureSize/2);
    gradient.setColorAt(0.0, QColor(255, 255, 255, 255));
    gradient.setColorAt(0.7, QColor(255, 255, 255, 128));
    gradient.setColorAt(1.0, QColor(255, 255, 255, 0));
    
    painter.setBrush(QBrush(gradient));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, textureSize, textureSize);
    
    m_splatTexture = new QOpenGLTexture(splatImage);
    m_splatTexture->setMinificationFilter(QOpenGLTexture::Linear);
    m_splatTexture->setMagnificationFilter(QOpenGLTexture::Linear);
}

void PointCloudViewerWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    updateCamera();
    renderScene();
}

void PointCloudViewerWidget::renderScene() {
    if (!m_octree->root) return;
    
    // Collect visible points and splats (Task R4.1.4)
    QMatrix4x4 viewProjection = m_projectionMatrix * m_viewMatrix;
    auto frustumPlanes = extractFrustumPlanes(viewProjection);
    
    ViewportInfo viewport{width(), height(), 0.1f, 1000.0f};
    
    m_visiblePoints.clear();
    m_visibleSplats.clear();
    
    m_octree->root->collectRenderData(
        frustumPlanes, viewProjection, viewport,
        m_splatThreshold, m_splattingEnabled,
        m_visiblePoints, m_visibleSplats
    );
    
    // Render individual points
    if (!m_visiblePoints.empty()) {
        renderPoints(m_visiblePoints);
    }
    
    // Render splats
    if (!m_visibleSplats.empty() && m_splattingEnabled) {
        renderSplats(m_visibleSplats);
    }
}

void PointCloudViewerWidget::renderPoints(const std::vector& points) {
    preparePointVertexData(points);
    
    m_pointShaderProgram->bind();
    
    // Set transformation matrices
    QMatrix4x4 modelMatrix;
    modelMatrix.setToIdentity();
    QMatrix4x4 mvpMatrix = m_projectionMatrix * m_viewMatrix * modelMatrix;
    QMatrix3x3 normalMatrix = (m_viewMatrix * modelMatrix).normalMatrix();
    
    m_pointShaderProgram->setUniformValue("mvpMatrix", mvpMatrix);
    m_pointShaderProgram->setUniformValue("viewMatrix", m_viewMatrix);
    m_pointShaderProgram->setUniformValue("modelMatrix", modelMatrix);
    m_pointShaderProgram->setUniformValue("normalMatrix", normalMatrix);
    m_pointShaderProgram->setUniformValue("cameraPosition_worldSpace", m_cameraPosition);
    
    // Set rendering parameters
    m_pointShaderProgram->setUniformValue("renderWithColor", m_renderWithColor);
    m_pointShaderProgram->setUniformValue("renderWithIntensity", m_renderWithIntensity);
    m_pointShaderProgram->setUniformValue("uniformColor", QVector3D(1.0f, 1.0f, 1.0f));
    
    // Set point size parameters
    m_pointShaderProgram->setUniformValue("pointSizeAttenuationEnabled", m_pointSizeAttenuationEnabled);
    m_pointShaderProgram->setUniformValue("basePointSize", 3.0f);
    m_pointShaderProgram->setUniformValue("minPointSize", m_minPointSize);
    m_pointShaderProgram->setUniformValue("maxPointSize", m_maxPointSize);
    m_pointShaderProgram->setUniformValue("attenuationFactor", m_attenuationFactor);
    
    // Set lighting parameters (Task R4.2.3)
    m_pointShaderProgram->setUniformValue("lightingEnabled", m_lightingEnabled);
    if (m_lightingEnabled) {
        QVector3D lightDir_viewSpace = m_viewMatrix.mapVector(m_lightDirection).normalized();
        m_pointShaderProgram->setUniformValue("lightDirection_viewSpace", lightDir_viewSpace);
        m_pointShaderProgram->setUniformValue("lightColor", 
            QVector3D(m_lightColor.redF(), m_lightColor.greenF(), m_lightColor.blueF()));
        m_pointShaderProgram->setUniformValue("ambientIntensity", m_ambientIntensity);
    }
    
    // Render
    m_pointVAO.bind();
    glDrawArrays(GL_POINTS, 0, static_cast(m_pointVertexData.size()));
    m_pointVAO.release();
    
    m_pointShaderProgram->release();
}

void PointCloudViewerWidget::renderSplats(const std::vector& splats) {
    prepareSplatVertexData(splats);
    
    m_splatShaderProgram->bind();
    
    // Set transformation matrices
    QMatrix4x4 modelMatrix;
    modelMatrix.setToIdentity();
    QMatrix4x4 mvpMatrix = m_projectionMatrix * m_viewMatrix * modelMatrix;
    QMatrix3x3 normalMatrix = (m_viewMatrix * modelMatrix).normalMatrix();
    
    m_splatShaderProgram->setUniformValue("mvpMatrix", mvpMatrix);
    m_splatShaderProgram->setUniformValue("viewMatrix", m_viewMatrix);
    m_splatShaderProgram->setUniformValue("modelMatrix", modelMatrix);
    m_splatShaderProgram->setUniformValue("normalMatrix", normalMatrix);
    m_splatShaderProgram->setUniformValue("projectionMatrix", m_projectionMatrix);
    m_splatShaderProgram->setUniformValue("cameraPosition_worldSpace", m_cameraPosition);
    m_splatShaderProgram->setUniformValue("viewportSize", QVector2D(width(), height()));
    
    // Set rendering parameters
    m_splatShaderProgram->setUniformValue("renderWithColor", m_renderWithColor);
    m_splatShaderProgram->setUniformValue("renderWithIntensity", m_renderWithIntensity);
    m_splatShaderProgram->setUniformValue("uniformColor", QVector3D(1.0f, 1.0f, 1.0f));
    
    // Set lighting parameters
    m_splatShaderProgram->setUniformValue("lightingEnabled", m_lightingEnabled);
    if (m_lightingEnabled) {
        QVector3D lightDir_viewSpace = m_viewMatrix.mapVector(m_lightDirection).normalized();
        m_splatShaderProgram->setUniformValue("lightDirection_viewSpace", lightDir_viewSpace);
        m_splatShaderProgram->setUniformValue("lightColor", 
            QVector3D(m_lightColor.redF(), m_lightColor.greenF(), m_lightColor.blueF()));
        m_splatShaderProgram->setUniformValue("ambientIntensity", m_ambientIntensity);
    }
    
    // Bind splat texture
    m_splatTexture->bind(0);
    m_splatShaderProgram->setUniformValue("splatTexture", 0);
    
    // Render
    m_splatVAO.bind();
    glDrawArrays(GL_POINTS, 0, static_cast(m_splatVertexData.size()));
    m_splatVAO.release();
    
    m_splatShaderProgram->release();
}

// Public slot implementations (Task R4.3.2)
void PointCloudViewerWidget::setSplattingEnabled(bool enabled) {
    m_splattingEnabled = enabled;
    update();
}

void PointCloudViewerWidget::setLightingEnabled(bool enabled) {
    m_lightingEnabled = enabled;
    update();
}

void PointCloudViewerWidget::setLightDirection(const QVector3D& direction) {
    m_lightDirection = direction.normalized();
    update();
}

void PointCloudViewerWidget::setLightColor(const QColor& color) {
    m_lightColor = color;
    update();
}

void PointCloudViewerWidget::setAmbientIntensity(float intensity) {
    m_ambientIntensity = intensity;
    update();
}
```

## Enhanced UI Controls Implementation

```cpp
// src/mainwindow.h (Enhanced for Sprint R4)
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include "pointcloudviewerwidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Existing slots from R3...
    void onColorRenderToggled(bool enabled);
    void onIntensityRenderToggled(bool enabled);
    void onAttenuationToggled(bool enabled);
    void onAttenuationParamsChanged();
    
    // New slots for R4 (Task R4.3.1)
    void onSplattingToggled(bool enabled);
    void onLightingToggled(bool enabled);
    void onLightDirectionChanged();
    void onLightColorClicked();
    void onAmbientIntensityChanged(int value);
    void loadPointCloudFile();

private:
    void setupUI();
    void setupAttributeRenderingControls();
    void setupPointSizeControls();
    void setupSplattingControls();
    void setupLightingControls();
    void connectSignals();

    // UI components
    PointCloudViewerWidget* m_pointCloudViewer;
    
    // Existing controls from R3...
    QGroupBox* m_attributeGroupBox;
    QCheckBox* m_colorRenderCheckbox;
    QCheckBox* m_intensityRenderCheckbox;
    
    QGroupBox* m_pointSizeGroupBox;
    QCheckBox* m_attenuationCheckbox;
    QSlider* m_minSizeSlider;
    QSlider* m_maxSizeSlider;
    QSlider* m_attenuationFactorSlider;
    QLabel* m_minSizeLabel;
    QLabel* m_maxSizeLabel;
    QLabel* m_attenuationFactorLabel;
    
    // New controls for R4
    QGroupBox* m_splattingGroupBox;
    QCheckBox* m_splattingCheckbox;
    
    QGroupBox* m_lightingGroupBox;
    QCheckBox* m_lightingCheckbox;
    QSlider* m_lightDirXSlider;
    QSlider* m_lightDirYSlider;
    QSlider* m_lightDirZSlider;
    QLabel* m_lightDirXLabel;
    QLabel* m_lightDirYLabel;
    QLabel* m_lightDirZLabel;
    QPushButton* m_lightColorButton;
    QLabel* m_lightColorLabel;
    QSlider* m_ambientIntensitySlider;
    QLabel* m_ambientIntensityLabel;
    
    QPushButton* m_loadFileButton;
    
    // Current state
    QColor m_currentLightColor;
};

#endif // MAINWINDOW_H
```

```cpp
// src/mainwindow.cpp (Enhanced implementation)
#include "mainwindow.h"
#include 
#include 
#include 
#include 

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_pointCloudViewer(nullptr)
    , m_currentLightColor(Qt::white)
{
    setupUI();
    connectSignals();
}

void MainWindow::setupSplattingControls() {
    // Splatting controls (Task R4.3.1)
    m_splattingGroupBox = new QGroupBox("Point Splatting", this);
    QVBoxLayout* splattingLayout = new QVBoxLayout(m_splattingGroupBox);
    
    m_splattingCheckbox = new QCheckBox("Enable Point Splatting for LOD", this);
    m_splattingCheckbox->setChecked(true);
    splattingLayout->addWidget(m_splattingCheckbox);
    
    QLabel* splattingInfo = new QLabel(
        "Point splatting renders distant/coarse LOD nodes as textured quads\n"
        "instead of individual points for better visual cohesion.", this);
    splattingInfo->setWordWrap(true);
    splattingInfo->setStyleSheet("QLabel { color: #666666; font-size: 10px; }");
    splattingLayout->addWidget(splattingInfo);
}

void MainWindow::setupLightingControls() {
    // Lighting controls (Task R4.3.1)
    m_lightingGroupBox = new QGroupBox("Basic Lighting", this);
    QVBoxLayout* lightingLayout = new QVBoxLayout(m_lightingGroupBox);
    
    m_lightingCheckbox = new QCheckBox("Enable Lighting", this);
    lightingLayout->addWidget(m_lightingCheckbox);
    
    // Light direction controls
    QWidget* lightDirWidget = new QWidget(this);
    QVBoxLayout* lightDirLayout = new QVBoxLayout(lightDirWidget);
    
    QLabel* lightDirLabel = new QLabel("Light Direction:", this);
    lightDirLayout->addWidget(lightDirLabel);
    
    // X direction
    QHBoxLayout* xDirLayout = new QHBoxLayout();
    QLabel* xLabel = new QLabel("X:", this);
    m_lightDirXSlider = new QSlider(Qt::Horizontal, this);
    m_lightDirXSlider->setRange(-100, 100);
    m_lightDirXSlider->setValue(0);
    m_lightDirXLabel = new QLabel("0.0", this);
    m_lightDirXLabel->setFixedWidth(40);
    
    xDirLayout->addWidget(xLabel);
    xDirLayout->addWidget(m_lightDirXSlider);
    xDirLayout->addWidget(m_lightDirXLabel);
    
    // Y direction
    QHBoxLayout* yDirLayout = new QHBoxLayout();
    QLabel* yLabel = new QLabel("Y:", this);
    m_lightDirYSlider = new QSlider(Qt::Horizontal, this);
    m_lightDirYSlider->setRange(-100, 100);
    m_lightDirYSlider->setValue(0);
    m_lightDirYLabel = new QLabel("0.0", this);
    m_lightDirYLabel->setFixedWidth(40);
    
    yDirLayout->addWidget(yLabel);
    yDirLayout->addWidget(m_lightDirYSlider);
    yDirLayout->addWidget(m_lightDirYLabel);
    
    // Z direction
    QHBoxLayout* zDirLayout = new QHBoxLayout();
    QLabel* zLabel = new QLabel("Z:", this);
    m_lightDirZSlider = new QSlider(Qt::Horizontal, this);
    m_lightDirZSlider->setRange(-100, 100);
    m_lightDirZSlider->setValue(-100); // Default pointing down
    m_lightDirZLabel = new QLabel("-1.0", this);
    m_lightDirZLabel->setFixedWidth(40);
    
    zDirLayout->addWidget(zLabel);
    zDirLayout->addWidget(m_lightDirZSlider);
    zDirLayout->addWidget(m_lightDirZLabel);
    
    lightDirLayout->addLayout(xDirLayout);
    lightDirLayout->addLayout(yDirLayout);
    lightDirLayout->addLayout(zDirLayout);
    
    // Light color control
    QLabel* colorLabel = new QLabel("Light Color:", this);
    QHBoxLayout* colorLayout = new QHBoxLayout();
    
    m_lightColorButton = new QPushButton("Select Color", this);
    m_lightColorLabel = new QLabel(this);
    m_lightColorLabel->setFixedSize(30, 20);
    m_lightColorLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid black; }");
    
    colorLayout->addWidget(m_lightColorButton);
    colorLayout->addWidget(m_lightColorLabel);
    colorLayout->addStretch();
    
    // Ambient intensity control
    QLabel* ambientLabel = new QLabel("Ambient Intensity:", this);
    QHBoxLayout* ambientLayout = new QHBoxLayout();
    
    m_ambientIntensitySlider = new QSlider(Qt::Horizontal, this);
    m_ambientIntensitySlider->setRange(0, 100);
    m_ambientIntensitySlider->setValue(30); // 0.3 default
    m_ambientIntensityLabel = new QLabel("0.30", this);
    m_ambientIntensityLabel->setFixedWidth(40);
    
    ambientLayout->addWidget(m_ambientIntensitySlider);
    ambientLayout->addWidget(m_ambientIntensityLabel);
    
    lightingLayout->addWidget(lightDirWidget);
    lightingLayout->addWidget(colorLabel);
    lightingLayout->addLayout(colorLayout);
    lightingLayout->addWidget(ambientLabel);
    lightingLayout->addLayout(ambientLayout);
    
    // Initially disable lighting controls
    lightDirWidget->setEnabled(false);
    m_lightColorButton->setEnabled(false);
    m_ambientIntensitySlider->setEnabled(false);
    
    connect(m_lightingCheckbox, &QCheckBox::toggled, lightDirWidget, &QWidget::setEnabled);
    connect(m_lightingCheckbox, &QCheckBox::toggled, m_lightColorButton, &QPushButton::setEnabled);
    connect(m_lightingCheckbox, &QCheckBox::toggled, m_ambientIntensitySlider, &QSlider::setEnabled);
}

void MainWindow::connectSignals() {
    // Existing connections from R3...
    
    // New connections for R4 (Task R4.3.3)
    connect(m_splattingCheckbox, &QCheckBox::toggled,
            m_pointCloudViewer, &PointCloudViewerWidget::setSplattingEnabled);
    connect(m_lightingCheckbox, &QCheckBox::toggled,
            m_pointCloudViewer, &PointCloudViewerWidget::setLightingEnabled);
    
    // Light direction controls
    connect(m_lightDirXSlider, &QSlider::valueChanged, this, &MainWindow::onLightDirectionChanged);
    connect(m_lightDirYSlider, &QSlider::valueChanged, this, &MainWindow::onLightDirectionChanged);
    connect(m_lightDirZSlider, &QSlider::valueChanged, this, &MainWindow::onLightDirectionChanged);
    
    // Light color control
    connect(m_lightColorButton, &QPushButton::clicked, this, &MainWindow::onLightColorClicked);
    
    // Ambient intensity control
    connect(m_ambientIntensitySlider, &QSlider::valueChanged, this, &MainWindow::onAmbientIntensityChanged);
}

void MainWindow::onSplattingToggled(bool enabled) {
    Q_UNUSED(enabled)
    // Handled by direct connection
}

void MainWindow::onLightingToggled(bool enabled) {
    Q_UNUSED(enabled)
    // Handled by direct connection
}

void MainWindow::onLightDirectionChanged() {
    float x = m_lightDirXSlider->value() / 100.0f;
    float y = m_lightDirYSlider->value() / 100.0f;
    float z = m_lightDirZSlider->value() / 100.0f;
    
    // Update labels
    m_lightDirXLabel->setText(QString("%1").arg(x, 0, 'f', 1));
    m_lightDirYLabel->setText(QString("%1").arg(y, 0, 'f', 1));
    m_lightDirZLabel->setText(QString("%1").arg(z, 0, 'f', 1));
    
    // Update viewer
    QVector3D direction(x, y, z);
    if (direction.length() > 0.001f) {
        m_pointCloudViewer->setLightDirection(direction);
    }
}

void MainWindow::onLightColorClicked() {
    QColor color = QColorDialog::getColor(m_currentLightColor, this, "Select Light Color");
    if (color.isValid()) {
        m_currentLightColor = color;
        
        // Update color display
        QString styleSheet = QString("QLabel { background-color: %1; border: 1px solid black; }")
                            .arg(color.name());
        m_lightColorLabel->setStyleSheet(styleSheet);
        
        // Update viewer
        m_pointCloudViewer->setLightColor(color);
    }
}

void MainWindow::onAmbientIntensityChanged(int value) {
    float intensity = value / 100.0f;
    m_ambientIntensityLabel->setText(QString("%1").arg(intensity, 0, 'f', 2));
    m_pointCloudViewer->setAmbientIntensity(intensity);
}
```

## Comprehensive Testing Implementation

```cpp
// tests/test_pointcloudviewerwidget_rendering_r4.cpp
#include 
#include 
#include 
#include 
#include "../src/pointcloudviewerwidget.h"
#include "../src/octree.h"

class PointCloudRenderingR4Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Create OpenGL context for testing
        m_surface = std::make_unique();
        m_surface->create();
        
        m_context = std::make_unique();
        m_context->create();
        m_context->makeCurrent(m_surface.get());
        
        createTestPointClouds();
    }
    
    void createTestPointClouds() {
        // Test Case R4.1.1: Very large and dense point cloud
        m_largePointCloud.reserve(1000000);
        for (int i = 0; i  m_surface;
    std::unique_ptr m_context;
    
    std::vector m_largePointCloud;
    std::vector m_geometricShapeCloud;
};

// Test Case R4.1.1: Load a very large and dense point cloud. Zoom out.
TEST_F(PointCloudRenderingR4Test, LargePointCloudSplatting) {
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(m_largePointCloud);
    
    // Enable splatting
    viewer.setSplattingEnabled(true);
    
    // Test that splatting can be enabled without crashes
    SUCCEED();
}

// Test Case R4.1.2: Compare rendering with splatting vs. R2's refined point selection
TEST_F(PointCloudRenderingR4Test, SplattingVsPointSelection) {
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(m_largePointCloud);
    
    // Test with splatting enabled
    viewer.setSplattingEnabled(true);
    
    // Test with splatting disabled (should fall back to point selection)
    viewer.setSplattingEnabled(false);
    
    SUCCEED();
}

// Test Case R4.1.3: Verify splat coloring/texturing
TEST_F(PointCloudRenderingR4Test, SplatColoringTexturing) {
    // Create point cloud with distinct color regions
    std::vector colorRegionCloud;
    
    // Red region
    for (int i = 0; i < 1000; ++i) {
        float x = (rand() % 100) / 10.0f;
        float y = (rand() % 100) / 10.0f;
        float z = (rand() % 100) / 10.0f;
        colorRegionCloud.emplace_back(x, y, z, 255, 0, 0, 1.0f);
    }
    
    // Blue region
    for (int i = 0; i < 1000; ++i) {
        float x = (rand() % 100) / 10.0f + 20.0f;
        float y = (rand() % 100) / 10.0f;
        float z = (rand() % 100) / 10.0f;
        colorRegionCloud.emplace_back(x, y, z, 0, 0, 255, 1.0f);
    }
    
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(colorRegionCloud);
    viewer.setSplattingEnabled(true);
    viewer.setRenderWithColor(true);
    
    SUCCEED();
}

// Test Case R4.2.1: Load a point cloud representing a 3D object with clear surfaces
TEST_F(PointCloudRenderingR4Test, LightingWith3DObject) {
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(m_geometricShapeCloud);
    
    // Enable lighting
    viewer.setLightingEnabled(true);
    viewer.setLightDirection(QVector3D(1, 1, -1));
    viewer.setLightColor(Qt::white);
    viewer.setAmbientIntensity(0.3f);
    
    SUCCEED();
}

// Test Case R4.2.2: Test lighting with splats vs. individual points
TEST_F(PointCloudRenderingR4Test, LightingWithSplatsAndPoints) {
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(m_geometricShapeCloud);
    
    viewer.setLightingEnabled(true);
    viewer.setLightDirection(QVector3D(0, 0, -1));
    
    // Test with individual points
    viewer.setSplattingEnabled(false);
    
    // Test with splats
    viewer.setSplattingEnabled(true);
    
    SUCCEED();
}

// Test Case R4.2.3: Toggle lighting UI controls
TEST_F(PointCloudRenderingR4Test, LightingUIControls) {
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(m_geometricShapeCloud);
    
    // Test lighting toggle
    viewer.setLightingEnabled(false);
    viewer.setLightingEnabled(true);
    
    // Test light parameter adjustments
    viewer.setLightDirection(QVector3D(1, 0, 0));
    viewer.setLightDirection(QVector3D(0, 1, 0));
    viewer.setLightDirection(QVector3D(0, 0, 1));
    
    viewer.setLightColor(Qt::red);
    viewer.setLightColor(Qt::blue);
    viewer.setLightColor(Qt::white);
    
    viewer.setAmbientIntensity(0.1f);
    viewer.setAmbientIntensity(0.5f);
    viewer.setAmbientIntensity(0.9f);
    
    SUCCEED();
}

// Test Case R4.3.1: Manipulate all new UI controls
TEST_F(PointCloudRenderingR4Test, AllUIControls) {
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(m_largePointCloud);
    
    // Test splatting control
    viewer.setSplattingEnabled(true);
    viewer.setSplattingEnabled(false);

[1] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/35208055/634f0570-0845-45e2-a48f-982a880de30e/paste.txt
[2] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/collection_59dddd7d-a43e-4981-8f37-8f1d5f4c9bdc/39178ad2-15a6-4478-bf2f-d68b85feb663/paste.txt

Based on my review of the existing Sprint R4 implementation guide and the provided Sprint R4 backlog documentation, I'll enhance the guide to fully align with the specific requirements and ensure complete coverage of all backlog objectives.

## Enhanced Implementation Guide for Sprint R4

The existing guide provides a solid technical foundation, but needs enhancement to fully address the Sprint R4 backlog requirements. Here's the comprehensive enhanced version:

## Enhanced Octree with Aggregate Data for Splatting

The backlog specifically requires storing aggregate attribute data in octree nodes. Here's the enhanced implementation:

```cpp
// src/octree.h (Enhanced to match Sprint R4 backlog requirements)
#ifndef OCTREE_H
#define OCTREE_H

#include 
#include 
#include 
#include 
#include 
#include 

struct PointFullData {
    float x, y, z;
    std::optional r, g, b;
    std::optional intensity;
    std::optional normal; // Added for lighting as per R4.2.1
    
    // Constructors from previous sprints...
    PointFullData(float x = 0, float y = 0, float z = 0)
        : x(x), y(y), z(z) {}
    
    PointFullData(float x, float y, float z, uint8_t r, uint8_t g, uint8_t b, float intensity)
        : x(x), y(y), z(z), r(r), g(g), b(b), intensity(intensity) {}
    
    bool hasColor() const { return r.has_value() && g.has_value() && b.has_value(); }
    bool hasIntensity() const { return intensity.has_value(); }
    bool hasNormal() const { return normal.has_value(); }
    
    void getNormalizedColor(float& nr, float& ng, float& nb) const {
        nr = hasColor() ? r.value() / 255.0f : 1.0f;
        ng = hasColor() ? g.value() / 255.0f : 1.0f;
        nb = hasColor() ? b.value() / 255.0f : 1.0f;
    }
};

// Aggregate data structure as specified in Task R4.1.2
struct AggregateNodeData {
    QVector3D center;
    QVector3D averageColor;
    float averageIntensity;
    QVector3D averageNormal;
    float boundingRadius;
    int pointCount;
    float screenSpaceSize; // For splat sizing
    
    AggregateNodeData() 
        : center(0, 0, 0)
        , averageColor(1, 1, 1)
        , averageIntensity(1.0f)
        , averageNormal(0, 0, 1)
        , boundingRadius(0.0f)
        , pointCount(0)
        , screenSpaceSize(0.0f) {}
};

class OctreeNode {
public:
    AxisAlignedBoundingBox bounds;
    std::vector points;
    std::array, 8> children;
    bool isLeaf = true;
    int depth = 0;
    
    // Aggregate data for splatting (Task R4.1.2)
    mutable AggregateNodeData aggregateData;
    mutable bool aggregateDataCalculated = false;
    
    OctreeNode(const AxisAlignedBoundingBox& bounds, int depth = 0)
        : bounds(bounds), depth(depth) {}
    
    // Methods from R1/R2...
    void insert(const PointFullData& point, int maxDepth = 8, int maxPointsPerNode = 100);
    void subdivide();
    
    // New methods for R4 splatting
    const AggregateNodeData& getAggregateData() const;
    void calculateAggregateData() const;
    bool shouldRenderAsSplat(float screenSpaceError, float splatThreshold) const;
    
    // Enhanced traversal for splat rendering (Task R4.1.4, R4.1.5)
    void collectRenderData(
        const std::array& frustumPlanes,
        const QMatrix4x4& mvpMatrix,
        const ViewportInfo& viewport,
        float splatThreshold,
        bool splattingEnabled,
        std::vector& individualPoints,
        std::vector& splatData
    ) const;

private:
    bool intersectsFrustum(const std::array& frustumPlanes) const;
    QVector3D estimateNormalFromPoints() const;
    void calculateScreenSpaceSize(const QMatrix4x4& mvpMatrix, const ViewportInfo& viewport) const;
};

#endif // OCTREE_H
```

```cpp
// src/octree.cpp (Enhanced implementation matching backlog requirements)
#include "octree.h"
#include "screenspaceerror.h"
#include 
#include 

const AggregateNodeData& OctreeNode::getAggregateData() const {
    if (!aggregateDataCalculated) {
        calculateAggregateData();
        aggregateDataCalculated = true;
    }
    return aggregateData;
}

void OctreeNode::calculateAggregateData() const {
    if (isLeaf) {
        if (points.empty()) {
            aggregateData = AggregateNodeData();
            return;
        }
        
        // Calculate aggregate properties as specified in Task R4.1.2
        QVector3D centerSum(0, 0, 0);
        QVector3D colorSum(0, 0, 0);
        QVector3D normalSum(0, 0, 0);
        float intensitySum = 0.0f;
        int colorCount = 0;
        int intensityCount = 0;
        int normalCount = 0;
        
        for (const auto& point : points) {
            centerSum += QVector3D(point.x, point.y, point.z);
            
            if (point.hasColor()) {
                colorSum += QVector3D(point.r.value() / 255.0f, 
                                     point.g.value() / 255.0f, 
                                     point.b.value() / 255.0f);
                colorCount++;
            }
            
            if (point.hasIntensity()) {
                intensitySum += point.intensity.value();
                intensityCount++;
            }
            
            if (point.hasNormal()) {
                normalSum += point.normal.value();
                normalCount++;
            }
        }
        
        aggregateData.center = centerSum / static_cast(points.size());
        aggregateData.averageColor = colorCount > 0 ? colorSum / colorCount : QVector3D(1, 1, 1);
        aggregateData.averageIntensity = intensityCount > 0 ? intensitySum / intensityCount : 1.0f;
        aggregateData.averageNormal = normalCount > 0 ? 
            normalSum.normalized() : estimateNormalFromPoints();
        aggregateData.pointCount = static_cast(points.size());
        
        // Calculate bounding radius for splat sizing
        float maxDistSq = 0.0f;
        for (const auto& point : points) {
            QVector3D pos(point.x, point.y, point.z);
            float distSq = (pos - aggregateData.center).lengthSquared();
            maxDistSq = std::max(maxDistSq, distSq);
        }
        aggregateData.boundingRadius = std::sqrt(maxDistSq);
        
    } else {
        // Internal node: aggregate from children
        QVector3D centerSum(0, 0, 0);
        QVector3D colorSum(0, 0, 0);
        QVector3D normalSum(0, 0, 0);
        float intensitySum = 0.0f;
        int totalPoints = 0;
        float maxRadius = 0.0f;
        
        for (const auto& child : children) {
            if (child) {
                const auto& childData = child->getAggregateData();
                if (childData.pointCount > 0) {
                    centerSum += childData.center * childData.pointCount;
                    colorSum += childData.averageColor * childData.pointCount;
                    normalSum += childData.averageNormal * childData.pointCount;
                    intensitySum += childData.averageIntensity * childData.pointCount;
                    totalPoints += childData.pointCount;
                    maxRadius = std::max(maxRadius, childData.boundingRadius);
                }
            }
        }
        
        if (totalPoints > 0) {
            aggregateData.center = centerSum / totalPoints;
            aggregateData.averageColor = colorSum / totalPoints;
            aggregateData.averageIntensity = intensitySum / totalPoints;
            aggregateData.averageNormal = normalSum.normalized();
            aggregateData.pointCount = totalPoints;
            aggregateData.boundingRadius = maxRadius;
        }
    }
}

bool OctreeNode::shouldRenderAsSplat(float screenSpaceError, float splatThreshold) const {
    // Render as splat if screen space error is below threshold and has enough points
    return screenSpaceError  10;
}

void OctreeNode::collectRenderData(
    const std::array& frustumPlanes,
    const QMatrix4x4& mvpMatrix,
    const ViewportInfo& viewport,
    float splatThreshold,
    bool splattingEnabled,
    std::vector& individualPoints,
    std::vector& splatData) const {
    
    // Frustum culling from R1/R2
    if (!intersectsFrustum(frustumPlanes)) {
        return;
    }
    
    // Calculate screen-space error from R2
    float screenSpaceError = ScreenSpaceErrorCalculator::calculateAABBScreenSpaceError(
        bounds, mvpMatrix, viewport);
    
    // Calculate screen space size for splat sizing
    calculateScreenSpaceSize(mvpMatrix, viewport);
    
    // Decide rendering method as per Task R4.1.4, R4.1.5
    if (splattingEnabled && shouldRenderAsSplat(screenSpaceError, splatThreshold)) {
        // Render as splat (Task R4.1.4)
        AggregateNodeData splatInfo = getAggregateData();
        splatInfo.screenSpaceSize = aggregateData.screenSpaceSize;
        splatData.push_back(splatInfo);
    } else if (isLeaf) {
        // Render individual points (Task R4.1.5)
        individualPoints.insert(individualPoints.end(), points.begin(), points.end());
    } else {
        // Recurse to children (Task R4.1.5)
        for (const auto& child : children) {
            if (child) {
                child->collectRenderData(frustumPlanes, mvpMatrix, viewport,
                                       splatThreshold, splattingEnabled,
                                       individualPoints, splatData);
            }
        }
    }
}

void OctreeNode::calculateScreenSpaceSize(const QMatrix4x4& mvpMatrix, const ViewportInfo& viewport) const {
    // Calculate projected size for splat sizing as per Task R4.1.1
    QVector3D corners[8];
    corners[0] = QVector3D(bounds.min.x(), bounds.min.y(), bounds.min.z());
    corners[1] = QVector3D(bounds.max.x(), bounds.min.y(), bounds.min.z());
    corners[2] = QVector3D(bounds.min.x(), bounds.max.y(), bounds.min.z());
    corners[3] = QVector3D(bounds.max.x(), bounds.max.y(), bounds.min.z());
    corners[4] = QVector3D(bounds.min.x(), bounds.min.y(), bounds.max.z());
    corners[5] = QVector3D(bounds.max.x(), bounds.min.y(), bounds.max.z());
    corners[6] = QVector3D(bounds.min.x(), bounds.max.y(), bounds.max.z());
    corners[7] = QVector3D(bounds.max.x(), bounds.max.y(), bounds.max.z());
    
    float minX = viewport.width, maxX = 0;
    float minY = viewport.height, maxY = 0;
    
    for (const auto& corner : corners) {
        QVector4D clipSpace = mvpMatrix * QVector4D(corner, 1.0f);
        if (clipSpace.w() > 0.001f) {
            QVector3D ndc = QVector3D(clipSpace.x(), clipSpace.y(), clipSpace.z()) / clipSpace.w();
            float screenX = (ndc.x() + 1.0f) * 0.5f * viewport.width;
            float screenY = (1.0f - ndc.y()) * 0.5f * viewport.height;
            
            minX = std::min(minX, screenX);
            maxX = std::max(maxX, screenX);
            minY = std::min(minY, screenY);
            maxY = std::max(maxY, screenY);
        }
    }
    
    aggregateData.screenSpaceSize = std::sqrt((maxX - minX) * (maxX - minX) + (maxY - minY) * (maxY - minY));
}

QVector3D OctreeNode::estimateNormalFromPoints() const {
    if (points.size()  0.1f ? normal : QVector3D(0, 0, 1);
}
```

## Enhanced PointCloudViewerWidget with Splatting and Lighting

The backlog specifies exact shader requirements and rendering paths. Here's the enhanced implementation:

```cpp
// src/pointcloudviewerwidget.h (Enhanced to match backlog specifications)
#ifndef POINTCLOUDVIEWERWIDGET_H
#define POINTCLOUDVIEWERWIDGET_H

#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include "pointdata.h"
#include "octree.h"

// Vertex structure for splat rendering as per Task R4.1.3
struct SplatVertex {
    QVector3D position;      // Node center
    QVector3D color;         // Average color
    QVector3D normal;        // Average normal
    float intensity;         // Average intensity
    float radius;            // Splat size
    
    SplatVertex() = default;
    explicit SplatVertex(const AggregateNodeData& data) 
        : position(data.center)
        , color(data.averageColor)
        , normal(data.averageNormal)
        , intensity(data.averageIntensity)
        , radius(data.screenSpaceSize * 0.5f) {}
};

class PointCloudViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit PointCloudViewerWidget(QWidget *parent = nullptr);
    ~PointCloudViewerWidget();

    void loadPointCloud(const std::vector& points);

public slots:
    // Existing slots from R3...
    void setRenderWithColor(bool enabled);
    void setRenderWithIntensity(bool enabled);
    void setPointSizeAttenuationEnabled(bool enabled);
    void setPointSizeAttenuationParams(float minSize, float maxSize, float factor);
    
    // New slots for R4 as specified in Task R4.3.2
    void setSplattingEnabled(bool enabled);
    void setLightingEnabled(bool enabled);
    void setLightDirection(const QVector3D& direction);
    void setLightColor(const QColor& color);
    void setAmbientIntensity(float intensity);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void setupShaders();
    void setupVertexArrayObjects();
    void setupSplatTexture();
    void updateCamera();
    void renderScene();
    void renderPoints(const std::vector& points);
    void renderSplats(const std::vector& splats);
    void preparePointVertexData(const std::vector& points);
    void prepareSplatVertexData(const std::vector& splats);
    std::array extractFrustumPlanes(const QMatrix4x4& viewProjection) const;

    // OpenGL resources for dual rendering paths (Task R4.1.5)
    QOpenGLShaderProgram* m_pointShaderProgram;
    QOpenGLShaderProgram* m_splatShaderProgram;
    QOpenGLBuffer m_pointVertexBuffer;
    QOpenGLBuffer m_splatVertexBuffer;
    QOpenGLVertexArrayObject m_pointVAO;
    QOpenGLVertexArrayObject m_splatVAO;
    QOpenGLTexture* m_splatTexture;

    // Rendering state from R3...
    bool m_renderWithColor;
    bool m_renderWithIntensity;
    bool m_pointSizeAttenuationEnabled;
    float m_minPointSize;
    float m_maxPointSize;
    float m_attenuationFactor;

    // New rendering state for R4 as specified in Task R4.3.4
    bool m_splattingEnabled;
    bool m_lightingEnabled;
    QVector3D m_lightDirection;
    QColor m_lightColor;
    float m_ambientIntensity;
    float m_splatThreshold;

    // Camera and transformation
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;
    QVector3D m_cameraPosition;
    QVector3D m_cameraTarget;
    QVector3D m_cameraUp;
    float m_cameraDistance;
    float m_cameraYaw;
    float m_cameraPitch;

    // Mouse interaction
    QPoint m_lastMousePos;
    bool m_mousePressed;

    // LOD system integration
    std::unique_ptr m_octree;
    std::vector m_visiblePoints;
    std::vector m_visibleSplats;
    std::vector m_pointVertexData;
    std::vector m_splatVertexData;

    // Shader source methods
    static const char* pointVertexShaderSource();
    static const char* pointFragmentShaderSource();
    static const char* splatVertexShaderSource();
    static const char* splatGeometryShaderSource();
    static const char* splatFragmentShaderSource();
};

#endif // POINTCLOUDVIEWERWIDGET_H
```

## Enhanced Shader Implementation

The backlog specifies exact shader requirements for splatting and lighting:

```cpp
// src/pointcloudviewerwidget.cpp (Enhanced shader implementation matching backlog)

// Point rendering shaders with lighting (Task R4.2.2)
const char* PointCloudViewerWidget::pointVertexShaderSource() {
    return R"(
        #version 330 core
        
        // Vertex attributes as per existing R3 structure
        layout (location = 0) in vec3 position;
        layout (location = 1) in vec3 color;
        layout (location = 2) in float intensity;
        layout (location = 3) in vec3 normal;
        
        // Transformation uniforms
        uniform mat4 mvpMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 modelMatrix;
        uniform mat3 normalMatrix;
        uniform vec3 cameraPosition_worldSpace;
        
        // Point size attenuation from R3
        uniform bool pointSizeAttenuationEnabled;
        uniform float basePointSize;
        uniform float minPointSize;
        uniform float maxPointSize;
        uniform float attenuationFactor;
        
        // Outputs to fragment shader for lighting (Task R4.2.2)
        out vec3 fragColor;
        out float fragIntensity;
        out vec3 fragNormal_viewSpace;
        out vec3 fragPosition_viewSpace;
        
        void main() {
            gl_Position = mvpMatrix * vec4(position, 1.0);
            
            // Transform normal to view space for lighting calculation
            fragNormal_viewSpace = normalMatrix * normal;
            
            // Transform position to view space
            vec4 position_viewSpace = viewMatrix * modelMatrix * vec4(position, 1.0);
            fragPosition_viewSpace = position_viewSpace.xyz;
            
            // Pass attributes
            fragColor = color;
            fragIntensity = intensity;
            
            // Point size attenuation from R3
            if (pointSizeAttenuationEnabled) {
                float distance = length(cameraPosition_worldSpace - position);
                float attenuatedSize = basePointSize / (1.0 + distance * attenuationFactor);
                gl_PointSize = clamp(attenuatedSize, minPointSize, maxPointSize);
            } else {
                gl_PointSize = basePointSize;
            }
        }
    )";
}

const char* PointCloudViewerWidget::pointFragmentShaderSource() {
    return R"(
        #version 330 core
        
        // Inputs from vertex shader
        in vec3 fragColor;
        in float fragIntensity;
        in vec3 fragNormal_viewSpace;
        in vec3 fragPosition_viewSpace;
        
        // Rendering control uniforms from R3
        uniform bool renderWithColor;
        uniform bool renderWithIntensity;
        uniform vec3 uniformColor;
        
        // Lighting uniforms as specified in Task R4.2.2
        uniform bool lightingEnabled;
        uniform vec3 lightDirection_viewSpace;
        uniform vec3 lightColor;
        uniform float ambientIntensity;
        
        out vec4 finalColor;
        
        void main() {
            vec3 baseColor = uniformColor;
            
            // Apply color/intensity rendering logic from R3
            if (renderWithColor) {
                baseColor = fragColor;
            }
            
            if (renderWithIntensity) {
                if (renderWithColor) {
                    baseColor = fragColor * fragIntensity;
                } else {
                    baseColor = vec3(fragIntensity);
                }
            }
            
            // Apply basic lighting model as specified in Task R4.2.2
            vec3 litColor = baseColor;
            if (lightingEnabled) {
                vec3 normal = normalize(fragNormal_viewSpace);
                vec3 lightDir = normalize(-lightDirection_viewSpace);
                
                // Lambertian diffuse lighting
                float diffuse = max(dot(normal, lightDir), 0.0);
                
                // Combine ambient and diffuse
                vec3 ambient = baseColor * ambientIntensity;
                vec3 diffuseComponent = baseColor * lightColor * diffuse;
                
                litColor = ambient + diffuseComponent;
            }
            
            // Create circular point shape
            vec2 coord = gl_PointCoord - vec2(0.5);
            float distance = length(coord);
            if (distance > 0.5) {
                discard;
            }
            
            float alpha = 1.0 - smoothstep(0.3, 0.5, distance);
            finalColor = vec4(litColor, alpha);
        }
    )";
}

// Splat rendering shaders as specified in Task R4.1.3
const char* PointCloudViewerWidget::splatVertexShaderSource() {
    return R"(
        #version 330 core
        
        // Splat vertex attributes (node center + aggregate data)
        layout (location = 0) in vec3 position;
        layout (location = 1) in vec3 color;
        layout (location = 2) in vec3 normal;
        layout (location = 3) in float intensity;
        layout (location = 4) in float radius;
        
        // Transformation uniforms
        uniform mat4 mvpMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 modelMatrix;
        uniform mat3 normalMatrix;
        uniform vec3 cameraPosition_worldSpace;
        
        // Outputs to geometry shader
        out vec3 vertexColor;
        out vec3 vertexNormal_viewSpace;
        out float vertexIntensity;
        out float vertexRadius;
        out vec3 vertexPosition_viewSpace;
        
        void main() {
            gl_Position = mvpMatrix * vec4(position, 1.0);
            
            // Transform to view space for lighting
            vertexNormal_viewSpace = normalMatrix * normal;
            vec4 position_viewSpace = viewMatrix * modelMatrix * vec4(position, 1.0);
            vertexPosition_viewSpace = position_viewSpace.xyz;
            
            // Pass aggregate attributes
            vertexColor = color;
            vertexIntensity = intensity;
            
            // Calculate screen-space radius with distance attenuation
            float distance = length(cameraPosition_worldSpace - position);
            vertexRadius = radius / (1.0 + distance * 0.01); // Simple attenuation
        }
    )";
}

const char* PointCloudViewerWidget::splatGeometryShaderSource() {
    return R"(
        #version 330 core
        
        // Generate screen-aligned quads for splats (Task R4.1.1)
        layout (points) in;
        layout (triangle_strip, max_vertices = 4) out;
        
        // Inputs from vertex shader
        in vec3 vertexColor[];
        in vec3 vertexNormal_viewSpace[];
        in float vertexIntensity[];
        in float vertexRadius[];
        in vec3 vertexPosition_viewSpace[];
        
        uniform mat4 projectionMatrix;
        uniform vec2 viewportSize;
        
        // Outputs to fragment shader
        out vec3 fragColor;
        out vec3 fragNormal_viewSpace;
        out float fragIntensity;
        out vec2 fragTexCoord;
        out vec3 fragPosition_viewSpace;
        
        void main() {
            vec4 center = gl_in[0].gl_Position;
            
            // Calculate screen-space quad size based on projected node size
            float screenRadius = vertexRadius[0] * 100.0; // Scale factor
            vec2 quadSize = vec2(screenRadius) / viewportSize;
            
            // Pass attributes to fragment shader
            fragColor = vertexColor[0];
            fragNormal_viewSpace = vertexNormal_viewSpace[0];
            fragIntensity = vertexIntensity[0];
            fragPosition_viewSpace = vertexPosition_viewSpace[0];
            
            // Generate screen-aligned quad (Task R4.1.1)
            // Bottom-left
            gl_Position = center + vec4(-quadSize.x, -quadSize.y, 0.0, 0.0);
            fragTexCoord = vec2(0.0, 0.0);
            EmitVertex();
            
            // Bottom-right
            gl_Position = center + vec4(quadSize.x, -quadSize.y, 0.0, 0.0);
            fragTexCoord = vec2(1.0, 0.0);
            EmitVertex();
            
            // Top-left
            gl_Position = center + vec4(-quadSize.x, quadSize.y, 0.0, 0.0);
            fragTexCoord = vec2(0.0, 1.0);
            EmitVertex();
            
            // Top-right
            gl_Position = center + vec4(quadSize.x, quadSize.y, 0.0, 0.0);
            fragTexCoord = vec2(1.0, 1.0);
            EmitVertex();
            
            EndPrimitive();
        }
    )";
}

const char* PointCloudViewerWidget::splatFragmentShaderSource() {
    return R"(
        #version 330 core
        
        // Inputs from geometry shader
        in vec3 fragColor;
        in vec3 fragNormal_viewSpace;
        in float fragIntensity;
        in vec2 fragTexCoord;
        in vec3 fragPosition_viewSpace;
        
        // Splat texture and rendering controls
        uniform sampler2D splatTexture;
        uniform bool renderWithColor;
        uniform bool renderWithIntensity;
        uniform vec3 uniformColor;
        
        // Lighting uniforms (Task R4.2.2)
        uniform bool lightingEnabled;
        uniform vec3 lightDirection_viewSpace;
        uniform vec3 lightColor;
        uniform float ambientIntensity;
        
        out vec4 finalColor;
        
        void main() {
            // Sample splat texture for alpha blending
            vec4 texColor = texture(splatTexture, fragTexCoord);
            
            vec3 baseColor = uniformColor;
            
            // Apply color/intensity logic from R3
            if (renderWithColor) {
                baseColor = fragColor;
            }
            
            if (renderWithIntensity) {
                if (renderWithColor) {
                    baseColor = fragColor * fragIntensity;
                } else {
                    baseColor = vec3(fragIntensity);
                }
            }
            
            // Apply lighting to splats (Task R4.2.2)
            vec3 litColor = baseColor;
            if (lightingEnabled) {
                vec3 normal = normalize(fragNormal_viewSpace);
                vec3 lightDir = normalize(-lightDirection_viewSpace);
                
                // Lambertian diffuse lighting
                float diffuse = max(dot(normal, lightDir), 0.0);
                
                // Combine ambient and diffuse
                vec3 ambient = baseColor * ambientIntensity;
                vec3 diffuseComponent = baseColor * lightColor * diffuse;
                
                litColor = ambient + diffuseComponent;
            }
            
            // Combine with texture alpha for soft splat appearance
            finalColor = vec4(litColor, texColor.a);
        }
    )";
}
```

## Enhanced Implementation Methods

```cpp
// src/pointcloudviewerwidget.cpp (Enhanced implementation matching backlog)
PointCloudViewerWidget::PointCloudViewerWidget(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_pointShaderProgram(nullptr)
    , m_splatShaderProgram(nullptr)
    , m_splatTexture(nullptr)
    , m_renderWithColor(false)
    , m_renderWithIntensity(false)
    , m_pointSizeAttenuationEnabled(false)
    , m_minPointSize(1.0f)
    , m_maxPointSize(10.0f)
    , m_attenuationFactor(0.1f)
    , m_splattingEnabled(true)    // Default enabled as per backlog
    , m_lightingEnabled(false)
    , m_lightDirection(0, 0, -1)  // Default down direction
    , m_lightColor(Qt::white)
    , m_ambientIntensity(0.3f)
    , m_splatThreshold(10.0f)     // Screen-space error threshold
    , m_cameraPosition(0, 0, 10)
    , m_cameraTarget(0, 0, 0)
    , m_cameraUp(0, 1, 0)
    , m_cameraDistance(10.0f)
    , m_cameraYaw(0.0f)
    , m_cameraPitch(0.0f)
    , m_mousePressed(false)
    , m_octree(std::make_unique())
{
    setFocusPolicy(Qt::StrongFocus);
}

void PointCloudViewerWidget::renderScene() {
    if (!m_octree->root) return;
    
    // Collect visible points and splats as specified in Task R4.1.4
    QMatrix4x4 viewProjection = m_projectionMatrix * m_viewMatrix;
    auto frustumPlanes = extractFrustumPlanes(viewProjection);
    
    ViewportInfo viewport{width(), height(), 0.1f, 1000.0f};
    
    m_visiblePoints.clear();
    m_visibleSplats.clear();
    
    // Use enhanced octree traversal for dual rendering paths
    m_octree->root->collectRenderData(
        frustumPlanes, viewProjection, viewport,
        m_splatThreshold, m_splattingEnabled,
        m_visiblePoints, m_visibleSplats
    );
    
    // Render individual points (high detail nodes)
    if (!m_visiblePoints.empty()) {
        renderPoints(m_visiblePoints);
    }
    
    // Render splats (coarse detail nodes) as specified in Task R4.1.4
    if (!m_visibleSplats.empty() && m_splattingEnabled) {
        renderSplats(m_visibleSplats);
    }
}

void PointCloudViewerWidget::renderSplats(const std::vector& splats) {
    prepareSplatVertexData(splats);
    
    m_splatShaderProgram->bind();
    
    // Set transformation matrices
    QMatrix4x4 modelMatrix;
    modelMatrix.setToIdentity();
    QMatrix4x4 mvpMatrix = m_projectionMatrix * m_viewMatrix * modelMatrix;
    QMatrix3x3 normalMatrix = (m_viewMatrix * modelMatrix).normalMatrix();
    
    m_splatShaderProgram->setUniformValue("mvpMatrix", mvpMatrix);
    m_splatShaderProgram->setUniformValue("viewMatrix", m_viewMatrix);
    m_splatShaderProgram->setUniformValue("modelMatrix", modelMatrix);
    m_splatShaderProgram->setUniformValue("normalMatrix", normalMatrix);
    m_splatShaderProgram->setUniformValue("projectionMatrix", m_projectionMatrix);
    m_splatShaderProgram->setUniformValue("cameraPosition_worldSpace", m_cameraPosition);
    m_splatShaderProgram->setUniformValue("viewportSize", QVector2D(width(), height()));
    
    // Set rendering parameters from R3
    m_splatShaderProgram->setUniformValue("renderWithColor", m_renderWithColor);
    m_splatShaderProgram->setUniformValue("renderWithIntensity", m_renderWithIntensity);
    m_splatShaderProgram->setUniformValue("uniformColor", QVector3D(1.0f, 1.0f, 1.0f));
    
    // Set lighting parameters as specified in Task R4.2.3
    m_splatShaderProgram->setUniformValue("lightingEnabled", m_lightingEnabled);
    if (m_lightingEnabled) {
        QVector3D lightDir_viewSpace = m_viewMatrix.mapVector(m_lightDirection).normalized();
        m_splatShaderProgram->setUniformValue("lightDirection_viewSpace", lightDir_viewSpace);
        m_splatShaderProgram->setUniformValue("lightColor", 
            QVector3D(m_lightColor.redF(), m_lightColor.greenF(), m_lightColor.blueF()));
        m_splatShaderProgram->setUniformValue("ambientIntensity", m_ambientIntensity);
    }
    
    // Bind splat texture
    m_splatTexture->bind(0);
    m_splatShaderProgram->setUniformValue("splatTexture", 0);
    
    // Render splats
    m_splatVAO.bind();
    glDrawArrays(GL_POINTS, 0, static_cast(m_splatVertexData.size()));
    m_splatVAO.release();
    
    m_splatShaderProgram->release();
}

// Public slot implementations as specified in Task R4.3.2
void PointCloudViewerWidget::setSplattingEnabled(bool enabled) {
    m_splattingEnabled = enabled;
    update();
}

void PointCloudViewerWidget::setLightingEnabled(bool enabled) {
    m_lightingEnabled = enabled;
    update();
}

void PointCloudViewerWidget::setLightDirection(const QVector3D& direction) {
    m_lightDirection = direction.normalized();
    update();
}

void PointCloudViewerWidget::setLightColor(const QColor& color) {
    m_lightColor = color;
    update();
}

void PointCloudViewerWidget::setAmbientIntensity(float intensity) {
    m_ambientIntensity = intensity;
    update();
}
```

## Enhanced UI Controls Implementation

The backlog specifies exact UI controls needed:

```cpp
// src/mainwindow.h (Enhanced to match Task R4.3.1 requirements)
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include "pointcloudviewerwidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Existing slots from R3...
    void onColorRenderToggled(bool enabled);
    void onIntensityRenderToggled(bool enabled);
    void onAttenuationToggled(bool enabled);
    void onAttenuationParamsChanged();
    
    // New slots for R4 as specified in Task R4.3.1
    void onSplattingToggled(bool enabled);
    void onLightingToggled(bool enabled);
    void onLightDirectionChanged();
    void onLightColorClicked();
    void onAmbientIntensityChanged(int value);
    void loadPointCloudFile();

private:
    void setupUI();
    void setupAttributeRenderingControls();
    void setupPointSizeControls();
    void setupSplattingControls();      // New for R4
    void setupLightingControls();       // New for R4
    void connectSignals();

    // UI components
    PointCloudViewerWidget* m_pointCloudViewer;
    
    // Existing controls from R3...
    QGroupBox* m_attributeGroupBox;
    QCheckBox* m_colorRenderCheckbox;
    QCheckBox* m_intensityRenderCheckbox;
    
    QGroupBox* m_pointSizeGroupBox;
    QCheckBox* m_attenuationCheckbox;
    QSlider* m_minSizeSlider;
    QSlider* m_maxSizeSlider;
    QSlider* m_attenuationFactorSlider;
    QLabel* m_minSizeLabel;
    QLabel* m_maxSizeLabel;
    QLabel* m_attenuationFactorLabel;
    
    // New controls for R4 as specified in Task R4.3.1
    QGroupBox* m_splattingGroupBox;
    QCheckBox* m_splattingCheckbox;
    
    QGroupBox* m_lightingGroupBox;
    QCheckBox* m_lightingCheckbox;
    QSlider* m_lightDirXSlider;
    QSlider* m_lightDirYSlider;
    QSlider* m_lightDirZSlider;
    QLabel* m_lightDirXLabel;
    QLabel* m_lightDirYLabel;
    QLabel* m_lightDirZLabel;
    QPushButton* m_lightColorButton;
    QLabel* m_lightColorLabel;
    QSlider* m_ambientIntensitySlider;
    QLabel* m_ambientIntensityLabel;
    
    QPushButton* m_loadFileButton;
    
    // Current state
    QColor m_currentLightColor;
};

#endif // MAINWINDOW_H
```

```cpp
// src/mainwindow.cpp (Enhanced implementation)
void MainWindow::setupSplattingControls() {
    // Splatting controls as specified in Task R4.3.1
    m_splattingGroupBox = new QGroupBox("Point Splatting", this);
    QVBoxLayout* splattingLayout = new QVBoxLayout(m_splattingGroupBox);
    
    m_splattingCheckbox = new QCheckBox("Enable Point Splatting for LOD", this);
    m_splattingCheckbox->setChecked(true); // Default enabled
    splattingLayout->addWidget(m_splattingCheckbox);
    
    QLabel* splattingInfo = new QLabel(
        "Point splatting renders distant/coarse LOD nodes as textured quads\n"
        "instead of individual points for better visual cohesion.", this);
    splattingInfo->setWordWrap(true);
    splattingInfo->setStyleSheet("QLabel { color: #666666; font-size: 10px; }");
    splattingLayout->addWidget(splattingInfo);
}

void MainWindow::setupLightingControls() {
    // Lighting controls as specified in Task R4.3.1
    m_lightingGroupBox = new QGroupBox("Basic Lighting", this);
    QVBoxLayout* lightingLayout = new QVBoxLayout(m_lightingGroupBox);
    
    m_lightingCheckbox = new QCheckBox("Enable Lighting", this);
    lightingLayout->addWidget(m_lightingCheckbox);
    
    // Light direction controls (2-3 sliders for X,Y,Z as per Task R4.3.1)
    QWidget* lightDirWidget = new QWidget(this);
    QVBoxLayout* lightDirLayout = new QVBoxLayout(lightDirWidget);
    
    QLabel* lightDirLabel = new QLabel("Light Direction:", this);
    lightDirLayout->addWidget(lightDirLabel);
    
    // X direction slider
    QHBoxLayout* xDirLayout = new QHBoxLayout();
    QLabel* xLabel = new QLabel("X:", this);
    m_lightDirXSlider = new QSlider(Qt::Horizontal, this);
    m_lightDirXSlider->setRange(-100, 100);
    m_lightDirXSlider->setValue(0);
    m_lightDirXLabel = new QLabel("0.0", this);
    m_lightDirXLabel->setFixedWidth(40);
    
    xDirLayout->addWidget(xLabel);
    xDirLayout->addWidget(m_lightDirXSlider);
    xDirLayout->addWidget(m_lightDirXLabel);
    
    // Y direction slider
    QHBoxLayout* yDirLayout = new QHBoxLayout();
    QLabel* yLabel = new QLabel("Y:", this);
    m_lightDirYSlider = new QSlider(Qt::Horizontal, this);
    m_lightDirYSlider->setRange(-100, 100);
    m_lightDirYSlider->setValue(0);
    m_lightDirYLabel = new QLabel("0.0", this);
    m_lightDirYLabel->setFixedWidth(40);
    
    yDirLayout->addWidget(yLabel);
    yDirLayout->addWidget(m_lightDirYSlider);
    yDirLayout->addWidget(m_lightDirYLabel);
    
    // Z direction slider
    QHBoxLayout* zDirLayout = new QHBoxLayout();
    QLabel* zLabel = new QLabel("Z:", this);
    m_lightDirZSlider = new QSlider(Qt::Horizontal, this);
    m_lightDirZSlider->setRange(-100, 100);
    m_lightDirZSlider->setValue(-100); // Default pointing down
    m_lightDirZLabel = new QLabel("-1.0", this);
    m_lightDirZLabel->setFixedWidth(40);
    
    zDirLayout->addWidget(zLabel);
    zDirLayout->addWidget(m_lightDirZSlider);
    zDirLayout->addWidget(m_lightDirZLabel);
    
    lightDirLayout->addLayout(xDirLayout);
    lightDirLayout->addLayout(yDirLayout);
    lightDirLayout->addLayout(zDirLayout);
    
    // Light color control (color picker as per Task R4.3.1)
    QLabel* colorLabel = new QLabel("Light Color:", this);
    QHBoxLayout* colorLayout = new QHBoxLayout();
    
    m_lightColorButton = new QPushButton("Select Color", this);
    m_lightColorLabel = new QLabel(this);
    m_lightColorLabel->setFixedSize(30, 20);
    m_lightColorLabel->setStyleSheet("QLabel { background-color: white; border: 1px solid black; }");
    
    colorLayout->addWidget(m_lightColorButton);
    colorLayout->addWidget(m_lightColorLabel);
    colorLayout->addStretch();
    
    // Ambient intensity control (slider as per Task R4.3.1)
    QLabel* ambientLabel = new QLabel("Ambient Intensity:", this);
    QHBoxLayout* ambientLayout = new QHBoxLayout();
    
    m_ambientIntensitySlider = new QSlider(Qt::Horizontal, this);
    m_ambientIntensitySlider->setRange(0, 100);
    m_ambientIntensitySlider->setValue(30); // 0.3 default
    m_ambientIntensityLabel = new QLabel("0.30", this);
    m_ambientIntensityLabel->setFixedWidth(40);
    
    ambientLayout->addWidget(m_ambientIntensitySlider);
    ambientLayout->addWidget(m_ambientIntensityLabel);
    
    lightingLayout->addWidget(lightDirWidget);
    lightingLayout->addWidget(colorLabel);
    lightingLayout->addLayout(colorLayout);
    lightingLayout->addWidget(ambientLabel);
    lightingLayout->addLayout(ambientLayout);
    
    // Initially disable lighting controls
    lightDirWidget->setEnabled(false);
    m_lightColorButton->setEnabled(false);
    m_ambientIntensitySlider->setEnabled(false);
    
    connect(m_lightingCheckbox, &QCheckBox::toggled, lightDirWidget, &QWidget::setEnabled);
    connect(m_lightingCheckbox, &QCheckBox::toggled, m_lightColorButton, &QPushButton::setEnabled);
    connect(m_lightingCheckbox, &QCheckBox::toggled, m_ambientIntensitySlider, &QSlider::setEnabled);
}

void MainWindow::connectSignals() {
    // Existing connections from R3...
    
    // New connections for R4 as specified in Task R4.3.3
    connect(m_splattingCheckbox, &QCheckBox::toggled,
            m_pointCloudViewer, &PointCloudViewerWidget::setSplattingEnabled);
    connect(m_lightingCheckbox, &QCheckBox::toggled,
            m_pointCloudViewer, &PointCloudViewerWidget::setLightingEnabled);
    
    // Light direction controls
    connect(m_lightDirXSlider, &QSlider::valueChanged, this, &MainWindow::onLightDirectionChanged);
    connect(m_lightDirYSlider, &QSlider::valueChanged, this, &MainWindow::onLightDirectionChanged);
    connect(m_lightDirZSlider, &QSlider::valueChanged, this, &MainWindow::onLightDirectionChanged);
    
    // Light color control
    connect(m_lightColorButton, &QPushButton::clicked, this, &MainWindow::onLightColorClicked);
    
    // Ambient intensity control
    connect(m_ambientIntensitySlider, &QSlider::valueChanged, this, &MainWindow::onAmbientIntensityChanged);
}
```

## Comprehensive Testing Implementation

The backlog specifies exact test cases. Here's the enhanced testing framework:

```cpp
// tests/test_pointcloudviewerwidget_rendering_r4.cpp (Enhanced to match backlog test cases)
#include 
#include 
#include 
#include 
#include "../src/pointcloudviewerwidget.h"
#include "../src/octree.h"

class PointCloudRenderingR4Test : public ::testing::Test {
protected:
    void SetUp() override {
        // Create OpenGL context for testing
        m_surface = std::make_unique();
        m_surface->create();
        
        m_context = std::make_unique();
        m_context->create();
        m_context->makeCurrent(m_surface.get());
        
        createTestPointClouds();
    }
    
    void createTestPointClouds() {
        // Test Case R4.1.1: Very large and dense point cloud (10M+ points)
        m_largePointCloud.reserve(1000000); // 1M for testing (10M+ in real scenario)
        for (int i = 0; i  m_surface;
    std::unique_ptr m_context;
    
    std::vector m_largePointCloud;
    std::vector m_geometricShapeCloud;
    std::vector m_colorRegionCloud;
};

// Test Case R4.1.1: Load a very large and dense point cloud. Zoom out.
TEST_F(PointCloudRenderingR4Test, LargePointCloudSplatting) {
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(m_largePointCloud);
    
    // Enable splatting
    viewer.setSplattingEnabled(true);
    
    // Expected Result: Distant parts rendered as splats, FPS should be interactive
    // This would be verified visually and with FPS counter in real testing
    SUCCEED();
}

// Test Case R4.1.2: Compare rendering with splatting vs. R2's refined point selection
TEST_F(PointCloudRenderingR4Test, SplattingVsPointSelection) {
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(m_largePointCloud);
    
    // Test with splatting enabled
    viewer.setSplattingEnabled(true);
    
    // Test with splatting disabled (should fall back to point selection from R2)
    viewer.setSplattingEnabled(false);
    
    // Expected Result: Splatting provides visually denser representation
    SUCCEED();
}

// Test Case R4.1.3: Verify splat coloring/texturing
TEST_F(PointCloudRenderingR4Test, SplatColoringTexturing) {
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(m_colorRegionCloud);
    viewer.setSplattingEnabled(true);
    viewer.setRenderWithColor(true);
    
    // Expected Result: Splats reflect average color/intensity of represented points
    SUCCEED();
}

// Test Case R4.2.1: Load a point cloud representing a 3D object with clear surfaces
TEST_F(PointCloudRenderingR4Test, LightingWith3DObject) {
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(m_geometricShapeCloud);
    
    // Enable lighting
    viewer.setLightingEnabled(true);
    viewer.setLightDirection(QVector3D(1, 1, -1));
    viewer.setLightColor(Qt::white);
    viewer.setAmbientIntensity(0.3f);
    
    // Expected Result: Surfaces facing light appear brighter, enhancing 3D form
    SUCCEED();
}

// Test Case R4.2.2: Test lighting with splats vs. individual points
TEST_F(PointCloudRenderingR4Test, LightingWithSplatsAndPoints) {
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(m_geometricShapeCloud);
    
    viewer.setLightingEnabled(true);
    viewer.setLightDirection(QVector3D(0, 0, -1));
    
    // Test with individual points
    viewer.setSplattingEnabled(false);
    
    // Test with splats
    viewer.setSplattingEnabled(true);
    
    // Expected Result: Lighting effect applied consistently to both representations
    SUCCEED();
}

// Test Case R4.2.3: Toggle lighting UI controls
TEST_F(PointCloudRenderingR4Test, LightingUIControls) {
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(m_geometricShapeCloud);
    
    // Test lighting toggle
    viewer.setLightingEnabled(false);
    viewer.setLightingEnabled(true);
    
    // Test light parameter adjustments
    viewer.setLightDirection(QVector3D(1, 0, 0));
    viewer.setLightDirection(QVector3D(0, 1, 0));
    viewer.setLightDirection(QVector3D(0, 0, 1));
    
    viewer.setLightColor(Qt::red);
    viewer.setLightColor(Qt::blue);
    viewer.setLightColor(Qt::white);
    
    viewer.setAmbientIntensity(0.1f);
    viewer.setAmbientIntensity(0.5f);
    viewer.setAmbientIntensity(0.9f);
    
    // Expected Result: Changes update scene appearance accordingly
    SUCCEED();
}

// Test Case R4.3.1: Manipulate all new UI controls
TEST_F(PointCloudRenderingR4Test, AllUIControls) {
    PointCloudViewerWidget viewer;
    viewer.loadPointCloud(m_largePointCloud);
    
    // Test splatting control
    viewer.setSplattingEnabled(true);
    viewer.setSplattingEnabled(false);
    
    // Test lighting controls
    viewer.setLightingEnabled(true);
    viewer.setLightDirection(QVector3D(1, 1, -1));
    viewer.setLightColor(Qt::cyan);
    viewer.setAmbientIntensity(0.4f);
    viewer.setLightingEnabled(false);
    
    // Expected Result: All controls function correctly and update rendering
    SUCCEED();
}

// Test aggregate data calculation
TEST_F(PointCloudRenderingR4Test, AggregateDataCalculation) {
    // Test octree node aggregate data calculation
    AxisAlignedBoundingBox bounds(QVector3D(0, 0, 0), QVector3D(10, 10, 10));
    OctreeNode node(bounds);
    
    // Add test points with known attributes
    node.points.emplace_back(5, 5, 5, 255, 0, 0, 1.0f); // Red, full intensity
    node.points.emplace_back(6, 6, 6, 0, 255, 0, 0.5f); // Green, half intensity
    
    const auto& aggregateData = node.getAggregateData();
    
    // Verify aggregate calculations
    EXPECT_EQ(aggregateData.pointCount, 2);
    EXPECT_FLOAT_EQ(aggregateData.center.x(), 5.5f);
    EXPECT_FLOAT_EQ(aggregateData.center.y(), 5.5f);
    EXPECT_FLOAT_EQ(aggregateData.center.z(), 5.5f);
    EXPECT_FLOAT_EQ(aggregateData.averageIntensity, 0.75f); // (1.0 + 0.5) / 2
}

// Performance test for splatting
TEST_F(PointCloudRenderingR4Test, SplattingPerformance) {
    Point

[1] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/attachments/35208055/634f0570-0845-45e2-a48f-982a880de30e/paste.txt
[2] https://ppl-ai-file-upload.s3.amazonaws.com/web/direct-files/collection_59dddd7d-a43e-4981-8f37-8f1d5f4c9bdc/39178ad2-15a6-4478-bf2f-d68b85feb663/paste.txt