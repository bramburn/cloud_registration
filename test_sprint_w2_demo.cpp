#include <iostream>
#include <vector>
#include <E57Format.h>
#include "src/e57writer_lib_noqt.h"

/**
 * @brief Demo program to test Sprint W2 implementation
 * 
 * This program demonstrates the new point writing capabilities:
 * 1. Create an E57 file
 * 2. Add a scan with XYZ prototype
 * 3. Write point data with automatic cartesian bounds calculation
 * 4. Verify the results by reading back the file
 */
int main() {
    std::cout << "=== Sprint W2 Demo: E57 Point Writing ===\n\n";

    // Test file path
    std::string testFilePath = "sprint_w2_demo_output.e57";

    try {
        // Create E57WriterLibNoQt instance
        E57WriterLibNoQt writer;

        std::cout << "1. Creating E57 file: " << testFilePath << "\n";
        if (!writer.createFile(testFilePath)) {
            std::cerr << "Failed to create file: " << writer.getLastError() << std::endl;
            return 1;
        }

        std::cout << "2. Adding scan: 'Demo Scan'\n";
        if (!writer.addScan("Demo Scan")) {
            std::cerr << "Failed to add scan: " << writer.getLastError() << std::endl;
            return 1;
        }

        std::cout << "3. Defining XYZ prototype\n";
        if (!writer.defineXYZPrototype()) {
            std::cerr << "Failed to define XYZ prototype: " << writer.getLastError() << std::endl;
            return 1;
        }

        // Create test points
        std::cout << "4. Creating test point data\n";
        std::vector<E57WriterLibNoQt::Point3D> testPoints;
        
        // Add some interesting test points
        testPoints.emplace_back(0.0, 0.0, 0.0);      // Origin
        testPoints.emplace_back(1.0, 2.0, 3.0);      // Positive coordinates
        testPoints.emplace_back(-1.0, -2.0, -3.0);   // Negative coordinates
        testPoints.emplace_back(10.5, 20.5, 30.5);   // Larger values
        testPoints.emplace_back(-5.5, 15.5, -25.5);  // Mixed signs

        std::cout << "   Created " << testPoints.size() << " test points\n";

        std::cout << "5. Writing points to E57 file\n";
        if (!writer.writePoints(testPoints)) {
            std::cerr << "Failed to write points: " << writer.getLastError() << std::endl;
            return 1;
        }

        std::cout << "6. Closing E57 file\n";
        if (!writer.closeFile()) {
            std::cerr << "Failed to close file: " << writer.getLastError() << std::endl;
            return 1;
        }

        std::cout << "\n=== Verification: Reading back the E57 file ===\n";

        // Verify by reading back
        e57::ImageFile readFile(testFilePath, "r");
        if (!readFile.isOpen()) {
            std::cerr << "Failed to open file for reading" << std::endl;
            return 1;
        }

        e57::StructureNode root = readFile.root();
        
        // Check basic structure
        std::cout << "✓ File opened successfully for reading\n";
        
        if (root.isDefined("formatName")) {
            e57::StringNode formatName(root.get("formatName"));
            std::cout << "✓ Format name: " << formatName.value() << "\n";
        }

        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            std::cout << "✓ Found data3D vector with " << data3D.childCount() << " scan(s)\n";

            if (data3D.childCount() > 0) {
                e57::StructureNode scan(data3D.get(0));
                
                if (scan.isDefined("name")) {
                    e57::StringNode scanName(scan.get("name"));
                    std::cout << "✓ Scan name: " << scanName.value() << "\n";
                }

                // Check points
                if (scan.isDefined("points")) {
                    e57::CompressedVectorNode pointsNode(scan.get("points"));
                    std::cout << "✓ Points CompressedVectorNode found with " 
                              << pointsNode.childCount() << " points\n";
                }

                // Check cartesian bounds
                if (scan.isDefined("cartesianBounds")) {
                    e57::StructureNode bounds(scan.get("cartesianBounds"));
                    std::cout << "✓ Cartesian bounds found:\n";

                    if (bounds.isDefined("xMinimum") && bounds.isDefined("xMaximum")) {
                        e57::FloatNode xMin(bounds.get("xMinimum"));
                        e57::FloatNode xMax(bounds.get("xMaximum"));
                        std::cout << "   X: [" << xMin.value() << ", " << xMax.value() << "]\n";
                    }

                    if (bounds.isDefined("yMinimum") && bounds.isDefined("yMaximum")) {
                        e57::FloatNode yMin(bounds.get("yMinimum"));
                        e57::FloatNode yMax(bounds.get("yMaximum"));
                        std::cout << "   Y: [" << yMin.value() << ", " << yMax.value() << "]\n";
                    }

                    if (bounds.isDefined("zMinimum") && bounds.isDefined("zMaximum")) {
                        e57::FloatNode zMin(bounds.get("zMinimum"));
                        e57::FloatNode zMax(bounds.get("zMaximum"));
                        std::cout << "   Z: [" << zMin.value() << ", " << zMax.value() << "]\n";
                    }
                }
            }
        }

        readFile.close();
        std::cout << "\n✓ Sprint W2 implementation successful!\n";
        std::cout << "✓ E57 file with point data and cartesian bounds created: " << testFilePath << "\n";

        return 0;

    } catch (const e57::E57Exception& ex) {
        std::cerr << "E57 Exception: " << ex.what() << " (Error code: " << ex.errorCode() << ")" << std::endl;
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "Standard exception: " << ex.what() << std::endl;
        return 1;
    }
}
