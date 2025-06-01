// Sprint 3 Demonstration Program
// Shows how to use the enhanced E57ParserLib with intensity and color data extraction

#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include <iomanip>
#include "src/e57parserlib.h"

void printPointSample(const std::vector<E57ParserLib::PointData>& points, int maxSamples = 10) {
    std::cout << "\n=== Point Data Sample ===\n";
    std::cout << std::fixed << std::setprecision(3);
    
    int sampleCount = std::min(static_cast<int>(points.size()), maxSamples);
    
    for (int i = 0; i < sampleCount; ++i) {
        const auto& point = points[i];
        
        std::cout << "Point " << i << ": ";
        std::cout << "XYZ(" << point.x << ", " << point.y << ", " << point.z << ")";
        
        if (point.hasIntensity) {
            std::cout << " Intensity(" << point.intensity << ")";
        }
        
        if (point.hasColor) {
            std::cout << " RGB(" << static_cast<int>(point.r) << ", " 
                      << static_cast<int>(point.g) << ", " 
                      << static_cast<int>(point.b) << ")";
        }
        
        std::cout << "\n";
    }
    
    if (points.size() > maxSamples) {
        std::cout << "... and " << (points.size() - maxSamples) << " more points\n";
    }
}

void analyzePointData(const std::vector<E57ParserLib::PointData>& points) {
    if (points.empty()) {
        std::cout << "No points to analyze.\n";
        return;
    }
    
    std::cout << "\n=== Point Data Analysis ===\n";
    std::cout << "Total points: " << points.size() << "\n";
    
    // Count points with different attributes
    int pointsWithIntensity = 0;
    int pointsWithColor = 0;
    float minIntensity = 1.0f, maxIntensity = 0.0f;
    
    for (const auto& point : points) {
        if (point.hasIntensity) {
            pointsWithIntensity++;
            minIntensity = std::min(minIntensity, point.intensity);
            maxIntensity = std::max(maxIntensity, point.intensity);
        }
        if (point.hasColor) {
            pointsWithColor++;
        }
    }
    
    std::cout << "Points with intensity: " << pointsWithIntensity 
              << " (" << (100.0 * pointsWithIntensity / points.size()) << "%)\n";
    
    if (pointsWithIntensity > 0) {
        std::cout << "Intensity range: " << minIntensity << " to " << maxIntensity << "\n";
    }
    
    std::cout << "Points with color: " << pointsWithColor 
              << " (" << (100.0 * pointsWithColor / points.size()) << "%)\n";
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    std::cout << "=== Sprint 3 E57ParserLib Demonstration ===\n";
    std::cout << "Enhanced E57 parsing with intensity and color support\n\n";
    
    // Check command line arguments
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <e57_file_path> [scan_index]\n";
        std::cout << "Example: " << argv[0] << " sample.e57 0\n\n";
        
        std::cout << "This demo shows the new Sprint 3 functionality:\n";
        std::cout << "- Enhanced point data extraction with intensity and color\n";
        std::cout << "- Automatic detection of available attributes in E57 prototype\n";
        std::cout << "- Proper normalization of intensity (0.0-1.0) and color (0-255) values\n";
        std::cout << "- Robust handling of CompressedVectorNode with multiple attributes\n\n";
        
        // Demonstrate the API without a file
        std::cout << "=== API Demonstration (without file) ===\n";
        E57ParserLib parser;
        
        std::cout << "Parser created. File open: " << (parser.isOpen() ? "Yes" : "No") << "\n";
        
        // Try to extract enhanced point data (will fail gracefully)
        auto points = parser.extractEnhancedPointData(0);
        std::cout << "Enhanced point extraction result: " << points.size() << " points\n";
        std::cout << "Last error: " << parser.getLastError() << "\n\n";
        
        // Show PointData structure capabilities
        std::cout << "=== PointData Structure Demo ===\n";
        E57ParserLib::PointData point1(1.5f, 2.3f, 4.7f);
        point1.intensity = 0.75f;
        point1.hasIntensity = true;
        point1.r = 255;
        point1.g = 128;
        point1.b = 64;
        point1.hasColor = true;
        
        std::cout << "Sample point with all attributes:\n";
        std::cout << "  XYZ: (" << point1.x << ", " << point1.y << ", " << point1.z << ")\n";
        std::cout << "  Intensity: " << point1.intensity << " (normalized 0.0-1.0)\n";
        std::cout << "  Color RGB: (" << static_cast<int>(point1.r) << ", " 
                  << static_cast<int>(point1.g) << ", " << static_cast<int>(point1.b) << ")\n";
        std::cout << "  Has intensity: " << (point1.hasIntensity ? "Yes" : "No") << "\n";
        std::cout << "  Has color: " << (point1.hasColor ? "Yes" : "No") << "\n";
        
        return 0;
    }
    
    std::string filePath = argv[1];
    int scanIndex = (argc >= 3) ? std::atoi(argv[2]) : 0;
    
    std::cout << "Opening E57 file: " << filePath << "\n";
    std::cout << "Target scan index: " << scanIndex << "\n\n";
    
    // Create parser and connect to progress signals
    E57ParserLib parser;
    
    QObject::connect(&parser, &E57ParserLib::progressUpdated,
                     [](int percentage, const QString& stage) {
                         std::cout << "Progress: " << percentage << "% - " 
                                   << stage.toStdString() << "\n";
                     });
    
    QObject::connect(&parser, &E57ParserLib::parsingFinished,
                     [](bool success, const QString& message, const std::vector<float>& /*points*/) {
                         std::cout << "Parsing finished: " << (success ? "SUCCESS" : "FAILED") 
                                   << " - " << message.toStdString() << "\n";
                     });
    
    // Open the E57 file
    if (!parser.openFile(filePath)) {
        std::cout << "ERROR: Failed to open E57 file: " << parser.getLastError() << "\n";
        return 1;
    }
    
    std::cout << "File opened successfully!\n";
    std::cout << "File GUID: " << parser.getGuid() << "\n";
    std::cout << "Number of scans: " << parser.getScanCount() << "\n";
    
    if (scanIndex >= parser.getScanCount()) {
        std::cout << "ERROR: Scan index " << scanIndex << " is out of range (0-" 
                  << (parser.getScanCount() - 1) << ")\n";
        return 1;
    }
    
    std::cout << "Points in scan " << scanIndex << ": " << parser.getPointCount(scanIndex) << "\n\n";
    
    // Extract enhanced point data using Sprint 3 functionality
    std::cout << "=== Extracting Enhanced Point Data (Sprint 3) ===\n";
    auto enhancedPoints = parser.extractEnhancedPointData(scanIndex);
    
    if (enhancedPoints.empty()) {
        std::cout << "No enhanced points extracted. Error: " << parser.getLastError() << "\n";
        
        // Fall back to legacy extraction
        std::cout << "\n=== Falling back to Legacy Point Data (Sprint 2) ===\n";
        auto legacyPoints = parser.extractPointData(scanIndex);
        
        if (!legacyPoints.empty()) {
            std::cout << "Legacy extraction successful: " << (legacyPoints.size() / 3) << " points\n";
            std::cout << "First few coordinates: ";
            for (int i = 0; i < std::min(9, static_cast<int>(legacyPoints.size())); i += 3) {
                std::cout << "(" << legacyPoints[i] << ", " << legacyPoints[i+1] 
                          << ", " << legacyPoints[i+2] << ") ";
            }
            std::cout << "\n";
        } else {
            std::cout << "Legacy extraction also failed: " << parser.getLastError() << "\n";
        }
    } else {
        std::cout << "Enhanced extraction successful!\n";
        analyzePointData(enhancedPoints);
        printPointSample(enhancedPoints, 5);
        
        // Compare with legacy extraction
        std::cout << "\n=== Comparison with Legacy Extraction ===\n";
        auto legacyPoints = parser.extractPointData(scanIndex);
        std::cout << "Legacy points: " << (legacyPoints.size() / 3) << "\n";
        std::cout << "Enhanced points: " << enhancedPoints.size() << "\n";
        
        if (!legacyPoints.empty() && !enhancedPoints.empty()) {
            std::cout << "Coordinate comparison (first point):\n";
            std::cout << "  Legacy: (" << legacyPoints[0] << ", " << legacyPoints[1] 
                      << ", " << legacyPoints[2] << ")\n";
            std::cout << "  Enhanced: (" << enhancedPoints[0].x << ", " << enhancedPoints[0].y 
                      << ", " << enhancedPoints[0].z << ")\n";
        }
    }
    
    parser.closeFile();
    std::cout << "\nFile closed. Demo complete.\n";
    
    return 0;
}
