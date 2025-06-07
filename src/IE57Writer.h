#ifndef IE57WRITER_H
#define IE57WRITER_H

#include <QString>
#include <QObject>
#include <vector>
#include <memory>

// Forward declarations for data structures
struct Point3D {
    float x, y, z;
    float intensity = 0.0f;
    uint8_t r = 255, g = 255, b = 255;
    
    Point3D() = default;
    Point3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    Point3D(float x_, float y_, float z_, float intensity_) 
        : x(x_), y(y_), z(z_), intensity(intensity_) {}
    Point3D(float x_, float y_, float z_, uint8_t r_, uint8_t g_, uint8_t b_)
        : x(x_), y(y_), z(z_), r(r_), g(g_), b(b_) {}
};

struct ScanMetadata {
    QString scanName;
    QString description;
    QString acquisitionDateTime;
    QString sensorVendor;
    QString sensorModel;
    QString sensorSerialNumber;
    
    // Coordinate system information
    double originX = 0.0;
    double originY = 0.0;
    double originZ = 0.0;
    
    // Transformation matrix (4x4, row-major order)
    std::vector<double> transformationMatrix;
    
    // Point cloud bounds
    double minX = 0.0, maxX = 0.0;
    double minY = 0.0, maxY = 0.0;
    double minZ = 0.0, maxZ = 0.0;
    
    ScanMetadata() {
        // Initialize as identity matrix
        transformationMatrix = {
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        };
    }
};

struct ExportOptions {
    bool includeIntensity = false;
    bool includeColor = false;
    bool compressData = true;
    double scaleFactor = 0.0001; // 0.1mm precision
    QString coordinateSystem = "CARTESIAN";
    
    // Quality settings
    int compressionLevel = 6; // 0-9, higher = better compression
    bool optimizeForSize = true;
    
    ExportOptions() = default;
};

/**
 * @brief Abstract interface for E57 file writing operations
 * 
 * This interface decouples the application from the specific implementation
 * details of libE57Format, allowing for easier testing and future changes.
 */
class IE57Writer : public QObject
{
    Q_OBJECT

public:
    explicit IE57Writer(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IE57Writer() = default;

    // File management
    virtual bool createFile(const QString& filePath) = 0;
    virtual bool closeFile() = 0;
    virtual bool isFileOpen() const = 0;

    // Scan management
    virtual bool addScan(const ScanMetadata& metadata) = 0;
    virtual bool setScanBounds(double minX, double maxX, double minY, double maxY, double minZ, double maxZ) = 0;
    virtual int getScanCount() const = 0;

    // Point data writing
    virtual bool writePoints(const std::vector<Point3D>& points, const ExportOptions& options = ExportOptions()) = 0;
    virtual bool writePointsXYZ(const std::vector<float>& points, const ExportOptions& options = ExportOptions()) = 0;
    virtual bool writePointsWithIntensity(const std::vector<float>& points, const std::vector<float>& intensity, const ExportOptions& options = ExportOptions()) = 0;
    virtual bool writePointsWithColor(const std::vector<float>& points, const std::vector<uint8_t>& colors, const ExportOptions& options = ExportOptions()) = 0;

    // Batch operations
    virtual bool beginPointWriting(size_t estimatedPointCount) = 0;
    virtual bool writePointBatch(const std::vector<Point3D>& points) = 0;
    virtual bool endPointWriting() = 0;

    // Error handling
    virtual QString getLastError() const = 0;
    virtual bool hasError() const = 0;
    virtual void clearError() = 0;

    // File information
    virtual QString getFilePath() const = 0;
    virtual size_t getFileSize() const = 0;
    virtual size_t getTotalPointsWritten() const = 0;

    // Validation
    virtual bool validateFile(const QString& filePath) = 0;
    virtual bool canWriteToPath(const QString& filePath) = 0;

signals:
    // Progress reporting
    void progressUpdated(int percentage, const QString& stage);
    void writeCompleted(bool success, const QString& message);
    void errorOccurred(const QString& error);

    // File operations
    void fileCreated(const QString& filePath);
    void fileClosed(const QString& filePath);
    void scanAdded(const QString& scanName);
    void pointsWritten(size_t pointCount);
};

#endif // IE57WRITER_H
