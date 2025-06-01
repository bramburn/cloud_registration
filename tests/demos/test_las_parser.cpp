#include <iostream>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QDataStream>
#include <QtEndian>
#include "../../src/lasparser.h"

// Helper function to create a simple test LAS file
QString createTestLasFile()
{
    QTemporaryFile* tempFile = new QTemporaryFile();
    tempFile->setAutoRemove(false);
    
    if (!tempFile->open()) {
        return QString();
    }
    
    QDataStream stream(tempFile);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    // Write LAS header
    tempFile->write("LASF", 4);                    // File signature
    stream << static_cast<quint16>(0);             // File source ID
    stream << static_cast<quint16>(0);             // Global encoding
    
    // GUID (16 bytes) - write zeros
    for (int i = 0; i < 16; ++i) {
        stream << static_cast<quint8>(0);
    }
    
    stream << static_cast<quint8>(1);              // Version major
    stream << static_cast<quint8>(2);              // Version minor
    
    // System identifier and generating software (64 bytes total)
    for (int i = 0; i < 64; ++i) {
        stream << static_cast<quint8>(0);
    }
    
    stream << static_cast<quint16>(1);             // Creation day
    stream << static_cast<quint16>(2024);          // Creation year
    stream << static_cast<quint16>(227);           // Header size
    stream << static_cast<quint32>(227);           // Point data offset
    stream << static_cast<quint32>(0);             // Number of VLRs
    stream << static_cast<quint8>(0);              // Point data format (Format 0)
    stream << static_cast<quint16>(20);            // Point data record length
    stream << static_cast<quint32>(3);             // Number of point records
    
    // Number of points by return (5 * 4 bytes)
    for (int i = 0; i < 5; ++i) {
        stream << static_cast<quint32>(0);
    }
    
    // Scale factors
    stream << 0.01;  // X scale
    stream << 0.01;  // Y scale
    stream << 0.01;  // Z scale
    
    // Offsets
    stream << 0.0;   // X offset
    stream << 0.0;   // Y offset
    stream << 0.0;   // Z offset
    
    // Min/Max values
    stream << 2.0;   // Max X
    stream << 0.0;   // Min X
    stream << 2.0;   // Max Y
    stream << 0.0;   // Min Y
    stream << 2.0;   // Max Z
    stream << 0.0;   // Min Z
    
    // Write 3 test points
    // Point 1: (0, 0, 0) -> scaled: (0, 0, 0)
    stream << static_cast<qint32>(0);    // X
    stream << static_cast<qint32>(0);    // Y
    stream << static_cast<qint32>(0);    // Z
    for (int i = 0; i < 8; ++i) {        // Fill remaining 8 bytes
        stream << static_cast<quint8>(0);
    }
    
    // Point 2: (1.0, 1.0, 1.0) -> scaled: (100, 100, 100)
    stream << static_cast<qint32>(100);  // X
    stream << static_cast<qint32>(100);  // Y
    stream << static_cast<qint32>(100);  // Z
    for (int i = 0; i < 8; ++i) {        // Fill remaining 8 bytes
        stream << static_cast<quint8>(0);
    }
    
    // Point 3: (2.0, 2.0, 2.0) -> scaled: (200, 200, 200)
    stream << static_cast<qint32>(200);  // X
    stream << static_cast<qint32>(200);  // Y
    stream << static_cast<qint32>(200);  // Z
    for (int i = 0; i < 8; ++i) {        // Fill remaining 8 bytes
        stream << static_cast<quint8>(0);
    }
    
    QString fileName = tempFile->fileName();
    tempFile->close();
    delete tempFile;
    
    return fileName;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    std::cout << "Testing LAS Parser..." << std::endl;
    
    // Create test file
    QString testFile = createTestLasFile();
    if (testFile.isEmpty()) {
        std::cout << "Failed to create test file" << std::endl;
        return 1;
    }
    
    std::cout << "Created test file: " << testFile.toStdString() << std::endl;
    
    // Test parser
    LasParser parser;
    
    // Test file validation
    bool isValid = parser.isValidLasFile(testFile);
    std::cout << "File validation: " << (isValid ? "PASS" : "FAIL") << std::endl;
    
    if (!isValid) {
        std::cout << "Error: " << parser.getLastError().toStdString() << std::endl;
        QFile::remove(testFile);
        return 1;
    }
    
    // Test parsing
    try {
        std::vector<float> points = parser.parse(testFile);
        
        std::cout << "Parsing: PASS" << std::endl;
        std::cout << "Number of points: " << points.size() / 3 << std::endl;
        std::cout << "Total coordinates: " << points.size() << std::endl;
        
        if (points.size() == 9) { // 3 points * 3 coordinates
            std::cout << "Point data:" << std::endl;
            for (size_t i = 0; i < points.size(); i += 3) {
                std::cout << "  Point " << (i/3 + 1) << ": (" 
                         << points[i] << ", " << points[i+1] << ", " << points[i+2] << ")" << std::endl;
            }
            
            // Verify expected values
            bool correct = (points[0] == 0.0f && points[1] == 0.0f && points[2] == 0.0f &&
                           points[3] == 1.0f && points[4] == 1.0f && points[5] == 1.0f &&
                           points[6] == 2.0f && points[7] == 2.0f && points[8] == 2.0f);
            
            std::cout << "Data verification: " << (correct ? "PASS" : "FAIL") << std::endl;
        } else {
            std::cout << "Data verification: FAIL (wrong number of points)" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Parsing: FAIL" << std::endl;
        std::cout << "Error: " << e.what() << std::endl;
        QFile::remove(testFile);
        return 1;
    }
    
    // Clean up
    QFile::remove(testFile);
    
    std::cout << "All tests completed!" << std::endl;
    return 0;
}
