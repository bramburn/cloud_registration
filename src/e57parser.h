#ifndef E57PARSER_H
#define E57PARSER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <QXmlStreamReader>
#include <QDomDocument>
#include <QVariantMap>
#include <vector>
#include <stdexcept>
// #include <E57Format.h>  // Commented out for testing without E57Format dependency

// E57 header structure according to ASTM E57 standard
struct E57Header {
    char signature[8];          // "ASTM-E57" file signature
    quint32 majorVersion;       // Major version number
    quint32 minorVersion;       // Minor version number
    quint64 filePhysicalLength; // Physical length of file
    quint64 xmlOffset;          // Offset to XML section
    quint64 xmlLength;          // Length of XML section
    quint64 pageSize;           // Page size (usually 1024)
};

// Sprint 2.1: Codec handling structures
struct CodecParams {
    QString type = "bitPackCodec";  // Default per ASTM E57
    QVariantMap parameters;
    bool isSupported = true;        // bitPackCodec is always supported
};

struct FieldDescriptor {
    QString name;                   // "cartesianX", "cartesianY", "cartesianZ"
    QString dataType;              // "Float", "Integer", "ScaledInteger"
    double minimum = 0.0;
    double maximum = 0.0;
    int precision = 64;            // bits (32 or 64 for floats)
    double scale = 1.0;            // for ScaledInteger
    double offset = 0.0;           // for ScaledInteger
    int byteStreamPosition = 0;    // position in compressed stream
};

struct CompressedVectorInfo {
    qint64 recordCount = 0;
    CodecParams codec;
    std::vector<FieldDescriptor> fields;
    qint64 binaryStartOffset = 0;
    qint64 binaryLength = 0;
};

class E57Parser : public QObject
{
    Q_OBJECT

public:
    explicit E57Parser(QObject *parent = nullptr);
    ~E57Parser();

    // Main parsing function
    std::vector<float> parse(const QString& filePath);

    // Utility functions
    bool isValidE57File(const QString& filePath);
    QString getLastError() const;

    // Public methods for testing (Sprint 1.2)
    bool parseData3D(const QDomElement& data3DElement);

public slots:
    void startParsing(const QString& filePath);

signals:
    void progressUpdated(int percentage, const QString &stage);
    void parsingFinished(bool success, const QString& message, const std::vector<float>& points);

private:
    // E57 file structure parsing
    bool parseHeader(QDataStream& stream);
    bool parseHeader(QFile& file);  // Enhanced header parsing with E57Header struct
    bool parseElementSection(QDataStream& stream);
    std::vector<float> parsePointData(QDataStream& stream, qint64 dataOffset, qint64 dataSize);

    // XML parsing methods
    bool parseXmlSection(QFile& file, qint64 xmlOffset, qint64 xmlLength);
    std::vector<float> extractPointsFromBinarySection(QFile& file, qint64 binaryOffset, qint64 recordCount);

    // Sprint 1.2: CompressedVector parsing methods
    bool parseCompressedVector(const QDomElement& pointsElement);
    bool parseCompressedVectorNode(const QDomElement& vectorNode);

    // Sprint 2.1: Enhanced codec parsing methods
    bool parseCompressedVectorWithCodec(const QDomElement& pointsElement, CompressedVectorInfo& vectorInfo);
    bool parseCodecsSection(const QDomElement& codecsElement, CodecParams& codec);
    bool parsePrototypeSection(const QDomElement& prototypeElement, std::vector<FieldDescriptor>& fields);
    bool parseFieldDescriptor(const QDomElement& fieldElement, FieldDescriptor& field);

    // Sprint 2.1: Decompression methods
    bool decompressWithBitPack(QDataStream& stream,
                              const CompressedVectorInfo& vectorInfo,
                              std::vector<float>& outPoints);

    // Sprint 2.1: Utility methods for bit manipulation
    quint64 readPackedBits(QDataStream& stream, int bitCount);
    double unpackFieldValue(quint64 packedValue, const FieldDescriptor& field);

    // Sprint 2.1: Error handling for codecs
    void reportCodecError(const QString& codecName);
    void reportDecompressionError(const QString& details);

    // Helper functions
    bool readE57String(QDataStream& stream, QString& result);
    bool readE57Integer(QDataStream& stream, qint64& result);
    bool readE57Float(QDataStream& stream, double& result);
    bool skipBytes(QDataStream& stream, qint64 count);

    // Mock data generation for testing (temporary)
    std::vector<float> generateMockPointCloud();

    // Error handling
    void setError(const QString& error);

    // Sprint 1.2: Enhanced error reporting with context
    void setDetailedError(const QDomElement& element, const QString& error, const QString& errorCode = QString());
    void setDetailedError(const QString& context, const QString& error, const QString& errorCode = QString());

    // Member variables
    QString m_lastError;
    bool m_hasError;

    // E57 file format constants
    static const quint32 E57_FILE_SIGNATURE;
    static const quint32 E57_MAJOR_VERSION;
    static const quint32 E57_MINOR_VERSION;

    // Parsing state
    qint64 m_fileSize;
    qint64 m_currentPosition;
    bool m_headerParsed;

    // E57 file structure data (from header)
    qint64 m_xmlOffset;
    qint64 m_xmlLength;
    qint64 m_filePhysicalLength;
    qint64 m_pageSize;

    // Point cloud metadata (from XML)
    qint64 m_pointCount;
    bool m_hasXYZ;
    bool m_hasColor;
    bool m_hasIntensity;
    QString m_pointDataType;    // Data type for coordinates (e.g., "single", "double")

    // Binary data information (from XML)
    qint64 m_binaryDataOffset;
    qint64 m_recordCount;

    // Sprint 2.1: Enhanced members for codec handling
    std::vector<CompressedVectorInfo> m_compressedVectors;
    bool m_hasUnsupportedCodec = false;
    QString m_unsupportedCodecName;
};

// Custom exception for E57 parsing errors
class E57ParseException : public std::runtime_error
{
public:
    explicit E57ParseException(const QString& message)
        : std::runtime_error(message.toStdString()) {}
};

#endif // E57PARSER_H
