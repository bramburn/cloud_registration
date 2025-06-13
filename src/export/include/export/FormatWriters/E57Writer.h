#ifndef E57WRITER_H
#define E57WRITER_H

#include <QFile>
#include <QTextStream>

#include <memory>

#include "../IFormatWriter.h"

/**
 * @brief E57 format writer implementation
 *
 * Sprint 6 User Story 1: Multi-Format Point Cloud Export
 * This class implements E57 format export functionality.
 * For Sprint 7, this is a stub implementation that will be enhanced
 * with actual E57 library integration in future sprints.
 */
class E57Writer : public IFormatWriter
{
public:
    E57Writer();
    virtual ~E57Writer();

    // IFormatWriter interface
    bool open(const QString& path) override;
    bool writeHeader(const HeaderInfo& info) override;
    bool writePoint(const Point& point) override;
    bool close() override;

    QString getFileExtension() const override
    {
        return "e57";
    }
    QString getFormatDescription() const override
    {
        return "E57 Point Cloud Format";
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
    std::unique_ptr<QTextStream> stream_;
    QString lastError_;
    HeaderInfo headerInfo_;
    size_t pointsWritten_;
    bool isOpen_;

    void setError(const QString& error);
    void clearError();
};

#endif  // E57WRITER_H
