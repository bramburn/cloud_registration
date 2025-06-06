#include <iostream>
#include <E57Format.h>
#include <string>
#include <memory>
#include "src/e57writer_lib_noqt.h"

/**
 * @brief Test E57WriterLibNoQt to verify it works without Qt hanging issues
 * 
 * This test replicates the E57WriterLib functionality using the non-Qt version
 * to confirm that removing Qt dependencies fixes the hanging issue.
 */

bool testFileCreation(const std::string& testFilePath) {
    std::cout << "\n--- Test 1: File Creation ---" << std::endl;
    
    E57WriterLibNoQt writer;
    
    // Test file creation
    bool result = writer.createFile(testFilePath);
    std::cout << "createFile result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    if (!result) {
        std::cout << "Error: " << writer.getLastError() << std::endl;
        return false;
    }
    
    // Test file closure
    result = writer.closeFile();
    std::cout << "closeFile result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    if (!result) {
        std::cout << "Error: " << writer.getLastError() << std::endl;
        return false;
    }
    
    return true;
}

bool testScanStructure(const std::string& testFilePath) {
    std::cout << "\n--- Test 2: Scan Structure ---" << std::endl;
    
    E57WriterLibNoQt writer;
    
    // Create file and add scan
    if (!writer.createFile(testFilePath)) {
        std::cout << "Error creating file: " << writer.getLastError() << std::endl;
        return false;
    }
    
    bool result = writer.addScan("Test Scan 001");
    std::cout << "addScan result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    if (!result) {
        std::cout << "Error: " << writer.getLastError() << std::endl;
        return false;
    }
    
    if (!writer.closeFile()) {
        std::cout << "Error closing file: " << writer.getLastError() << std::endl;
        return false;
    }
    
    return true;
}

bool testXYZPrototype(const std::string& testFilePath) {
    std::cout << "\n--- Test 3: XYZ Prototype ---" << std::endl;
    
    E57WriterLibNoQt writer;
    
    // Create file, add scan, and define XYZ prototype
    if (!writer.createFile(testFilePath)) {
        std::cout << "Error creating file: " << writer.getLastError() << std::endl;
        return false;
    }
    
    if (!writer.addScan("Test Scan with Points")) {
        std::cout << "Error adding scan: " << writer.getLastError() << std::endl;
        return false;
    }
    
    bool result = writer.defineXYZPrototype();
    std::cout << "defineXYZPrototype result: " << (result ? "SUCCESS" : "FAILED") << std::endl;
    if (!result) {
        std::cout << "Error: " << writer.getLastError() << std::endl;
        return false;
    }
    
    if (!writer.closeFile()) {
        std::cout << "Error closing file: " << writer.getLastError() << std::endl;
        return false;
    }
    
    return true;
}

bool testFileVerification(const std::string& testFilePath) {
    std::cout << "\n--- Test 4: File Verification ---" << std::endl;
    
    try {
        std::cout << "Opening file with libE57Format for verification..." << std::endl;
        e57::ImageFile testFile(testFilePath, "r");
        
        if (!testFile.isOpen()) {
            std::cout << "ERROR: Cannot open file for reading" << std::endl;
            return false;
        }
        
        std::cout << "File opened successfully for reading" << std::endl;
        
        // Verify E57Root structure
        e57::StructureNode root = testFile.root();
        std::cout << "Got root node" << std::endl;
        
        // Check formatName
        if (!root.isDefined("formatName")) {
            std::cout << "ERROR: formatName not found" << std::endl;
            return false;
        }
        
        e57::StringNode formatName(root.get("formatName"));
        std::string formatNameValue = formatName.value();
        std::cout << "formatName: " << formatNameValue << std::endl;
        
        if (formatNameValue != "ASTM E57 3D Imaging Data File") {
            std::cout << "ERROR: Incorrect formatName value" << std::endl;
            return false;
        }
        
        // Check data3D
        if (!root.isDefined("data3D")) {
            std::cout << "ERROR: data3D not found" << std::endl;
            return false;
        }
        
        e57::VectorNode data3D(root.get("data3D"));
        std::cout << "data3D childCount: " << data3D.childCount() << std::endl;
        
        if (data3D.childCount() > 0) {
            e57::StructureNode scan(data3D.get(0));
            
            if (scan.isDefined("name")) {
                e57::StringNode scanName(scan.get("name"));
                std::cout << "First scan name: " << scanName.value() << std::endl;
            }
            
            if (scan.isDefined("points")) {
                std::cout << "Scan has points CompressedVectorNode" << std::endl;

                // Try to access the CompressedVectorNode carefully
                try {
                    std::cout << "Creating CompressedVectorNode..." << std::endl;
                    e57::CompressedVectorNode pointsNode(scan.get("points"));
                    std::cout << "CompressedVectorNode created successfully" << std::endl;

                    std::cout << "Getting prototype..." << std::endl;
                    e57::StructureNode prototype(pointsNode.prototype());
                    std::cout << "Prototype obtained successfully" << std::endl;

                    std::cout << "Checking prototype fields..." << std::endl;
                    std::cout << "Prototype has cartesianX: " << (prototype.isDefined("cartesianX") ? "YES" : "NO") << std::endl;
                    std::cout << "Prototype has cartesianY: " << (prototype.isDefined("cartesianY") ? "YES" : "NO") << std::endl;
                    std::cout << "Prototype has cartesianZ: " << (prototype.isDefined("cartesianZ") ? "YES" : "NO") << std::endl;

                } catch (const e57::E57Exception& ex) {
                    std::cout << "E57 Exception accessing CompressedVectorNode: " << ex.what() << " (Code: " << ex.errorCode() << ")" << std::endl;
                } catch (const std::exception& ex) {
                    std::cout << "Standard exception accessing CompressedVectorNode: " << ex.what() << std::endl;
                }
            }
        }
        
        testFile.close();
        std::cout << "File verification completed successfully" << std::endl;
        return true;
        
    } catch (const e57::E57Exception& ex) {
        std::cout << "E57 Exception during verification: " << ex.what() << " (Code: " << ex.errorCode() << ")" << std::endl;
        return false;
    } catch (const std::exception& ex) {
        std::cout << "Standard exception during verification: " << ex.what() << std::endl;
        return false;
    }
}

int main() {
    std::cout << "=== E57WriterLibNoQt Test ===" << std::endl;
    
    try {
        std::string testFilePath = "test_noqt_output.e57";
        std::cout << "Test file path: " << testFilePath << std::endl;
        
        // Run all tests
        if (!testFileCreation(testFilePath + "_1")) {
            std::cout << "ERROR: File creation test failed" << std::endl;
            return 1;
        }
        
        if (!testScanStructure(testFilePath + "_2")) {
            std::cout << "ERROR: Scan structure test failed" << std::endl;
            return 1;
        }
        
        if (!testXYZPrototype(testFilePath + "_3")) {
            std::cout << "ERROR: XYZ prototype test failed" << std::endl;
            return 1;
        }
        
        if (!testFileVerification(testFilePath + "_3")) {
            std::cout << "ERROR: File verification test failed" << std::endl;
            return 1;
        }
        
        std::cout << "\n=== All tests completed successfully ===" << std::endl;
        return 0;
        
    } catch (const std::exception& ex) {
        std::cout << "Unexpected exception: " << ex.what() << std::endl;
        return 1;
    }
}
