#ifndef E57WRITER_LIB_H
#define E57WRITER_LIB_H

#include "IE57Writer.h"
#include <string>
#include <memory>
#include <vector>
#include <QObject>
#include <QString>
#include <QUuid>
#include <QDateTime>
#include <QtGui/QVector3D>
#include <QtGui/QQuaternion>
#include <QtGui/QMatrix4x4>
#include <QtGui/QMatrix3x3>
#include <cstdint>

// Forward declarations to avoid including E57Format.h in header
namespace e57 {
    class ImageFile;
    class StructureNode;
    class VectorNode;
    class CompressedVectorNode;
}

/**
 * @brief E57WriterLib - A wrapper class for libE57Format library writing capabilities
 *
 * This class provides a simplified interface to the libE57Format library
 * for creating E57 files, writing metadata, and setting up point cloud data structures.
 * Implements Sprint W1-W3 requirements for E57 file creation with XYZ, intensity, and color support.
 *
 * Sprint 2 Decoupling: Now implements IE57Writer interface for loose coupling.
 */
class E57WriterLib : public IE57Writer {
    Q_OBJECT

public:
    // Use data structures from IE57Writer interface
    using ScanPose = IE57Writer::ScanPose;
    using ScanMetadata = IE57Writer::ScanMetadata;
    using Point3D = IE57Writer::Point3D;
    using ExportOptions = IE57Writer::ExportOptions;
    using ScanData = IE57Writer::ScanData;

    explicit E57WriterLib(QObject *parent = nullptr);
    ~E57WriterLib();

    // IE57Writer interface implementation
    /**
     * @brief Create and initialize a new E57 file for writing
     * @param filePath Path where the E57 file should be created
     * @return true if file created successfully, false otherwise
     *
     * User Story W1.1: Initialize E57 File for Writing
     * Creates an e57::ImageFile in write mode and sets up basic E57Root elements
     */
    bool createFile(const QString& filePath) override;

    /**
     * @brief Add a scan to the E57 file with basic header information
     * @param scanName Name for the scan (default: "Default Scan 001")
     * @return true if scan added successfully, false otherwise
     *
     * User Story W1.2: Define Core E57 XML Structure for a Single Scan
     * Creates /data3D VectorNode and adds a Data3D StructureNode with basic metadata
     */
    bool addScan(const QString& scanName = "Default Scan 001") override;

    /**
     * @brief Add a scan to the E57 file with comprehensive metadata
     * @param metadata Complete scan metadata including pose, timestamps, and scanner info
     * @return true if scan added successfully, false otherwise
     *
     * User Story W4.1: Write Scanner Pose Metadata to E57 Data3D Header
     * Creates /data3D VectorNode and adds a Data3D StructureNode with full metadata including pose
     */
    bool addScan(const ScanMetadata& metadata) override;

    /**
     * @brief Define point prototype for the current scan with optional intensity and color
     * @param options Export options specifying which attributes to include
     * @return true if prototype defined successfully, false otherwise
     *
     * User Story W3.1 & W3.2: Define Point Prototype for XYZ, Intensity, and Color Data
     * Creates a CompressedVectorNode with prototype for cartesianX, Y, Z and optionally intensity and color fields
     */
    bool definePointPrototype(const ExportOptions& options = ExportOptions()) override;

    /**
     * @brief Legacy method - Define XYZ-only prototype for backward compatibility
     * @return true if prototype defined successfully, false otherwise
     *
     * User Story W1.3: Define Point Prototype for XYZ Data
     * Creates a CompressedVectorNode with prototype for cartesianX, Y, Z fields only
     */
    bool defineXYZPrototype() override;

    /**
     * @brief Write point data to the current scan's CompressedVectorNode
     * @param points Vector of Point3D structures containing XYZ and optional intensity/color
     * @param options Export options specifying which attributes to write
     * @return true if points written successfully, false otherwise
     *
     * User Story W3.3: Write XYZ, Intensity, and Color Data to E57 CompressedVectorNode
     * Writes point data in blocks using CompressedVectorWriter and calculates bounds/limits
     */
    bool writePoints(const std::vector<Point3D>& points, const ExportOptions& options = ExportOptions()) override;

    /**
     * @brief Write point data to a specific scan's CompressedVectorNode
     * @param scanIndex Index of the scan to write points to (0-based)
     * @param points Vector of Point3D structures containing XYZ and optional intensity/color
     * @param options Export options specifying which attributes to write
     * @return true if points written successfully, false otherwise
     *
     * User Story W3.3: Write XYZ, Intensity, and Color Data to E57 CompressedVectorNode
     * Writes point data in blocks using CompressedVectorWriter and calculates bounds/limits
     */
    bool writePoints(int scanIndex, const std::vector<Point3D>& points, const ExportOptions& options = ExportOptions()) override;

    /**
     * @brief Legacy method - Write XYZ point data to the current scan's CompressedVectorNode
     * @param points Vector of Point3D structures containing XYZ coordinates
     * @return true if points written successfully, false otherwise
     *
     * User Story W2.1: Write XYZ Point Data to E57 CompressedVectorNode (backward compatibility)
     * Writes point data in blocks using CompressedVectorWriter and calculates cartesian bounds
     */
    bool writePoints(const std::vector<Point3D>& points) override;

    /**
     * @brief Legacy method - Write XYZ point data to a specific scan's CompressedVectorNode
     * @param scanIndex Index of the scan to write points to (0-based)
     * @param points Vector of Point3D structures containing XYZ coordinates
     * @return true if points written successfully, false otherwise
     *
     * User Story W2.1: Write XYZ Point Data to E57 CompressedVectorNode (backward compatibility)
     * Writes point data in blocks using CompressedVectorWriter and calculates cartesian bounds
     */
    bool writePoints(int scanIndex, const std::vector<Point3D>& points) override;

    /**
     * @brief Close the E57 file and finalize writing
     * @return true if file closed successfully, false otherwise
     */
    bool closeFile() override;

    /**
     * @brief Get the last error message
     * @return Last error message as QString
     */
    QString getLastError() const override;

    /**
     * @brief Check if a file is currently open for writing
     * @return true if file is open, false otherwise
     */
    bool isFileOpen() const override;

    /**
     * @brief Get the current file path
     * @return Current file path as QString
     */
    QString getCurrentFilePath() const override;

    /**
     * @brief Get the number of scans currently in the file
     * @return Number of scans added to the file
     */
    int getScanCount() const override;

    /**
     * @brief Write multiple scans to E57 file in a single operation
     * @param scansData Vector of scan data with metadata and points
     * @return true if all scans written successfully, false otherwise
     *
     * User Story W4.2: Support Multiple Scans in Single E57 File
     * Efficiently writes multiple scans with their respective metadata and point data
     */
    bool writeMultipleScans(const std::vector<ScanData>& scansData) override;

private:
    // Core E57 writing functionality
    bool initializeE57Root();
    bool createData3DVectorNode();
    bool createScanStructureNode(const QString& scanName);
    bool createScanStructureNode(const ScanMetadata& metadata);
    QString generateGUID() const;

    // Sprint W4: Pose and metadata writing helpers
    bool writePoseMetadata(e57::StructureNode& scanNode, const ScanPose& pose);
    bool writeAcquisitionMetadata(e57::StructureNode& scanNode, const ScanMetadata& metadata);
    bool writeE57RootMetadata();

    // Sprint W2: Point writing and bounds calculation helpers
    bool writePointsToScan(e57::StructureNode& scanNode, const std::vector<Point3D>& points);
    bool writePointsToScan(e57::StructureNode& scanNode, const std::vector<Point3D>& points, const ExportOptions& options);
    bool calculateAndWriteCartesianBounds(e57::StructureNode& scanNode, const std::vector<Point3D>& points);
    e57::StructureNode* getScanNode(int scanIndex);

    // Sprint W3: Intensity and color support helpers
    bool calculateAndWriteIntensityLimits(e57::StructureNode& scanNode, const std::vector<Point3D>& points);
    bool calculateAndWriteColorLimits(e57::StructureNode& scanNode, const std::vector<Point3D>& points);
    bool hasValidIntensityData(const std::vector<Point3D>& points) const;
    bool hasValidColorData(const std::vector<Point3D>& points) const;

    // Error handling
    void setError(const QString& errorMessage);
    void handleE57Exception(const std::exception& ex, const QString& context);

    // Data members
    std::unique_ptr<e57::ImageFile> m_imageFile;
    QString m_currentFilePath;
    QString m_lastError;
    bool m_fileOpen;
    int m_scanCount;

    // Current scan tracking
    std::shared_ptr<e57::StructureNode> m_currentScanNode;
    std::shared_ptr<e57::VectorNode> m_data3DNode;
};

#endif // E57WRITER_LIB_H
