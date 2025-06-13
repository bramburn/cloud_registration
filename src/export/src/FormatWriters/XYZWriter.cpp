#include "export/FormatWriters/XYZWriter.h"
#include <QDebug>
#include <QFileInfo>

XYZWriter::XYZWriter()
    : pointsWritten_(0)
    , isOpen_(false)
{
    clearError();
}

XYZWriter::~XYZWriter()
{
    if (isOpen_) {
        close();
    }
}

bool XYZWriter::open(const QString& path)
{
    clearError();
    
    if (isOpen_) {
        setError("File already open");
        return false;
    }
    
    file_ = std::make_unique<QFile>(path);
    if (!file_->open(QIODevice::WriteOnly | QIODevice::Text)) {
        setError(QString("Failed to open file: %1").arg(file_->errorString()));
        return false;
    }
    
    stream_ = std::make_unique<QTextStream>(file_.get());
    isOpen_ = true;
    pointsWritten_ = 0;
    
    qDebug() << "XYZWriter: Opened file for writing:" << path;
    return true;
}

bool XYZWriter::writeHeader(const HeaderInfo& info)
{
    if (!isOpen_) {
        setError("File not open");
        return false;
    }
    
    headerInfo_ = info;
    
    // Write optional header comments for XYZ format
    *stream_ << "# XYZ Point Cloud Export\n";
    if (!info.projectName.isEmpty()) {
        *stream_ << "# Project: " << info.projectName << "\n";
    }
    if (!info.description.isEmpty()) {
        *stream_ << "# Description: " << info.description << "\n";
    }
    if (!info.coordinateSystem.isEmpty()) {
        *stream_ << "# Coordinate System: " << info.coordinateSystem << "\n";
    }
    *stream_ << "# Point Count: " << info.pointCount << "\n";
    *stream_ << "# Format: X Y Z\n";
    *stream_ << "#\n";
    
    qDebug() << "XYZWriter: Header written for" << info.pointCount << "points";
    return true;
}

bool XYZWriter::writePoint(const Point& point)
{
    if (!isOpen_) {
        setError("File not open");
        return false;
    }
    
    // Write point in simple XYZ format
    *stream_ << point.x << " " << point.y << " " << point.z << "\n";
    
    pointsWritten_++;
    return true;
}

bool XYZWriter::close()
{
    if (!isOpen_) {
        return true; // Already closed
    }
    
    // Write footer comment
    *stream_ << "# End of file - " << pointsWritten_ << " points written\n";
    
    stream_.reset();
    file_->close();
    file_.reset();
    
    isOpen_ = false;
    
    qDebug() << "XYZWriter: File closed," << pointsWritten_ << "points written";
    return true;
}

void XYZWriter::setError(const QString& error)
{
    lastError_ = error;
    qWarning() << "XYZWriter error:" << error;
}

void XYZWriter::clearError()
{
    lastError_.clear();
}
