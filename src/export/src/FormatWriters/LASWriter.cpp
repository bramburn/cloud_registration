#include "export/FormatWriters/LASWriter.h"

#include <QDebug>
#include <QFileInfo>

LASWriter::LASWriter() : pointsWritten_(0), isOpen_(false)
{
    clearError();
}

LASWriter::~LASWriter()
{
    if (isOpen_)
    {
        close();
    }
}

bool LASWriter::open(const QString& path)
{
    clearError();

    if (isOpen_)
    {
        setError("File already open");
        return false;
    }

    file_ = std::make_unique<QFile>(path);
    if (!file_->open(QIODevice::WriteOnly))
    {
        setError(QString("Failed to open file: %1").arg(file_->errorString()));
        return false;
    }

    stream_ = std::make_unique<QDataStream>(file_.get());
    stream_->setByteOrder(QDataStream::LittleEndian);

    isOpen_ = true;
    pointsWritten_ = 0;

    qDebug() << "LASWriter: Opened file for writing:" << path;
    return true;
}

bool LASWriter::writeHeader(const HeaderInfo& info)
{
    if (!isOpen_)
    {
        setError("File not open");
        return false;
    }

    headerInfo_ = info;

    if (!writeLASHeader())
    {
        return false;
    }

    qDebug() << "LASWriter: Header written for" << info.pointCount << "points";
    return true;
}

bool LASWriter::writePoint(const Point& point)
{
    if (!isOpen_)
    {
        setError("File not open");
        return false;
    }

    // Write LAS point record (simplified format)
    // Scale coordinates to integer values
    qint32 x = static_cast<qint32>(point.x * 1000);  // mm precision
    qint32 y = static_cast<qint32>(point.y * 1000);
    qint32 z = static_cast<qint32>(point.z * 1000);

    quint16 intensity = static_cast<quint16>(point.intensity * 65535);

    *stream_ << x << y << z << intensity;
    *stream_ << point.r << point.g << point.b;

    pointsWritten_++;
    return true;
}

bool LASWriter::close()
{
    if (!isOpen_)
    {
        return true;  // Already closed
    }

    // Update header with actual point count
    updateHeaderPointCount();

    stream_.reset();
    file_->close();
    file_.reset();

    isOpen_ = false;

    qDebug() << "LASWriter: File closed," << pointsWritten_ << "points written";
    return true;
}

bool LASWriter::writeLASHeader()
{
    // Write simplified LAS header
    QByteArray signature = "LASF";
    file_->write(signature);

    // File source ID
    *stream_ << static_cast<quint16>(0);

    // Global encoding
    *stream_ << static_cast<quint16>(0);

    // Project ID (16 bytes)
    for (int i = 0; i < 16; ++i)
    {
        *stream_ << static_cast<quint8>(0);
    }

    // Version
    *stream_ << static_cast<quint8>(1) << static_cast<quint8>(2);

    // System identifier (32 bytes)
    QByteArray systemId = "CloudRegistration";
    systemId.resize(32, '\0');
    file_->write(systemId);

    // Generating software (32 bytes)
    QByteArray software = "CloudRegistration Export";
    software.resize(32, '\0');
    file_->write(software);

    // Creation date
    *stream_ << static_cast<quint16>(1) << static_cast<quint16>(1);

    // Header size
    *stream_ << static_cast<quint16>(227);

    // Offset to point data
    *stream_ << static_cast<quint32>(227);

    // Number of variable length records
    *stream_ << static_cast<quint32>(0);

    // Point data format
    *stream_ << static_cast<quint8>(2);  // Format 2 (XYZ + intensity + RGB)

    // Point data record length
    *stream_ << static_cast<quint16>(26);

    // Number of points (will be updated in close())
    *stream_ << static_cast<quint32>(0);

    // Number of points by return
    for (int i = 0; i < 5; ++i)
    {
        *stream_ << static_cast<quint32>(0);
    }

    // Scale factors
    *stream_ << 0.001 << 0.001 << 0.001;  // 1mm precision

    // Offsets
    *stream_ << 0.0 << 0.0 << 0.0;

    // Min/Max coordinates
    *stream_ << headerInfo_.minX << headerInfo_.maxX;
    *stream_ << headerInfo_.minY << headerInfo_.maxY;
    *stream_ << headerInfo_.minZ << headerInfo_.maxZ;

    return true;
}

void LASWriter::updateHeaderPointCount()
{
    if (!file_)
        return;

    // Seek to point count location in header
    file_->seek(107);
    *stream_ << static_cast<quint32>(pointsWritten_);
}

void LASWriter::setError(const QString& error)
{
    lastError_ = error;
    qWarning() << "LASWriter error:" << error;
}

void LASWriter::clearError()
{
    lastError_.clear();
}
