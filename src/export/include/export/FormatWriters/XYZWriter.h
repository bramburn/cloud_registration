#ifndef XYZWRITER_H
#define XYZWRITER_H

#include <QFile>
#include <QTextStream>

#include <memory>

#include "../IFormatWriter.h"

/**
 * @brief XYZ format writer implementation
 *
 * Sprint 6 User Story 1: Multi-Format Point Cloud Export
 * This class implements simple XYZ format export functionality.
 */
class XYZWriter : public IFormatWriter
{
public:
    XYZWriter();
    virtual ~XYZWriter();

    // IFormatWriter interface
    bool open(const QString& path) override;
    bool writeHeader(const HeaderInfo& info) override;
    bool writePoint(const Point& point) override;
    bool close() override;

    QString getFileExtension() const override
    {
        return "xyz";
    }
    QString getFormatDescription() const override
    {
        return "XYZ Point Cloud Format";
    }
    bool supportsColor() const override
    {
        return false;
    }
    bool supportsIntensity() const override
    {
        return false;
    }

    // Error handling
    QString getLastError() const
    {
        return lastError_;
    }

private:
    std::unique_ptr<QFile> file_;
    std::unique_ptr<QTextStream> stream_;
    QString lastError_;
    HeaderInfo headerInfo_;
    size_t pointsWritten_;
    bool isOpen_;

    void setError(const QString& error);
    void clearError();
};

#endif  // XYZWRITER_H
