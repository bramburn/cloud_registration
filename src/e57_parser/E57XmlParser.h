#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "E57BinaryReader.h"

// Forward declarations for libE57Format
namespace e57 {
    class ImageFile;
    class StructureNode;
    class VectorNode;
    class CompressedVectorNode;
    class Node;
    enum ElementType;
}

namespace E57Parser {

/**
 * @brief Structure representing a point attribute in an E57 file
 */
struct PointAttribute {
    std::string name;           // Attribute name (e.g., "cartesianX", "colorRed")
    int elementType;            // E57 element type (stored as int to avoid E57Format dependency)
    double minimum = 0.0;       // Minimum value for this attribute
    double maximum = 0.0;       // Maximum value for this attribute
    bool hasLimits = false;     // Whether min/max limits are defined
    
    PointAttribute() : elementType(0) {}
    PointAttribute(const std::string& attrName, int type) 
        : name(attrName), elementType(type) {}
};

/**
 * @brief Structure representing coordinate system metadata
 */
struct CoordinateMetadata {
    std::string coordinateSystemName;   // Name of the coordinate system
    std::string datum;                  // Datum information
    std::string projection;             // Projection information
    
    CoordinateMetadata() = default;
};

/**
 * @brief Structure representing metadata for a single scan
 */
struct ScanMetadata {
    std::string guid;                           // Unique identifier for this scan
    std::string name;                           // Human-readable name
    std::string description;                    // Description of the scan
    std::vector<PointAttribute> pointAttributes; // Available point attributes
    CoordinateMetadata coordinates;             // Coordinate system information
    uint64_t pointCount = 0;                    // Number of points in this scan
    uint64_t binaryOffset = 0;                  // Offset to binary data (if available)
    uint64_t binaryLength = 0;                  // Length of binary data (if available)
    
    ScanMetadata() = default;
};

/**
 * @brief Structure representing complete E57 file metadata
 */
struct E57FileMetadata {
    std::string fileGuid;                   // File-level GUID
    std::string creationDateTime;           // When the file was created
    std::string coordinateMetadata;         // File-level coordinate metadata
    std::vector<ScanMetadata> scans;        // All scans in the file
    std::vector<std::string> images2D;      // 2D image GUIDs (if any)
    
    E57FileMetadata() = default;
};

/**
 * @brief E57XmlParser - Robust XML parser for E57 files using libE57Format
 * 
 * This class provides comprehensive parsing of E57 XML sections to extract
 * metadata, scan information, and binary section details. It uses libE57Format
 * for low-level XML parsing and provides a high-level interface for accessing
 * E57 file structure.
 * 
 * Key features:
 * - Complete E57 XML DOM navigation
 * - Extraction of scan metadata and point attributes
 * - Binary section location and structure information
 * - Support for multi-scan files
 * - Robust error handling with detailed error messages
 * 
 * Usage:
 *   E57XmlParser parser("file.e57");
 *   auto metadata = parser.parseFile();
 *   for (const auto& scan : metadata.scans) {
 *       auto binarySection = parser.getBinarySectionInfo(scan.guid);
 *       // Process scan...
 *   }
 */
class E57XmlParser {
public:
    /**
     * @brief Constructor
     * @param filePath Path to the E57 file to parse
     */
    explicit E57XmlParser(const std::string& filePath);
    
    /**
     * @brief Destructor
     */
    ~E57XmlParser();

    /**
     * @brief Parse the complete E57 file structure and extract all metadata
     * @return E57FileMetadata structure containing all file information
     * @throws std::runtime_error if parsing fails
     */
    E57FileMetadata parseFile();
    
    /**
     * @brief Parse only the data3D sections to get scan information
     * @return Vector of ScanMetadata for all scans in the file
     * @throws std::runtime_error if parsing fails
     */
    std::vector<ScanMetadata> parseData3DSections();
    
    /**
     * @brief Parse images2D section to get 2D image information
     * @return Vector of image GUIDs
     * @throws std::runtime_error if parsing fails
     */
    std::vector<std::string> parseImages2D();
    
    /**
     * @brief Get binary section information for a specific scan
     * @param scanGuid GUID of the scan to get binary info for
     * @return BinarySection structure with offset, length, and GUID
     * @throws std::runtime_error if scan not found or binary info unavailable
     */
    BinarySection getBinarySectionInfo(const std::string& scanGuid);
    
    /**
     * @brief Get the total number of scans in the file
     * @return Number of data3D sections found
     */
    int getScanCount();
    
    /**
     * @brief Check if the file is valid and can be parsed
     * @return true if file is valid E57 format
     */
    bool isValidE57File();

private:
    std::string m_filePath;                         // Path to the E57 file
    std::unique_ptr<e57::ImageFile> m_imageFile;    // libE57Format ImageFile instance
    std::map<std::string, ScanMetadata> m_scanCache; // Cache of parsed scan metadata
    
    /**
     * @brief Open the E57 file using libE57Format
     */
    void openFile();
    
    /**
     * @brief Close the E57 file and clean up resources
     */
    void closeFile();
    
    /**
     * @brief Parse a single scan node from the data3D vector
     * @param scanNode The scan structure node to parse
     * @param scanIndex Index of the scan for error reporting
     * @return ScanMetadata structure with parsed information
     */
    ScanMetadata parseScanNode(const e57::Node& scanNode, int scanIndex);
    
    /**
     * @brief Parse the point prototype to extract available attributes
     * @param prototype The prototype structure node
     * @return Vector of PointAttribute structures
     */
    std::vector<PointAttribute> parsePointPrototype(const e57::StructureNode& prototype);
    
    /**
     * @brief Parse coordinate metadata from a node
     * @param coordNode The coordinate metadata node
     * @return CoordinateMetadata structure
     */
    CoordinateMetadata parseCoordinateMetadata(const e57::Node& coordNode);
    
    /**
     * @brief Parse a single attribute node to extract type and limits
     * @param attrNode The attribute node to parse
     * @return PointAttribute structure with parsed information
     */
    PointAttribute parseAttributeNode(const e57::Node& attrNode);
    
    /**
     * @brief Extract binary section information from a CompressedVectorNode
     * @param pointsNode The points CompressedVectorNode
     * @param scanGuid GUID of the scan for error reporting
     * @return BinarySection with offset and length information
     */
    BinarySection extractBinarySectionInfo(const e57::CompressedVectorNode& pointsNode, 
                                          const std::string& scanGuid);
    
    /**
     * @brief Validate that required elements exist in a scan node
     * @param scanNode The scan node to validate
     * @param scanIndex Index for error reporting
     * @return true if scan has required elements
     */
    bool validateScanNode(const e57::StructureNode& scanNode, int scanIndex);
    
    /**
     * @brief Convert E57 element type to string for debugging
     * @param elementType The E57 element type
     * @return String representation of the type
     */
    std::string elementTypeToString(int elementType);
};

} // namespace E57Parser
