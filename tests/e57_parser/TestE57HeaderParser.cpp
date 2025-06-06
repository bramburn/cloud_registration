#include <gtest/gtest.h>
#include "../../src/e57_parser/E57HeaderParser.h"
#include <fstream>
#include <vector>
#include <cstring>
#include <filesystem>

class E57HeaderParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create temporary directory for test files
        testDir = "test_e57_headers";
        // Create directory if it doesn't exist
        std::filesystem::create_directories(testDir);
    }

    void TearDown() override {
        // Clean up test files
        for (const auto& file : createdFiles) {
            std::remove(file.c_str());
        }
        // Remove test directory
        std::filesystem::remove_all(testDir);
    }
    
    /**
     * @brief Create a valid E57 header with specified parameters
     */
    std::vector<uint8_t> CreateValidHeader(uint32_t majorVer = 1, uint32_t minorVer = 0,
                                          uint64_t fileLen = 1024, uint64_t xmlOffset = 48,
                                          uint64_t xmlLen = 512) {
        std::vector<uint8_t> header(48, 0);
        size_t pos = 0;
        
        // File signature (32 bytes)
        const char* signature = "ASTM E57 3D Image File Format Std. V1.0";
        std::memcpy(header.data() + pos, signature, std::strlen(signature));
        pos += 32;
        
        // Major version (4 bytes, little-endian)
        header[pos++] = majorVer & 0xFF;
        header[pos++] = (majorVer >> 8) & 0xFF;
        header[pos++] = (majorVer >> 16) & 0xFF;
        header[pos++] = (majorVer >> 24) & 0xFF;
        
        // Minor version (4 bytes, little-endian)
        header[pos++] = minorVer & 0xFF;
        header[pos++] = (minorVer >> 8) & 0xFF;
        header[pos++] = (minorVer >> 16) & 0xFF;
        header[pos++] = (minorVer >> 24) & 0xFF;
        
        // File length (8 bytes, little-endian)
        for (int i = 0; i < 8; i++) {
            header[pos++] = (fileLen >> (i * 8)) & 0xFF;
        }
        
        // XML offset (8 bytes, little-endian)
        for (int i = 0; i < 8; i++) {
            header[pos++] = (xmlOffset >> (i * 8)) & 0xFF;
        }
        
        // XML length (8 bytes, little-endian)
        for (int i = 0; i < 8; i++) {
            header[pos++] = (xmlLen >> (i * 8)) & 0xFF;
        }
        
        return header;
    }
    
    /**
     * @brief Create a test file with the given content
     */
    std::string CreateTestFile(const std::string& filename, const std::vector<uint8_t>& content) {
        std::string fullPath = testDir + "/" + filename;
        std::ofstream file(fullPath, std::ios::binary);
        if (file.is_open()) {
            file.write(reinterpret_cast<const char*>(content.data()), content.size());
            file.close();
            createdFiles.push_back(fullPath);
        }
        return fullPath;
    }
    
    std::string testDir;
    std::vector<std::string> createdFiles;
};

// Test Case 1.1: Parse a valid E57 v1.0 file header
TEST_F(E57HeaderParserTest, ParseValidE57Header) {
    // Create valid header
    auto headerData = CreateValidHeader(1, 0, 2048, 48, 1000);
    
    // Add some dummy content to reach the specified file length
    headerData.resize(2048, 0x00);
    
    std::string testFile = CreateTestFile("valid_header.e57", headerData);
    
    E57HeaderParser parser;
    ASSERT_TRUE(parser.Parse(testFile));
    
    const E57HeaderData& data = parser.GetData();
    
    // Verify signature
    EXPECT_STREQ(data.fileSignature, "ASTM E57 3D Image File Format Std. V1.0");
    
    // Verify version numbers
    EXPECT_EQ(data.majorVersion, 1u);
    EXPECT_EQ(data.minorVersion, 0u);
    
    // Verify file and XML information
    EXPECT_EQ(data.fileLength, 2048u);
    EXPECT_EQ(data.xmlPayloadOffset, 48u);
    EXPECT_EQ(data.xmlPayloadLength, 1000u);
    
    // Verify no error
    EXPECT_TRUE(parser.GetLastError().empty());
}

// Test Case 1.2: Attempt to parse a non-E57 file
TEST_F(E57HeaderParserTest, ParseNonE57File) {
    // Create file with wrong signature
    std::vector<uint8_t> wrongHeader(48, 0);
    const char* wrongSig = "This is not an E57 file signature";
    std::memcpy(wrongHeader.data(), wrongSig, std::strlen(wrongSig));
    
    std::string testFile = CreateTestFile("wrong_signature.txt", wrongHeader);
    
    E57HeaderParser parser;
    ASSERT_FALSE(parser.Parse(testFile));
    
    std::string error = parser.GetLastError();
    EXPECT_TRUE(error.find("Invalid file signature") != std::string::npos);
}

// Test Case 1.3: Attempt to parse a truncated file
TEST_F(E57HeaderParserTest, ParseTruncatedFile) {
    // Create file that's too short (only 20 bytes)
    std::vector<uint8_t> shortFile(20, 0x42);
    
    std::string testFile = CreateTestFile("truncated.e57", shortFile);
    
    E57HeaderParser parser;
    ASSERT_FALSE(parser.Parse(testFile));
    
    std::string error = parser.GetLastError();
    EXPECT_TRUE(error.find("File too short") != std::string::npos);
}

// Test Case 1.4: Attempt to parse header with invalid XML offset/length
TEST_F(E57HeaderParserTest, ParseInvalidXmlOffsetLength) {
    // Create header with XML offset beyond file length
    auto headerData = CreateValidHeader(1, 0, 1024, 2048, 100); // XML offset > file length
    headerData.resize(1024, 0x00);
    
    std::string testFile = CreateTestFile("invalid_xml_offset.e57", headerData);
    
    E57HeaderParser parser;
    ASSERT_FALSE(parser.Parse(testFile));
    
    std::string error = parser.GetLastError();
    EXPECT_TRUE(error.find("Invalid XML offset") != std::string::npos);
}

// Additional test: XML section extends beyond file
TEST_F(E57HeaderParserTest, ParseXmlSectionBeyondFile) {
    // Create header where XML offset + length > file length
    auto headerData = CreateValidHeader(1, 0, 1024, 500, 600); // 500 + 600 > 1024
    headerData.resize(1024, 0x00);
    
    std::string testFile = CreateTestFile("xml_beyond_file.e57", headerData);
    
    E57HeaderParser parser;
    ASSERT_FALSE(parser.Parse(testFile));
    
    std::string error = parser.GetLastError();
    EXPECT_TRUE(error.find("Invalid XML section") != std::string::npos);
}

// Test file not found
TEST_F(E57HeaderParserTest, ParseNonExistentFile) {
    E57HeaderParser parser;
    ASSERT_FALSE(parser.Parse("non_existent_file.e57"));
    
    std::string error = parser.GetLastError();
    EXPECT_TRUE(error.find("File not found or inaccessible") != std::string::npos);
}

// Test XML offset before header end
TEST_F(E57HeaderParserTest, ParseXmlOffsetTooEarly) {
    // Create header with XML offset before header ends (< 48)
    auto headerData = CreateValidHeader(1, 0, 1024, 32, 100); // XML offset < 48
    headerData.resize(1024, 0x00);
    
    std::string testFile = CreateTestFile("xml_offset_early.e57", headerData);
    
    E57HeaderParser parser;
    ASSERT_FALSE(parser.Parse(testFile));
    
    std::string error = parser.GetLastError();
    EXPECT_TRUE(error.find("Invalid XML offset") != std::string::npos);
}

// Test little-endian byte order parsing
TEST_F(E57HeaderParserTest, ParseLittleEndianValues) {
    // Create header with specific values to test byte order
    auto headerData = CreateValidHeader(0x12345678, 0x9ABCDEF0, 0x123456789ABCDEF0, 48, 100);
    headerData.resize(0x200, 0x00); // Make file large enough
    
    std::string testFile = CreateTestFile("endian_test.e57", headerData);
    
    E57HeaderParser parser;
    ASSERT_TRUE(parser.Parse(testFile));
    
    const E57HeaderData& data = parser.GetData();
    
    // Verify little-endian parsing
    EXPECT_EQ(data.majorVersion, 0x12345678u);
    EXPECT_EQ(data.minorVersion, 0x9ABCDEF0u);
    EXPECT_EQ(data.fileLength, 0x123456789ABCDEF0u);
}
