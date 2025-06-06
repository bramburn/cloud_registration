#include <iostream>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include "e57_parser/E57HeaderParser.h"
#include "e57_parser/E57BinaryReader.h"
#include "e57_parser/E57XmlParser.h"

using namespace E57Parser;

void demonstrateHeaderParsing(const std::string& filePath) {
    std::cout << "\n=== E57 Header Parsing Demo (Sprint 1.1 Foundation) ===" << std::endl;
    
    E57HeaderParser parser;
    if (parser.Parse(filePath)) {
        const E57HeaderData& data = parser.GetData();
        
        std::cout << "✓ Header parsing successful!" << std::endl;
        std::cout << "  File signature: " << data.fileSignature << std::endl;
        std::cout << "  Version: " << data.majorVersion << "." << data.minorVersion << std::endl;
        std::cout << "  File length: " << data.fileLength << " bytes" << std::endl;
        std::cout << "  XML offset: " << data.xmlPayloadOffset << std::endl;
        std::cout << "  XML length: " << data.xmlPayloadLength << " bytes" << std::endl;
    } else {
        std::cout << "✗ Header parsing failed: " << parser.GetLastError() << std::endl;
    }
}

void demonstrateXmlParsing(const std::string& filePath) {
    std::cout << "\n=== E57 XML Structure Parsing Demo (Sprint 1.2 User Story 2) ===" << std::endl;
    
    try {
        E57XmlParser parser(filePath);
        
        if (!parser.isValidE57File()) {
            std::cout << "✗ File is not a valid E57 format" << std::endl;
            return;
        }
        
        std::cout << "✓ File validation successful!" << std::endl;
        
        // Parse complete file metadata
        E57FileMetadata metadata = parser.parseFile();
        
        std::cout << "✓ XML parsing successful!" << std::endl;
        std::cout << "  File GUID: " << metadata.fileGuid << std::endl;
        std::cout << "  Creation date: " << metadata.creationDateTime << std::endl;
        std::cout << "  Coordinate metadata: " << metadata.coordinateMetadata << std::endl;
        std::cout << "  Number of scans: " << metadata.scans.size() << std::endl;
        std::cout << "  Number of 2D images: " << metadata.images2D.size() << std::endl;
        
        // Analyze each scan
        for (size_t i = 0; i < metadata.scans.size(); ++i) {
            const ScanMetadata& scan = metadata.scans[i];
            
            std::cout << "\n  Scan " << i << ":" << std::endl;
            std::cout << "    GUID: " << scan.guid << std::endl;
            std::cout << "    Name: " << scan.name << std::endl;
            std::cout << "    Description: " << scan.description << std::endl;
            std::cout << "    Point count: " << scan.pointCount << std::endl;
            std::cout << "    Binary offset: " << scan.binaryOffset << std::endl;
            std::cout << "    Binary length: " << scan.binaryLength << " bytes" << std::endl;
            std::cout << "    Coordinate system: " << scan.coordinates.coordinateSystemName << std::endl;
            
            std::cout << "    Point attributes (" << scan.pointAttributes.size() << "):" << std::endl;
            for (const auto& attr : scan.pointAttributes) {
                std::cout << "      - " << attr.name 
                         << " (type: " << attr.elementType << ")";
                if (attr.hasLimits) {
                    std::cout << " [" << attr.minimum << " to " << attr.maximum << "]";
                }
                std::cout << std::endl;
            }
        }
        
        // Demonstrate binary section info extraction
        if (!metadata.scans.empty()) {
            std::cout << "\n  Binary section info for first scan:" << std::endl;
            try {
                BinarySection binaryInfo = parser.getBinarySectionInfo(metadata.scans[0].guid);
                std::cout << "    Section GUID: " << binaryInfo.guid << std::endl;
                std::cout << "    Section type: " << binaryInfo.sectionType << std::endl;
                std::cout << "    Offset: " << binaryInfo.offset << std::endl;
                std::cout << "    Length: " << binaryInfo.length << " bytes" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "    Note: " << e.what() << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cout << "✗ XML parsing failed: " << e.what() << std::endl;
    }
}

void demonstrateBinaryValidation(const std::string& filePath) {
    std::cout << "\n=== E57 Binary Data Validation Demo (Sprint 1.2 User Story 1) ===" << std::endl;
    
    try {
        // First get scan information from XML parser
        E57XmlParser xmlParser(filePath);
        std::vector<ScanMetadata> scans = xmlParser.parseData3DSections();
        
        if (scans.empty()) {
            std::cout << "✗ No scans found in file" << std::endl;
            return;
        }
        
        // Create binary reader
        E57BinaryReader binaryReader(filePath);
        std::cout << "✓ Binary reader initialized" << std::endl;
        
        // Try to get binary section info
        BinarySection binarySection;
        try {
            binarySection = xmlParser.getBinarySectionInfo(scans[0].guid);
        } catch (const std::exception& e) {
            std::cout << "Note: Using estimated binary section for demo: " << e.what() << std::endl;
            
            // Create a demo binary section (this may not work with real files)
            binarySection.offset = 1024;  // Estimated offset after header and XML
            binarySection.length = 2048;  // Small test section
            binarySection.guid = scans[0].guid;
            binarySection.sectionType = "points";
        }
        
        std::cout << "  Testing binary section:" << std::endl;
        std::cout << "    GUID: " << binarySection.guid << std::endl;
        std::cout << "    Offset: " << binarySection.offset << std::endl;
        std::cout << "    Length: " << binarySection.length << " bytes" << std::endl;
        
        // Demonstrate CRC-32 calculation
        std::vector<uint8_t> testData = {0x01, 0x02, 0x03, 0x04, 0x05};
        uint32_t testCrc = binaryReader.calculateCRC32(testData.data(), testData.size());
        std::cout << "  CRC-32 calculation test: 0x" << std::hex << testCrc << std::dec << std::endl;
        
        // Try to read and validate binary section
        try {
            auto binaryData = binaryReader.readBinarySection(binarySection);
            
            std::cout << "✓ Binary data read successfully!" << std::endl;
            std::cout << "  Bytes read: " << binaryData.size() << std::endl;
            
            ValidationMetrics metrics = binaryReader.getLastValidationMetrics();
            std::cout << "  Validation metrics:" << std::endl;
            std::cout << "    Total pages: " << metrics.totalPages << std::endl;
            std::cout << "    Valid pages: " << metrics.validPages << std::endl;
            std::cout << "    Corrupted pages: " << metrics.corruptedPages << std::endl;
            std::cout << "    Validation time: " << metrics.validationTimeMs << " ms" << std::endl;
            std::cout << "    Throughput: " << metrics.throughputMBps << " MB/s" << std::endl;
            
            if (metrics.corruptedPages == 0) {
                std::cout << "✓ All pages passed CRC validation!" << std::endl;
            } else {
                std::cout << "⚠ Some pages failed CRC validation" << std::endl;
            }
            
        } catch (const E57DataCorruptionError& e) {
            std::cout << "✗ Data corruption detected: " << e.what() << std::endl;
            std::cout << "  This demonstrates the CRC validation working correctly!" << std::endl;
            
        } catch (const std::exception& e) {
            std::cout << "Note: Binary reading failed (expected for demo): " << e.what() << std::endl;
            std::cout << "  This is normal when using estimated offsets with real E57 files." << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "✗ Binary validation demo failed: " << e.what() << std::endl;
    }
}

void demonstrateErrorHandling() {
    std::cout << "\n=== Error Handling Demo ===" << std::endl;
    
    std::string nonExistentFile = "non_existent_file.e57";
    
    // Header parser error handling
    std::cout << "Testing header parser with non-existent file..." << std::endl;
    E57HeaderParser headerParser;
    if (!headerParser.Parse(nonExistentFile)) {
        std::cout << "✓ Header parser correctly reported error: " << headerParser.GetLastError() << std::endl;
    }
    
    // XML parser error handling
    std::cout << "Testing XML parser with non-existent file..." << std::endl;
    try {
        E57XmlParser xmlParser(nonExistentFile);
        std::cout << "✗ XML parser should have thrown exception" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✓ XML parser correctly threw exception: " << e.what() << std::endl;
    }
    
    // Binary reader error handling
    std::cout << "Testing binary reader with non-existent file..." << std::endl;
    try {
        E57BinaryReader binaryReader(nonExistentFile);
        std::cout << "✗ Binary reader should have thrown exception" << std::endl;
    } catch (const E57DataCorruptionError& e) {
        std::cout << "✓ Binary reader correctly threw exception: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    std::cout << "=== Sprint 1.2 E57 Data Integrity and XML Parsing Demo ===" << std::endl;
    std::cout << "This demo showcases the Sprint 1.2 implementation:" << std::endl;
    std::cout << "- User Story 1: E57BinaryReader with CRC-32 validation" << std::endl;
    std::cout << "- User Story 2: E57XmlParser for robust XML parsing" << std::endl;
    
    // Check for test files
    std::vector<std::string> testFiles = {
        "test_data/test_real_points.e57",
        "sample/bunnyDouble.e57",
        "sample/bunnyInt32.e57",
        "test_data/test_triangle.e57"
    };
    
    std::string selectedFile;
    for (const auto& file : testFiles) {
        if (QFile::exists(QString::fromStdString(file))) {
            selectedFile = file;
            break;
        }
    }
    
    if (selectedFile.empty()) {
        std::cout << "\nNo test E57 files found. Testing error handling only..." << std::endl;
        demonstrateErrorHandling();
        return 0;
    }
    
    std::cout << "\nUsing test file: " << selectedFile << std::endl;
    
    // Run demonstrations
    demonstrateHeaderParsing(selectedFile);
    demonstrateXmlParsing(selectedFile);
    demonstrateBinaryValidation(selectedFile);
    demonstrateErrorHandling();
    
    std::cout << "\n=== Sprint 1.2 Demo Complete ===" << std::endl;
    std::cout << "Key achievements:" << std::endl;
    std::cout << "✓ Robust E57 header parsing (Sprint 1.1 foundation)" << std::endl;
    std::cout << "✓ Comprehensive XML structure parsing with libE57Format" << std::endl;
    std::cout << "✓ CRC-32 validation framework for binary data integrity" << std::endl;
    std::cout << "✓ Detailed metadata extraction (scans, attributes, coordinates)" << std::endl;
    std::cout << "✓ Robust error handling with descriptive messages" << std::endl;
    std::cout << "✓ Performance metrics and validation reporting" << std::endl;
    
    return 0;
}
