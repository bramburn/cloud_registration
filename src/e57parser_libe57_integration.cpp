// E57Parser libE57Format Integration - Implementation Sample
// This shows how the E57Parser would be implemented using libE57Format

#include "e57parser_libe57_integration.h"
#include <QDebug>
#include <QFileInfo>
#include <algorithm>

E57ParserLibE57Integration::E57ParserLibE57Integration(QObject *parent)
    : QObject(parent)
    , m_hasError(false)
    , m_pointCount(0)
    , m_hasXYZ(false)
    , m_hasColor(false)
    , m_hasIntensity(false)
    , m_pointDataType("single")
    , m_scanCount(0)
{
}

E57ParserLibE57Integration::~E57ParserLibE57Integration()
{
}

std::vector<float> E57ParserLibE57Integration::parse(const QString& filePath)
{
    m_lastError.clear();
    m_hasError = false;

    qDebug() << "Parsing E57 file with libE57Format:" << filePath;

    // Check if file exists
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        setError("File does not exist: " + filePath);
        emit parsingFinished(false, getLastError(), std::vector<float>());
        return std::vector<float>();
    }

    if (!fileInfo.isReadable()) {
        setError("File is not readable: " + filePath);
        emit parsingFinished(false, getLastError(), std::vector<float>());
        return std::vector<float>();
    }

    try {
        if (parseWithLibE57Format(filePath)) {
            qDebug() << "Successfully parsed E57 file with libE57Format";
            emit parsingFinished(true, QString("Successfully loaded %1 points").arg(m_pointCount), std::vector<float>());
            return std::vector<float>(); // Placeholder - actual implementation would return points
        } else {
            emit parsingFinished(false, getLastError(), std::vector<float>());
            return std::vector<float>();
        }
    } catch (const e57::E57Exception& e) {
        handleE57Exception(e, "E57 file parsing");
        emit parsingFinished(false, getLastError(), std::vector<float>());
        return std::vector<float>();
    } catch (const std::exception& e) {
        setError(QString("Standard exception during E57 parsing: %1").arg(e.what()));
        emit parsingFinished(false, getLastError(), std::vector<float>());
        return std::vector<float>();
    }
}

bool E57ParserLibE57Integration::parseWithLibE57Format(const QString& filePath)
{
    try {
        // Open E57 file using libE57Format
        e57::Reader reader(filePath.toStdString());
        
        qDebug() << "E57 file opened successfully with libE57Format";
        
        // Extract metadata
        if (!extractMetadata(reader)) {
            return false;
        }
        
        // Extract point data (for now, just validate - actual extraction would be here)
        // std::vector<float> points = extractPointData(reader, 0);
        
        return true;
        
    } catch (const e57::E57Exception& e) {
        handleE57Exception(e, "libE57Format parsing");
        return false;
    }
}

bool E57ParserLibE57Integration::extractMetadata(e57::Reader& reader)
{
    try {
        // Get number of scans
        m_scanCount = reader.GetData3DCount();
        qDebug() << "Found" << m_scanCount << "scans in E57 file";
        
        if (m_scanCount == 0) {
            setError("No 3D data found in E57 file");
            return false;
        }
        
        // Read header for first scan
        e57::Data3D scanHeader;
        reader.ReadData3D(0, scanHeader);
        
        // Validate scan header
        if (!validateScanHeader(scanHeader)) {
            return false;
        }
        
        // Extract point count
        m_pointCount = scanHeader.pointsSize;
        qDebug() << "Scan 0 contains" << m_pointCount << "points";
        
        // Extract coordinate information
        m_hasXYZ = scanHeader.pointFields.cartesianXField && 
                   scanHeader.pointFields.cartesianYField && 
                   scanHeader.pointFields.cartesianZField;
        
        if (!m_hasXYZ) {
            setError("Scan does not contain required XYZ coordinates");
            return false;
        }
        
        // Extract color information
        m_hasColor = scanHeader.pointFields.colorRedField && 
                     scanHeader.pointFields.colorGreenField && 
                     scanHeader.pointFields.colorBlueField;
        
        // Extract intensity information
        m_hasIntensity = scanHeader.pointFields.intensityField;
        
        // Determine data type (simplified - libE57Format handles precision internally)
        m_pointDataType = "single"; // Default assumption
        
        qDebug() << "Metadata extracted successfully:";
        qDebug() << "  Points:" << m_pointCount;
        qDebug() << "  Has XYZ:" << m_hasXYZ;
        qDebug() << "  Has Color:" << m_hasColor;
        qDebug() << "  Has Intensity:" << m_hasIntensity;
        
        // Store scan header for later use
        m_scanHeaders.clear();
        m_scanHeaders.push_back(scanHeader);
        
        return true;
        
    } catch (const e57::E57Exception& e) {
        handleE57Exception(e, "metadata extraction");
        return false;
    }
}

bool E57ParserLibE57Integration::validateScanHeader(const e57::Data3D& scanHeader)
{
    // Validate point fields
    if (!validatePointFields(scanHeader)) {
        return false;
    }
    
    // Validate point count
    if (scanHeader.pointsSize <= 0) {
        setError("Invalid point count in scan header");
        return false;
    }
    
    if (scanHeader.pointsSize > MAX_POINTS_LIMIT) {
        setError(QString("Point count exceeds maximum limit: %1 > %2")
                .arg(scanHeader.pointsSize).arg(MAX_POINTS_LIMIT));
        return false;
    }
    
    return true;
}

bool E57ParserLibE57Integration::validatePointFields(const e57::Data3D& scanHeader)
{
    // Check for required XYZ fields
    if (!scanHeader.pointFields.cartesianXField ||
        !scanHeader.pointFields.cartesianYField ||
        !scanHeader.pointFields.cartesianZField) {
        setError("Scan missing required cartesian coordinate fields");
        return false;
    }
    
    return true;
}

void E57ParserLibE57Integration::handleE57Exception(const e57::E57Exception& e, const QString& context)
{
    QString errorMsg = QString("libE57Format error in %1: %2").arg(context, e.what());
    setError(errorMsg);
    qCritical() << errorMsg;
}

void E57ParserLibE57Integration::setError(const QString& error)
{
    m_lastError = error;
    m_hasError = true;
    qCritical() << "E57Parser Error:" << error;
}

QString E57ParserLibE57Integration::getLastError() const
{
    return m_lastError;
}

bool E57ParserLibE57Integration::isValidE57File(const QString& filePath)
{
    try {
        e57::Reader reader(filePath.toStdString());
        // If we can create a reader without exception, file is likely valid
        return reader.GetData3DCount() >= 0;
    } catch (const e57::E57Exception&) {
        return false;
    } catch (...) {
        return false;
    }
}

void E57ParserLibE57Integration::startParsing(const QString& filePath)
{
    // This slot is called from the worker thread
    try {
        std::vector<float> points = parse(filePath);
        // The parse() method already emits the parsingFinished signal
    } catch (const std::exception& e) {
        emit parsingFinished(false, QString("Error in startParsing: %1").arg(e.what()), std::vector<float>());
    }
}
