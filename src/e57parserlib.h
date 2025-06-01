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

    // Sprint 2: Point data extraction methods

    /**
     * @brief Extract XYZ point data from the first scan
     * @return Vector of floats in interleaved format (X1,Y1,Z1,X2,Y2,Z2,...)
     */
    std::vector<float> extractPointData();

    /**
     * @brief Extract XYZ point data from a specific scan
     * @param scanIndex Index of the scan to extract (0-based)
     * @return Vector of floats in interleaved format (X1,Y1,Z1,X2,Y2,Z2,...)
     */
    std::vector<float> extractPointData(int scanIndex);

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

    // Prototype information for current scan
    struct PrototypeInfo {
        bool hasCartesianX = false;
        bool hasCartesianY = false;
        bool hasCartesianZ = false;
        bool isDoublePrec = false;
        std::string xFieldName = "cartesianX";
        std::string yFieldName = "cartesianY";
        std::string zFieldName = "cartesianZ";
    };
    PrototypeInfo m_prototypeInfo;

    // Helper methods
    void clearError();
    void setError(const std::string& error);

    // Sprint 2: Point data extraction helpers
    bool accessFirstScanData(e57::StructureNode& scanHeader);
    bool inspectPointPrototype(const e57::StructureNode& scanHeader);
    bool extractUncompressedXYZData(const e57::StructureNode& scanHeader);
    void validatePrototypeFields(const e57::StructureNode& prototype);
};

#endif // E57PARSERLIB_H
