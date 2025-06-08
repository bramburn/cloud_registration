#ifndef IFORMATWRITER_H
#define IFORMATWRITER_H

#include <QString>
#include <QVariant>
#include <vector>

/**
 * @brief Point structure for export operations
 */
struct Point {
    float x, y, z;
    float intensity = 0.0f;
    uint8_t r = 255, g = 255, b = 255;
    
    Point() = default;
    Point(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    Point(float x_, float y_, float z_, float intensity_) 
        : x(x_), y(y_), z(z_), intensity(intensity_) {}
    Point(float x_, float y_, float z_, uint8_t r_, uint8_t g_, uint8_t b_)
        : x(x_), y(y_), z(z_), r(r_), g(g_), b(b_) {}
};

/**
 * @brief Header information for export files
 */
struct HeaderInfo {
    QString projectName;
    QString description;
    QString coordinateSystem;
    size_t pointCount = 0;
    double minX = 0.0, minY = 0.0, minZ = 0.0;
    double maxX = 0.0, maxY = 0.0, maxZ = 0.0;
    QVariantMap customFields;
};

/**
 * @brief Abstract base class for format writers
 * 
 * Sprint 6 User Story 1: Multi-Format Point Cloud Export
 * This interface defines the contract for all format writers,
 * enabling consistent export functionality across different file formats.
 */
class IFormatWriter
{
public:
    virtual ~IFormatWriter() = default;
    
    /**
     * @brief Open file for writing
     * @param path Output file path
     * @return true if successful
     */
    virtual bool open(const QString& path) = 0;
    
    /**
     * @brief Write header information
     * @param info Header data
     * @return true if successful
     */
    virtual bool writeHeader(const HeaderInfo& info) = 0;
    
    /**
     * @brief Write a single point
     * @param point Point data
     * @return true if successful
     */
    virtual bool writePoint(const Point& point) = 0;
    
    /**
     * @brief Close file and finalize
     * @return true if successful
     */
    virtual bool close() = 0;
    
    /**
     * @brief Get format-specific file extension
     * @return File extension (e.g., "e57", "las")
     */
    virtual QString getFileExtension() const = 0;
    
    /**
     * @brief Get format description
     * @return Human-readable format name
     */
    virtual QString getFormatDescription() const = 0;
    
    /**
     * @brief Check if format supports color
     * @return true if color is supported
     */
    virtual bool supportsColor() const = 0;
    
    /**
     * @brief Check if format supports intensity
     * @return true if intensity is supported
     */
    virtual bool supportsIntensity() const = 0;
};

#endif // IFORMATWRITER_H
