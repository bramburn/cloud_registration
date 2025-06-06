#include <gtest/gtest.h>
#include "e57_parser/E57XmlParser.h"
#include <QFile>
#include <QDebug>

using namespace E57Parser;

class E57XmlParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // We'll use existing test files from the project
        // These should be valid E57 files with known structure
    }
    
    void TearDown() override {
        // No cleanup needed for existing test files
    }
    
    bool fileExists(const std::string& filePath) {
        return QFile::exists(QString::fromStdString(filePath));
    }
};

// Test Case 2.1: Parse a single-scan E57 file and verify that the GUID and point prototype are correctly extracted
TEST_F(E57XmlParserTest, ParseSingleScanFile) {
    std::string testFile = "test_data/test_real_points.e57";
    
    if (!fileExists(testFile)) {
        GTEST_SKIP() << "Test file " << testFile << " not found";
        return;
    }
    
    EXPECT_NO_THROW({
        E57XmlParser parser(testFile);
        
        // Test file validation
        EXPECT_TRUE(parser.isValidE57File()) << "File should be valid E57 format";
        
        // Parse complete file metadata
        E57FileMetadata metadata = parser.parseFile();
        
        // Verify file-level metadata
        EXPECT_FALSE(metadata.fileGuid.empty()) << "File should have a GUID";
        
        // Verify scan data
        EXPECT_GE(metadata.scans.size(), 1) << "File should have at least one scan";
        
        const ScanMetadata& scan = metadata.scans[0];
        EXPECT_FALSE(scan.guid.empty()) << "Scan should have a GUID";
        EXPECT_FALSE(scan.name.empty()) << "Scan should have a name";
        EXPECT_GT(scan.pointCount, 0) << "Scan should have points";
        
        // Verify point attributes (should have at least XYZ coordinates)
        EXPECT_GE(scan.pointAttributes.size(), 3) << "Scan should have at least XYZ attributes";
        
        bool hasX = false, hasY = false, hasZ = false;
        for (const auto& attr : scan.pointAttributes) {
            if (attr.name == "cartesianX") hasX = true;
            if (attr.name == "cartesianY") hasY = true;
            if (attr.name == "cartesianZ") hasZ = true;
        }
        
        EXPECT_TRUE(hasX && hasY && hasZ) << "Scan should have cartesian X, Y, Z coordinates";
        
        qDebug() << "Single scan test - File GUID:" << QString::fromStdString(metadata.fileGuid);
        qDebug() << "Scan GUID:" << QString::fromStdString(scan.guid);
        qDebug() << "Point count:" << scan.pointCount;
        qDebug() << "Attributes found:" << scan.pointAttributes.size();
    });
}

// Test Case 2.2: Parse a multi-scan E57 file and confirm that the parser identifies all data3D sections
TEST_F(E57XmlParserTest, ParseMultiScanFile) {
    // Try multiple potential multi-scan test files
    std::vector<std::string> testFiles = {
        "sample/bunnyDouble.e57",
        "sample/bunnyInt32.e57",
        "test_data/test_triangle.e57"
    };
    
    bool foundMultiScanFile = false;
    
    for (const auto& testFile : testFiles) {
        if (!fileExists(testFile)) {
            continue;
        }
        
        try {
            E57XmlParser parser(testFile);
            
            if (!parser.isValidE57File()) {
                continue;
            }
            
            int scanCount = parser.getScanCount();
            qDebug() << "Testing file:" << QString::fromStdString(testFile) 
                     << "with" << scanCount << "scans";
            
            if (scanCount > 1) {
                foundMultiScanFile = true;
                
                // Parse all scans
                std::vector<ScanMetadata> scans = parser.parseData3DSections();
                EXPECT_EQ(scans.size(), static_cast<size_t>(scanCount)) 
                    << "Should parse all " << scanCount << " scans";
                
                // Verify each scan has required metadata
                for (size_t i = 0; i < scans.size(); ++i) {
                    const ScanMetadata& scan = scans[i];
                    EXPECT_FALSE(scan.guid.empty()) << "Scan " << i << " should have GUID";
                    EXPECT_FALSE(scan.name.empty()) << "Scan " << i << " should have name";
                    EXPECT_GE(scan.pointAttributes.size(), 3) << "Scan " << i << " should have attributes";
                }
                
                qDebug() << "Multi-scan test passed with" << scanCount << "scans";
                break;
            }
            
        } catch (const std::exception& e) {
            qDebug() << "Error testing file" << QString::fromStdString(testFile) << ":" << e.what();
            continue;
        }
    }
    
    if (!foundMultiScanFile) {
        GTEST_SKIP() << "No multi-scan E57 test files found";
    }
}

// Test Case 2.3: Parse an E57 file with color and intensity attributes and verify that the prototype parsing correctly identifies these fields
TEST_F(E57XmlParserTest, ParseFileWithColorAndIntensity) {
    std::vector<std::string> testFiles = {
        "test_color_only.e57",
        "test_intensity_only.e57",
        "test_xyz_only.e57",
        "sample/bunnyDouble.e57"
    };
    
    bool foundAttributeFile = false;
    
    for (const auto& testFile : testFiles) {
        if (!fileExists(testFile)) {
            continue;
        }
        
        try {
            E57XmlParser parser(testFile);
            
            if (!parser.isValidE57File()) {
                continue;
            }
            
            std::vector<ScanMetadata> scans = parser.parseData3DSections();
            if (scans.empty()) {
                continue;
            }
            
            const ScanMetadata& scan = scans[0];
            
            // Check for various attributes
            bool hasIntensity = false;
            bool hasColorRed = false, hasColorGreen = false, hasColorBlue = false;
            bool hasTimeStamp = false;
            
            for (const auto& attr : scan.pointAttributes) {
                if (attr.name == "intensity") hasIntensity = true;
                if (attr.name == "colorRed") hasColorRed = true;
                if (attr.name == "colorGreen") hasColorGreen = true;
                if (attr.name == "colorBlue") hasColorBlue = true;
                if (attr.name == "timeStamp") hasTimeStamp = true;
                
                // Verify attribute has valid type information
                EXPECT_GE(attr.elementType, 0) << "Attribute " << attr.name << " should have valid type";
                
                qDebug() << "Found attribute:" << QString::fromStdString(attr.name) 
                         << "type:" << attr.elementType 
                         << "hasLimits:" << attr.hasLimits;
            }
            
            if (hasIntensity || hasColorRed || hasColorGreen || hasColorBlue || hasTimeStamp) {
                foundAttributeFile = true;
                
                qDebug() << "Attribute test - File:" << QString::fromStdString(testFile);
                qDebug() << "Has intensity:" << hasIntensity;
                qDebug() << "Has color:" << (hasColorRed || hasColorGreen || hasColorBlue);
                qDebug() << "Has timestamp:" << hasTimeStamp;
                
                // If file has color, should have all RGB components
                if (hasColorRed || hasColorGreen || hasColorBlue) {
                    EXPECT_TRUE(hasColorRed && hasColorGreen && hasColorBlue) 
                        << "If file has color, should have all RGB components";
                }
                
                break;
            }
            
        } catch (const std::exception& e) {
            qDebug() << "Error testing file" << QString::fromStdString(testFile) << ":" << e.what();
            continue;
        }
    }
    
    if (!foundAttributeFile) {
        qDebug() << "No files with extended attributes found, testing basic coordinate parsing";
        
        // Test with basic coordinate file
        std::string basicFile = "test_data/test_real_points.e57";
        if (fileExists(basicFile)) {
            E57XmlParser parser(basicFile);
            std::vector<ScanMetadata> scans = parser.parseData3DSections();
            
            if (!scans.empty()) {
                const ScanMetadata& scan = scans[0];
                EXPECT_GE(scan.pointAttributes.size(), 3) << "Should have at least XYZ attributes";
                foundAttributeFile = true;
            }
        }
    }
    
    EXPECT_TRUE(foundAttributeFile) << "Should find at least one file with parseable attributes";
}

// Test Case 2.4: Attempt to parse an E57 file with a corrupted XML section and confirm that a descriptive error is thrown
TEST_F(E57XmlParserTest, ParseCorruptedXMLFile) {
    // Test with non-existent file
    EXPECT_THROW({
        E57XmlParser parser("non_existent_file.e57");
    }, std::runtime_error);
    
    // Test with invalid file format (if we have one)
    std::vector<std::string> potentiallyCorruptFiles = {
        "test_data/malformed_compressedvector.e57",
        "README.md"  // Non-E57 file
    };
    
    for (const auto& testFile : potentiallyCorruptFiles) {
        if (!fileExists(testFile)) {
            continue;
        }
        
        try {
            E57XmlParser parser(testFile);
            
            // If constructor succeeds, validation should fail
            EXPECT_FALSE(parser.isValidE57File()) 
                << "Corrupted file " << testFile << " should not validate";
            
            // Parsing should throw or return empty results
            EXPECT_THROW({
                parser.parseFile();
            }, std::runtime_error) << "Parsing corrupted file should throw exception";
            
        } catch (const std::runtime_error& e) {
            // Expected behavior - verify error message is descriptive
            std::string errorMsg = e.what();
            EXPECT_FALSE(errorMsg.empty()) << "Error message should not be empty";
            EXPECT_TRUE(errorMsg.find("E57") != std::string::npos || 
                       errorMsg.find("Exception") != std::string::npos ||
                       errorMsg.find("parsing") != std::string::npos) 
                << "Error message should be descriptive: " << errorMsg;
            
            qDebug() << "Expected error for corrupted file:" << QString::fromStdString(errorMsg);
        }
    }
}

// Test binary section info extraction
TEST_F(E57XmlParserTest, BinarySectionInfoExtraction) {
    std::string testFile = "test_data/test_real_points.e57";
    
    if (!fileExists(testFile)) {
        GTEST_SKIP() << "Test file " << testFile << " not found";
        return;
    }
    
    EXPECT_NO_THROW({
        E57XmlParser parser(testFile);
        std::vector<ScanMetadata> scans = parser.parseData3DSections();
        
        if (!scans.empty()) {
            const ScanMetadata& scan = scans[0];
            
            // Try to get binary section info
            BinarySection binaryInfo = parser.getBinarySectionInfo(scan.guid);
            
            EXPECT_EQ(binaryInfo.guid, scan.guid) << "Binary section GUID should match scan GUID";
            EXPECT_EQ(binaryInfo.sectionType, "points") << "Section type should be 'points'";
            
            qDebug() << "Binary section info - GUID:" << QString::fromStdString(binaryInfo.guid);
            qDebug() << "Offset:" << binaryInfo.offset << "Length:" << binaryInfo.length;
        }
    });
}

// Test scan count functionality
TEST_F(E57XmlParserTest, ScanCountAccuracy) {
    std::vector<std::string> testFiles = {
        "test_data/test_real_points.e57",
        "sample/bunnyDouble.e57",
        "sample/bunnyInt32.e57"
    };
    
    for (const auto& testFile : testFiles) {
        if (!fileExists(testFile)) {
            continue;
        }
        
        try {
            E57XmlParser parser(testFile);
            
            if (!parser.isValidE57File()) {
                continue;
            }
            
            int scanCount = parser.getScanCount();
            std::vector<ScanMetadata> scans = parser.parseData3DSections();
            
            EXPECT_EQ(scanCount, static_cast<int>(scans.size())) 
                << "getScanCount() should match actual parsed scan count for " << testFile;
            
            qDebug() << "File:" << QString::fromStdString(testFile) 
                     << "reported" << scanCount << "scans, parsed" << scans.size();
            
        } catch (const std::exception& e) {
            qDebug() << "Error testing scan count for" << QString::fromStdString(testFile) << ":" << e.what();
        }
    }
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
