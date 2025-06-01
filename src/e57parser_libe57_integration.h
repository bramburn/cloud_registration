// E57Parser libE57Format Integration - Header Sample
// This shows how the E57Parser class would be modified to use libE57Format

#ifndef E57PARSER_LIBE57_INTEGRATION_H
#define E57PARSER_LIBE57_INTEGRATION_H

#include <QObject>
#include <QString>
#include <vector>

// libE57Format includes
#include <E57Format.h>

class E57ParserLibE57Integration : public QObject
{
    Q_OBJECT

public:
    explicit E57ParserLibE57Integration(QObject *parent = nullptr);
    ~E57ParserLibE57Integration();

    // Main parsing function - now using libE57Format
    std::vector<float> parse(const QString& filePath);
    
    // Utility functions
    bool isValidE57File(const QString& filePath);
    QString getLastError() const;

public slots:
    void startParsing(const QString& filePath);

signals:
    void progressUpdated(int percentage);
    void parsingFinished(bool success, const QString& message, const std::vector<float>& points);

private:
    // libE57Format-based parsing methods
    bool parseWithLibE57Format(const QString& filePath);
    bool extractMetadata(e57::Reader& reader);
    std::vector<float> extractPointData(e57::Reader& reader, int scanIndex = 0);

    // Helper methods for different data types
    std::vector<float> extractUncompressedPoints(e57::Reader& reader, int scanIndex);
    std::vector<float> extractCompressedVectorPoints(e57::Reader& reader, int scanIndex);

    // Validation methods
    bool validateScanHeader(const e57::Data3D& scanHeader);
    bool validatePointFields(const e57::Data3D& scanHeader);

    // Error handling
    void setError(const QString& error);
    void handleE57Exception(const e57::E57Exception& e, const QString& context);

    // Member variables
    QString m_lastError;
    bool m_hasError;

    // Point cloud metadata (extracted from libE57Format)
    int64_t m_pointCount;
    bool m_hasXYZ;
    bool m_hasColor;
    bool m_hasIntensity;
    QString m_pointDataType;

    // Scan information
    int m_scanCount;
    std::vector<e57::Data3D> m_scanHeaders;

    // Configuration
    static const int64_t MAX_BUFFER_SIZE = 100000; // Points per read buffer
    static const int64_t MAX_POINTS_LIMIT = 10000000; // Safety limit
};

// Custom exception for E57 parsing errors
class E57ParseException : public std::runtime_error
{
public:
    explicit E57ParseException(const QString& message)
        : std::runtime_error(message.toStdString()) {}
};

#endif // E57PARSER_LIBE57_INTEGRATION_H
