#ifndef XYZWRITER_H
#define XYZWRITER_H

#include "../IFormatWriter.h"
#include <QFile>
#include <QTextStream>
#include <memory>

/**
 * @brief XYZ format writer implementation
 * 
 * Sprint 6 User Story 1: Multi-Format Point Cloud Export
 * Implements simple XYZ text format writing for basic point cloud data.
 */
class XYZWriter : public IFormatWriter
{
public:
    enum class Format {
        XYZ,        // x y z
        XYZI,       // x y z intensity
        XYZRGB,     // x y z r g b
        XYZIRGB     // x y z intensity r g b
    };
    
    XYZWriter(Format format = Format::XYZ);
    ~XYZWriter() override;
    
    // IFormatWriter interface
    bool open(const QString& path) override;
    bool writeHeader(const HeaderInfo& info) override;
    bool writePoint(const Point& point) override;
    bool close() override;
    
    QString getFileExtension() const override { return "xyz"; }
    QString getFormatDescription() const override { return "XYZ Text Format"; }
    bool supportsColor() const override { 
        return m_format == Format::XYZRGB || m_format == Format::XYZIRGB; 
    }
    bool supportsIntensity() const override { 
        return m_format == Format::XYZI || m_format == Format::XYZIRGB; 
    }
    
    // Configuration
    void setFormat(Format format) { m_format = format; }
    void setSeparator(const QString& separator) { m_separator = separator; }
    void setPrecision(int precision) { m_precision = precision; }
    void setWriteHeader(bool writeHeader) { m_writeHeaderComment = writeHeader; }
    
private:
    void writeHeaderComment();
    
    std::unique_ptr<QFile> m_file;
    std::unique_ptr<QTextStream> m_stream;
    HeaderInfo m_headerInfo;
    Format m_format = Format::XYZ;
    QString m_separator = " ";
    int m_precision = 6;
    bool m_writeHeaderComment = true;
    bool m_isOpen = false;
    size_t m_pointsWritten = 0;
};

#endif // XYZWRITER_H
