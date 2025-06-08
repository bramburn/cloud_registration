#ifndef LASWRITER_H
#define LASWRITER_H

#include "../IFormatWriter.h"
#include <QFile>
#include <QDataStream>
#include <memory>

/**
 * @brief LAS format writer implementation
 * 
 * Sprint 6 User Story 1: Multi-Format Point Cloud Export
 * Implements LAS 1.2 format writing for laser scanning data.
 */
class LASWriter : public IFormatWriter
{
public:
    LASWriter();
    ~LASWriter() override;
    
    // IFormatWriter interface
    bool open(const QString& path) override;
    bool writeHeader(const HeaderInfo& info) override;
    bool writePoint(const Point& point) override;
    bool close() override;
    
    QString getFileExtension() const override { return "las"; }
    QString getFormatDescription() const override { return "LAS Point Cloud Format"; }
    bool supportsColor() const override { return true; }
    bool supportsIntensity() const override { return true; }
    
private:
    struct LASHeader {
        char fileSignature[4] = {'L', 'A', 'S', 'F'};
        uint16_t fileSourceId = 0;
        uint16_t globalEncoding = 0;
        uint32_t guidData1 = 0;
        uint16_t guidData2 = 0;
        uint16_t guidData3 = 0;
        uint8_t guidData4[8] = {0};
        uint8_t versionMajor = 1;
        uint8_t versionMinor = 2;
        char systemIdentifier[32] = "CloudRegistration";
        char generatingSoftware[32] = "CloudRegistration 1.0";
        uint16_t creationDayOfYear = 0;
        uint16_t creationYear = 0;
        uint16_t headerSize = 227;
        uint32_t offsetToPointData = 227;
        uint32_t numberOfVariableLengthRecords = 0;
        uint8_t pointDataRecordFormat = 2; // Format 2: X,Y,Z,Intensity,RGB
        uint16_t pointDataRecordLength = 26;
        uint32_t numberOfPointRecords = 0;
        uint32_t numberOfPointsByReturn[5] = {0};
        double xScaleFactor = 0.001;
        double yScaleFactor = 0.001;
        double zScaleFactor = 0.001;
        double xOffset = 0.0;
        double yOffset = 0.0;
        double zOffset = 0.0;
        double maxX = 0.0;
        double minX = 0.0;
        double maxY = 0.0;
        double minY = 0.0;
        double maxZ = 0.0;
        double minZ = 0.0;
    };
    
    struct LASPointRecord {
        int32_t x;
        int32_t y;
        int32_t z;
        uint16_t intensity;
        uint8_t returnInfo = 0x11; // First return of 1
        uint8_t classification = 1; // Unclassified
        int8_t scanAngleRank = 0;
        uint8_t userData = 0;
        uint16_t pointSourceId = 0;
        uint16_t red;
        uint16_t green;
        uint16_t blue;
    };
    
    void writeHeader();
    void updateHeader();
    int32_t scaleCoordinate(double value, double scale, double offset);
    
    std::unique_ptr<QFile> m_file;
    std::unique_ptr<QDataStream> m_stream;
    LASHeader m_header;
    HeaderInfo m_headerInfo;
    bool m_isOpen = false;
    uint32_t m_pointsWritten = 0;
    qint64 m_headerPosition = 0;
};

#endif // LASWRITER_H
