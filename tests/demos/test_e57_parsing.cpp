#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include "../../src/e57parser.h"

// Create a minimal valid E57 file for testing
bool createTestE57File(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Failed to create test file:" << filePath;
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    // Write E57 header (48 bytes)
    // File signature "ASTF"
    stream << static_cast<quint32>(0x41535446);
    
    // Version numbers
    stream << static_cast<quint32>(1); // Major version
    stream << static_cast<quint32>(0); // Minor version
    
    // File physical length (will be updated later)
    qint64 fileLengthPos = stream.device()->pos();
    stream << static_cast<quint64>(0); // Placeholder
    
    // XML length and offset (will be updated later)
    qint64 xmlLengthPos = stream.device()->pos();
    stream << static_cast<quint64>(0); // XML length placeholder
    qint64 xmlOffsetPos = stream.device()->pos();
    stream << static_cast<quint64>(0); // XML offset placeholder
    
    // Page size (1024 bytes is standard)
    stream << static_cast<quint64>(1024);
    
    // Reserved fields (fill to 48 bytes)
    for (int i = 0; i < 4; ++i) {
        stream << static_cast<quint64>(0);
    }
    
    // Create simple XML content
    QString xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<e57Root type="Structure" xmlns="http://www.astm.org/COMMIT/E57/2010-e57-v1.0">
    <formatName type="String">ASTM E57 3D Imaging Data File</formatName>
    <guid type="String">{12345678-1234-1234-1234-123456789012}</guid>
    <versionMajor type="Integer">1</versionMajor>
    <versionMinor type="Integer">0</versionMinor>
    <e57LibraryVersion type="String">Test E57 Parser</e57LibraryVersion>
    <coordinateMetadata type="String">Test coordinate system</coordinateMetadata>
    <data3D type="Vector" allowHeterogeneousChildren="1">
        <vectorChild type="Structure">
            <guid type="String">{87654321-4321-4321-4321-210987654321}</guid>
            <name type="String">Test Point Cloud</name>
            <description type="String">Test point cloud data</description>
            <points type="CompressedVector" fileOffset="1024" recordCount="3">
                <prototype type="Structure">
                    <cartesianX type="Float" precision="single"/>
                    <cartesianY type="Float" precision="single"/>
                    <cartesianZ type="Float" precision="single"/>
                </prototype>
                <codecs type="Vector">
                    <vectorChild type="CompressedVectorNode">
                        <recordCount type="Integer">3</recordCount>
                        <binarySection type="String">test_binary_section</binarySection>
                    </vectorChild>
                </codecs>
            </points>
        </vectorChild>
    </data3D>
</e57Root>)";
    
    // Write XML content
    qint64 xmlOffset = stream.device()->pos();
    QByteArray xmlBytes = xmlContent.toUtf8();
    stream.writeRawData(xmlBytes.data(), xmlBytes.size());
    qint64 xmlLength = xmlBytes.size();
    
    // Write some test point data (3 points)
    qint64 binaryOffset = stream.device()->pos();
    
    // Point 1: (1.0, 2.0, 3.0)
    stream << 1.0f << 2.0f << 3.0f;
    // Point 2: (4.0, 5.0, 6.0)
    stream << 4.0f << 5.0f << 6.0f;
    // Point 3: (7.0, 8.0, 9.0)
    stream << 7.0f << 8.0f << 9.0f;
    
    qint64 fileLength = stream.device()->pos();
    
    // Update header with actual values
    stream.device()->seek(fileLengthPos);
    stream << static_cast<quint64>(fileLength);
    
    stream.device()->seek(xmlLengthPos);
    stream << static_cast<quint64>(xmlLength);
    
    stream.device()->seek(xmlOffsetPos);
    stream << static_cast<quint64>(xmlOffset);
    
    file.close();
    
    qDebug() << "Created test E57 file:" << filePath;
    qDebug() << "File length:" << fileLength;
    qDebug() << "XML offset:" << xmlOffset << "length:" << xmlLength;
    qDebug() << "Binary offset:" << binaryOffset;
    
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== E57 Parser Test ===";
    
    // Create test file
    QString testFile = "test_real.e57";
    if (!createTestE57File(testFile)) {
        return 1;
    }
    
    // Test parsing
    E57Parser parser;
    
    try {
        qDebug() << "\n=== Testing E57 Parsing ===";
        std::vector<float> points = parser.parse(testFile);
        
        qDebug() << "Parsing completed!";
        qDebug() << "Points extracted:" << (points.size() / 3);
        
        if (!points.empty()) {
            qDebug() << "Point data:";
            for (size_t i = 0; i < points.size(); i += 3) {
                if (i + 2 < points.size()) {
                    qDebug() << "Point" << (i/3 + 1) << ":" << points[i] << points[i+1] << points[i+2];
                }
            }
        }
        
    } catch (const std::exception& e) {
        qCritical() << "Parsing failed:" << e.what();
        return 1;
    }
    
    // Clean up
    QFile::remove(testFile);
    
    qDebug() << "\n=== Test completed successfully ===";
    return 0;
}
