#ifndef POINTDATA_H
#define POINTDATA_H

#include <QVector3D>
#include <QMatrix4x4>
#include "octree.h"

// Simple 3D point structure for algorithms and analysis
struct Point3D {
    float x, y, z;
    float intensity;
    bool hasIntensity;

    Point3D() : x(0), y(0), z(0), intensity(0), hasIntensity(false) {}
    Point3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_), intensity(0), hasIntensity(false) {}
    Point3D(float x_, float y_, float z_, float intensity_) : x(x_), y(y_), z(z_), intensity(intensity_), hasIntensity(true) {}

    // Conversion from PointFullData
    explicit Point3D(const PointFullData& point) : x(point.x), y(point.y), z(point.z), intensity(point.intensity), hasIntensity(true) {}

    // Conversion to QVector3D
    QVector3D toQVector3D() const { return QVector3D(x, y, z); }
};

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
