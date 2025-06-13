#ifndef LASPARSER_H
#define LASPARSER_H

#include <QByteArray>
#include <QDataStream>
#include <QFile>
#include <QObject>
#include <QString>
// #include <QVector3D>  // Commented out for testing compatibility
#include <stdexcept>
#include <vector>

#include "core/lasheadermetadata.h"
#include "core/voxelgridfilter.h"

// Forward declarations
struct LoadingSettings;

class LasParser : public QObject
{
    Q_OBJECT

public:
    explicit LasParser(QObject* parent = nullptr);
    ~LasParser();

    // Main parsing function
    std::vector<float> parse(const QString& filePath);
    std::vector<float> parse(const QString& filePath, const LoadingSettings& settings);

    // Utility functions
    bool isValidLasFile(const QString& filePath);
    QString getLastError() const;

    // Sprint 1.3: Accessor methods for header information
    uint8_t getVersionMajor() const
    {
        return m_versionMajor;
    }
    uint8_t getVersionMinor() const
    {
        return m_versionMinor;
    }
    uint8_t getPointDataFormat() const
    {
        return m_pointFormat;
    }
    uint16_t getPointDataRecordLength() const
    {
        return m_pointDataRecordLength;
    }
    uint16_t getHeaderSize() const
    {
        return m_headerSize;
    }

public slots:
    void startParsing(const QString& filePath);
    void startParsing(const QString& filePath, const LoadingSettings& settings);

signals:
    void progressUpdated(int percentage, const QString& stage);
    void parsingFinished(bool success, const QString& message, const std::vector<float>& points);
    void headerParsed(const LasHeaderMetadata& metadata);

private:
    // Enhanced LAS file header structure for LAS 1.2-1.4 support (Sprint 1.3)
    struct LasHeader
    {
        char signature[4];  // "LASF"
        uint16_t fileSourceId;
        uint16_t globalEncoding;
        uint32_t guidData1;
        uint16_t guidData2;
        uint16_t guidData3;
        uint8_t guidData4[8];
        uint8_t versionMajor;  // Must be 1
        uint8_t versionMinor;  // 2, 3, or 4 supported (Sprint 1.3)
        char systemIdentifier[32];
        char generatingSoftware[32];
        uint16_t creationDayOfYear;
        uint16_t creationYear;
        uint16_t headerSize;
        uint32_t pointDataOffset;
        uint32_t numberOfVLRs;
        uint8_t pointDataFormat;  // 0-3 supported (Sprint 1.3)
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

        // LAS 1.3+ extensions (Sprint 1.3)
        uint64_t startOfWaveformData;  // LAS 1.3+

        // LAS 1.4+ extensions (Sprint 1.3)
        uint64_t startOfFirstEVLR;         // LAS 1.4+
        uint32_t numEVLRRecords;           // LAS 1.4+
        uint64_t numPointRecords64;        // LAS 1.4+
        uint64_t numPointsByReturn64[15];  // LAS 1.4+
    };

    // Point data format structures
    struct PointFormat0
    {
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

    struct PointFormat1
    {
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
    template <typename T>
    bool readValue(QFile& file, T& value);

    // Coordinate transformation helper
    void transformAndAddPoint(std::vector<float>& points, int32_t x, int32_t y, int32_t z, const LasHeader& header);

    // Progress update helper
    void updateProgressIfNeeded(uint32_t currentPoint, uint32_t totalPoints);

    // Error handling
    void setError(const QString& error);

    // Sprint 1.3: Enhanced validation helpers
    bool isVersionSupported(uint8_t major, uint8_t minor) const;
    uint16_t getExpectedRecordLength(uint8_t pointDataFormat) const;
    uint16_t getExpectedHeaderSize(uint8_t versionMinor) const;
    bool validateRecordLength(const LasHeader& header) const;
    bool validateScaleFactors(const LasHeader& header) const;

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

    // Sprint 1.3: Enhanced header information storage
    uint8_t m_versionMajor;
    uint8_t m_versionMinor;
    uint16_t m_pointDataRecordLength;
    uint16_t m_headerSize;

    // Header metadata for signals (simplified for testing)
    // Use Vector3D from lasheadermetadata.h to avoid conflicts
    Vector3D m_boundingBoxMin;
    Vector3D m_boundingBoxMax;
};

// Custom exception for LAS parsing errors
class LasParseException : public std::runtime_error
{
public:
    explicit LasParseException(const QString& message) : std::runtime_error(message.toStdString()) {}

    LasParseException(const QString& message, qint64 offset)
        : std::runtime_error((message + QString(" at offset %1").arg(offset)).toStdString())
    {
    }
};

#endif  // LASPARSER_H
