#include "E57HeaderParser.h"
#include <cstring>
#include <algorithm>

// E57 standard constants
static const char* EXPECTED_SIGNATURE = "ASTM E57 3D Image File Format Std. V1.0";
static const size_t HEADER_SIZE = 48;  // As per ASTM E2807 standard
static const size_t SIGNATURE_SIZE = 32;

E57HeaderParser::E57HeaderParser() {
    ClearError();
}

E57HeaderParser::~E57HeaderParser() {
    // Nothing to clean up
}

bool E57HeaderParser::Parse(const std::string& filePath) {
    ClearError();
    
    // Open file in binary mode
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        SetError("File not found or inaccessible: " + filePath);
        return false;
    }
    
    // Check file size
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (fileSize < HEADER_SIZE) {
        SetError("File too short: expected at least " + std::to_string(HEADER_SIZE) + 
                " bytes, got " + std::to_string(fileSize));
        return false;
    }
    
    // Read and validate signature
    if (!ReadAndValidateSignature(file)) {
        return false;
    }
    
    // Read version numbers
    if (!ReadVersionNumbers(file)) {
        return false;
    }
    
    // Read file length and XML information
    if (!ReadFileLengthAndXmlInfo(file)) {
        return false;
    }
    
    // Validate header field consistency
    if (!ValidateHeaderFields()) {
        return false;
    }
    
    return true;
}

const E57HeaderData& E57HeaderParser::GetData() const {
    return m_headerData;
}

const std::string& E57HeaderParser::GetLastError() const {
    return m_lastError;
}

void E57HeaderParser::ClearError() {
    m_lastError.clear();
}

bool E57HeaderParser::ReadAndValidateSignature(std::ifstream& file) {
    // Read signature (32 bytes)
    char signature[SIGNATURE_SIZE];
    file.read(signature, SIGNATURE_SIZE);
    
    if (file.gcount() != SIGNATURE_SIZE) {
        SetError("Failed to read file signature");
        return false;
    }
    
    // Ensure null termination for comparison
    signature[SIGNATURE_SIZE - 1] = '\0';
    
    // Validate signature
    if (std::strcmp(signature, EXPECTED_SIGNATURE) != 0) {
        SetError("Invalid file signature: expected '" + std::string(EXPECTED_SIGNATURE) + 
                "', got '" + std::string(signature) + "'");
        return false;
    }
    
    // Store signature
    std::memcpy(m_headerData.fileSignature, signature, SIGNATURE_SIZE);
    
    return true;
}

bool E57HeaderParser::ReadVersionNumbers(std::ifstream& file) {
    // Read major version (4 bytes, little-endian)
    if (!ReadUInt32LE(file, m_headerData.majorVersion)) {
        SetError("Failed to read major version");
        return false;
    }
    
    // Read minor version (4 bytes, little-endian)
    if (!ReadUInt32LE(file, m_headerData.minorVersion)) {
        SetError("Failed to read minor version");
        return false;
    }
    
    return true;
}

bool E57HeaderParser::ReadFileLengthAndXmlInfo(std::ifstream& file) {
    // Read file length (8 bytes, little-endian)
    if (!ReadUInt64LE(file, m_headerData.fileLength)) {
        SetError("Failed to read file length");
        return false;
    }
    
    // Read XML payload offset (8 bytes, little-endian)
    if (!ReadUInt64LE(file, m_headerData.xmlPayloadOffset)) {
        SetError("Failed to read XML payload offset");
        return false;
    }
    
    // Read XML payload length (8 bytes, little-endian)
    if (!ReadUInt64LE(file, m_headerData.xmlPayloadLength)) {
        SetError("Failed to read XML payload length");
        return false;
    }
    
    return true;
}

bool E57HeaderParser::ValidateHeaderFields() {
    // Validate XML offset is within file bounds
    if (m_headerData.xmlPayloadOffset >= m_headerData.fileLength) {
        SetError("Invalid XML offset: " + std::to_string(m_headerData.xmlPayloadOffset) + 
                " >= file length " + std::to_string(m_headerData.fileLength));
        return false;
    }
    
    // Validate XML section doesn't extend beyond file
    uint64_t xmlEnd = m_headerData.xmlPayloadOffset + m_headerData.xmlPayloadLength;
    if (xmlEnd > m_headerData.fileLength) {
        SetError("Invalid XML section: offset " + std::to_string(m_headerData.xmlPayloadOffset) + 
                " + length " + std::to_string(m_headerData.xmlPayloadLength) + 
                " = " + std::to_string(xmlEnd) + " > file length " + 
                std::to_string(m_headerData.fileLength));
        return false;
    }
    
    // Validate XML offset is at least after header
    if (m_headerData.xmlPayloadOffset < HEADER_SIZE) {
        SetError("Invalid XML offset: " + std::to_string(m_headerData.xmlPayloadOffset) + 
                " < header size " + std::to_string(HEADER_SIZE));
        return false;
    }
    
    return true;
}

void E57HeaderParser::SetError(const std::string& error) {
    m_lastError = error;
}

bool E57HeaderParser::ReadUInt32LE(std::ifstream& file, uint32_t& value) {
    uint8_t bytes[4];
    file.read(reinterpret_cast<char*>(bytes), 4);
    
    if (file.gcount() != 4) {
        return false;
    }
    
    // Convert from little-endian
    value = static_cast<uint32_t>(bytes[0]) |
            (static_cast<uint32_t>(bytes[1]) << 8) |
            (static_cast<uint32_t>(bytes[2]) << 16) |
            (static_cast<uint32_t>(bytes[3]) << 24);
    
    return true;
}

bool E57HeaderParser::ReadUInt64LE(std::ifstream& file, uint64_t& value) {
    uint8_t bytes[8];
    file.read(reinterpret_cast<char*>(bytes), 8);
    
    if (file.gcount() != 8) {
        return false;
    }
    
    // Convert from little-endian
    value = static_cast<uint64_t>(bytes[0]) |
            (static_cast<uint64_t>(bytes[1]) << 8) |
            (static_cast<uint64_t>(bytes[2]) << 16) |
            (static_cast<uint64_t>(bytes[3]) << 24) |
            (static_cast<uint64_t>(bytes[4]) << 32) |
            (static_cast<uint64_t>(bytes[5]) << 40) |
            (static_cast<uint64_t>(bytes[6]) << 48) |
            (static_cast<uint64_t>(bytes[7]) << 56);
    
    return true;
}
