#ifndef LASWRITER_H
#define LASWRITER_H

#include <QDataStream>
#include <QFile>

#include <memory>

#include "../IFormatWriter.h"

/**
 * @brief LAS format writer implementation
 *
 * Sprint 6 User Story 1: Multi-Format Point Cloud Export
 * This class implements LAS format export functionality.
 * For Sprint 7, this is a simplified implementation.
 */
class LASWriter : public IFormatWriter
{
public:
    LASWriter();
    virtual ~LASWriter();

    // IFormatWriter interface
    bool open(const QString& path) override;
    bool writeHeader(const HeaderInfo& info) override;
    bool writePoint(const Point& point) override;
    bool close() override;

    QString getFileExtension() const override
    {
        return "las";
    }
    QString getFormatDescription() const override
    {
        return "LAS Point Cloud Format";
    }
    bool supportsColor() const override
    {
        return true;
    }
    bool supportsIntensity() const override
    {
        return true;
    }

    // Error handling
    QString getLastError() const
    {
        return lastError_;
    }

private:
    std::unique_ptr<QFile> file_;
    std::unique_ptr<QDataStream> stream_;
    QString lastError_;
    HeaderInfo headerInfo_;
    size_t pointsWritten_;
    bool isOpen_;

    void setError(const QString& error);
    void clearError();
    bool writeLASHeader();
    void updateHeaderPointCount();
};

#endif  // LASWRITER_H
