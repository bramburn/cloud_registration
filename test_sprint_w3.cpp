#include <iostream>
#include <vector>
#include "src/e57writer_lib.h"

int main() {
    std::cout << "Testing Sprint W3 Implementation..." << std::endl;
    
    E57WriterLib writer;
    QString testFile = "test_sprint_w3_output.e57";
    
    // Test 1: Create file with intensity and color prototype
    std::cout << "\n=== Test 1: Create file with intensity and color prototype ===" << std::endl;
    
    if (!writer.createFile(testFile)) {
        std::cout << "FAILED: Could not create file: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ File created successfully" << std::endl;
    
    if (!writer.addScan("Sprint W3 Test Scan")) {
        std::cout << "FAILED: Could not add scan: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ Scan added successfully" << std::endl;
    
    // Define prototype with both intensity and color
    E57WriterLib::ExportOptions fullOptions(true, true); // intensity=true, color=true
    if (!writer.definePointPrototype(fullOptions)) {
        std::cout << "FAILED: Could not define prototype: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ Prototype with intensity and color defined successfully" << std::endl;
    
    // Test 2: Write points with intensity and color data
    std::cout << "\n=== Test 2: Write points with intensity and color data ===" << std::endl;
    
    std::vector<E57WriterLib::Point3D> testPoints = {
        E57WriterLib::Point3D(1.0, 2.0, 3.0, 0.2f, 255, 128, 64),  // XYZ + intensity + color
        E57WriterLib::Point3D(4.0, 5.0, 6.0, 0.6f, 128, 255, 32),  // XYZ + intensity + color
        E57WriterLib::Point3D(7.0, 8.0, 9.0, 0.8f, 64, 32, 255)    // XYZ + intensity + color
    };
    
    if (!writer.writePoints(testPoints, fullOptions)) {
        std::cout << "FAILED: Could not write points: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ Points with intensity and color written successfully" << std::endl;
    
    if (!writer.closeFile()) {
        std::cout << "FAILED: Could not close file: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ File closed successfully" << std::endl;
    
    // Test 3: Create file with intensity only
    std::cout << "\n=== Test 3: Create file with intensity only ===" << std::endl;
    
    QString intensityFile = "test_intensity_only.e57";
    if (!writer.createFile(intensityFile)) {
        std::cout << "FAILED: Could not create intensity file: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    
    if (!writer.addScan("Intensity Only Scan")) {
        std::cout << "FAILED: Could not add intensity scan: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    
    E57WriterLib::ExportOptions intensityOptions(true, false); // intensity=true, color=false
    if (!writer.definePointPrototype(intensityOptions)) {
        std::cout << "FAILED: Could not define intensity prototype: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ Intensity-only prototype defined successfully" << std::endl;
    
    std::vector<E57WriterLib::Point3D> intensityPoints = {
        E57WriterLib::Point3D(1.0, 2.0, 3.0, 0.1f),  // XYZ + intensity
        E57WriterLib::Point3D(4.0, 5.0, 6.0, 0.5f),  // XYZ + intensity
        E57WriterLib::Point3D(7.0, 8.0, 9.0, 0.9f)   // XYZ + intensity
    };
    
    if (!writer.writePoints(intensityPoints, intensityOptions)) {
        std::cout << "FAILED: Could not write intensity points: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ Intensity-only points written successfully" << std::endl;
    
    if (!writer.closeFile()) {
        std::cout << "FAILED: Could not close intensity file: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ Intensity file closed successfully" << std::endl;
    
    // Test 4: Create file with color only
    std::cout << "\n=== Test 4: Create file with color only ===" << std::endl;
    
    QString colorFile = "test_color_only.e57";
    if (!writer.createFile(colorFile)) {
        std::cout << "FAILED: Could not create color file: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    
    if (!writer.addScan("Color Only Scan")) {
        std::cout << "FAILED: Could not add color scan: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    
    E57WriterLib::ExportOptions colorOptions(false, true); // intensity=false, color=true
    if (!writer.definePointPrototype(colorOptions)) {
        std::cout << "FAILED: Could not define color prototype: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ Color-only prototype defined successfully" << std::endl;
    
    std::vector<E57WriterLib::Point3D> colorPoints = {
        E57WriterLib::Point3D(1.0, 2.0, 3.0, 255, 0, 0),    // XYZ + red
        E57WriterLib::Point3D(4.0, 5.0, 6.0, 0, 255, 0),    // XYZ + green
        E57WriterLib::Point3D(7.0, 8.0, 9.0, 0, 0, 255)     // XYZ + blue
    };
    
    if (!writer.writePoints(colorPoints, colorOptions)) {
        std::cout << "FAILED: Could not write color points: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ Color-only points written successfully" << std::endl;
    
    if (!writer.closeFile()) {
        std::cout << "FAILED: Could not close color file: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ Color file closed successfully" << std::endl;
    
    // Test 5: Test backward compatibility with XYZ-only
    std::cout << "\n=== Test 5: Test backward compatibility with XYZ-only ===" << std::endl;
    
    QString xyzFile = "test_xyz_only.e57";
    if (!writer.createFile(xyzFile)) {
        std::cout << "FAILED: Could not create XYZ file: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    
    if (!writer.addScan("XYZ Only Scan")) {
        std::cout << "FAILED: Could not add XYZ scan: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    
    // Test legacy method
    if (!writer.defineXYZPrototype()) {
        std::cout << "FAILED: Could not define XYZ prototype: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ Legacy XYZ prototype defined successfully" << std::endl;
    
    std::vector<E57WriterLib::Point3D> xyzPoints = {
        E57WriterLib::Point3D(1.0, 2.0, 3.0),
        E57WriterLib::Point3D(4.0, 5.0, 6.0),
        E57WriterLib::Point3D(7.0, 8.0, 9.0)
    };
    
    E57WriterLib::ExportOptions xyzOptions(false, false); // XYZ only
    if (!writer.writePoints(xyzPoints, xyzOptions)) {
        std::cout << "FAILED: Could not write XYZ points: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ XYZ-only points written successfully" << std::endl;
    
    if (!writer.closeFile()) {
        std::cout << "FAILED: Could not close XYZ file: " << writer.getLastError().toStdString() << std::endl;
        return 1;
    }
    std::cout << "✓ XYZ file closed successfully" << std::endl;
    
    std::cout << "\n🎉 ALL SPRINT W3 TESTS PASSED! 🎉" << std::endl;
    std::cout << "\nGenerated test files:" << std::endl;
    std::cout << "- " << testFile.toStdString() << " (XYZ + Intensity + Color)" << std::endl;
    std::cout << "- " << intensityFile.toStdString() << " (XYZ + Intensity)" << std::endl;
    std::cout << "- " << colorFile.toStdString() << " (XYZ + Color)" << std::endl;
    std::cout << "- " << xyzFile.toStdString() << " (XYZ only)" << std::endl;
    
    return 0;
}
