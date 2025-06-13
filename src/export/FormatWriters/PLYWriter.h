#ifndef PLYWRITER_H
#define PLYWRITER_H

#include <QFile>
#include <QTextStream>

#include <memory>

#include "../IFormatWriter.h"

/**
 * @brief PLY format writer implementation
 *
 * Sprint 6 User Story 1: Multi-Format Point Cloud Export
 * Implements ASCII PLY format writing for polygon/point cloud data.
 */
class PLYWriter : public IFormatWriter
{
public:
    PLYWriter();
    ~PLYWriter() override;

    // IFormatWriter interface
    bool open(const QString& path) override;
    bool writeHeader(const HeaderInfo& info) override;
    bool writePoint(const Point& point) override;
    bool close() override;

    QString getFileExtension() const override
    {
        return "ply";
    }
    QString getFormatDescription() const override
    {
        return "PLY Polygon Format";
    }
    bool supportsColor() const override
    {
        return true;
    }
    bool supportsIntensity() const override
    {
        return true;
    }

private:
    void writePLYHeader();

    std::unique_ptr<QFile> m_file;
    std::unique_ptr<QTextStream> m_stream;
    HeaderInfo m_headerInfo;
    bool m_isOpen = false;
    bool m_headerWritten = false;
    size_t m_pointsWritten = 0;
};

#endif  // PLYWRITER_H
