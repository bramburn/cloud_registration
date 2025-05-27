#ifndef LASPARSER_H
#define LASPARSER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <vector>
#include <stdexcept>

class LasParser : public QObject
{
    Q_OBJECT

public:
    explicit LasParser(QObject *parent = nullptr);
    ~LasParser();

    // Main parsing function
    std::vector<float> parse(const QString& filePath);

    // Utility functions
    bool isValidLasFile(const QString& filePath);
    QString getLastError() const;

public slots:
    void startParsing(const QString& filePath);

signals:
    void progressUpdated(int percentage);
    void parsingFinished(bool success, const QString& message, const std::vector<float>& points);

private:
    // LAS file header structure
    struct LasHeader {
        char signature[4];              // "LASF"
        uint16_t fileSourceId;
        uint16_t globalEncoding;
        uint32_t guidData1;
        uint16_t guidData2;
        uint16_t guidData3;
        uint8_t guidData4[8];
        uint8_t versionMajor;
        uint8_t versionMinor;
        char systemIdentifier[32];
        char generatingSoftware[32];
        uint16_t creationDayOfYear;
        uint16_t creationYear;
        uint16_t headerSize;
        uint32_t pointDataOffset;
        uint32_t numberOfVLRs;
        uint8_t pointDataFormat;
        uint16_t pointDataRecordLength;
        uint32_t numberOfPointRecords;
        uint32_t numberOfPointsByReturn[5];
        double xScaleFactor;
        double yScaleFactor;
        double zScaleFactor;
        double xOffset;
        double yOffset;
        double zOffset;
        double maxX;
        double minX;
        double maxY;
        double minY;
        double maxZ;
        double minZ;
    };

    // Point data format structures
    struct PointFormat0 {
        int32_t x;
        int32_t y;
        int32_t z;
        uint16_t intensity;
        uint8_t returnInfo;
        uint8_t classification;
        int8_t scanAngle;
        uint8_t userData;
        uint16_t pointSourceId;
    };

    struct PointFormat1 {
        int32_t x;
        int32_t y;
        int32_t z;
        uint16_t intensity;
        uint8_t returnInfo;
        uint8_t classification;
        int8_t scanAngle;
        uint8_t userData;
        uint16_t pointSourceId;
        double gpsTime;
    };

    // LAS file parsing functions
    bool readHeader(QFile& file, LasHeader& header);
    bool validateHeader(const LasHeader& header);
    std::vector<float> readPointData(QFile& file, const LasHeader& header);

    // Point format specific readers
    std::vector<float> readPointFormat0(QFile& file, const LasHeader& header);
    std::vector<float> readPointFormat1(QFile& file, const LasHeader& header);
    std::vector<float> readPointFormat2(QFile& file, const LasHeader& header);
    std::vector<float> readPointFormat3(QFile& file, const LasHeader& header);

    // Helper functions
    bool readLittleEndian(QFile& file, void* data, size_t size);
    template<typename T>
    bool readValue(QFile& file, T& value);

    // Coordinate transformation helper
    void transformAndAddPoint(std::vector<float>& points, int32_t x, int32_t y, int32_t z, const LasHeader& header);

    // Progress update helper
    void updateProgressIfNeeded(uint32_t currentPoint, uint32_t totalPoints);

    // Error handling
    void setError(const QString& error);

    // Member variables
    QString m_lastError;
    bool m_hasError;

    // LAS file format constants
    static const char LAS_FILE_SIGNATURE[4];
    static const uint8_t SUPPORTED_VERSION_MAJOR;
    static const uint8_t MIN_VERSION_MINOR;
    static const uint8_t MAX_VERSION_MINOR;
    static const uint8_t MAX_SUPPORTED_POINT_FORMAT;

    // Parsing state
    qint64 m_fileSize;
    qint64 m_currentPosition;
    bool m_headerParsed;

    // Point cloud metadata
    uint32_t m_pointCount;
    uint8_t m_pointFormat;
    double m_xScale, m_yScale, m_zScale;
    double m_xOffset, m_yOffset, m_zOffset;
};

// Custom exception for LAS parsing errors
class LasParseException : public std::runtime_error
{
public:
    explicit LasParseException(const QString& message)
        : std::runtime_error(message.toStdString()) {}

    LasParseException(const QString& message, qint64 offset)
        : std::runtime_error((message + QString(" at offset %1").arg(offset)).toStdString()) {}
};

#endif // LASPARSER_H
