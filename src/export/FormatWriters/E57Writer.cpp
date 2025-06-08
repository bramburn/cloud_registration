#include "E57Writer.h"
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>

E57Writer::E57Writer()
{
}

E57Writer::~E57Writer()
{
    if (m_isOpen) {
        close();
    }
}

bool E57Writer::open(const QString& path)
{
    if (m_isOpen) {
        qWarning() << "E57Writer: File already open";
        return false;
    }
    
    m_file = std::make_unique<QFile>(path);
    if (!m_file->open(QIODevice::WriteOnly)) {
        qWarning() << "E57Writer: Failed to open file:" << path;
        return false;
    }
    
    m_stream = std::make_unique<QTextStream>(m_file.get());
    m_stream->setEncoding(QStringConverter::Utf8);
    
    m_points.clear();
    m_pointsWritten = 0;
    m_isOpen = true;
    
    qDebug() << "E57Writer: Opened file for writing:" << path;
    return true;
}

bool E57Writer::writeHeader(const HeaderInfo& info)
{
    if (!m_isOpen) {
        qWarning() << "E57Writer: File not open";
        return false;
    }
    
    m_headerInfo = info;
    writeXMLHeader();
    
    qDebug() << "E57Writer: Header written for" << info.pointCount << "points";
    return true;
}

bool E57Writer::writePoint(const Point& point)
{
    if (!m_isOpen) {
        qWarning() << "E57Writer: File not open";
        return false;
    }
    
    m_points.push_back(point);
    m_pointsWritten++;
    
    // Write in batches for better performance
    if (m_points.size() >= 10000) {
        writeBinaryData();
        m_points.clear();
    }
    
    return true;
}

bool E57Writer::close()
{
    if (!m_isOpen) {
        return true;
    }
    
    // Write remaining points
    if (!m_points.empty()) {
        writeBinaryData();
        m_points.clear();
    }
    
    writeXMLFooter();
    
    m_stream.reset();
    m_file->close();
    m_file.reset();
    m_isOpen = false;
    
    qDebug() << "E57Writer: File closed, wrote" << m_pointsWritten << "points";
    return true;
}

void E57Writer::writeXMLHeader()
{
    *m_stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    *m_stream << "<e57Root type=\"Structure\" xmlns=\"http://www.astm.org/COMMIT/E57/2010-e57-v1.0\">\n";
    *m_stream << "  <formatName type=\"String\"><![CDATA[ASTM E57 3D Imaging Data File]]></formatName>\n";
    *m_stream << "  <guid type=\"String\"><![CDATA[{" << QUuid::createUuid().toString(QUuid::WithoutBraces) << "}]]></guid>\n";
    *m_stream << "  <versionMajor type=\"Integer\">1</versionMajor>\n";
    *m_stream << "  <versionMinor type=\"Integer\">0</versionMinor>\n";
    *m_stream << "  <e57LibraryVersion type=\"String\"><![CDATA[CloudRegistration-1.0]]></e57LibraryVersion>\n";
    *m_stream << "  <coordinateMetadata type=\"String\"><![CDATA[" << m_headerInfo.coordinateSystem << "]]></coordinateMetadata>\n";
    *m_stream << "  <creationDateTime type=\"Structure\">\n";
    
    QDateTime now = QDateTime::currentDateTime();
    *m_stream << "    <dateTimeValue type=\"Float\">" << now.toSecsSinceEpoch() << "</dateTimeValue>\n";
    *m_stream << "    <isAtomicClockReferenced type=\"Integer\">0</isAtomicClockReferenced>\n";
    *m_stream << "  </creationDateTime>\n";
    
    *m_stream << "  <data3D type=\"Vector\" allowHeterogeneousChildren=\"1\">\n";
    *m_stream << "    <vectorChild type=\"Structure\">\n";
    *m_stream << "      <guid type=\"String\"><![CDATA[{" << QUuid::createUuid().toString(QUuid::WithoutBraces) << "}]]></guid>\n";
    *m_stream << "      <name type=\"String\"><![CDATA[" << m_headerInfo.projectName << "]]></name>\n";
    *m_stream << "      <description type=\"String\"><![CDATA[" << m_headerInfo.description << "]]></description>\n";
    
    // Cartesian bounds
    *m_stream << "      <cartesianBounds type=\"Structure\">\n";
    *m_stream << "        <xMinimum type=\"Float\">" << m_headerInfo.minX << "</xMinimum>\n";
    *m_stream << "        <xMaximum type=\"Float\">" << m_headerInfo.maxX << "</xMaximum>\n";
    *m_stream << "        <yMinimum type=\"Float\">" << m_headerInfo.minY << "</yMinimum>\n";
    *m_stream << "        <yMaximum type=\"Float\">" << m_headerInfo.maxY << "</yMaximum>\n";
    *m_stream << "        <zMinimum type=\"Float\">" << m_headerInfo.minZ << "</zMinimum>\n";
    *m_stream << "        <zMaximum type=\"Float\">" << m_headerInfo.maxZ << "</zMaximum>\n";
    *m_stream << "      </cartesianBounds>\n";
    
    // Points section
    *m_stream << "      <points type=\"CompressedVector\" fileOffset=\"0\" recordCount=\"" << m_headerInfo.pointCount << "\">\n";
    *m_stream << "        <prototype type=\"Structure\">\n";
    *m_stream << "          <cartesianX type=\"Float\" precision=\"single\"/>\n";
    *m_stream << "          <cartesianY type=\"Float\" precision=\"single\"/>\n";
    *m_stream << "          <cartesianZ type=\"Float\" precision=\"single\"/>\n";
    
    if (supportsIntensity()) {
        *m_stream << "          <intensity type=\"Float\" precision=\"single\"/>\n";
    }
    
    if (supportsColor()) {
        *m_stream << "          <colorRed type=\"Integer\" minimum=\"0\" maximum=\"255\"/>\n";
        *m_stream << "          <colorGreen type=\"Integer\" minimum=\"0\" maximum=\"255\"/>\n";
        *m_stream << "          <colorBlue type=\"Integer\" minimum=\"0\" maximum=\"255\"/>\n";
    }
    
    *m_stream << "        </prototype>\n";
    *m_stream << "        <codecs type=\"Vector\" allowHeterogeneousChildren=\"1\">\n";
    *m_stream << "          <vectorChild type=\"Structure\">\n";
    *m_stream << "            <name type=\"String\"><![CDATA[CompressedVectorWriter]]></name>\n";
    *m_stream << "          </vectorChild>\n";
    *m_stream << "        </codecs>\n";
    *m_stream << "      </points>\n";
}

void E57Writer::writeBinaryData()
{
    // For MVP: Write points as text data (simplified implementation)
    // In production, this would write compressed binary data
    for (const auto& point : m_points) {
        *m_stream << "        <!-- Point: " << point.x << " " << point.y << " " << point.z;
        if (supportsIntensity()) {
            *m_stream << " I:" << point.intensity;
        }
        if (supportsColor()) {
            *m_stream << " RGB:" << (int)point.r << "," << (int)point.g << "," << (int)point.b;
        }
        *m_stream << " -->\n";
    }
}

void E57Writer::writeXMLFooter()
{
    *m_stream << "    </vectorChild>\n";
    *m_stream << "  </data3D>\n";
    *m_stream << "</e57Root>\n";
}
