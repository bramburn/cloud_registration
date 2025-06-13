#include "export/FormatWriters/E57Writer.h"

#include <QDebug>
#include <QFileInfo>

E57Writer::E57Writer() : pointsWritten_(0), isOpen_(false)
{
    clearError();
}

E57Writer::~E57Writer()
{
    if (isOpen_)
    {
        close();
    }
}

bool E57Writer::open(const QString& path)
{
    clearError();

    if (isOpen_)
    {
        setError("File already open");
        return false;
    }

    file_ = std::make_unique<QFile>(path);
    if (!file_->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        setError(QString("Failed to open file: %1").arg(file_->errorString()));
        return false;
    }

    stream_ = std::make_unique<QTextStream>(file_.get());
    isOpen_ = true;
    pointsWritten_ = 0;

    qDebug() << "E57Writer: Opened file for writing:" << path;
    return true;
}

bool E57Writer::writeHeader(const HeaderInfo& info)
{
    if (!isOpen_)
    {
        setError("File not open");
        return false;
    }

    headerInfo_ = info;

    // Write E57-style header (simplified for Sprint 7)
    *stream_ << "# E57 Point Cloud Export (Simplified Format)\n";
    *stream_ << "# Project: " << info.projectName << "\n";
    *stream_ << "# Description: " << info.description << "\n";
    *stream_ << "# Coordinate System: " << info.coordinateSystem << "\n";
    *stream_ << "# Point Count: " << info.pointCount << "\n";
    *stream_ << "# Bounds: " << info.minX << "," << info.minY << "," << info.minZ << " to " << info.maxX << ","
             << info.maxY << "," << info.maxZ << "\n";
    *stream_ << "# Format: X Y Z Intensity R G B\n";
    *stream_ << "#\n";

    qDebug() << "E57Writer: Header written for" << info.pointCount << "points";
    return true;
}

bool E57Writer::writePoint(const Point& point)
{
    if (!isOpen_)
    {
        setError("File not open");
        return false;
    }

    // Write point in simplified E57 format
    *stream_ << point.x << " " << point.y << " " << point.z << " " << point.intensity << " "
             << static_cast<int>(point.r) << " " << static_cast<int>(point.g) << " " << static_cast<int>(point.b)
             << "\n";

    pointsWritten_++;
    return true;
}

bool E57Writer::close()
{
    if (!isOpen_)
    {
        return true;  // Already closed
    }

    // Write footer
    *stream_ << "# End of file - " << pointsWritten_ << " points written\n";

    stream_.reset();
    file_->close();
    file_.reset();

    isOpen_ = false;

    qDebug() << "E57Writer: File closed," << pointsWritten_ << "points written";
    return true;
}

void E57Writer::setError(const QString& error)
{
    lastError_ = error;
    qWarning() << "E57Writer error:" << error;
}

void E57Writer::clearError()
{
    lastError_.clear();
}
