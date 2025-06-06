#pragma once

#include <QString>
#include <QVector>
#include <QList>
#include <QException>
#include <QObject>
#include <QMutex>
#include <memory>
#include <vector>

// Forward declarations to avoid including E57Format.h in header
namespace e57 {
    class ImageFile;
    class Reader;
    class Writer;
    class StructureNode;
    class CompressedVectorNode;
    class VectorNode;
}

/**
 * @brief Point data structure for E57DataManager
 * 
 * This structure holds point cloud data with optional color and intensity attributes
 * as specified in the A1 sprint requirements.
 */
struct PointData {
    double x, y, z;                    // Position coordinates
    uint8_t r = 255, g = 255, b = 255; // RGB color (0-255 range)
    float intensity = 1.0f;            // Intensity (normalized 0-1 range)
    bool hasColor = false;             // Whether color data is valid
    bool hasIntensity = false;         // Whether intensity data is valid
    
    // Default constructor
    PointData() : x(0.0), y(0.0), z(0.0) {}
    
    // Constructor with position only
    PointData(double x, double y, double z) 
        : x(x), y(y), z(z) {}
    
    // Constructor with position and color
    PointData(double x, double y, double z, uint8_t r, uint8_t g, uint8_t b)
        : x(x), y(y), z(z), r(r), g(g), b(b), hasColor(true) {}
    
    // Constructor with position and intensity
    PointData(double x, double y, double z, float intensity)
        : x(x), y(y), z(z), intensity(intensity), hasIntensity(true) {}
    
    // Constructor with position, color, and intensity
    PointData(double x, double y, double z, uint8_t r, uint8_t g, uint8_t b, float intensity)
        : x(x), y(y), z(z), r(r), g(g), b(b), intensity(intensity), hasColor(true), hasIntensity(true) {}
};

/**
 * @brief Scan metadata structure
 * 
 * Holds metadata information for individual scans within an E57 file.
 */
struct ScanMetadata {
    QString guid;                      // Unique identifier for the scan
    QString name;                      // Human-readable scan name
    QString acquisitionTime;           // When the scan was acquired
    double pose[6] = {0,0,0,0,0,0};   // Position and orientation (X,Y,Z,Roll,Pitch,Yaw)
    size_t pointCount = 0;             // Number of points in the scan
    
    // Bounding box information
    double minX = 0, minY = 0, minZ = 0;
    double maxX = 0, maxY = 0, maxZ = 0;
    
    // Data availability flags
    bool hasColorData = false;
    bool hasIntensityData = false;
};

/**
 * @brief Custom exception class for E57DataManager operations
 */
class E57Exception : public QException {
public:
    explicit E57Exception(const QString& message) : m_message(message) {}
    
    void raise() const override { throw *this; }
    E57Exception* clone() const override { return new E57Exception(*this); }
    
    QString message() const { return m_message; }
    const char* what() const noexcept override { return m_message.toUtf8().constData(); }

private:
    QString m_message;
};

/**
 * @brief E57DataManager - High-level interface for E57 file operations
 * 
 * This class provides a simplified, robust interface for reading from and writing to
 * E57 files using the libE57Format library. It abstracts the complexities of the E57
 * format and provides progress reporting and error handling.
 * 
 * Key features:
 * - Import multi-scan E57 files with XYZ, color, and intensity data
 * - Export point clouds to compliant E57 files
 * - Progress reporting for large file operations
 * - Robust error handling with user-friendly messages
 * - Thread-safe operations
 */
class E57DataManager : public QObject {
    Q_OBJECT

public:
    explicit E57DataManager(QObject* parent = nullptr);
    ~E57DataManager();

    /**
     * @brief Import an E57 file and extract all scans
     * @param filePath Path to the E57 file to import
     * @return Vector of scans, each containing a vector of points
     * @throws E57Exception on error
     */
    QVector<QVector<PointData>> importE57File(const QString& filePath);

    /**
     * @brief Export point cloud data to an E57 file
     * @param filePath Path where the E57 file should be saved
     * @param scans Vector of scans to export
     * @throws E57Exception on error
     */
    void exportE57File(const QString& filePath, const QVector<QVector<PointData>>& scans);

    /**
     * @brief Get metadata for all scans in an E57 file without loading point data
     * @param filePath Path to the E57 file
     * @return Vector of scan metadata
     * @throws E57Exception on error
     */
    QVector<ScanMetadata> getScanMetadata(const QString& filePath);

    /**
     * @brief Check if a file is a valid E57 file
     * @param filePath Path to the file to check
     * @return true if the file is a valid E57 file, false otherwise
     */
    bool isValidE57File(const QString& filePath);

    /**
     * @brief Get the last error message
     * @return Last error message, or empty string if no error
     */
    QString getLastError() const;

signals:
    /**
     * @brief Emitted during import/export operations to report progress
     * @param percent Progress percentage (0-100)
     */
    void progress(int percent);

    /**
     * @brief Emitted when an operation starts
     * @param operation Description of the operation
     */
    void operationStarted(const QString& operation);

    /**
     * @brief Emitted when an operation completes
     */
    void operationCompleted();

    /**
     * @brief Emitted when an error occurs
     * @param error Error message
     */
    void errorOccurred(const QString& error);

private:
    /**
     * @brief Parse a single scan from an E57 file
     * @param reader E57 reader instance
     * @param scanIndex Index of the scan to parse
     * @param outPoints Output vector for points
     * @param metadata Output metadata for the scan
     */
    void parseScan(e57::Reader& reader, size_t scanIndex, 
                   QVector<PointData>& outPoints, ScanMetadata& metadata);

    /**
     * @brief Write a single scan to an E57 file
     * @param writer E57 writer instance
     * @param points Points to write
     * @param metadata Metadata for the scan
     * @param scanIndex Index of the scan being written
     */
    void writeScan(e57::Writer& writer, const QVector<PointData>& points,
                   const ScanMetadata& metadata, size_t scanIndex);

    /**
     * @brief Calculate bounding box for a set of points
     * @param points Points to analyze
     * @param minX, minY, minZ, maxX, maxY, maxZ Output bounding box coordinates
     */
    void calculateBounds(const QVector<PointData>& points,
                        double& minX, double& minY, double& minZ,
                        double& maxX, double& maxY, double& maxZ);

    /**
     * @brief Handle E57 exceptions and convert to user-friendly messages
     * @param ex Exception to handle
     * @param context Context where the exception occurred
     */
    void handleE57Exception(const std::exception& ex, const QString& context);

    /**
     * @brief Set error message and emit error signal
     * @param error Error message
     */
    void setError(const QString& error);

    /**
     * @brief Clear the last error
     */
    void clearError();

    /**
     * @brief Parse a single scan directly from E57 file structures
     * @param imageFile E57 image file instance
     * @param data3D Data3D vector node
     * @param scanIndex Index of the scan to parse
     * @param outPoints Output vector for points
     * @param metadata Output metadata for the scan
     */
    void parseScanDirect(e57::ImageFile& imageFile, e57::VectorNode& data3D,
                        int64_t scanIndex, QVector<PointData>& outPoints,
                        ScanMetadata& metadata);

    /**
     * @brief Write a single scan directly to E57 file structures
     * @param imageFile E57 image file instance
     * @param data3D Data3D vector node
     * @param points Points to write
     * @param metadata Metadata for the scan
     * @param scanIndex Index of the scan being written
     */
    void writeScanDirect(e57::ImageFile& imageFile, e57::VectorNode& data3D,
                        const QVector<PointData>& points, const ScanMetadata& metadata,
                        size_t scanIndex);

private:
    mutable QMutex m_mutex;           // Thread safety
    QString m_lastError;              // Last error message

    // Constants for chunked processing
    static constexpr size_t CHUNK_SIZE = 100000;  // Points per chunk
    static constexpr int PROGRESS_UPDATE_INTERVAL = 10000; // Points between progress updates
};
