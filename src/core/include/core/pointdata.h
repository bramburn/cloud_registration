#ifndef POINTDATA_H
#define POINTDATA_H

#include <QVector3D>
#include <QMatrix4x4>
#include "octree.h"

// Vertex data structure for OpenGL (interleaved X,Y,Z,R,G,B,I as per Sprint R3 backlog)
struct VertexData {
    float position[3];    // X, Y, Z
    float color[3];       // R, G, B (normalized 0-1)
    float intensity;      // Intensity (0-1)
    
    VertexData() : position{0,0,0}, color{1,1,1}, intensity(1.0f) {}
    
    explicit VertexData(const PointFullData& point) {
        position[0] = point.x;
        position[1] = point.y;
        position[2] = point.z;
        
        point.getNormalizedColor(color[0], color[1], color[2]);
        intensity = point.hasIntensity() ? point.intensity.value() : 1.0f;
    }
};

// Sprint R4: Splat vertex data structure for splatting (Task R4.1.3)
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

// Viewport information for screen-space error calculations
struct ViewportInfo {
    int width = 1920;
    int height = 1080;
    float fov = 45.0f;
    QMatrix4x4 viewMatrix;
    QMatrix4x4 projectionMatrix;
    QVector3D cameraPosition;

    ViewportInfo() = default;
};

#endif // POINTDATA_H
