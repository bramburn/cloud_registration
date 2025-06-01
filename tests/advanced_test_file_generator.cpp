#include "advanced_test_file_generator.h"
#include <QFile>
#include <QDataStream>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QCryptographicHash>
#include <cmath>

AdvancedTestFileGenerator::AdvancedTestFileGenerator(QObject *parent) 
    : QObject(parent)
    , m_randomGenerator(QRandomGenerator::global())
{
    m_outputDirectory = "tests/data/advanced";
    QDir().mkpath(m_outputDirectory);
}

bool AdvancedTestFileGenerator::generateTestFile(TestScenario scenario, const QString &outputPath)
{
    qDebug() << "Generating test file for scenario:" << static_cast<int>(scenario);
    updateProgress(0, "Initializing test file generation...");
    
    switch (scenario) {
        case TestScenario::VeryLargePointCloud:
            return generateVeryLargeE57(outputPath);
        case TestScenario::MultipleDataSections:
            return generateMultiScanE57(outputPath);
        case TestScenario::ExtremeCoordinates:
            return generateExtremeCoordinatesLAS(outputPath);
        case TestScenario::ManyVLRs:
            return generateManyVLRsLAS(outputPath);
        case TestScenario::CorruptedHeaders:
            return generateCorruptedE57(outputPath, "header_corruption");
        case TestScenario::MemoryStressTest:
            return generateMemoryStressE57(outputPath);
        case TestScenario::EdgeCasePDRF:
            return generateEdgeCasePDRFLAS(outputPath);
        default:
            qWarning() << "Unknown test scenario:" << static_cast<int>(scenario);
            return false;
    }
}

bool AdvancedTestFileGenerator::generateVeryLargeE57(const QString &filePath, int pointCount)
{
    qDebug() << "Generating very large E57 file with" << pointCount << "points";
    updateProgress(5, "Creating large point dataset...");
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create file:" << filePath;
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    // E57 file signature
    stream.writeRawData("ASTM-E57", 8);
    
    // Version info
    quint32 majorVersion = 1;
    quint32 minorVersion = 0;
    stream << majorVersion << minorVersion;
    
    // Calculate proper offsets for E57 file structure
    qint64 headerSize = 48; // E57 header is 48 bytes
    qint64 xmlOffset = headerSize;
    qint64 binaryDataSize = pointCount * 3 * sizeof(double);

    // Generate XML with placeholder binary offset
    QString xml = generateE57XmlHeader(pointCount, 0);
    QByteArray xmlData = xml.toUtf8();
    qint64 xmlLength = xmlData.size();
    qint64 binaryOffset = xmlOffset + xmlLength;

    // Update XML with correct binary offset
    xml = generateE57XmlHeader(pointCount, binaryOffset);
    xmlData = xml.toUtf8();
    xmlLength = xmlData.size();

    qint64 filePhysicalLength = binaryOffset + binaryDataSize;

    // Write proper E57 header structure
    stream << filePhysicalLength;  // File physical length
    stream << xmlOffset;           // XML offset
    stream << xmlLength;           // XML length
    
    // Write page size (dummy)
    quint64 pageSize = 1024;
    stream << pageSize;
    
    updateProgress(10, "Writing XML structure...");
    
    // Write XML data
    file.seek(xmlOffset);
    stream.writeRawData(xmlData.data(), xmlData.size());
    
    updateProgress(20, "Generating point cloud data...");
    
    // Generate and write binary point data in chunks to manage memory
    file.seek(binaryOffset);
    const int chunkSize = 100000; // Process 100k points at a time
    int pointsWritten = 0;
    
    while (pointsWritten < pointCount) {
        int currentChunkSize = qMin(chunkSize, pointCount - pointsWritten);
        std::vector<float> chunkPoints;
        
        generateRandomPointData(chunkPoints, currentChunkSize * 3,
                               -1000.0, 1000.0,  // X range
                               -1000.0, 1000.0,  // Y range
                               0.0, 100.0);      // Z range
        
        // Write chunk as binary data
        for (float point : chunkPoints) {
            double doublePoint = static_cast<double>(point);
            stream << doublePoint;
        }
        
        pointsWritten += currentChunkSize;
        int progress = 20 + (pointsWritten * 70) / pointCount;
        updateProgress(progress, QString("Writing points: %1/%2").arg(pointsWritten).arg(pointCount));
    }
    
    updateProgress(100, "Large E57 file generation completed");
    emit generationCompleted(filePath, true);
    return true;
}

bool AdvancedTestFileGenerator::generateMultiScanE57(const QString &filePath, int scanCount)
{
    qDebug() << "Generating multi-scan E57 file with" << scanCount << "scans";
    updateProgress(0, "Creating multi-scan E57 structure...");
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create file:" << filePath;
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    // E57 file signature
    stream.writeRawData("ASTM-E57", 8);
    
    // Version info
    quint32 majorVersion = 1;
    quint32 minorVersion = 0;
    stream << majorVersion << minorVersion;
    
    // Generate point counts for each scan
    QList<int> scanPointCounts;
    for (int i = 0; i < scanCount; ++i) {
        int points = 1000 + (i * 500); // Varying point counts
        scanPointCounts.append(points);
    }
    
    updateProgress(20, "Generating XML structure for multiple scans...");
    
    // Generate XML with multiple data3D sections
    QString xml = generateMultiScanE57Xml(scanCount, scanPointCounts);
    QByteArray xmlData = xml.toUtf8();
    
    qint64 xmlOffset = 24;
    qint64 xmlLength = xmlData.size();
    qint64 binaryOffset = xmlOffset + xmlLength + 100;
    
    // Write offsets
    stream << xmlOffset << xmlLength;
    quint64 pageSize = 1024;
    stream << pageSize;
    
    // Write XML
    file.seek(xmlOffset);
    stream.writeRawData(xmlData.data(), xmlData.size());
    
    updateProgress(50, "Writing binary data for multiple scans...");
    
    // Write binary data for each scan
    file.seek(binaryOffset);
    Q_UNUSED(binaryOffset); // Used for seek above
    
    for (int i = 0; i < scanCount; ++i) {
        std::vector<float> scanPoints;
        
        // Each scan has slightly different coordinate ranges
        double offset = i * 100.0;
        generateRandomPointData(scanPoints, scanPointCounts[i] * 3,
                               -1000.0 + offset, 1000.0 + offset,
                               -1000.0 + offset, 1000.0 + offset,
                               0.0, 100.0);
        
        for (float point : scanPoints) {
            double doublePoint = static_cast<double>(point);
            stream << doublePoint;
        }
        
        int progress = 50 + ((i + 1) * 40) / scanCount;
        updateProgress(progress, QString("Written scan %1/%2").arg(i + 1).arg(scanCount));
    }
    
    updateProgress(100, "Multi-scan E57 file generation completed");
    emit generationCompleted(filePath, true);
    return true;
}

bool AdvancedTestFileGenerator::generateExtremeCoordinatesLAS(const QString &filePath)
{
    qDebug() << "Generating LAS file with extreme coordinates";
    updateProgress(0, "Creating extreme coordinate LAS file...");
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create file:" << filePath;
        return false;
    }
    
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);
    
    updateProgress(20, "Writing LAS header with extreme scale/offset...");
    
    // Generate header with extreme scale/offset values
    QByteArray header = generateLASHeader(100000,
                                         0.000001,    // Very small X scale
                                         1000.0,      // Very large Y scale  
                                         0.1,         // Normal Z scale
                                         -1000000000.0, // Large negative X offset
                                         500000000.0,   // Large positive Y offset
                                         0.0);          // Normal Z offset
    
    stream.writeRawData(header.data(), header.size());
    
    updateProgress(50, "Generating point data with extreme coordinates...");
    
    // Generate point data with extreme coordinate ranges
    std::vector<float> points;
    generateRandomPointData(points, 100000 * 3,
                           0.0, 2000000000.0,    // Large X range (will be scaled down)
                           0.0, 1000.0,          // Small Y range (will be scaled up)
                           0.0, 1000.0);         // Normal Z range
    
    updateProgress(80, "Writing point data...");
    
    // Write point data in LAS format (simplified PDRF 1)
    for (size_t i = 0; i < points.size(); i += 3) {
        // Convert to scaled integers as per LAS specification
        quint32 x = static_cast<quint32>(points[i]);
        quint32 y = static_cast<quint32>(points[i + 1]);
        quint32 z = static_cast<quint32>(points[i + 2]);
        
        stream << x << y << z;
        
        // Add dummy intensity, return info, etc. for PDRF 1
        quint16 intensity = m_randomGenerator->bounded(65536);
        quint8 returnInfo = m_randomGenerator->bounded(256);
        quint8 classification = m_randomGenerator->bounded(32);
        qint8 scanAngle = m_randomGenerator->bounded(-90, 91);
        quint8 userData = 0;
        quint16 pointSourceId = 1;
        double gpsTime = i * 0.001;
        
        stream << intensity << returnInfo << classification << scanAngle 
               << userData << pointSourceId << gpsTime;
    }
    
    updateProgress(100, "Extreme coordinates LAS file generation completed");
    emit generationCompleted(filePath, true);
    return true;
}

bool AdvancedTestFileGenerator::generateManyVLRsLAS(const QString &filePath, int vlrCount)
{
    qDebug() << "Generating LAS file with" << vlrCount << "VLRs";
    updateProgress(0, "Creating LAS file with many VLRs...");

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to create file:" << filePath;
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    updateProgress(20, "Writing LAS header...");

    // Generate header with many VLRs
    QByteArray header = generateLASHeader(50000, 0.01, 0.01, 0.01, 0.0, 0.0, 0.0, vlrCount);
    stream.writeRawData(header.data(), header.size());

    updateProgress(40, "Writing Variable Length Records...");

    // Write VLRs
    QByteArray vlrData = generateLASVLRs(vlrCount);
    stream.writeRawData(vlrData.data(), vlrData.size());

    updateProgress(70, "Writing point data...");

    // Generate and write point data
    std::vector<float> points;
    generateRandomPointData(points, 50000 * 3, -100.0, 100.0, -100.0, 100.0, 0.0, 50.0);

    QByteArray pointData = generateLASPointData(points, 1); // PDRF 1
    stream.writeRawData(pointData.data(), pointData.size());

    updateProgress(100, "Many VLRs LAS file generation completed");
    emit generationCompleted(filePath, true);
    return true;
}

bool AdvancedTestFileGenerator::generateCorruptedE57(const QString &filePath, const QString &corruptionType)
{
    qDebug() << "Generating corrupted E57 file with corruption type:" << corruptionType;
    updateProgress(0, "Creating corrupted E57 file...");

    // First generate a valid E57 file
    QString tempPath = filePath + ".temp";
    if (!generateVeryLargeE57(tempPath, 1000)) {
        return false;
    }

    updateProgress(50, "Applying corruption...");

    // Read the valid file
    QFile tempFile(tempPath);
    if (!tempFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray validData = tempFile.readAll();
    tempFile.close();

    // Apply corruption
    QByteArray corruptedData = corruptData(validData, corruptionType);

    // Write corrupted file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(corruptedData);
    file.close();

    // Clean up temp file
    QFile::remove(tempPath);

    updateProgress(100, "Corrupted E57 file generation completed");
    emit generationCompleted(filePath, true);
    return true;
}

bool AdvancedTestFileGenerator::generateMemoryStressE57(const QString &filePath)
{
    // Generate a file that approaches typical system memory limits
    return generateVeryLargeE57(filePath, 50000000); // 50M points
}

bool AdvancedTestFileGenerator::generateEdgeCasePDRFLAS(const QString &filePath)
{
    qDebug() << "Generating LAS file with edge case PDRF";
    updateProgress(0, "Creating edge case PDRF LAS file...");

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Use PDRF 3 (GPS time + RGB)
    QByteArray header = generateLASHeader(10000, 0.01, 0.01, 0.01, 0.0, 0.0, 0.0, 0);

    // Modify header to use PDRF 3
    header[104] = 3; // Point Data Record Format
    *reinterpret_cast<quint16*>(&header.data()[105]) = 34; // Record length for PDRF 3

    stream.writeRawData(header.data(), header.size());

    updateProgress(50, "Writing PDRF 3 point data...");

    // Generate point data with RGB colors
    std::vector<float> points;
    generateRandomPointData(points, 10000 * 3, -50.0, 50.0, -50.0, 50.0, 0.0, 25.0);

    for (size_t i = 0; i < points.size(); i += 3) {
        quint32 x = static_cast<quint32>(points[i] / 0.01);
        quint32 y = static_cast<quint32>(points[i + 1] / 0.01);
        quint32 z = static_cast<quint32>(points[i + 2] / 0.01);

        stream << x << y << z;

        // PDRF 3 additional fields
        quint16 intensity = m_randomGenerator->bounded(65536);
        quint8 returnInfo = m_randomGenerator->bounded(256);
        quint8 classification = m_randomGenerator->bounded(32);
        qint8 scanAngle = m_randomGenerator->bounded(-90, 91);
        quint8 userData = 0;
        quint16 pointSourceId = 1;
        double gpsTime = i * 0.001;

        // RGB values
        quint16 red = m_randomGenerator->bounded(65536);
        quint16 green = m_randomGenerator->bounded(65536);
        quint16 blue = m_randomGenerator->bounded(65536);

        stream << intensity << returnInfo << classification << scanAngle
               << userData << pointSourceId << gpsTime << red << green << blue;
    }

    updateProgress(100, "Edge case PDRF LAS file generation completed");
    emit generationCompleted(filePath, true);
    return true;
}

QJsonObject AdvancedTestFileGenerator::generateTestMetadata(TestScenario scenario, const QString &filePath)
{
    QJsonObject metadata;

    switch (scenario) {
        case TestScenario::VeryLargePointCloud:
            metadata["scenario"] = "VeryLargePointCloud";
            metadata["expectedPointCount"] = 25000000;
            metadata["shouldLoad"] = true;
            metadata["expectedMemoryMB"] = 2000;
            metadata["testFocus"] = "Memory usage and loading performance";
            break;

        case TestScenario::MultipleDataSections:
            metadata["scenario"] = "MultipleDataSections";
            metadata["expectedScanCount"] = 5;
            metadata["shouldLoad"] = true; // Should load first scan
            metadata["expectedBehavior"] = "Load first scan, warn about additional scans";
            break;

        case TestScenario::ExtremeCoordinates:
            metadata["scenario"] = "ExtremeCoordinates";
            metadata["shouldLoad"] = true;
            metadata["testFocus"] = "Coordinate transformation with extreme scale/offset";
            break;

        case TestScenario::ManyVLRs:
            metadata["scenario"] = "ManyVLRs";
            metadata["expectedVLRCount"] = 100;
            metadata["shouldLoad"] = true;
            metadata["testFocus"] = "Header parsing with numerous VLRs";
            break;

        case TestScenario::CorruptedHeaders:
            metadata["scenario"] = "CorruptedHeaders";
            metadata["shouldLoad"] = false;
            metadata["expectedBehavior"] = "Graceful failure with error message";
            break;

        case TestScenario::MemoryStressTest:
            metadata["scenario"] = "MemoryStressTest";
            metadata["expectedPointCount"] = 50000000;
            metadata["shouldLoad"] = true;
            metadata["expectedMemoryMB"] = 4000;
            metadata["testFocus"] = "Memory limits and performance";
            break;

        case TestScenario::EdgeCasePDRF:
            metadata["scenario"] = "EdgeCasePDRF";
            metadata["pdrf"] = 3;
            metadata["shouldLoad"] = true;
            metadata["testFocus"] = "PDRF 3 with RGB data parsing";
            break;
    }

    metadata["filePath"] = filePath;
    metadata["generatedAt"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (QFile::exists(filePath)) {
        metadata["fileSize"] = QFileInfo(filePath).size();
    }

    return metadata;
}

void AdvancedTestFileGenerator::generateRandomPointData(std::vector<float> &points, int count,
                                                       double xMin, double xMax,
                                                       double yMin, double yMax,
                                                       double zMin, double zMax)
{
    points.clear();
    points.reserve(count);

    for (int i = 0; i < count; i += 3) {
        points.push_back(static_cast<float>(xMin + m_randomGenerator->generateDouble() * (xMax - xMin)));   // X
        points.push_back(static_cast<float>(yMin + m_randomGenerator->generateDouble() * (yMax - yMin)));   // Y
        points.push_back(static_cast<float>(zMin + m_randomGenerator->generateDouble() * (zMax - zMin)));   // Z
    }
}

QString AdvancedTestFileGenerator::generateE57XmlHeader(int pointCount, qint64 binaryOffset)
{
    return QString(R"(<?xml version="1.0" encoding="UTF-8"?>
<e57Root type="Structure" xmlns="http://www.astm.org/COMMIT/E57/2010-e57-v1.0">
    <formatName type="String">ASTM E57 3D Imaging Data File</formatName>
    <guid type="String">{LARGE-TEST-FILE-GUID-123456789ABC}</guid>
    <versionMajor type="Integer">1</versionMajor>
    <versionMinor type="Integer">0</versionMinor>
    <data3D type="Vector" allowHeterogeneousChildren="1">
        <vectorChild type="Structure">
            <points type="CompressedVector" fileOffset="%2" recordCount="%1">
                <prototype type="Structure">
                    <cartesianX type="Float" precision="double"/>
                    <cartesianY type="Float" precision="double"/>
                    <cartesianZ type="Float" precision="double"/>
                </prototype>
                <codecs type="Vector" allowHeterogeneousChildren="1">
                    <vectorChild type="Structure">
                        <type type="Integer">1</type>
                    </vectorChild>
                </codecs>
            </points>
            <pointCount type="Integer">%1</pointCount>
        </vectorChild>
    </data3D>
</e57Root>)").arg(pointCount).arg(binaryOffset);
}

QString AdvancedTestFileGenerator::generateMultiScanE57Xml(int scanCount, const QList<int> &pointCounts)
{
    QString xmlStart = R"(<?xml version="1.0" encoding="UTF-8"?>
<e57Root type="Structure" xmlns="http://www.astm.org/COMMIT/E57/2010-e57-v1.0">
    <formatName type="String">ASTM E57 3D Imaging Data File</formatName>
    <guid type="String">{MULTI-SCAN-TEST-FILE-GUID}</guid>
    <versionMajor type="Integer">1</versionMajor>
    <versionMinor type="Integer">0</versionMinor>
    <data3D type="Vector" allowHeterogeneousChildren="1">)";

    QString xmlEnd = R"(
    </data3D>
</e57Root>)";

    QString xmlScans;
    qint64 currentOffset = 2000; // Start after XML

    for (int i = 0; i < scanCount; ++i) {
        int pointsInScan = pointCounts[i];
        qint64 scanDataSize = pointsInScan * 3 * sizeof(double);

        xmlScans += QString(R"(
        <vectorChild type="Structure">
            <guid type="String">{SCAN-%1-GUID}</guid>
            <name type="String">Scan %1</name>
            <description type="String">Test scan number %1</description>
            <points type="CompressedVector" fileOffset="%3" recordCount="%2">
                <prototype type="Structure">
                    <cartesianX type="Float" precision="double"/>
                    <cartesianY type="Float" precision="double"/>
                    <cartesianZ type="Float" precision="double"/>
                </prototype>
                <codecs type="Vector" allowHeterogeneousChildren="1">
                    <vectorChild type="Structure">
                        <type type="Integer">1</type>
                    </vectorChild>
                </codecs>
            </points>
            <pointCount type="Integer">%2</pointCount>
        </vectorChild>)").arg(i).arg(pointsInScan).arg(currentOffset);

        currentOffset += scanDataSize;
    }

    return xmlStart + xmlScans + xmlEnd;
}

QByteArray AdvancedTestFileGenerator::generateLASHeader(int pointCount, double xScale, double yScale, double zScale,
                                                       double xOffset, double yOffset, double zOffset, int vlrCount)
{
    QByteArray header(375, 0); // LAS 1.2 header size

    // File signature
    header.replace(0, 4, "LASF");

    // File source ID
    *reinterpret_cast<quint16*>(&header.data()[4]) = 0;

    // Global encoding
    *reinterpret_cast<quint16*>(&header.data()[6]) = 0;

    // GUID (16 bytes of zeros for simplicity)
    // Skip bytes 8-23

    // Version
    header[24] = 1; // Major
    header[25] = 2; // Minor

    // System identifier (32 bytes)
    QByteArray sysId = "ADVANCED_TEST_GENERATOR";
    header.replace(26, qMin(32, sysId.size()), sysId);

    // Generating software (32 bytes)
    QByteArray software = "Sprint 2.4 Test Suite";
    header.replace(58, qMin(32, software.size()), software);

    // Creation date
    QDate today = QDate::currentDate();
    *reinterpret_cast<quint16*>(&header.data()[90]) = today.dayOfYear();
    *reinterpret_cast<quint16*>(&header.data()[92]) = today.year();

    // Header size
    *reinterpret_cast<quint16*>(&header.data()[94]) = 375;

    // Offset to point data
    quint32 pointDataOffset = 375 + (vlrCount * 54); // Header + VLRs
    *reinterpret_cast<quint32*>(&header.data()[96]) = pointDataOffset;

    // Number of VLRs
    *reinterpret_cast<quint32*>(&header.data()[100]) = vlrCount;

    // Point data format
    header[104] = 1; // PDRF 1

    // Point data record length
    *reinterpret_cast<quint16*>(&header.data()[105]) = 28;

    // Number of point records
    *reinterpret_cast<quint32*>(&header.data()[107]) = pointCount;

    // Scale factors
    *reinterpret_cast<double*>(&header.data()[131]) = xScale;
    *reinterpret_cast<double*>(&header.data()[139]) = yScale;
    *reinterpret_cast<double*>(&header.data()[147]) = zScale;

    // Offsets
    *reinterpret_cast<double*>(&header.data()[155]) = xOffset;
    *reinterpret_cast<double*>(&header.data()[163]) = yOffset;
    *reinterpret_cast<double*>(&header.data()[171]) = zOffset;

    // Min/Max values (simplified - set to reasonable defaults)
    *reinterpret_cast<double*>(&header.data()[179]) = xOffset - 1000.0; // X max
    *reinterpret_cast<double*>(&header.data()[187]) = xOffset + 1000.0; // X min
    *reinterpret_cast<double*>(&header.data()[195]) = yOffset - 1000.0; // Y max
    *reinterpret_cast<double*>(&header.data()[203]) = yOffset + 1000.0; // Y min
    *reinterpret_cast<double*>(&header.data()[211]) = zOffset;          // Z max
    *reinterpret_cast<double*>(&header.data()[219]) = zOffset + 100.0;  // Z min

    return header;
}

QByteArray AdvancedTestFileGenerator::generateLASVLRs(int count)
{
    QByteArray vlrData;

    for (int i = 0; i < count; ++i) {
        QByteArray vlr(54, 0); // Standard VLR header size

        // Reserved
        *reinterpret_cast<quint16*>(&vlr.data()[0]) = 0;

        // User ID
        QString userId = QString("TEST_USER_%1").arg(i, 3, 10, QChar('0'));
        vlr.replace(2, qMin(16, userId.size()), userId.toUtf8());

        // Record ID
        *reinterpret_cast<quint16*>(&vlr.data()[18]) = i;

        // Record length after header
        *reinterpret_cast<quint16*>(&vlr.data()[20]) = 0; // No data after header

        // Description
        QString desc = QString("Test VLR number %1").arg(i);
        vlr.replace(22, qMin(32, desc.size()), desc.toUtf8());

        vlrData.append(vlr);
    }

    return vlrData;
}

QByteArray AdvancedTestFileGenerator::generateLASPointData(const std::vector<float> &points, int pdrf)
{
    QByteArray pointData;
    QDataStream stream(&pointData, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    for (size_t i = 0; i < points.size(); i += 3) {
        // Coordinates (scaled to integers)
        quint32 x = static_cast<quint32>(points[i] / 0.01);
        quint32 y = static_cast<quint32>(points[i + 1] / 0.01);
        quint32 z = static_cast<quint32>(points[i + 2] / 0.01);

        stream << x << y << z;

        // Additional fields based on PDRF
        quint16 intensity = m_randomGenerator->bounded(65536);
        quint8 returnInfo = m_randomGenerator->bounded(256);
        quint8 classification = m_randomGenerator->bounded(32);
        qint8 scanAngle = m_randomGenerator->bounded(-90, 91);
        quint8 userData = 0;
        quint16 pointSourceId = 1;

        stream << intensity << returnInfo << classification << scanAngle << userData << pointSourceId;

        if (pdrf >= 1) {
            // GPS time for PDRF 1, 3, 4, 5
            double gpsTime = i * 0.001;
            stream << gpsTime;
        }

        if (pdrf == 2 || pdrf == 3 || pdrf == 5) {
            // RGB for PDRF 2, 3, 5
            quint16 red = m_randomGenerator->bounded(65536);
            quint16 green = m_randomGenerator->bounded(65536);
            quint16 blue = m_randomGenerator->bounded(65536);
            stream << red << green << blue;
        }
    }

    return pointData;
}

QByteArray AdvancedTestFileGenerator::corruptData(const QByteArray &data, const QString &corruptionType)
{
    QByteArray corrupted = data;

    if (corruptionType == "header_corruption") {
        // Corrupt the file signature
        if (corrupted.size() >= 8) {
            corrupted[0] = 'X';
            corrupted[1] = 'X';
            corrupted[2] = 'X';
            corrupted[3] = 'X';
        }
    } else if (corruptionType == "xml_corruption") {
        // Find and corrupt XML section
        int xmlStart = corrupted.indexOf("<?xml");
        if (xmlStart >= 0 && xmlStart + 50 < corrupted.size()) {
            // Corrupt XML structure
            corrupted[xmlStart + 20] = 'X';
            corrupted[xmlStart + 21] = 'X';
            corrupted[xmlStart + 22] = 'X';
        }
    } else if (corruptionType == "binary_corruption") {
        // Corrupt binary data section
        if (corrupted.size() > 1000) {
            int corruptStart = corrupted.size() / 2;
            for (int i = 0; i < 100 && corruptStart + i < corrupted.size(); ++i) {
                corrupted[corruptStart + i] = static_cast<char>(m_randomGenerator->bounded(256));
            }
        }
    } else if (corruptionType == "truncation") {
        // Truncate file
        if (corrupted.size() > 100) {
            corrupted = corrupted.left(corrupted.size() / 2);
        }
    }

    return corrupted;
}

void AdvancedTestFileGenerator::updateProgress(int percentage, const QString &status)
{
    emit generationProgress(percentage, status);
    qDebug() << QString("Test file generation: %1% - %2").arg(percentage).arg(status);
}
