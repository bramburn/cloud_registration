#ifndef PLYWRITER_H
#define PLYWRITER_H

#include "../IFormatWriter.h"
#include <QFile>
#include <QTextStream>
#include <memory>

/**
 * @brief PLY format writer implementation
 * 
 * Sprint 6 User Story 1: Multi-Format Point Cloud Export
 * This class implements PLY format export functionality.
 */
class PLYWriter : public IFormatWriter
{
public:
    PLYWriter();
    virtual ~PLYWriter();

    // IFormatWriter interface
    bool open(const QString& path) override;
    bool writeHeader(const HeaderInfo& info) override;
    bool writePoint(const Point& point) override;
    bool close() override;
    
    QString getFileExtension() const override { return "ply"; }
    QString getFormatDescription() const override { return "PLY Polygon File Format"; }
    bool supportsColor() const override { return true; }
    bool supportsIntensity() const override { return true; }
    
    // Error handling
    QString getLastError() const { return lastError_; }

private:
    std::unique_ptr<QFile> file_;
    std::unique_ptr<QTextStream> stream_;
    QString lastError_;
    HeaderInfo headerInfo_;
    size_t pointsWritten_;
    bool isOpen_;
    bool headerWritten_;
    
    void setError(const QString& error);
    void clearError();
};

#endif // PLYWRITER_H
