#ifndef E57PARSERLIB_H
#define E57PARSERLIB_H

#include <string>
#include <memory>
#include <vector>
#include <QObject>
#include <QString>

// Forward declarations to avoid including E57Format.h in header
namespace e57 {
    class ImageFile;
    class StructureNode;
    class CompressedVectorNode;
}

/**
 * @brief E57ParserLib - A wrapper class for libE57Format library
 *
 * This class provides a simplified interface to the libE57Format library
 * for opening E57 files, extracting metadata, and reading point cloud data.
 * Implements Sprint 1 & 2 requirements for E57 library integration.
 */
class E57ParserLib : public QObject {
    Q_OBJECT

public:
    explicit E57ParserLib(QObject *parent = nullptr);
    ~E57ParserLib();

    /**
     * @brief Open an E57 file for reading
     * @param filePath Path to the E57 file
     * @return true if file opened successfully, false otherwise
     */
    bool openFile(const std::string& filePath);

    /**
     * @brief Close the currently opened E57 file
     */
    void closeFile();

    /**
     * @brief Get the GUID of the opened E57 file
     * @return File GUID as string, empty if not available
     */
    std::string getGuid() const;

    /**
     * @brief Get the E57 standard version of the opened file
     * @return Pair of (major, minor) version numbers
     */
    std::pair<int, int> getVersion() const;

    /**
     * @brief Get the number of scans (Data3D sections) in the file
     * @return Number of scans, 0 if none or file not open
     */
    int getScanCount() const;

    /**
     * @brief Get the last error message
     * @return Error message string, empty if no error
     */
    std::string getLastError() const;

    /**
     * @brief Check if a file is open
     * @return true if file is open, false otherwise
     */
    bool isOpen() const;

    // Sprint 2 & 3: Point data extraction methods

    /**
     * @brief Enhanced point data structure for Sprint 3
     */
    struct PointData {
        float x, y, z;
        float intensity = 0.0f;
        uint8_t r = 0, g = 0, b = 0;
        bool hasIntensity = false;
        bool hasColor = false;

        PointData() = default;
        PointData(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    };

    /**
     * @brief Extract XYZ point data from the first scan (legacy method)
     * @return Vector of floats in interleaved format (X1,Y1,Z1,X2,Y2,Z2,...)
     */
    std::vector<float> extractPointData();

    /**
     * @brief Extract XYZ point data from a specific scan (legacy method)
     * @param scanIndex Index of the scan to extract (0-based)
     * @return Vector of floats in interleaved format (X1,Y1,Z1,X2,Y2,Z2,...)
     */
    std::vector<float> extractPointData(int scanIndex);

    /**
     * @brief Extract enhanced point data with intensity and color (Sprint 3)
     * @param scanIndex Index of the scan to extract (0-based)
     * @return Vector of PointData structures with all available attributes
     */
    std::vector<PointData> extractEnhancedPointData(int scanIndex = 0);

    /**
     * @brief Get the number of points in a specific scan
     * @param scanIndex Index of the scan (0-based)
     * @return Number of points, 0 if scan doesn't exist or error
     */
    int64_t getPointCount(int scanIndex = 0) const;

signals:
    /**
     * @brief Emitted during point data extraction to report progress
     * @param percentage Progress percentage (0-100)
     * @param stage Description of current processing stage
     */
    void progressUpdated(int percentage, const QString& stage);

    /**
     * @brief Emitted when point data extraction is complete
     * @param success True if extraction succeeded, false otherwise
     * @param message Success or error message
     * @param points Extracted point data (empty if failed)
     */
    void parsingFinished(bool success, const QString& message, const std::vector<float>& points);

private:
    // Private implementation to hide libE57Format details
    std::unique_ptr<e57::ImageFile> m_imageFile;
    std::string m_lastError;

    // Sprint 2: Point data storage and metadata
    std::vector<float> m_points;

    // Prototype information for current scan (enhanced for Sprint 3)
    struct PrototypeInfo {
        // XYZ coordinates (Sprint 2)
        bool hasCartesianX = false;
        bool hasCartesianY = false;
        bool hasCartesianZ = false;
        bool isDoublePrec = false;
        std::string xFieldName = "cartesianX";
        std::string yFieldName = "cartesianY";
        std::string zFieldName = "cartesianZ";

        // Intensity data (Sprint 3)
        bool hasIntensity = false;
        std::string intensityFieldName = "intensity";
        std::string intensityDataType; // "float", "integer", "scaledInteger"

        // Color data (Sprint 3)
        bool hasColorRed = false;
        bool hasColorGreen = false;
        bool hasColorBlue = false;
        std::string colorRedFieldName = "colorRed";
        std::string colorGreenFieldName = "colorGreen";
        std::string colorBlueFieldName = "colorBlue";
        std::string colorDataType; // "integer", "scaledInteger"
    };
    PrototypeInfo m_prototypeInfo;

    // Sprint 3: Intensity and color limits for normalization
    struct DataLimits {
        double intensityMin = 0.0;
        double intensityMax = 1.0;
        double colorRedMin = 0.0;
        double colorRedMax = 255.0;
        double colorGreenMin = 0.0;
        double colorGreenMax = 255.0;
        double colorBlueMin = 0.0;
        double colorBlueMax = 255.0;
        bool hasIntensityLimits = false;
        bool hasColorLimits = false;
    };
    DataLimits m_dataLimits;

    // Helper methods
    void clearError();
    void setError(const std::string& error);

    // Sprint 2: Point data extraction helpers
    bool accessFirstScanData(e57::StructureNode& scanHeader);
    bool inspectPointPrototype(const e57::StructureNode& scanHeader);
    bool extractUncompressedXYZData(const e57::StructureNode& scanHeader);
    void validatePrototypeFields(const e57::StructureNode& prototype);

    // Sprint 3: Enhanced point data extraction helpers
    bool inspectEnhancedPrototype(const e57::StructureNode& scanHeader);
    bool extractDataLimits(const e57::StructureNode& scanHeader);
    bool extractEnhancedPointData(const e57::StructureNode& scanHeader, std::vector<PointData>& points);

    // Sprint 3: Normalization helpers
    float normalizeIntensity(float rawValue) const;
    uint8_t normalizeColorChannel(float rawValue, double minVal, double maxVal) const;
};

#endif // E57PARSERLIB_H
