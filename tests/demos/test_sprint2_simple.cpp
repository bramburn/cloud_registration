#include <iostream>
#include <QCoreApplication>
#include "../../src/e57parserlib.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    std::cout << "=== Sprint 2 E57ParserLib Test ===" << std::endl;
    
    // Test 1: Basic instantiation
    std::cout << "\n--- Test 1: Basic Instantiation ---" << std::endl;
    E57ParserLib parser;
    std::cout << "✓ E57ParserLib created successfully" << std::endl;
    
    // Test 2: File opening
    std::cout << "\n--- Test 2: File Opening ---" << std::endl;
    std::string testFile = "sample/bunnyDouble.e57";
    
    if (parser.openFile(testFile)) {
        std::cout << "✓ File opened successfully: " << testFile << std::endl;
        
        // Test 3: Basic metadata
        std::cout << "\n--- Test 3: Basic Metadata ---" << std::endl;
        std::cout << "GUID: " << parser.getGuid() << std::endl;
        auto version = parser.getVersion();
        std::cout << "Version: " << version.first << "." << version.second << std::endl;
        std::cout << "Scan count: " << parser.getScanCount() << std::endl;
        
        // Test 4: Point count
        std::cout << "\n--- Test 4: Point Count ---" << std::endl;
        int64_t pointCount = parser.getPointCount(0);
        std::cout << "Point count in scan 0: " << pointCount << std::endl;
        
        if (pointCount > 0) {
            // Test 5: Point data extraction
            std::cout << "\n--- Test 5: Point Data Extraction ---" << std::endl;
            
            // Connect to signals to monitor progress
            QObject::connect(&parser, &E57ParserLib::progressUpdated, 
                [](int percentage, const QString& stage) {
                    std::cout << "Progress: " << percentage << "% - " << stage.toStdString() << std::endl;
                });
            
            QObject::connect(&parser, &E57ParserLib::parsingFinished,
                [](bool success, const QString& message, const std::vector<float>& points) {
                    std::cout << "Parsing finished: " << (success ? "SUCCESS" : "FAILED") << std::endl;
                    std::cout << "Message: " << message.toStdString() << std::endl;
                    std::cout << "Points extracted: " << points.size() / 3 << " points (" << points.size() << " coordinates)" << std::endl;
                    
                    if (!points.empty()) {
                        std::cout << "First point: (" << points[0] << ", " << points[1] << ", " << points[2] << ")" << std::endl;
                        if (points.size() >= 6) {
                            std::cout << "Second point: (" << points[3] << ", " << points[4] << ", " << points[5] << ")" << std::endl;
                        }
                    }
                });
            
            std::vector<float> points = parser.extractPointData();
            
            if (!points.empty()) {
                std::cout << "✓ Point extraction successful!" << std::endl;
                std::cout << "Total coordinates: " << points.size() << std::endl;
                std::cout << "Total points: " << points.size() / 3 << std::endl;
            } else {
                std::cout << "✗ Point extraction failed" << std::endl;
                std::cout << "Error: " << parser.getLastError() << std::endl;
            }
        } else {
            std::cout << "⚠ No points found in scan 0, skipping extraction test" << std::endl;
        }
        
        parser.closeFile();
        std::cout << "\n✓ File closed successfully" << std::endl;
        
    } else {
        std::cout << "✗ Failed to open file: " << testFile << std::endl;
        std::cout << "Error: " << parser.getLastError() << std::endl;
        
        // Try with alternative file
        std::string altFile = "sample/bunnyInt32.e57";
        std::cout << "\nTrying alternative file: " << altFile << std::endl;
        
        if (parser.openFile(altFile)) {
            std::cout << "✓ Alternative file opened successfully" << std::endl;
            std::cout << "Scan count: " << parser.getScanCount() << std::endl;
            parser.closeFile();
        } else {
            std::cout << "✗ Alternative file also failed" << std::endl;
            std::cout << "Error: " << parser.getLastError() << std::endl;
        }
    }
    
    std::cout << "\n=== Sprint 2 Test Complete ===" << std::endl;
    return 0;
}
