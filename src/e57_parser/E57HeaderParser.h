#pragma once

#include <string>
#include <cstdint>
#include <fstream>

/**
 * @brief Data structure containing all E57 header fields as per ASTM E2807 standard
 * 
 * This structure holds the parsed header information from an E57 file.
 * The E57 header is exactly 48 bytes and contains critical file metadata.
 */
struct E57HeaderData {
    char fileSignature[32];        // File signature string (null-terminated)
    uint32_t majorVersion;         // Major version number
    uint32_t minorVersion;         // Minor version number  
    uint64_t fileLength;           // Total file length in bytes
    uint64_t xmlPayloadOffset;     // Offset to XML section
    uint64_t xmlPayloadLength;     // Length of XML section
    
    // Default constructor
    E57HeaderData() : majorVersion(0), minorVersion(0), fileLength(0), 
                      xmlPayloadOffset(0), xmlPayloadLength(0) {
        fileSignature[0] = '\0';
    }
};

/**
 * @brief E57 Header Parser class for low-level E57 file header parsing
 * 
 * This class implements robust parsing of E57 file headers according to the
 * ASTM E2807 standard. It provides validation and error handling for various
 * failure scenarios including invalid signatures, truncated files, and 
 * inconsistent header field values.
 * 
 * Usage:
 *   E57HeaderParser parser;
 *   if (parser.Parse("file.e57")) {
 *       const E57HeaderData& data = parser.GetData();
 *       // Use header data...
 *   } else {
 *       std::string error = parser.GetLastError();
 *       // Handle error...
 *   }
 */
class E57HeaderParser {
public:
    /**
     * @brief Constructor
     */
    E57HeaderParser();
    
    /**
     * @brief Destructor
     */
    ~E57HeaderParser();
    
    /**
     * @brief Parse E57 file header from the given file path
     * 
     * @param filePath Path to the E57 file to parse
     * @return true if parsing succeeded, false otherwise
     */
    bool Parse(const std::string& filePath);
    
    /**
     * @brief Get the parsed header data
     * 
     * @return const reference to the parsed header data
     * @note Only valid if Parse() returned true
     */
    const E57HeaderData& GetData() const;
    
    /**
     * @brief Get the last error message
     * 
     * @return Error message string, empty if no error
     */
    const std::string& GetLastError() const;
    
    /**
     * @brief Clear any previous error state
     */
    void ClearError();

private:
    E57HeaderData m_headerData;     // Parsed header data
    std::string m_lastError;        // Last error message
    
    /**
     * @brief Read and validate the file signature
     * 
     * @param file Input file stream
     * @return true if signature is valid
     */
    bool ReadAndValidateSignature(std::ifstream& file);
    
    /**
     * @brief Read version numbers from header
     * 
     * @param file Input file stream
     * @return true if versions were read successfully
     */
    bool ReadVersionNumbers(std::ifstream& file);
    
    /**
     * @brief Read file length and XML section information
     * 
     * @param file Input file stream
     * @return true if data was read successfully
     */
    bool ReadFileLengthAndXmlInfo(std::ifstream& file);
    
    /**
     * @brief Validate header field consistency
     * 
     * @return true if all fields are logically consistent
     */
    bool ValidateHeaderFields();
    
    /**
     * @brief Set error message
     * 
     * @param error Error message to set
     */
    void SetError(const std::string& error);
    
    /**
     * @brief Read a little-endian uint32_t from file
     * 
     * @param file Input file stream
     * @param value Output value
     * @return true if read successfully
     */
    bool ReadUInt32LE(std::ifstream& file, uint32_t& value);
    
    /**
     * @brief Read a little-endian uint64_t from file
     * 
     * @param file Input file stream
     * @param value Output value
     * @return true if read successfully
     */
    bool ReadUInt64LE(std::ifstream& file, uint64_t& value);
};
