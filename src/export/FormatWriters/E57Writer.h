#ifndef E57WRITER_H
#define E57WRITER_H

#include "../IFormatWriter.h"
#include <QFile>
#include <QTextStream>
#include <memory>

/**
 * @brief E57 format writer implementation
 * 
 * Sprint 6 User Story 1: Multi-Format Point Cloud Export
 * Implements E57 format writing using industry-standard specifications.
 * This is a simplified implementation for MVP purposes.
 */
class E57Writer : public IFormatWriter
{
public:
    E57Writer();
    ~E57Writer() override;
    
    // IFormatWriter interface
    bool open(const QString& path) override;
    bool writeHeader(const HeaderInfo& info) override;
    bool writePoint(const Point& point) override;
    bool close() override;
    
    QString getFileExtension() const override { return "e57"; }
    QString getFormatDescription() const override { return "E57 Point Cloud Format"; }
    bool supportsColor() const override { return true; }
    bool supportsIntensity() const override { return true; }
    
private:
    void writeXMLHeader();
    void writeXMLFooter();
    void writeBinaryData();
    
    std::unique_ptr<QFile> m_file;
    std::unique_ptr<QTextStream> m_stream;
    HeaderInfo m_headerInfo;
    std::vector<Point> m_points;
    bool m_isOpen = false;
    size_t m_pointsWritten = 0;
};

#endif // E57WRITER_H
