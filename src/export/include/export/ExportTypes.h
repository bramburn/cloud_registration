#pragma once

#include <QString>
#include <QDateTime>
#include <QVector3D>
#include <QMatrix4x4>
#include <QStringList>

/**
 * @brief Export format enumeration
 */
enum class ExportFormat
{
    E57,        // E57 format
    LAS,        // LAS/LAZ format
    PLY,        // PLY format
    XYZ,        // Simple XYZ text format
    PCD,        // Point Cloud Data format
    OBJ         // Wavefront OBJ format
};

/**
 * @brief Coordinate system enumeration for export
 */
enum class CoordinateSystem
{
    Local,      // Local coordinate system
    UTM,        // Universal Transverse Mercator
    Geographic, // Geographic (WGS84)
    Custom      // Custom coordinate system
};

/**
 * @brief Compression level enumeration
 */
enum class CompressionLevel
{
    None,       // No compression
    Low,        // Low compression
    Medium,     // Medium compression
    High        // High compression
};

/**
 * @brief Header information for exported files
 */
struct HeaderInfo
{
    QString title;
    QString description;
    QString author;
    QString organization;
    QString software;
    QString version;
    QDateTime creationDate;
    QString coordinateSystemName;
    QString units;
    QVector3D boundingBoxMin;
    QVector3D boundingBoxMax;
    int totalPoints;
    
    HeaderInfo()
        : creationDate(QDateTime::currentDateTime())
        , units("meters")
        , totalPoints(0)
    {}
};

/**
 * @brief Export options configuration
 */
struct ExportOptions
{
    // File settings
    QString outputPath;
    ExportFormat format;
    bool overwriteExisting;
    
    // Coordinate system
    CoordinateSystem coordinateSystem;
    QMatrix4x4 transformationMatrix;
    QString customCRS;
    
    // Data selection
    bool exportColors;
    bool exportNormals;
    bool exportIntensity;
    bool exportClassification;
    bool exportTimestamp;
    
    // Quality settings
    CompressionLevel compression;
    float precision;
    bool validateOutput;
    
    // Filtering
    QVector3D boundingBoxMin;
    QVector3D boundingBoxMax;
    bool useBoundingBoxFilter;
    float intensityMin;
    float intensityMax;
    bool useIntensityFilter;
    QStringList classificationFilter;
    
    // Header information
    HeaderInfo headerInfo;
    
    // Performance
    int maxPointsPerChunk;
    bool useMultiThreading;
    int threadCount;
    
    // Progress reporting
    bool reportProgress;
    int progressUpdateInterval;
    
    ExportOptions()
        : format(ExportFormat::E57)
        , overwriteExisting(false)
        , coordinateSystem(CoordinateSystem::Local)
        , transformationMatrix(QMatrix4x4())
        , exportColors(true)
        , exportNormals(true)
        , exportIntensity(true)
        , exportClassification(false)
        , exportTimestamp(false)
        , compression(CompressionLevel::Medium)
        , precision(0.001f)
        , validateOutput(true)
        , useBoundingBoxFilter(false)
        , intensityMin(0.0f)
        , intensityMax(1.0f)
        , useIntensityFilter(false)
        , maxPointsPerChunk(1000000)
        , useMultiThreading(true)
        , threadCount(-1)  // Auto-detect
        , reportProgress(true)
        , progressUpdateInterval(1000)
    {}
};

/**
 * @brief Export result information
 */
struct ExportResult
{
    bool success;
    QString errorMessage;
    QString outputPath;
    qint64 fileSize;
    int pointsExported;
    QDateTime exportTime;
    double exportDuration;
    QString formatUsed;
    QString compressionUsed;
    
    // Quality metrics
    bool validationPassed;
    QString validationMessage;
    QVector3D actualBoundingBoxMin;
    QVector3D actualBoundingBoxMax;
    
    // Performance metrics
    double averagePointsPerSecond;
    qint64 memoryUsed;
    int threadsUsed;
    
    ExportResult()
        : success(false)
        , fileSize(0)
        , pointsExported(0)
        , exportTime(QDateTime::currentDateTime())
        , exportDuration(0.0)
        , validationPassed(false)
        , averagePointsPerSecond(0.0)
        , memoryUsed(0)
        , threadsUsed(1)
    {}
};

/**
 * @brief Export statistics for monitoring
 */
struct ExportStatistics
{
    int totalPointsProcessed;
    int pointsFiltered;
    int pointsTransformed;
    int pointsWritten;
    double processingTime;
    double writingTime;
    double validationTime;
    
    ExportStatistics()
        : totalPointsProcessed(0)
        , pointsFiltered(0)
        , pointsTransformed(0)
        , pointsWritten(0)
        , processingTime(0.0)
        , writingTime(0.0)
        , validationTime(0.0)
    {}
};

/**
 * @brief Utility functions for export types
 */
namespace ExportUtils
{
    /**
     * @brief Get file extension for export format
     */
    QString getFileExtension(ExportFormat format);
    
    /**
     * @brief Get format description
     */
    QString getFormatDescription(ExportFormat format);
    
    /**
     * @brief Check if format supports feature
     */
    bool supportsColors(ExportFormat format);
    bool supportsNormals(ExportFormat format);
    bool supportsIntensity(ExportFormat format);
    bool supportsClassification(ExportFormat format);
    bool supportsTimestamp(ExportFormat format);
    bool supportsCompression(ExportFormat format);
    
    /**
     * @brief Validate export options
     */
    QString validateExportOptions(const ExportOptions& options);
    
    /**
     * @brief Get default options for format
     */
    ExportOptions getDefaultOptions(ExportFormat format);
}
