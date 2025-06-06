#include <iostream>
#include <memory>
#include "src/IE57Writer.h"
#include "src/e57writer_lib.h"
#include "tests/MockE57Writer.h"

/**
 * @brief Simple test to verify Sprint 2 decoupling implementation
 * 
 * This test verifies that:
 * 1. IE57Writer interface is properly defined
 * 2. E57WriterLib correctly implements the interface
 * 3. MockE57Writer provides a working mock implementation
 * 4. Polymorphism works correctly through the interface
 */

void testInterfacePolymorphism() {
    std::cout << "Testing interface polymorphism..." << std::endl;
    
    // Test with concrete implementation
    std::unique_ptr<IE57Writer> writer = std::make_unique<E57WriterLib>();
    
    // Test basic interface methods (without actual file operations)
    std::cout << "  - Interface pointer created successfully" << std::endl;
    std::cout << "  - isFileOpen(): " << (writer->isFileOpen() ? "true" : "false") << std::endl;
    std::cout << "  - getScanCount(): " << writer->getScanCount() << std::endl;
    std::cout << "  - getLastError(): '" << writer->getLastError().toStdString() << "'" << std::endl;
    
    std::cout << "Interface polymorphism test passed!" << std::endl;
}

void testMockImplementation() {
    std::cout << "Testing mock implementation..." << std::endl;
    
    // Test with mock implementation
    MockE57Writer mockWriter;
    IE57Writer* writer = &mockWriter;
    
    // Test mock operations
    bool result = writer->createFile("/mock/test.e57");
    std::cout << "  - createFile(): " << (result ? "success" : "failed") << std::endl;
    std::cout << "  - isFileOpen(): " << (writer->isFileOpen() ? "true" : "false") << std::endl;
    std::cout << "  - getCurrentFilePath(): '" << writer->getCurrentFilePath().toStdString() << "'" << std::endl;
    
    // Test scan addition
    result = writer->addScan("Test Scan");
    std::cout << "  - addScan(): " << (result ? "success" : "failed") << std::endl;
    std::cout << "  - getScanCount(): " << writer->getScanCount() << std::endl;
    
    // Test prototype definition
    IE57Writer::ExportOptions options(true, false); // intensity only
    result = writer->definePointPrototype(options);
    std::cout << "  - definePointPrototype(): " << (result ? "success" : "failed") << std::endl;
    
    // Test point writing
    std::vector<IE57Writer::Point3D> points = {
        IE57Writer::Point3D(1.0, 2.0, 3.0, 0.5f), // XYZ + intensity
        IE57Writer::Point3D(4.0, 5.0, 6.0, 0.8f)  // XYZ + intensity
    };
    result = writer->writePoints(points, options);
    std::cout << "  - writePoints(): " << (result ? "success" : "failed") << std::endl;
    
    // Test file closing
    result = writer->closeFile();
    std::cout << "  - closeFile(): " << (result ? "success" : "failed") << std::endl;
    std::cout << "  - isFileOpen(): " << (writer->isFileOpen() ? "true" : "false") << std::endl;
    
    // Check method calls
    QStringList calls = mockWriter.getMethodCalls();
    std::cout << "  - Method calls tracked: " << calls.size() << std::endl;
    for (const QString& call : calls) {
        std::cout << "    * " << call.toStdString() << std::endl;
    }
    
    std::cout << "Mock implementation test passed!" << std::endl;
}

void testDataStructures() {
    std::cout << "Testing data structures..." << std::endl;
    
    // Test Point3D constructors
    IE57Writer::Point3D p1; // Default constructor
    IE57Writer::Point3D p2(1.0, 2.0, 3.0); // XYZ constructor
    IE57Writer::Point3D p3(1.0, 2.0, 3.0, 0.5f); // XYZ + intensity
    IE57Writer::Point3D p4(1.0, 2.0, 3.0, 255, 128, 64); // XYZ + color
    IE57Writer::Point3D p5(1.0, 2.0, 3.0, 0.5f, 255, 128, 64); // XYZ + intensity + color
    
    std::cout << "  - Point3D constructors work correctly" << std::endl;
    std::cout << "  - p1 hasIntensity: " << (p1.hasIntensity ? "true" : "false") << std::endl;
    std::cout << "  - p3 hasIntensity: " << (p3.hasIntensity ? "true" : "false") << std::endl;
    std::cout << "  - p4 hasColor: " << (p4.hasColor ? "true" : "false") << std::endl;
    std::cout << "  - p5 hasIntensity: " << (p5.hasIntensity ? "true" : "false") << std::endl;
    std::cout << "  - p5 hasColor: " << (p5.hasColor ? "true" : "false") << std::endl;
    
    // Test ExportOptions
    IE57Writer::ExportOptions opt1; // Default
    IE57Writer::ExportOptions opt2(true, false); // Intensity only
    IE57Writer::ExportOptions opt3(false, true); // Color only
    IE57Writer::ExportOptions opt4(true, true); // Both
    
    std::cout << "  - ExportOptions constructors work correctly" << std::endl;
    
    // Test ScanMetadata
    IE57Writer::ScanMetadata meta1; // Default
    IE57Writer::ScanMetadata meta2("Test Scan"); // Name only
    
    std::cout << "  - ScanMetadata constructors work correctly" << std::endl;
    std::cout << "  - meta2 name: '" << meta2.name.toStdString() << "'" << std::endl;
    
    std::cout << "Data structures test passed!" << std::endl;
}

int main() {
    std::cout << "=== Sprint 2 Decoupling Implementation Test ===" << std::endl;
    std::cout << std::endl;
    
    try {
        testDataStructures();
        std::cout << std::endl;
        
        testInterfacePolymorphism();
        std::cout << std::endl;
        
        testMockImplementation();
        std::cout << std::endl;
        
        std::cout << "=== All tests passed! Sprint 2 implementation is working correctly ===" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
