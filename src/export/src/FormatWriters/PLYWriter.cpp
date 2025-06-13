#include "export/FormatWriters/PLYWriter.h"
#include <QDebug>
#include <QFileInfo>

PLYWriter::PLYWriter()
    : pointsWritten_(0)
    , isOpen_(false)
    , headerWritten_(false)
{
    clearError();
}

PLYWriter::~PLYWriter()
{
    if (isOpen_) {
        close();
    }
}

bool PLYWriter::open(const QString& path)
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
    headerWritten_ = false;
    
    qDebug() << "PLYWriter: Opened file for writing:" << path;
    return true;
}

bool PLYWriter::writeHeader(const HeaderInfo& info)
{
    if (!isOpen_) {
        setError("File not open");
        return false;
    }
    
    if (headerWritten_) {
        setError("Header already written");
        return false;
    }
    
    headerInfo_ = info;
    
    // Write PLY header
    *stream_ << "ply\n";
    *stream_ << "format ascii 1.0\n";
    *stream_ << "comment Created by CloudRegistration\n";
    
    if (!info.projectName.isEmpty()) {
        *stream_ << "comment Project: " << info.projectName << "\n";
    }
    if (!info.description.isEmpty()) {
        *stream_ << "comment Description: " << info.description << "\n";
    }
    if (!info.coordinateSystem.isEmpty()) {
        *stream_ << "comment Coordinate System: " << info.coordinateSystem << "\n";
    }
    
    *stream_ << "element vertex " << info.pointCount << "\n";
    *stream_ << "property float x\n";
    *stream_ << "property float y\n";
    *stream_ << "property float z\n";
    *stream_ << "property float intensity\n";
    *stream_ << "property uchar red\n";
    *stream_ << "property uchar green\n";
    *stream_ << "property uchar blue\n";
    *stream_ << "end_header\n";
    
    headerWritten_ = true;
    
    qDebug() << "PLYWriter: Header written for" << info.pointCount << "points";
    return true;
}

bool PLYWriter::writePoint(const Point& point)
{
    if (!isOpen_) {
        setError("File not open");
        return false;
    }
    
    if (!headerWritten_) {
        setError("Header not written");
        return false;
    }
    
    // Write point in PLY ASCII format
    *stream_ << point.x << " " << point.y << " " << point.z << " " 
             << point.intensity << " " 
             << static_cast<int>(point.r) << " " 
             << static_cast<int>(point.g) << " " 
             << static_cast<int>(point.b) << "\n";
    
    pointsWritten_++;
    return true;
}

bool PLYWriter::close()
{
    if (!isOpen_) {
        return true; // Already closed
    }
    
    stream_.reset();
    file_->close();
    file_.reset();
    
    isOpen_ = false;
    headerWritten_ = false;
    
    qDebug() << "PLYWriter: File closed," << pointsWritten_ << "points written";
    return true;
}

void PLYWriter::setError(const QString& error)
{
    lastError_ = error;
    qWarning() << "PLYWriter error:" << error;
}

void PLYWriter::clearError()
{
    lastError_.clear();
}
