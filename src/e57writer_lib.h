#ifndef E57WRITER_LIB_H
#define E57WRITER_LIB_H

#include <string>
#include <memory>
#include <vector>
#include <QObject>
#include <QString>
#include <QUuid>
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
 */
class E57WriterLib : public QObject {
    Q_OBJECT

public:
    // Sprint W3: Enhanced point data structure with optional intensity and color
    struct Point3D {
        double x, y, z;

        // Sprint W3: Optional intensity data (normalized 0.0-1.0)
        bool hasIntensity;
        float intensity;

        // Sprint W3: Optional RGB color data (0-255 per channel)
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

    // Sprint W3: Export configuration options
    struct ExportOptions {
        bool includeIntensity;
        bool includeColor;

        ExportOptions() : includeIntensity(false), includeColor(false) {}
        ExportOptions(bool intensity, bool color) : includeIntensity(intensity), includeColor(color) {}
    };

    explicit E57WriterLib(QObject *parent = nullptr);
    ~E57WriterLib();

    /**
     * @brief Create and initialize a new E57 file for writing
     * @param filePath Path where the E57 file should be created
     * @return true if file created successfully, false otherwise
     * 
     * User Story W1.1: Initialize E57 File for Writing
     * Creates an e57::ImageFile in write mode and sets up basic E57Root elements
     */
    bool createFile(const QString& filePath);

    /**
     * @brief Add a scan to the E57 file with basic header information
     * @param scanName Name for the scan (default: "Default Scan 001")
     * @return true if scan added successfully, false otherwise
     * 
     * User Story W1.2: Define Core E57 XML Structure for a Single Scan
     * Creates /data3D VectorNode and adds a Data3D StructureNode with basic metadata
     */
    bool addScan(const QString& scanName = "Default Scan 001");

    /**
     * @brief Define point prototype for the current scan with optional intensity and color
     * @param options Export options specifying which attributes to include
     * @return true if prototype defined successfully, false otherwise
     *
     * User Story W3.1 & W3.2: Define Point Prototype for XYZ, Intensity, and Color Data
     * Creates a CompressedVectorNode with prototype for cartesianX, Y, Z and optionally intensity and color fields
     */
    bool definePointPrototype(const ExportOptions& options = ExportOptions());

    /**
     * @brief Legacy method - Define XYZ-only prototype for backward compatibility
     * @return true if prototype defined successfully, false otherwise
     *
     * User Story W1.3: Define Point Prototype for XYZ Data
     * Creates a CompressedVectorNode with prototype for cartesianX, Y, Z fields only
     */
    bool defineXYZPrototype();

    /**
     * @brief Write point data to the current scan's CompressedVectorNode
     * @param points Vector of Point3D structures containing XYZ and optional intensity/color
     * @param options Export options specifying which attributes to write
     * @return true if points written successfully, false otherwise
     *
     * User Story W3.3: Write XYZ, Intensity, and Color Data to E57 CompressedVectorNode
     * Writes point data in blocks using CompressedVectorWriter and calculates bounds/limits
     */
    bool writePoints(const std::vector<Point3D>& points, const ExportOptions& options = ExportOptions());

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
    bool writePoints(int scanIndex, const std::vector<Point3D>& points, const ExportOptions& options = ExportOptions());

    /**
     * @brief Legacy method - Write XYZ point data to the current scan's CompressedVectorNode
     * @param points Vector of Point3D structures containing XYZ coordinates
     * @return true if points written successfully, false otherwise
     *
     * User Story W2.1: Write XYZ Point Data to E57 CompressedVectorNode (backward compatibility)
     * Writes point data in blocks using CompressedVectorWriter and calculates cartesian bounds
     */
    bool writePoints(const std::vector<Point3D>& points);

    /**
     * @brief Legacy method - Write XYZ point data to a specific scan's CompressedVectorNode
     * @param scanIndex Index of the scan to write points to (0-based)
     * @param points Vector of Point3D structures containing XYZ coordinates
     * @return true if points written successfully, false otherwise
     *
     * User Story W2.1: Write XYZ Point Data to E57 CompressedVectorNode (backward compatibility)
     * Writes point data in blocks using CompressedVectorWriter and calculates cartesian bounds
     */
    bool writePoints(int scanIndex, const std::vector<Point3D>& points);

    /**
     * @brief Close the E57 file and finalize writing
     * @return true if file closed successfully, false otherwise
     */
    bool closeFile();

    /**
     * @brief Get the last error message
     * @return Last error message as QString
     */
    QString getLastError() const;

    /**
     * @brief Check if a file is currently open for writing
     * @return true if file is open, false otherwise
     */
    bool isFileOpen() const;

    /**
     * @brief Get the current file path
     * @return Current file path as QString
     */
    QString getCurrentFilePath() const;

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

private:
    // Core E57 writing functionality
    bool initializeE57Root();
    bool createData3DVectorNode();
    bool createScanStructureNode(const QString& scanName);
    QString generateGUID() const;

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
