#ifndef LASHEADERMETADATA_H
#define LASHEADERMETADATA_H

#include <QString>
// #include <QVector3D>  // Commented out for testing compatibility
#include <cstdint>

// Simple 3D vector structure for testing compatibility
struct Vector3D { double x, y, z; };

// Struct to hold key metadata extracted from a LAS file header
struct LasHeaderMetadata {
    uint32_t numberOfPointRecords = 0; // Total number of points in the file
    Vector3D minBounds;                // Minimum X, Y, Z coordinates of the bounding box
    Vector3D maxBounds;                // Maximum X, Y, Z coordinates of the bounding box
    QString filePath;                  // The path of the file this metadata belongs to (for display)
    // Additional relevant header information can be added here in future sprints
};

#endif // LASHEADERMETADATA_H
