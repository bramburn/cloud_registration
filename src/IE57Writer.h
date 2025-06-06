#ifndef IE57WRITER_H
#define IE57WRITER_H

#include <string>
#include <vector>
#include <memory>
#include <QObject>
#include <QString>
#include <QUuid>
#include <QDateTime>
#include <QtGui/QVector3D>
#include <QtGui/QQuaternion>
#include <QtGui/QMatrix4x4>
#include <QtGui/QMatrix3x3>
#include <cstdint>

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
 * - Maintains compatibility with existing E57WriterLib interface
 */
class IE57Writer : public QObject {
    Q_OBJECT

public:
    // Forward declarations for data structures
    struct ScanPose {
        QVector3D translation;      // Scanner position in world coordinates
        QQuaternion rotation;       // Scanner orientation as normalized quaternion
        QDateTime acquisitionTime;  // Time when scan was acquired

        ScanPose() : translation(0, 0, 0), rotation(1, 0, 0, 0) {}
        ScanPose(const QVector3D& trans, const QQuaternion& rot)
            : translation(trans), rotation(rot.normalized()) {}
        ScanPose(const QVector3D& trans, const QQuaternion& rot, const QDateTime& time)
            : translation(trans), rotation(rot.normalized()), acquisitionTime(time) {}

        // Convert from 4x4 transformation matrix
        static ScanPose fromMatrix(const QMatrix4x4& matrix) {
            // Extract translation from the last column
            QVector3D trans(matrix(0, 3), matrix(1, 3), matrix(2, 3));

            // Extract rotation matrix (upper-left 3x3)
            QMatrix3x3 rotMatrix;
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    rotMatrix(i, j) = matrix(i, j);
                }
            }

            // Convert rotation matrix to quaternion
            QQuaternion rot = QQuaternion::fromRotationMatrix(rotMatrix);

            return ScanPose(trans, rot);
        }

        QMatrix4x4 toMatrix() const {
            QMatrix4x4 matrix;
            matrix.setToIdentity();

            // Set rotation part
            QMatrix3x3 rotMatrix = rotation.toRotationMatrix();
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    matrix(i, j) = rotMatrix(i, j);
                }
            }

            // Set translation part
            matrix(0, 3) = translation.x();
            matrix(1, 3) = translation.y();
            matrix(2, 3) = translation.z();

            return matrix;
        }
    };

    struct ScanMetadata {
        QString name;               // Scan name
        QString guid;               // Unique identifier (auto-generated if empty)
        QString description;        // Optional scan description
        QString sensorModel;        // Scanner model/type
        ScanPose pose;             // Scanner pose information
        QDateTime acquisitionStart; // Scan start time
        QDateTime acquisitionEnd;   // Scan end time (optional)

        // Optional metadata
        QString originalGuids;      // For multi-source data
        QString associatedData3DGuids; // Related scans

        ScanMetadata() {}
        ScanMetadata(const QString& scanName) : name(scanName) {}
        ScanMetadata(const QString& scanName, const ScanPose& scanPose)
            : name(scanName), pose(scanPose) {}
    };

    struct Point3D {
        double x, y, z;

        // Optional intensity data (normalized 0.0-1.0)
        bool hasIntensity;
        float intensity;

        // Optional RGB color data (0-255 per channel)
        bool hasColor;
        uint8_t colorRed, colorGreen, colorBlue;

        Point3D() : x(0.0), y(0.0), z(0.0), hasIntensity(false), intensity(0.0f),
                   hasColor(false), colorRed(0), colorGreen(0), colorBlue(0) {}
        Point3D(double x_, double y_, double z_) : x(x_), y(y_), z(z_),
                   hasIntensity(false), intensity(0.0f), hasColor(false),
                   colorRed(0), colorGreen(0), colorBlue(0) {}
        Point3D(double x_, double y_, double z_, float intensity_) : x(x_), y(y_), z(z_),
                   hasIntensity(true), intensity(intensity_), hasColor(false),
                   colorRed(0), colorGreen(0), colorBlue(0) {}
        Point3D(double x_, double y_, double z_, uint8_t r, uint8_t g, uint8_t b) :
                   x(x_), y(y_), z(z_), hasIntensity(false), intensity(0.0f),
                   hasColor(true), colorRed(r), colorGreen(g), colorBlue(b) {}
        Point3D(double x_, double y_, double z_, float intensity_, uint8_t r, uint8_t g, uint8_t b) :
                   x(x_), y(y_), z(z_), hasIntensity(true), intensity(intensity_),
                   hasColor(true), colorRed(r), colorGreen(g), colorBlue(b) {}
    };

    struct ExportOptions {
        bool includeIntensity;
        bool includeColor;

        ExportOptions() : includeIntensity(false), includeColor(false) {}
        ExportOptions(bool intensity, bool color) : includeIntensity(intensity), includeColor(color) {}
    };

    struct ScanData {
        ScanMetadata metadata;
        std::vector<Point3D> points;
        ExportOptions options;

        ScanData() {}
        ScanData(const ScanMetadata& meta, const std::vector<Point3D>& pts, const ExportOptions& opts = ExportOptions())
            : metadata(meta), points(pts), options(opts) {}
    };

public:
    explicit IE57Writer(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IE57Writer() = default;

    /**
     * @brief Create and initialize a new E57 file for writing
     * @param filePath Path where the E57 file should be created
     * @return true if file created successfully, false otherwise
     */
    virtual bool createFile(const QString& filePath) = 0;

    /**
     * @brief Add a scan to the E57 file with basic header information
     * @param scanName Name for the scan (default: "Default Scan 001")
     * @return true if scan added successfully, false otherwise
     */
    virtual bool addScan(const QString& scanName = "Default Scan 001") = 0;

    /**
     * @brief Add a scan to the E57 file with comprehensive metadata
     * @param metadata Complete scan metadata including pose, timestamps, and scanner info
     * @return true if scan added successfully, false otherwise
     */
    virtual bool addScan(const ScanMetadata& metadata) = 0;

    /**
     * @brief Define point prototype for the current scan with optional intensity and color
     * @param options Export options specifying which attributes to include
     * @return true if prototype defined successfully, false otherwise
     */
    virtual bool definePointPrototype(const ExportOptions& options = ExportOptions()) = 0;

    /**
     * @brief Legacy method - Define XYZ-only prototype for backward compatibility
     * @return true if prototype defined successfully, false otherwise
     */
    virtual bool defineXYZPrototype() = 0;

    /**
     * @brief Write point data to the current scan's CompressedVectorNode
     * @param points Vector of Point3D structures containing XYZ and optional intensity/color
     * @param options Export options specifying which attributes to write
     * @return true if points written successfully, false otherwise
     */
    virtual bool writePoints(const std::vector<Point3D>& points, const ExportOptions& options = ExportOptions()) = 0;

    /**
     * @brief Write point data to a specific scan's CompressedVectorNode
     * @param scanIndex Index of the scan to write points to (0-based)
     * @param points Vector of Point3D structures containing XYZ and optional intensity/color
     * @param options Export options specifying which attributes to write
     * @return true if points written successfully, false otherwise
     */
    virtual bool writePoints(int scanIndex, const std::vector<Point3D>& points, const ExportOptions& options = ExportOptions()) = 0;

    /**
     * @brief Legacy method - Write XYZ point data to the current scan's CompressedVectorNode
     * @param points Vector of Point3D structures containing XYZ coordinates
     * @return true if points written successfully, false otherwise
     */
    virtual bool writePoints(const std::vector<Point3D>& points) = 0;

    /**
     * @brief Legacy method - Write XYZ point data to a specific scan's CompressedVectorNode
     * @param scanIndex Index of the scan to write points to (0-based)
     * @param points Vector of Point3D structures containing XYZ coordinates
     * @return true if points written successfully, false otherwise
     */
    virtual bool writePoints(int scanIndex, const std::vector<Point3D>& points) = 0;

    /**
     * @brief Close the E57 file and finalize writing
     * @return true if file closed successfully, false otherwise
     */
    virtual bool closeFile() = 0;

    /**
     * @brief Get the last error message
     * @return Last error message as QString
     */
    virtual QString getLastError() const = 0;

    /**
     * @brief Check if a file is currently open for writing
     * @return true if file is open, false otherwise
     */
    virtual bool isFileOpen() const = 0;

    /**
     * @brief Get the current file path
     * @return Current file path as QString
     */
    virtual QString getCurrentFilePath() const = 0;

    /**
     * @brief Get the number of scans currently in the file
     * @return Number of scans added to the file
     */
    virtual int getScanCount() const = 0;

    /**
     * @brief Write multiple scans to E57 file in a single operation
     * @param scansData Vector of scan data with metadata and points
     * @return true if all scans written successfully, false otherwise
     */
    virtual bool writeMultipleScans(const std::vector<ScanData>& scansData) = 0;

signals:
    /**
     * @brief Emitted when file creation is completed
     * @param success true if successful, false if failed
     * @param filePath Path of the created file
     */
    void fileCreated(bool success, const QString& filePath);

    /**
     * @brief Emitted when scan is added
     * @param success true if successful, false if failed
     * @param scanName Name of the added scan
     */
    void scanAdded(bool success, const QString& scanName);

    /**
     * @brief Emitted when an error occurs
     * @param errorMessage Description of the error
     */
    void errorOccurred(const QString& errorMessage);
};

#endif // IE57WRITER_H
