#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <array>

namespace E57Parser {

/**
 * @brief Exception thrown when E57 binary data corruption is detected
 */
class E57DataCorruptionError : public std::runtime_error {
public:
    explicit E57DataCorruptionError(const std::string& message) 
        : std::runtime_error(message) {}
};

/**
 * @brief Structure representing a binary section in an E57 file
 */
struct BinarySection {
    uint64_t offset;        // Offset in file where binary section starts
    uint64_t length;        // Length of binary section in bytes
    std::string guid;       // GUID identifying this binary section
    std::string sectionType; // Type of section ("points", "images2D", etc.)
    
    BinarySection() : offset(0), length(0) {}
    BinarySection(uint64_t off, uint64_t len, const std::string& id, const std::string& type = "")
        : offset(off), length(len), guid(id), sectionType(type) {}
};

/**
 * @brief Result of page validation with detailed error information
 */
struct PageValidationResult {
    bool isValid;               // Whether the page passed CRC validation
    uint32_t storedCRC;         // CRC value stored in the page header
    uint32_t calculatedCRC;     // CRC value calculated from payload
    size_t pageIndex;           // Index of the page in the binary section
    std::string errorMessage;   // Detailed error message if validation failed
    
    PageValidationResult() : isValid(false), storedCRC(0), calculatedCRC(0), pageIndex(0) {}
};

/**
 * @brief Performance metrics for binary data validation
 */
struct ValidationMetrics {
    size_t totalPages = 0;          // Total number of pages processed
    size_t validPages = 0;          // Number of pages that passed validation
    size_t corruptedPages = 0;      // Number of pages that failed validation
    double validationTimeMs = 0.0;  // Time spent on validation in milliseconds
    double throughputMBps = 0.0;    // Throughput in megabytes per second
};

/**
 * @brief E57BinaryReader - Reads and validates binary sections of E57 files
 * 
 * This class implements CRC-32 validation for E57 binary data according to the
 * ASTM E2807 standard. E57 binary sections are organized in 1024-byte pages,
 * where each page contains a 4-byte CRC-32 checksum followed by 1020 bytes of payload data.
 * 
 * Key features:
 * - Page-by-page CRC-32 validation
 * - Detailed error reporting with corruption detection
 * - Performance metrics for large datasets
 * - Memory-efficient streaming processing
 * 
 * Usage:
 *   E57BinaryReader reader("file.e57");
 *   BinarySection section{offset, length, "scan-guid"};
 *   try {
 *       auto data = reader.readBinarySection(section);
 *       // Process validated data...
 *   } catch (const E57DataCorruptionError& e) {
 *       // Handle corruption...
 *   }
 */
class E57BinaryReader {
public:
    /**
     * @brief Constructor
     * @param filePath Path to the E57 file to read from
     */
    explicit E57BinaryReader(const std::string& filePath);
    
    /**
     * @brief Destructor
     */
    ~E57BinaryReader();

    /**
     * @brief Read and validate a complete binary section with CRC-32 checking
     * @param section Binary section information (offset, length, GUID)
     * @return Vector containing the validated payload data (without CRC headers)
     * @throws E57DataCorruptionError if any page fails CRC validation
     */
    std::vector<uint8_t> readBinarySection(const BinarySection& section);
    
    /**
     * @brief Validate a single page and return detailed results
     * @param pageData Pointer to 1024-byte page data
     * @param pageIndex Index of the page for error reporting
     * @return PageValidationResult with validation details
     */
    PageValidationResult validatePage(const uint8_t* pageData, size_t pageIndex);
    
    /**
     * @brief Validate all pages in a binary section without reading payload
     * @param section Binary section to validate
     * @return Vector of validation results for each page
     */
    std::vector<PageValidationResult> validateAllPages(const BinarySection& section);
    
    /**
     * @brief Calculate CRC-32 checksum using optimized lookup table
     * @param data Pointer to data to calculate CRC for
     * @param length Length of data in bytes
     * @return CRC-32 checksum value
     */
    uint32_t calculateCRC32(const uint8_t* data, size_t length);
    
    /**
     * @brief Get performance metrics from the last validation operation
     * @return ValidationMetrics structure with timing and throughput data
     */
    ValidationMetrics getLastValidationMetrics() const { return m_lastMetrics; }

private:
    // E57 binary format constants
    static constexpr size_t PAGE_SIZE = 1024;           // Total page size
    static constexpr size_t CRC_SIZE = 4;               // CRC header size
    static constexpr size_t PAYLOAD_SIZE = PAGE_SIZE - CRC_SIZE; // Payload size per page
    static constexpr uint32_t CRC32_POLYNOMIAL = 0xEDB88320UL;   // CRC-32 polynomial
    
    std::string m_filePath;                     // Path to the E57 file
    std::unique_ptr<std::ifstream> m_fileStream; // File stream for reading
    ValidationMetrics m_lastMetrics;            // Performance metrics from last operation
    
    // Optimized CRC-32 lookup table for performance
    static std::array<uint32_t, 256> s_crcTable;
    static bool s_tableInitialized;
    
    /**
     * @brief Initialize the CRC-32 lookup table
     */
    void initializeCRCTable();
    
    /**
     * @brief Open the file stream for reading
     */
    void openFile();
    
    /**
     * @brief Close the file stream
     */
    void closeFile();
    
    /**
     * @brief Update performance metrics after an operation
     * @param totalBytes Total bytes processed
     * @param elapsedMs Time elapsed in milliseconds
     */
    void updateMetrics(size_t totalBytes, double elapsedMs);
};

} // namespace E57Parser
