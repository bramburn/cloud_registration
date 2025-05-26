#ifndef E57PARSER_H
#define E57PARSER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <vector>
#include <stdexcept>

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

signals:
    void progressUpdated(int percentage);
    void parsingFinished(bool success, const QString& message);

private:
    // E57 file structure parsing
    bool parseHeader(QDataStream& stream);
    bool parseElementSection(QDataStream& stream);
    std::vector<float> parsePointData(QDataStream& stream, qint64 dataOffset, qint64 dataSize);
    
    // Helper functions
    bool readE57String(QDataStream& stream, QString& result);
    bool readE57Integer(QDataStream& stream, qint64& result);
    bool readE57Float(QDataStream& stream, double& result);
    bool skipBytes(QDataStream& stream, qint64 count);
    
    // Mock data generation for testing (temporary)
    std::vector<float> generateMockPointCloud();
    
    // Error handling
    void setError(const QString& error);
    
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
    
    // Point cloud metadata
    qint64 m_pointCount;
    bool m_hasXYZ;
    bool m_hasColor;
    bool m_hasIntensity;
};

// Custom exception for E57 parsing errors
class E57ParseException : public std::runtime_error
{
public:
    explicit E57ParseException(const QString& message)
        : std::runtime_error(message.toStdString()) {}
};

#endif // E57PARSER_H
