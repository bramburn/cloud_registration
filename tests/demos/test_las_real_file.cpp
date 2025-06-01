#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <iostream>
#include "../../src/lasparser.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== LAS Parser Real File Test ===";
    qDebug() << "Current directory:" << QDir::currentPath();
    
    // Try to find the real LAS file - adjust paths for tests/demos/ location
    QStringList possiblePaths = {
        "../../sample/S2max-Power line202503.las",
        "../../../sample/S2max-Power line202503.las",
        "sample/S2max-Power line202503.las"
    };
    
    QString realLasFile;
    for (const QString& path : possiblePaths) {
        if (QFileInfo::exists(path)) {
            realLasFile = path;
            qDebug() << "Found real LAS file at:" << realLasFile;
            break;
        }
    }
    
    if (realLasFile.isEmpty()) {
        qDebug() << "Error: Real LAS file not found!";
        qDebug() << "Tried paths:";
        for (const QString& path : possiblePaths) {
            qDebug() << "  " << path << "- exists:" << QFileInfo::exists(path);
        }
        return 1;
    }
    
    // Test the LAS parser
    LasParser parser;
    
    qDebug() << "\n=== Testing LAS File Validation ===";
    bool isValid = parser.isValidLasFile(realLasFile);
    qDebug() << "Is valid LAS file:" << isValid;
    
    if (!isValid) {
        qDebug() << "Error: File is not a valid LAS file";
        return 1;
    }
    
    qDebug() << "\n=== Testing LAS File Parsing ===";

    // First, let's examine the header in detail
    qDebug() << "\n=== Detailed Header Analysis ===";
    QFile file(realLasFile);
    if (file.open(QIODevice::ReadOnly)) {
        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);

        // Read key header fields
        file.seek(24); // Version position
        quint8 versionMajor, versionMinor;
        stream >> versionMajor >> versionMinor;
        qDebug() << "Version:" << versionMajor << "." << versionMinor;

        file.seek(104); // Point data format position
        quint8 pointDataFormat;
        stream >> pointDataFormat;
        qDebug() << "Point Data Format:" << pointDataFormat;

        file.seek(105); // Point data record length position
        quint16 recordLength;
        stream >> recordLength;
        qDebug() << "Actual record length:" << recordLength;
        qDebug() << "Expected record length for PDRF" << pointDataFormat << ":" <<
                    (pointDataFormat == 0 ? 20 : pointDataFormat == 1 ? 28 : pointDataFormat == 2 ? 26 : pointDataFormat == 3 ? 34 : 0);

        file.seek(107); // Number of points position
        quint32 numPoints;
        stream >> numPoints;
        qDebug() << "Number of points:" << numPoints;

        file.close();
    }

    try {
        std::vector<float> points = parser.parse(realLasFile);
        
        qDebug() << "Parsing successful!";
        qDebug() << "Point count:" << (points.size() / 3);
        qDebug() << "Total coordinates:" << points.size();
        
        // Display header information
        qDebug() << "\n=== LAS Header Information ===";
        qDebug() << "Version:" << parser.getVersionMajor() << "." << parser.getVersionMinor();
        qDebug() << "Point Data Format:" << parser.getPointDataFormat();
        qDebug() << "Header size:" << parser.getHeaderSize();
        qDebug() << "Record length:" << parser.getPointDataRecordLength();
        
        // Display sample coordinates
        if (points.size() >= 9) {
            qDebug() << "\n=== Sample Coordinates ===";
            qDebug() << "Point 1:" << points[0] << points[1] << points[2];
            qDebug() << "Point 2:" << points[3] << points[4] << points[5];
            qDebug() << "Point 3:" << points[6] << points[7] << points[8];
            
            if (points.size() >= 30) {
                qDebug() << "Point 10:" << points[27] << points[28] << points[29];
            }
        }
        
        // Validate Sprint 1.3 requirements
        qDebug() << "\n=== Sprint 1.3 Validation ===";
        
        // Check version support (1.2, 1.3, 1.4)
        bool versionSupported = (parser.getVersionMajor() == 1 && 
                               parser.getVersionMinor() >= 2 && 
                               parser.getVersionMinor() <= 4);
        qDebug() << "Version supported (1.2-1.4):" << versionSupported;
        
        // Check PDRF support (0-3)
        bool pdrfSupported = (parser.getPointDataFormat() >= 0 && 
                            parser.getPointDataFormat() <= 3);
        qDebug() << "PDRF supported (0-3):" << pdrfSupported;
        
        if (versionSupported && pdrfSupported) {
            qDebug() << "\n✓ Sprint 1.3 Enhanced LAS Format Support: PASSED";
            qDebug() << "✓ LAS" << parser.getVersionMajor() << "." << parser.getVersionMinor() 
                     << "PDRF" << parser.getPointDataFormat() << "successfully parsed";
        } else {
            qDebug() << "\n✗ Sprint 1.3 Enhanced LAS Format Support: FAILED";
            if (!versionSupported) {
                qDebug() << "  - Unsupported version:" << parser.getVersionMajor() << "." << parser.getVersionMinor();
            }
            if (!pdrfSupported) {
                qDebug() << "  - Unsupported PDRF:" << parser.getPointDataFormat();
            }
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        qDebug() << "Parsing failed with exception:" << e.what();
        qDebug() << "Last error:" << parser.getLastError();
        return 1;
    }
}
