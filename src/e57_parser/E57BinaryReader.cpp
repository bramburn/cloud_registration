#include "E57BinaryReader.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace E57Parser {

// Static member initialization
std::array<uint32_t, 256> E57BinaryReader::s_crcTable;
bool E57BinaryReader::s_tableInitialized = false;

E57BinaryReader::E57BinaryReader(const std::string& filePath) 
    : m_filePath(filePath) {
    if (!s_tableInitialized) {
        initializeCRCTable();
        s_tableInitialized = true;
    }
    openFile();
}

E57BinaryReader::~E57BinaryReader() {
    closeFile();
}

void E57BinaryReader::initializeCRCTable() {
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) {
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
            } else {
                crc >>= 1;
            }
        }
        s_crcTable[i] = crc;
    }
}

uint32_t E57BinaryReader::calculateCRC32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFFUL;
    
    // Process data byte by byte using lookup table
    for (size_t i = 0; i < length; ++i) {
        uint8_t tableIndex = (crc ^ data[i]) & 0xFF;
        crc = (crc >> 8) ^ s_crcTable[tableIndex];
    }
    
    return crc ^ 0xFFFFFFFFUL;
}

PageValidationResult E57BinaryReader::validatePage(const uint8_t* pageData, size_t pageIndex) {
    PageValidationResult result;
    result.pageIndex = pageIndex;
    
    // Extract stored CRC from first 4 bytes (little-endian)
    result.storedCRC = static_cast<uint32_t>(pageData[0]) |
                      (static_cast<uint32_t>(pageData[1]) << 8) |
                      (static_cast<uint32_t>(pageData[2]) << 16) |
                      (static_cast<uint32_t>(pageData[3]) << 24);
    
    // Calculate CRC for the 1020-byte payload
    result.calculatedCRC = calculateCRC32(pageData + CRC_SIZE, PAYLOAD_SIZE);
    
    result.isValid = (result.storedCRC == result.calculatedCRC);
    
    if (!result.isValid) {
        result.errorMessage = "CRC mismatch in page " + std::to_string(pageIndex) + 
                             ": stored=0x" + std::to_string(result.storedCRC) + 
                             ", calculated=0x" + std::to_string(result.calculatedCRC);
    }
    
    return result;
}

std::vector<uint8_t> E57BinaryReader::readBinarySection(const BinarySection& section) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (!m_fileStream || !m_fileStream->is_open()) {
        throw E57DataCorruptionError("File stream not open for reading");
    }
    
    // Seek to the binary section offset
    m_fileStream->seekg(section.offset, std::ios::beg);
    if (m_fileStream->fail()) {
        throw E57DataCorruptionError("Failed to seek to binary section offset " + 
                                    std::to_string(section.offset) + " for section " + section.guid);
    }
    
    std::vector<uint8_t> result;
    result.reserve(section.length);
    
    size_t bytesRemaining = section.length;
    size_t pageIndex = 0;
    size_t totalPagesProcessed = 0;
    size_t validPagesCount = 0;
    size_t corruptedPagesCount = 0;
    
    while (bytesRemaining > 0) {
        std::array<uint8_t, PAGE_SIZE> pageBuffer;
        size_t bytesToRead = std::min(bytesRemaining, PAGE_SIZE);
        
        // Read page data
        m_fileStream->read(reinterpret_cast<char*>(pageBuffer.data()), bytesToRead);
        if (m_fileStream->gcount() != static_cast<std::streamsize>(bytesToRead)) {
            throw E57DataCorruptionError("Failed to read complete page " + std::to_string(pageIndex) + 
                                        " from binary section " + section.guid + 
                                        ": expected " + std::to_string(bytesToRead) + 
                                        " bytes, got " + std::to_string(m_fileStream->gcount()));
        }
        
        // Validate CRC for complete pages only
        if (bytesToRead == PAGE_SIZE) {
            PageValidationResult validation = validatePage(pageBuffer.data(), pageIndex);
            totalPagesProcessed++;
            
            if (validation.isValid) {
                validPagesCount++;
                // Add payload data (skip 4-byte CRC header)
                result.insert(result.end(), 
                             pageBuffer.begin() + CRC_SIZE, 
                             pageBuffer.begin() + PAGE_SIZE);
            } else {
                corruptedPagesCount++;
                throw E57DataCorruptionError("CRC validation failed for page " + 
                                            std::to_string(pageIndex) + " in binary section " + 
                                            section.guid + ": " + validation.errorMessage);
            }
        } else {
            // Handle partial last page (no CRC validation for incomplete pages)
            result.insert(result.end(), pageBuffer.begin(), pageBuffer.begin() + bytesToRead);
        }
        
        bytesRemaining -= bytesToRead;
        pageIndex++;
    }
    
    // Update performance metrics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto elapsedMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    
    m_lastMetrics.totalPages = totalPagesProcessed;
    m_lastMetrics.validPages = validPagesCount;
    m_lastMetrics.corruptedPages = corruptedPagesCount;
    m_lastMetrics.validationTimeMs = elapsedMs;
    m_lastMetrics.throughputMBps = (section.length / (1024.0 * 1024.0)) / (elapsedMs / 1000.0);
    
    return result;
}

std::vector<PageValidationResult> E57BinaryReader::validateAllPages(const BinarySection& section) {
    std::vector<PageValidationResult> results;
    
    if (!m_fileStream || !m_fileStream->is_open()) {
        throw E57DataCorruptionError("File stream not open for validation");
    }
    
    // Seek to the binary section offset
    m_fileStream->seekg(section.offset, std::ios::beg);
    if (m_fileStream->fail()) {
        throw E57DataCorruptionError("Failed to seek to binary section offset for validation");
    }
    
    size_t bytesRemaining = section.length;
    size_t pageIndex = 0;
    
    while (bytesRemaining >= PAGE_SIZE) {
        std::array<uint8_t, PAGE_SIZE> pageBuffer;
        
        m_fileStream->read(reinterpret_cast<char*>(pageBuffer.data()), PAGE_SIZE);
        if (m_fileStream->gcount() != PAGE_SIZE) {
            PageValidationResult errorResult;
            errorResult.pageIndex = pageIndex;
            errorResult.isValid = false;
            errorResult.errorMessage = "Failed to read complete page for validation";
            results.push_back(errorResult);
            break;
        }
        
        PageValidationResult validation = validatePage(pageBuffer.data(), pageIndex);
        results.push_back(validation);
        
        bytesRemaining -= PAGE_SIZE;
        pageIndex++;
    }
    
    return results;
}

void E57BinaryReader::openFile() {
    m_fileStream = std::make_unique<std::ifstream>(m_filePath, std::ios::binary);
    if (!m_fileStream->is_open()) {
        throw E57DataCorruptionError("Failed to open E57 file for reading: " + m_filePath);
    }
}

void E57BinaryReader::closeFile() {
    if (m_fileStream && m_fileStream->is_open()) {
        m_fileStream->close();
    }
}

void E57BinaryReader::updateMetrics(size_t totalBytes, double elapsedMs) {
    m_lastMetrics.validationTimeMs = elapsedMs;
    m_lastMetrics.throughputMBps = (totalBytes / (1024.0 * 1024.0)) / (elapsedMs / 1000.0);
}

} // namespace E57Parser
