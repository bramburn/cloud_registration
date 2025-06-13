#include "PLYWriter.h"

#include <QDebug>

PLYWriter::PLYWriter() {}

PLYWriter::~PLYWriter()
{
    if (m_isOpen)
    {
        close();
    }
}

bool PLYWriter::open(const QString& path)
{
    if (m_isOpen)
    {
        qWarning() << "PLYWriter: File already open";
        return false;
    }

    m_file = std::make_unique<QFile>(path);
    if (!m_file->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qWarning() << "PLYWriter: Failed to open file:" << path;
        return false;
    }

    m_stream = std::make_unique<QTextStream>(m_file.get());
    m_stream->setEncoding(QStringConverter::Utf8);

    m_pointsWritten = 0;
    m_headerWritten = false;
    m_isOpen = true;

    qDebug() << "PLYWriter: Opened file for writing:" << path;
    return true;
}

bool PLYWriter::writeHeader(const HeaderInfo& info)
{
    if (!m_isOpen)
    {
        qWarning() << "PLYWriter: File not open";
        return false;
    }

    m_headerInfo = info;
    writePLYHeader();
    m_headerWritten = true;

    qDebug() << "PLYWriter: Header written for" << info.pointCount << "points";
    return true;
}

bool PLYWriter::writePoint(const Point& point)
{
    if (!m_isOpen)
    {
        qWarning() << "PLYWriter: File not open";
        return false;
    }

    if (!m_headerWritten)
    {
        qWarning() << "PLYWriter: Header not written";
        return false;
    }

    // Write point data: x y z [intensity] [r g b]
    *m_stream << point.x << " " << point.y << " " << point.z;

    if (supportsIntensity())
    {
        // Convert intensity to 0-255 range for PLY
        int intensityValue = static_cast<int>(point.intensity * 255.0f);
        *m_stream << " " << intensityValue;
    }

    if (supportsColor())
    {
        *m_stream << " " << static_cast<int>(point.r) << " " << static_cast<int>(point.g) << " "
                  << static_cast<int>(point.b);
    }

    *m_stream << "\n";

    m_pointsWritten++;
    return true;
}

bool PLYWriter::close()
{
    if (!m_isOpen)
    {
        return true;
    }

    m_stream.reset();
    m_file->close();
    m_file.reset();
    m_isOpen = false;

    qDebug() << "PLYWriter: File closed, wrote" << m_pointsWritten << "points";
    return true;
}

void PLYWriter::writePLYHeader()
{
    *m_stream << "ply\n";
    *m_stream << "format ascii 1.0\n";
    *m_stream << "comment Created by CloudRegistration\n";

    if (!m_headerInfo.projectName.isEmpty())
    {
        *m_stream << "comment Project: " << m_headerInfo.projectName << "\n";
    }

    if (!m_headerInfo.description.isEmpty())
    {
        *m_stream << "comment Description: " << m_headerInfo.description << "\n";
    }

    if (!m_headerInfo.coordinateSystem.isEmpty())
    {
        *m_stream << "comment Coordinate System: " << m_headerInfo.coordinateSystem << "\n";
    }

    *m_stream << "element vertex " << m_headerInfo.pointCount << "\n";
    *m_stream << "property float x\n";
    *m_stream << "property float y\n";
    *m_stream << "property float z\n";

    if (supportsIntensity())
    {
        *m_stream << "property uchar intensity\n";
    }

    if (supportsColor())
    {
        *m_stream << "property uchar red\n";
        *m_stream << "property uchar green\n";
        *m_stream << "property uchar blue\n";
    }

    *m_stream << "end_header\n";
}
