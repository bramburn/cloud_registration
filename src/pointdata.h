#ifndef POINTDATA_H
#define POINTDATA_H

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

#endif // POINTDATA_H
