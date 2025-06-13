#ifndef E57PARSERCORE_H
#define E57PARSERCORE_H

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// Forward declarations to avoid including E57Format.h in header
namespace e57
{
class ImageFile;
class StructureNode;
class CompressedVectorNode;
}  // namespace e57

/**
 * @brief Core E57 parsing functionality without Qt dependencies
 *
 * This class provides the core E57 file parsing logic using only standard C++
 * and the libE57Format library. It is completely independent of Qt and can be
 * used in any C++ application or tested in isolation.
 *
 * Sprint 2 Decoupling: Extracted from E57ParserLib to separate concerns.
 * The Qt-specific wrapper (E57ParserLib) delegates to this core implementation.
 */

// Data structures for point cloud data (Qt-independent)
struct CorePointData
{
    float x, y, z;
    float intensity = 0.0f;
    uint8_t red = 0, green = 0, blue = 0;
    bool hasIntensity = false;
    bool hasColor = false;
};

struct CoreScanMetadata
{
    std::string name;
    std::string guid;
    int64_t pointCount = 0;
    std::string acquisitionDateTime;
    std::string description;

    // Bounding box
    double minX = 0.0, maxX = 0.0;
    double minY = 0.0, maxY = 0.0;
    double minZ = 0.0, maxZ = 0.0;

    bool isValid() const
    {
        return pointCount > 0 && !name.empty();
    }
};

struct CoreLoadingSettings
{
    int64_t maxPoints = 1000000;
    bool loadIntensity = true;
    bool loadColor = true;
    double voxelSize = 0.0;  // 0.0 means no voxel filtering

    // Spatial filtering
    bool enableSpatialFilter = false;
    double filterMinX = 0.0, filterMaxX = 0.0;
    double filterMinY = 0.0, filterMaxY = 0.0;
    double filterMinZ = 0.0, filterMaxZ = 0.0;
};

// Progress callback type
using ProgressCallback = std::function<void(int percentage, const std::string& stage)>;

// Exception types for error handling
class E57CoreException : public std::runtime_error
{
public:
    explicit E57CoreException(const std::string& message) : std::runtime_error(message) {}
};

class E57FileNotFoundException : public E57CoreException
{
public:
    explicit E57FileNotFoundException(const std::string& filePath) : E57CoreException("E57 file not found: " + filePath)
    {
    }
};

class E57InvalidFormatException : public E57CoreException
{
public:
    explicit E57InvalidFormatException(const std::string& message) : E57CoreException("Invalid E57 format: " + message)
    {
    }
};

/**
 * @brief E57ParserCore - Core E57 parsing implementation
 *
 * This class handles all direct interactions with the libE57Format library
 * and provides a clean, Qt-independent interface for E57 file operations.
 */
class E57ParserCore
{
public:
    E57ParserCore();
    ~E57ParserCore();

    // File operations
    bool openFile(const std::string& filePath);
    void closeFile();
    bool isOpen() const;

    // File validation
    static bool isValidE57File(const std::string& filePath);

    // File metadata
    std::string getGuid() const;
    std::pair<int, int> getVersion() const;
    int getScanCount() const;
    CoreScanMetadata getScanMetadata(int scanIndex) const;

    // Point data extraction
    std::vector<float> extractXYZData(int scanIndex = 0, const CoreLoadingSettings& settings = CoreLoadingSettings());
    std::vector<CorePointData> extractPointData(int scanIndex = 0,
                                                const CoreLoadingSettings& settings = CoreLoadingSettings());
    int64_t getPointCount(int scanIndex = 0) const;

    // Progress tracking
    void setProgressCallback(ProgressCallback callback);
    void clearProgressCallback();

    // Error handling
    std::string getLastError() const;
    void clearError();

private:
    // Core parsing implementation
    bool openE57FileInternal(const std::string& filePath);
    void closeE57FileInternal();

    // Metadata extraction
    CoreScanMetadata extractScanMetadataInternal(int scanIndex) const;
    bool extractBoundingBox(const e57::StructureNode& scanHeader, CoreScanMetadata& metadata) const;

    // Point data extraction helpers
    std::vector<CorePointData> extractPointDataFromScan(int scanIndex, const CoreLoadingSettings& settings);
    bool inspectPointPrototype(const e57::StructureNode& scanHeader);
    bool extractPointDataFromCompressedVector(e57::CompressedVectorNode& cvNode,
                                              std::vector<CorePointData>& points,
                                              const CoreLoadingSettings& settings);

    // Data validation and filtering
    bool validatePointData(const CorePointData& point, const CoreLoadingSettings& settings) const;
    void applySpatialFilter(std::vector<CorePointData>& points, const CoreLoadingSettings& settings) const;
    void applyVoxelFilter(std::vector<CorePointData>& points, double voxelSize) const;

    // Progress reporting
    void reportProgress(int percentage, const std::string& stage);

    // Error handling
    void setError(const std::string& error);
    void handleE57Exception(const std::exception& ex, const std::string& context);

    // Data members
    std::unique_ptr<e57::ImageFile> m_imageFile;
    std::string m_currentFilePath;
    mutable std::string m_lastError;
    ProgressCallback m_progressCallback;

    // Prototype information for current scan
    struct PrototypeInfo
    {
        bool hasIntensity = false;
        bool hasColorRed = false;
        bool hasColorGreen = false;
        bool hasColorBlue = false;
        std::string intensityDataType;
        std::string colorDataType;

        void reset()
        {
            hasIntensity = false;
            hasColorRed = hasColorGreen = hasColorBlue = false;
            intensityDataType.clear();
            colorDataType.clear();
        }
    } m_prototypeInfo;

    // Data limits for normalization
    struct DataLimits
    {
        double intensityMin = 0.0;
        double intensityMax = 1.0;
        double colorMin = 0.0;
        double colorMax = 255.0;

        void reset()
        {
            intensityMin = 0.0;
            intensityMax = 1.0;
            colorMin = 0.0;
            colorMax = 255.0;
        }
    } m_dataLimits;
};

#endif  // E57PARSERCORE_H
