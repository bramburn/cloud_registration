#ifndef LASHEADERMETADATA_H
#define LASHEADERMETADATA_H

#include <QString>
// #include <QVector3D>  // Commented out for testing compatibility
#include <cstdint>

// Simple 3D vector structure for testing compatibility
struct Vector3D
{
    double x, y, z;
};

// Struct to hold key metadata extracted from a LAS file header
struct LasHeaderMetadata
{
    uint32_t numberOfPointRecords = 0;  // Total number of points in the file
    Vector3D minBounds;                 // Minimum X, Y, Z coordinates of the bounding box
    Vector3D maxBounds;                 // Maximum X, Y, Z coordinates of the bounding box
    QString filePath;                   // The path of the file this metadata belongs to (for display)

    // Sprint 1.3: Enhanced metadata for better error reporting and display
    uint8_t versionMajor = 0;     // LAS version major (should be 1)
    uint8_t versionMinor = 0;     // LAS version minor (2, 3, or 4)
    uint8_t pointDataFormat = 0;  // Point Data Record Format (0-3 supported)
    QString systemIdentifier;     // System that created the file
    QString generatingSoftware;   // Software that created the file
};

#endif  // LASHEADERMETADATA_H
