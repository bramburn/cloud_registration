#ifndef IE57WRITER_H
#define IE57WRITER_H

#include <QString>
#include <QObject>
#include <vector>
#include <memory>

/**
 * @brief Point3D - Basic 3D point structure for E57 writing
 */
struct Point3D {
    double x, y, z;
    
    Point3D() : x(0.0), y(0.0), z(0.0) {}
    Point3D(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
};

/**
 * @brief ScanPose - Pose information for a scan
 */
struct ScanPose {
    double translation[3] = {0.0, 0.0, 0.0};
    double rotation[4] = {1.0, 0.0, 0.0, 0.0}; // quaternion (w, x, y, z)
};

/**
 * @brief ScanMetadata - Metadata for a scan
 */
struct ScanMetadata {
    QString name;
    QString description;
    ScanPose pose;
    int64_t pointCount = 0;
    QString acquisitionDateTime;
    QString sensorVendor;
    QString sensorModel;
    QString sensorSerialNumber;
    double temperatureCelsius = 0.0;
    double relativeHumidity = 0.0;
    double atmosphericPressure = 0.0;
};

/**
 * @brief ExportOptions - Options for E57 export
 */
struct ExportOptions {
    bool includeIntensity = false;
    bool includeColor = false;
    bool compressData = true;
    double coordinateScaleFactor = 0.0001; // 0.1mm precision
    QString coordinateSystem = "CARTESIAN";
};

/**
 * @brief ScanData - Complete scan data structure
 */
struct ScanData {
    ScanMetadata metadata;
    std::vector<Point3D> points;
    std::vector<float> intensities;
    std::vector<uint8_t> colors; // RGB interleaved
};

/**
 * @brief IE57Writer - Abstract interface for E57 file writing
 * 
 * This interface defines the contract for all E57 writer implementations.
 * It enables loose coupling between the writing logic and the rest of the application,
 * allowing for easy testing with mock implementations and future substitution
 * of different E57 writing libraries.
 * 
 * Sprint 2 Decoupling Requirements:
 * - Provides abstraction layer for E57 writing operations
 * - Enables dependency injection and polymorphic usage
 * - Supports unit testing with mock implementations
 * - Maintains compatibility with existing application interface
 */
class IE57Writer : public QObject {
    Q_OBJECT

public:
    explicit IE57Writer(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IE57Writer() = default;

    /**
     * @brief Create a new E57 file for writing
     * @param filePath Path where the E57 file should be created
     * @return true if file created successfully, false otherwise
     */
    virtual bool createFile(const QString& filePath) = 0;

    /**
     * @brief Add a new scan to the E57 file
     * @param metadata Metadata for the scan
     * @return true if scan added successfully, false otherwise
     */
    virtual bool addScan(const ScanMetadata& metadata) = 0;

    /**
     * @brief Write points to the current scan
     * @param points Vector of 3D points to write
     * @param options Export options for the points
     * @return true if points written successfully, false otherwise
     */
    virtual bool writePoints(const std::vector<Point3D>& points, const ExportOptions& options = ExportOptions()) = 0;

    /**
     * @brief Write complete scan data (points + attributes)
     * @param scanData Complete scan data including metadata, points, and attributes
     * @return true if scan data written successfully, false otherwise
     */
    virtual bool writeScanData(const ScanData& scanData) = 0;

    /**
     * @brief Close the current E57 file
     * @return true if file closed successfully, false otherwise
     */
    virtual bool closeFile() = 0;

    /**
     * @brief Check if a file is currently open for writing
     * @return true if file is open, false otherwise
     */
    virtual bool isOpen() const = 0;

    /**
     * @brief Get the last error message
     * @return Error message string, empty if no error
     */
    virtual QString getLastError() const = 0;

    /**
     * @brief Get the current file path
     * @return Path of the currently open file, empty if no file is open
     */
    virtual QString getCurrentFilePath() const = 0;

    /**
     * @brief Get the number of scans written to the file
     * @return Number of scans in the file
     */
    virtual int getScanCount() const = 0;

    /**
     * @brief Set file-level metadata
     * @param guid File GUID (if empty, auto-generated)
     * @param description File description
     * @param creationDateTime Creation date/time (if empty, current time used)
     * @return true if metadata set successfully, false otherwise
     */
    virtual bool setFileMetadata(const QString& guid = QString(), 
                                const QString& description = QString(),
                                const QString& creationDateTime = QString()) = 0;

signals:
    /**
     * @brief Emitted when file creation starts
     * @param filePath Path of the file being created
     */
    void fileCreationStarted(const QString& filePath);

    /**
     * @brief Emitted when file creation completes
     * @param success true if successful, false if failed
     * @param message Success or error message
     */
    void fileCreationFinished(bool success, const QString& message);

    /**
     * @brief Emitted when a scan is added
     * @param scanIndex Index of the added scan
     * @param scanName Name of the added scan
     */
    void scanAdded(int scanIndex, const QString& scanName);

    /**
     * @brief Emitted during point writing to report progress
     * @param percentage Progress percentage (0-100)
     * @param pointsWritten Number of points written so far
     */
    void progressUpdated(int percentage, int64_t pointsWritten);

    /**
     * @brief Emitted when an error occurs
     * @param errorMessage Description of the error
     */
    void errorOccurred(const QString& errorMessage);
};

#endif // IE57WRITER_H
