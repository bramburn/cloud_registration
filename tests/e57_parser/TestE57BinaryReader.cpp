#include <gtest/gtest.h>
#include "e57_parser/E57BinaryReader.h"
#include <fstream>
#include <vector>
#include <filesystem>

using namespace E57Parser;

class E57BinaryReaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test data files
        createValidTestFile();
        createCorruptedTestFile();
        createMultiPageTestFile();
    }
    
    void TearDown() override {
        // Clean up test files
        std::filesystem::remove("valid_test.e57");
        std::filesystem::remove("corrupted_test.e57");
        std::filesystem::remove("multipage_test.e57");
    }
    
    void createValidTestFile() {
        std::ofstream file("valid_test.e57", std::ios::binary);
        
        // Create a valid page with correct CRC
        std::vector<uint8_t> payload(1020, 0x42); // Fill with test data
        
        // Calculate CRC for the payload
        E57BinaryReader tempReader("dummy_path"); // Just for CRC calculation
        uint32_t crc = tempReader.calculateCRC32(payload.data(), payload.size());
        
        // Write CRC in little-endian format followed by payload
        file.put(static_cast<char>(crc & 0xFF));
        file.put(static_cast<char>((crc >> 8) & 0xFF));
        file.put(static_cast<char>((crc >> 16) & 0xFF));
        file.put(static_cast<char>((crc >> 24) & 0xFF));
        file.write(reinterpret_cast<const char*>(payload.data()), payload.size());
        file.close();
    }
    
    void createCorruptedTestFile() {
        std::ofstream file("corrupted_test.e57", std::ios::binary);
        
        // Create a page with incorrect CRC
        uint32_t wrongCrc = 0xDEADBEEF;
        std::vector<uint8_t> payload(1020, 0x42);
        
        // Write wrong CRC in little-endian format
        file.put(static_cast<char>(wrongCrc & 0xFF));
        file.put(static_cast<char>((wrongCrc >> 8) & 0xFF));
        file.put(static_cast<char>((wrongCrc >> 16) & 0xFF));
        file.put(static_cast<char>((wrongCrc >> 24) & 0xFF));
        file.write(reinterpret_cast<const char*>(payload.data()), payload.size());
        file.close();
    }
    
    void createMultiPageTestFile() {
        std::ofstream file("multipage_test.e57", std::ios::binary);
        
        E57BinaryReader tempReader("dummy_path");
        
        // Create 3 valid pages
        for (int page = 0; page < 3; ++page) {
            std::vector<uint8_t> payload(1020, static_cast<uint8_t>(0x10 + page));
            uint32_t crc = tempReader.calculateCRC32(payload.data(), payload.size());
            
            // Write CRC in little-endian format
            file.put(static_cast<char>(crc & 0xFF));
            file.put(static_cast<char>((crc >> 8) & 0xFF));
            file.put(static_cast<char>((crc >> 16) & 0xFF));
            file.put(static_cast<char>((crc >> 24) & 0xFF));
            file.write(reinterpret_cast<const char*>(payload.data()), payload.size());
        }
        file.close();
    }
};

// Test Case 1.1: Load a valid E57 file and verify that all data is read correctly without any CRC errors
TEST_F(E57BinaryReaderTest, ValidFileLoadsSuccessfully) {
    E57BinaryReader reader("valid_test.e57");
    BinarySection section{0, 1024, "test-guid", "points"};

    std::vector<uint8_t> data;
    EXPECT_NO_THROW(data = reader.readBinarySection(section));

    EXPECT_EQ(data.size(), 1020); // Payload size without CRC

    // Verify all payload bytes are 0x42
    for (size_t i = 0; i < data.size(); ++i) {
        EXPECT_EQ(data[i], 0x42) << "Byte " << i << " should be 0x42";
    }

    // Check metrics
    auto metrics = reader.getLastValidationMetrics();
    EXPECT_EQ(metrics.totalPages, 1);
    EXPECT_EQ(metrics.validPages, 1);
    EXPECT_EQ(metrics.corruptedPages, 0);
    EXPECT_GT(metrics.throughputMBps, 0.0);
}

// Test Case 1.2: Create a test E57 file with a single corrupted binary page and verify that the application detects the CRC error
TEST_F(E57BinaryReaderTest, CorruptedFileThrowsException) {
    E57BinaryReader reader("corrupted_test.e57");
    BinarySection section{0, 1024, "test-guid", "points"};
    
    EXPECT_THROW({
        reader.readBinarySection(section);
    }, E57DataCorruptionError);
    
    // Verify metrics show corruption
    auto metrics = reader.getLastValidationMetrics();
    EXPECT_EQ(metrics.totalPages, 0); // Should fail before completing
    EXPECT_EQ(metrics.validPages, 0);
    EXPECT_EQ(metrics.corruptedPages, 1);
}

// Test Case 1.3: Test with multiple pages and ensure the application reports the first error it encounters
TEST_F(E57BinaryReaderTest, MultiPageValidation) {
    E57BinaryReader reader("multipage_test.e57");
    BinarySection section{0, 3 * 1024, "test-guid", "points"};
    
    auto data = reader.readBinarySection(section);
    EXPECT_EQ(data.size(), 3 * 1020); // 3 pages of payload data
    
    // Verify data from each page
    for (int page = 0; page < 3; ++page) {
        uint8_t expectedValue = static_cast<uint8_t>(0x10 + page);
        for (int i = 0; i < 1020; ++i) {
            size_t index = page * 1020 + i;
            EXPECT_EQ(data[index], expectedValue) 
                << "Page " << page << ", byte " << i << " should be " << std::hex << expectedValue;
        }
    }
    
    auto metrics = reader.getLastValidationMetrics();
    EXPECT_EQ(metrics.totalPages, 3);
    EXPECT_EQ(metrics.validPages, 3);
    EXPECT_EQ(metrics.corruptedPages, 0);
}

// Test Case 1.4: Test with an E57 file that has an empty binary section to ensure no errors are thrown
TEST_F(E57BinaryReaderTest, EmptyBinarySectionHandling) {
    // Create empty file
    std::ofstream emptyFile("empty_test.e57", std::ios::binary);
    emptyFile.close();
    
    E57BinaryReader reader("empty_test.e57");
    BinarySection section{0, 0, "empty-guid", "points"};
    
    auto data = reader.readBinarySection(section);
    EXPECT_TRUE(data.empty());
    
    auto metrics = reader.getLastValidationMetrics();
    EXPECT_EQ(metrics.totalPages, 0);
    EXPECT_EQ(metrics.validPages, 0);
    EXPECT_EQ(metrics.corruptedPages, 0);
    
    std::filesystem::remove("empty_test.e57");
}

// Test CRC-32 calculation correctness
TEST_F(E57BinaryReaderTest, CRCCalculationIsCorrect) {
    E57BinaryReader reader("valid_test.e57");
    
    // Test with known data
    std::vector<uint8_t> testData = {0x01, 0x02, 0x03, 0x04};
    uint32_t crc = reader.calculateCRC32(testData.data(), testData.size());
    
    // Verify against known CRC-32 value for this data
    EXPECT_EQ(crc, 0xB63CFBCD) << "CRC-32 for {0x01, 0x02, 0x03, 0x04} should be 0xB63CFBCD";
    
    // Test with empty data
    uint32_t emptyCrc = reader.calculateCRC32(nullptr, 0);
    EXPECT_EQ(emptyCrc, 0x00000000) << "CRC-32 for empty data should be 0";
    
    // Test with single byte
    uint8_t singleByte = 0xFF;
    uint32_t singleCrc = reader.calculateCRC32(&singleByte, 1);
    EXPECT_NE(singleCrc, 0) << "CRC-32 for single byte should not be 0";
}

// Test page validation with detailed results
TEST_F(E57BinaryReaderTest, PageValidationDetails) {
    E57BinaryReader reader("valid_test.e57");
    
    // Read the page data manually
    std::ifstream file("valid_test.e57", std::ios::binary);
    std::array<uint8_t, 1024> pageData;
    file.read(reinterpret_cast<char*>(pageData.data()), 1024);
    file.close();
    
    PageValidationResult result = reader.validatePage(pageData.data(), 0);
    
    EXPECT_TRUE(result.isValid);
    EXPECT_EQ(result.pageIndex, 0);
    EXPECT_EQ(result.storedCRC, result.calculatedCRC);
    EXPECT_TRUE(result.errorMessage.empty());
}

// Test batch validation
TEST_F(E57BinaryReaderTest, BatchValidation) {
    E57BinaryReader reader("multipage_test.e57");
    BinarySection section{0, 3 * 1024, "test-guid", "points"};
    
    auto results = reader.validateAllPages(section);
    EXPECT_EQ(results.size(), 3);
    
    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_TRUE(results[i].isValid) << "Page " << i << " should be valid";
        EXPECT_EQ(results[i].pageIndex, i);
        EXPECT_TRUE(results[i].errorMessage.empty());
    }
}

// Test error handling for non-existent file
TEST_F(E57BinaryReaderTest, NonExistentFileThrowsException) {
    EXPECT_THROW({
        E57BinaryReader reader("non_existent_file.e57");
    }, E57DataCorruptionError);
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
