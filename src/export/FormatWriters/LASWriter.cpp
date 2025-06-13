#include "LASWriter.h"

#include <QDateTime>
#include <QDebug>
#include <QUuid>

#include <cstring>

LASWriter::LASWriter() {}

LASWriter::~LASWriter()
{
    if (m_isOpen)
    {
        close();
    }
}

bool LASWriter::open(const QString& path)
{
    if (m_isOpen)
    {
        qWarning() << "LASWriter: File already open";
        return false;
    }

    m_file = std::make_unique<QFile>(path);
    if (!m_file->open(QIODevice::WriteOnly))
    {
        qWarning() << "LASWriter: Failed to open file:" << path;
        return false;
    }

    m_stream = std::make_unique<QDataStream>(m_file.get());
    m_stream->setByteOrder(QDataStream::LittleEndian);

    m_pointsWritten = 0;
    m_isOpen = true;

    qDebug() << "LASWriter: Opened file for writing:" << path;
    return true;
}

bool LASWriter::writeHeader(const HeaderInfo& info)
{
    if (!m_isOpen)
    {
        qWarning() << "LASWriter: File not open";
        return false;
    }

    m_headerInfo = info;

    // Initialize header with bounds
    m_header.minX = info.minX;
    m_header.maxX = info.maxX;
    m_header.minY = info.minY;
    m_header.maxY = info.maxY;
    m_header.minZ = info.minZ;
    m_header.maxZ = info.maxZ;
    m_header.numberOfPointRecords = static_cast<uint32_t>(info.pointCount);

    // Set creation date
    QDateTime now = QDateTime::currentDateTime();
    m_header.creationDayOfYear = static_cast<uint16_t>(now.date().dayOfYear());
    m_header.creationYear = static_cast<uint16_t>(now.date().year());

    // Generate GUID
    QUuid guid = QUuid::createUuid();
    m_header.guidData1 = guid.data1;
    m_header.guidData2 = guid.data2;
    m_header.guidData3 = guid.data3;
    memcpy(m_header.guidData4, guid.data4, 8);

    // Calculate scale factors and offsets for optimal precision
    double rangeX = m_header.maxX - m_header.minX;
    double rangeY = m_header.maxY - m_header.minY;
    double rangeZ = m_header.maxZ - m_header.minZ;

    // Use appropriate scale factors
    m_header.xScaleFactor = rangeX > 1000 ? 0.01 : 0.001;
    m_header.yScaleFactor = rangeY > 1000 ? 0.01 : 0.001;
    m_header.zScaleFactor = rangeZ > 1000 ? 0.01 : 0.001;

    // Set offsets to minimize coordinate values
    m_header.xOffset = m_header.minX;
    m_header.yOffset = m_header.minY;
    m_header.zOffset = m_header.minZ;

    m_headerPosition = m_file->pos();
    writeHeader();

    qDebug() << "LASWriter: Header written for" << info.pointCount << "points";
    return true;
}

bool LASWriter::writePoint(const Point& point)
{
    if (!m_isOpen)
    {
        qWarning() << "LASWriter: File not open";
        return false;
    }

    LASPointRecord record;

    // Scale coordinates
    record.x = scaleCoordinate(point.x, m_header.xScaleFactor, m_header.xOffset);
    record.y = scaleCoordinate(point.y, m_header.yScaleFactor, m_header.yOffset);
    record.z = scaleCoordinate(point.z, m_header.zScaleFactor, m_header.zOffset);

    // Convert intensity (0-1 float to 0-65535 uint16)
    record.intensity = static_cast<uint16_t>(point.intensity * 65535.0f);

    // Convert RGB (0-255 uint8 to 0-65535 uint16)
    record.red = static_cast<uint16_t>(point.r * 257);  // 257 = 65535/255
    record.green = static_cast<uint16_t>(point.g * 257);
    record.blue = static_cast<uint16_t>(point.b * 257);

    // Write point record
    m_stream->writeRawData(reinterpret_cast<const char*>(&record), sizeof(record));

    m_pointsWritten++;
    return true;
}

bool LASWriter::close()
{
    if (!m_isOpen)
    {
        return true;
    }

    // Update header with actual point count
    updateHeader();

    m_stream.reset();
    m_file->close();
    m_file.reset();
    m_isOpen = false;

    qDebug() << "LASWriter: File closed, wrote" << m_pointsWritten << "points";
    return true;
}

void LASWriter::writeHeader()
{
    m_stream->writeRawData(reinterpret_cast<const char*>(&m_header), sizeof(m_header));
}

void LASWriter::updateHeader()
{
    // Update point count in header
    m_header.numberOfPointRecords = m_pointsWritten;
    m_header.numberOfPointsByReturn[0] = m_pointsWritten;  // All points as first return

    // Save current position
    qint64 currentPos = m_file->pos();

    // Seek to header and rewrite
    m_file->seek(m_headerPosition);
    writeHeader();

    // Restore position
    m_file->seek(currentPos);
}

int32_t LASWriter::scaleCoordinate(double value, double scale, double offset)
{
    return static_cast<int32_t>((value - offset) / scale);
}
