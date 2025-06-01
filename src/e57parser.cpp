#include "e57parser.h"
#include "performance_profiler.h"
#include <QDebug>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QXmlStreamReader>
#include <QDomDocument>
#include <cmath>
#include <cstring>
// #include <E57Format.h>  // Commented out for testing without E57Format dependency

// E57 file format constants
const quint32 E57Parser::E57_FILE_SIGNATURE = 0x41535446; // "ASTF" in little-endian
const quint32 E57Parser::E57_MAJOR_VERSION = 1;
const quint32 E57Parser::E57_MINOR_VERSION = 0;

E57Parser::E57Parser(QObject *parent)
    : QObject(parent)
    , m_hasError(false)
    , m_fileSize(0)
    , m_currentPosition(0)
    , m_headerParsed(false)
    , m_xmlOffset(0)
    , m_xmlLength(0)
    , m_filePhysicalLength(0)
    , m_pageSize(0)
    , m_pointCount(0)
    , m_hasXYZ(true)
    , m_hasColor(false)
    , m_hasIntensity(false)
    , m_pointDataType("single")
    , m_binaryDataOffset(0)
    , m_recordCount(0)
{
}

E57Parser::~E57Parser()
{
}

std::vector<float> E57Parser::parse(const QString& filePath)
{
    PROFILE_FUNCTION();

    m_lastError.clear();
    m_hasError = false;

    qDebug() << "Attempting to parse E57 file:" << filePath;

    // Check if file exists
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        setError("File does not exist: " + filePath);
        throw E57ParseException(m_lastError);
    }

    if (!fileInfo.isReadable()) {
        setError("File is not readable: " + filePath);
        throw E57ParseException(m_lastError);
    }

    m_fileSize = fileInfo.size();

    // For Sprint 1, we'll implement a simplified parser that can handle basic E57 files
    // or generate mock data for testing purposes

    QFile file(filePath);
    {
        PROFILE_SECTION("E57::FileOpen");
        if (!file.open(QIODevice::ReadOnly)) {
            setError("Failed to open file: " + file.errorString());
            throw E57ParseException(m_lastError);
        }
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Task 1.3.2: Remove mock data fallbacks - parse E57 file or fail with error
    try {
        if (isValidE57File(filePath)) {
            qDebug() << "Detected valid E57 file, attempting to parse...";
            emit progressUpdated(1, "Initializing...");

            // Task 1.1.1: Use enhanced header parsing
            {
                PROFILE_SECTION("E57::HeaderParse");
                emit progressUpdated(5, "Reading E57 header...");
                if (!parseHeader(file)) {
                    setError("Failed to parse E57 header");
                    emit parsingFinished(false, getLastError(), std::vector<float>());
                    return std::vector<float>();
                }
                emit progressUpdated(10, "Header parsed successfully");
            }

            // Task 1.1.2: Parse XML section to extract point cloud metadata
            {
                PROFILE_SECTION("E57::XMLParse");
                emit progressUpdated(15, "Parsing XML structure...");
                if (!parseXmlSection(file, m_xmlOffset, m_xmlLength)) {
                    setError("Failed to parse E57 XML section: " + getLastError());
                    emit parsingFinished(false, getLastError(), std::vector<float>());
                    return std::vector<float>();
                }
                emit progressUpdated(25, "XML structure parsed");
            }

            // Extract actual point data from binary section
            if (m_recordCount > 0 && m_binaryDataOffset > 0) {
                qDebug() << "=== ATTEMPTING REAL E57 POINT EXTRACTION ===";
                qDebug() << "Extracting" << m_recordCount << "points from binary section at offset" << m_binaryDataOffset;
                emit progressUpdated(30, "Reading point cloud data...");
                std::vector<float> points;
                {
                    PROFILE_SECTION("E57::BinaryDataExtraction");
                    points = extractPointsFromBinarySection(file, m_binaryDataOffset, m_recordCount);
                }

                if (!points.empty()) {
                    qDebug() << "=== SUCCESS: REAL E57 DATA EXTRACTED ===";
                    qDebug() << "Successfully extracted" << (points.size() / 3) << "points from E57 file";
                    qDebug() << "This is ACTUAL point cloud data, not mock data!";

                    // Log sample coordinates to verify real data validity (User Story 1)
                    if (points.size() >= 9) {
                        qDebug() << "Sample real E57 coordinates - First point:" << points[0] << points[1] << points[2];
                        if (points.size() >= 6) {
                            size_t midIndex = (points.size() / 6) * 3; // Middle point
                            if (midIndex + 2 < points.size()) {
                                qDebug() << "Sample real E57 coordinates - Middle point:" << points[midIndex] << points[midIndex + 1] << points[midIndex + 2];
                            }
                        }
                        size_t lastIndex = points.size() - 3;
                        qDebug() << "Sample real E57 coordinates - Last point:" << points[lastIndex] << points[lastIndex + 1] << points[lastIndex + 2];
                    }

                    emit progressUpdated(100, "Loading complete");
                    emit parsingFinished(true, QString("Successfully loaded %1 points from E57 file").arg(points.size() / 3), points);
                    return points;
                } else {
                    // Task 1.3.2: No mock data fallback - report error instead
                    setError("Failed to extract points from binary section");
                    emit parsingFinished(false, getLastError(), std::vector<float>());
                    return std::vector<float>();
                }
            } else {
                // Task 1.3.2: No mock data fallback - report error instead
                setError(QString("Invalid point data parameters - Record count: %1, Binary offset: %2")
                        .arg(m_recordCount).arg(m_binaryDataOffset));
                emit parsingFinished(false, getLastError(), std::vector<float>());
                return std::vector<float>();
            }

        } else {
            // Task 1.3.2: No mock data fallback - report error for invalid files
            setError("File is not a valid E57 file");
            emit parsingFinished(false, getLastError(), std::vector<float>());
            return std::vector<float>();
        }
    } catch (const std::exception& e) {
        // Task 1.3.2: No mock data fallback - report parsing errors
        setError(QString("E57 parsing failed: %1").arg(e.what()));
        emit parsingFinished(false, getLastError(), std::vector<float>());
        return std::vector<float>();
    }
}

void E57Parser::startParsing(const QString& filePath)
{
    // This slot is called from the worker thread
    try {
        std::vector<float> points = parse(filePath);
        // The parse() method already emits the parsingFinished signal
    } catch (const std::exception& e) {
        emit parsingFinished(false, QString("Error in startParsing: %1").arg(e.what()), std::vector<float>());
    }
}

bool E57Parser::isValidE57File(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // Check file signature - E57 files start with "ASTM-E57" (8 bytes)
    char signature[8];
    qint64 bytesRead = file.read(signature, 8);

    if (bytesRead != 8) {
        return false;
    }

    if (memcmp(signature, "ASTM-E57", 8) != 0) {
        return false;
    }

    // Check version numbers
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    quint32 majorVersion, minorVersion;
    stream >> majorVersion >> minorVersion;

    // Accept version 1.0 and potentially compatible versions
    if (majorVersion != E57_MAJOR_VERSION) {
        qWarning() << "Unsupported E57 major version:" << majorVersion;
        return false;
    }

    qDebug() << "Valid E57 file detected - Version:" << majorVersion << "." << minorVersion;
    return true;
}

QString E57Parser::getLastError() const
{
    // DEFENSIVE: Add debugging and validation to catch heap corruption
    try {
        // Check if m_lastError is in a valid state
        if (m_lastError.isNull()) {
            qWarning() << "E57Parser::getLastError() - m_lastError is null";
            return QString("Error: m_lastError is null");
        }

        // Try to access the string data safely
        QString result = m_lastError;
        qDebug() << "E57Parser::getLastError() returning:" << result.left(100) << "...";
        return result;

    } catch (const std::exception& e) {
        qCritical() << "E57Parser::getLastError() - Standard exception:" << e.what();
        return QString("Error: Exception in getLastError() - %1").arg(e.what());
    } catch (...) {
        qCritical() << "E57Parser::getLastError() - Unknown exception caught";
        return QString("Error: Unknown exception in getLastError()");
    }
}

bool E57Parser::parseHeader(QDataStream& stream)
{
    try {
        // Skip signature (already checked)
        stream.device()->seek(4);

        quint32 majorVersion, minorVersion;
        stream >> majorVersion >> minorVersion;

        qDebug() << "E57 Version:" << majorVersion << "." << minorVersion;

        // Read file physical length
        stream >> m_filePhysicalLength;

        qDebug() << "File physical length:" << m_filePhysicalLength;

        // Read XML length and offset
        stream >> m_xmlLength >> m_xmlOffset;

        qDebug() << "XML section - Length:" << m_xmlLength << "Offset:" << m_xmlOffset;

        m_headerParsed = true;
        return true;

    } catch (const std::exception& e) {
        setError("Failed to parse E57 header: " + QString(e.what()));
        return false;
    }
}

// Enhanced header parsing method (Task 1.1.1)
bool E57Parser::parseHeader(QFile& file)
{
    qDebug() << "=== E57Parser::parseHeader (Enhanced) ===";

    if (!file.seek(0)) {
        setError("Failed to seek to beginning of file");
        return false;
    }

    E57Header header;
    qint64 bytesRead = file.read(reinterpret_cast<char*>(&header), sizeof(E57Header));

    if (bytesRead != sizeof(E57Header)) {
        setError(QString("Failed to read complete E57 header. Expected %1 bytes, got %2")
                .arg(sizeof(E57Header)).arg(bytesRead));
        return false;
    }

    // Validate signature
    if (memcmp(header.signature, "ASTM-E57", 8) != 0) {
        setError("Invalid E57 file signature");
        return false;
    }

    // Validate version
    if (header.majorVersion != E57_MAJOR_VERSION) {
        setError(QString("Unsupported E57 major version: %1").arg(header.majorVersion));
        return false;
    }

    // Store header data in member variables
    m_filePhysicalLength = header.filePhysicalLength;
    m_xmlOffset = header.xmlOffset;
    m_xmlLength = header.xmlLength;
    m_pageSize = header.pageSize;

    qDebug() << "E57 Header parsed successfully:";
    qDebug() << "  Version:" << header.majorVersion << "." << header.minorVersion;
    qDebug() << "  File physical length:" << m_filePhysicalLength;
    qDebug() << "  XML offset:" << m_xmlOffset;
    qDebug() << "  XML length:" << m_xmlLength;
    qDebug() << "  Page size:" << m_pageSize;

    // Validate header fields
    if (m_xmlOffset <= 0 || m_xmlLength <= 0) {
        setError("Invalid XML section parameters in header");
        return false;
    }

    if (m_xmlOffset + m_xmlLength > m_filePhysicalLength) {
        setError("XML section extends beyond file length");
        return false;
    }

    m_headerParsed = true;
    return true;
}

bool E57Parser::parseElementSection(QDataStream& /*stream*/)
{
    // This is a simplified implementation
    // A full E57 parser would need to parse the XML section to understand the data structure
    qDebug() << "Parsing element section (simplified implementation)";

    // For now, just return success
    // TODO: Implement full XML parsing and data extraction
    return true;
}

std::vector<float> E57Parser::parsePointData(QDataStream& /*stream*/, qint64 dataOffset, qint64 dataSize)
{
    std::vector<float> points;

    // This would contain the actual binary point data parsing
    // For now, return empty vector
    qDebug() << "Parsing point data at offset" << dataOffset << "size" << dataSize;

    return points;
}

std::vector<float> E57Parser::generateMockPointCloud()
{
    // Debug logging for mock data generation (User Story 1)
    qDebug() << "=== E57Parser::generateMockPointCloud ===";
    qDebug() << "Generating mock point cloud for testing";
    qDebug() << "NOTE: This is mock data - E57 parsing is not fully implemented yet";

    std::vector<float> points;
    const int numPoints = 10000; // Generate 10,000 points
    qDebug() << "Target number of mock points:" << numPoints;

    QRandomGenerator* rng = QRandomGenerator::global();

    // Generate a simple geometric shape (sphere with some noise)
    for (int i = 0; i < numPoints; ++i) {
        // Generate points on a sphere with radius 1.0
        float theta = rng->generateDouble() * 2.0f * M_PI; // Azimuth angle
        float phi = rng->generateDouble() * M_PI; // Polar angle
        float radius = 0.8f + rng->generateDouble() * 0.4f; // Add some variation

        float x = radius * sin(phi) * cos(theta);
        float y = radius * sin(phi) * sin(theta);
        float z = radius * cos(phi);

        // Add some noise
        x += (rng->generateDouble() - 0.5f) * 0.1f;
        y += (rng->generateDouble() - 0.5f) * 0.1f;
        z += (rng->generateDouble() - 0.5f) * 0.1f;

        points.push_back(x);
        points.push_back(y);
        points.push_back(z);

        // Update progress every 1000 points
        if (i % 1000 == 0) {
            int progress = static_cast<int>((i * 100) / numPoints);
            emit progressUpdated(progress, QString("Generating mock data: %1/%2 points").arg(i).arg(numPoints));
        }
    }

    qDebug() << "Generated" << numPoints << "mock points";
    qDebug() << "Total coordinates in vector:" << points.size();
    qDebug() << "Actual number of points:" << (points.size() / 3);

    // Log sample coordinates to verify mock data validity
    if (!points.empty() && points.size() >= 9) {
        qDebug() << "Sample mock coordinates - First point:" << points[0] << points[1] << points[2];
        if (points.size() >= 6) {
            size_t midIndex = (points.size() / 6) * 3; // Middle point
            if (midIndex + 2 < points.size()) {
                qDebug() << "Sample mock coordinates - Middle point:" << points[midIndex] << points[midIndex + 1] << points[midIndex + 2];
            }
        }
        size_t lastIndex = points.size() - 3;
        qDebug() << "Sample mock coordinates - Last point:" << points[lastIndex] << points[lastIndex + 1] << points[lastIndex + 2];
    }

    emit progressUpdated(100, "Mock data generation complete");
    emit parsingFinished(true, QString("Generated %1 mock points").arg(numPoints), points);

    return points;
}

void E57Parser::setError(const QString& error)
{
    // DEFENSIVE: Add validation and debugging to catch heap corruption
    try {
        // Log the error first before any QString operations
        qCritical() << "E57Parser Error:" << error;

        // Validate the input error string
        if (error.isNull()) {
            qWarning() << "E57Parser::setError() - Input error string is null";
            m_lastError = QString("Error: null error string provided to setError()");
        } else {
            // Create a defensive copy
            QString errorCopy = QString(error);
            m_lastError = errorCopy;
        }

        m_hasError = true;
        qDebug() << "E57Parser::setError() - Error stored successfully, length:" << m_lastError.length();

    } catch (const std::exception& e) {
        qCritical() << "E57Parser::setError() - Standard exception:" << e.what();
        m_lastError = QString("Error: Exception in setError() - %1").arg(e.what());
        m_hasError = true;
    } catch (...) {
        qCritical() << "E57Parser::setError() - Unknown exception caught";
        m_lastError = QString("Error: Unknown exception in setError()");
        m_hasError = true;
    }
}

// Sprint 1.2: Enhanced error reporting with context
// REFINED VERSION - More Defensive Against Heap Corruption
void E57Parser::setDetailedError(const QDomElement& element, const QString& errorMessage, const QString& errorCode)
{
    QString localTagName;
    QStringList localAttributeStrings;
    bool domAccessFailed = false;
    bool elementWasNull = false;

    if (element.isNull()) {
        elementWasNull = true;
    } else {
        try {
            // Attempt to copy all necessary data from QDomElement first
            localTagName = element.tagName(); // Copy tag name

            if (element.hasAttributes()) {
                QDomNamedNodeMap attrsMap = element.attributes();
                for (int i = 0; i < attrsMap.count(); ++i) {
                    QDomNode attrNode = attrsMap.item(i);
                    if (!attrNode.isNull()) {
                        QString attrNameStr = attrNode.nodeName();   // Copy attribute name
                        QString attrValueStr = attrNode.nodeValue(); // Copy attribute value
                        localAttributeStrings << QString("%1='%2'").arg(attrNameStr, attrValueStr);
                    }
                }
            }
        } catch (const std::exception& e) {
            // Catch standard C++ exceptions, though Qt DOM operations usually don't throw these for logical errors.
            qWarning() << "E57Parser: Standard exception during QDomElement access in setDetailedError: " << e.what();
            domAccessFailed = true;
        } catch (...) {
            // Catch any other exceptions (e.g., potentially from Qt internals if severely corrupted)
            qWarning() << "E57Parser: Unknown exception during QDomElement access in setDetailedError.";
            domAccessFailed = true;
        }
    }

    // Construct the final error string using only copied/local data
    QString finalDetailedErrorStr;
    if (!errorCode.isEmpty()) {
        finalDetailedErrorStr = QString("[%1] ").arg(errorCode);
    }

    if (elementWasNull) {
        finalDetailedErrorStr += QString("Error in null element: %1").arg(errorMessage);
    } else if (domAccessFailed) {
        // If DOM access failed, provide a message indicating that, but still include the original error context
        finalDetailedErrorStr += QString("Error in element (DOM access failed during attribute/tag retrieval for context): %1").arg(errorMessage);
    } else {
        finalDetailedErrorStr += QString("Error in element '%1': %2").arg(localTagName, errorMessage);
        if (!localAttributeStrings.isEmpty()) {
            finalDetailedErrorStr += QString(" (attributes: %1)").arg(localAttributeStrings.join(", "));
        }
    }

    // Call the base setError to store the fully formed string
    setError(finalDetailedErrorStr);
}

void E57Parser::setDetailedError(const QString& context, const QString& error, const QString& errorCode)
{
    QString detailedError;
    if (!errorCode.isEmpty()) {
        detailedError = QString("[%1] ").arg(errorCode);
    }

    detailedError += QString("Error in %1: %2").arg(context, error);
    setError(detailedError);
}

// Helper functions (simplified implementations)
bool E57Parser::readE57String(QDataStream& stream, QString& result)
{
    // Simplified string reading
    quint32 length;
    stream >> length;

    if (length > 1000000) { // Sanity check
        return false;
    }

    QByteArray data(length, 0);
    stream.readRawData(data.data(), length);
    result = QString::fromUtf8(data);

    return true;
}

bool E57Parser::readE57Integer(QDataStream& stream, qint64& result)
{
    stream >> result;
    return !stream.atEnd();
}

bool E57Parser::readE57Float(QDataStream& stream, double& result)
{
    stream >> result;
    return !stream.atEnd();
}

bool E57Parser::skipBytes(QDataStream& stream, qint64 count)
{
    return stream.device()->seek(stream.device()->pos() + count);
}

bool E57Parser::parseXmlSection(QFile& file, qint64 xmlOffset, qint64 xmlLength)
{
    qDebug() << "=== E57Parser::parseXmlSection (Enhanced) ===";
    qDebug() << "XML Offset:" << xmlOffset << "Length:" << xmlLength;

    // Task 1.1.4: Validate parameters
    if (xmlLength <= 0 || xmlOffset <= 0) {
        setDetailedError("XML section validation",
                        QString("Invalid parameters - offset: %1, length: %2").arg(xmlOffset).arg(xmlLength),
                        "E57_ERROR_INVALID_XML_PARAMS");
        return false;
    }

    // Task 1.1.2: Load XML content from file using offset and length
    if (!file.seek(xmlOffset)) {
        setDetailedError("XML section reading",
                        QString("Failed to seek to offset %1").arg(xmlOffset),
                        "E57_ERROR_SEEK_FAILED");
        return false;
    }

    QByteArray xmlData = file.read(xmlLength);
    if (xmlData.size() != xmlLength) {
        setDetailedError("XML section reading",
                        QString("Failed to read complete XML section - expected %1 bytes, got %2").arg(xmlLength).arg(xmlData.size()),
                        "E57_ERROR_READ_INCOMPLETE");
        return false;
    }

    qDebug() << "Read" << xmlData.size() << "bytes of XML data";

    // Task 1.1.2: Parse XML using QDomDocument
    QDomDocument doc;
    auto parseResult = doc.setContent(xmlData);

    if (!parseResult) {
        setDetailedError("XML parsing",
                        QString("Failed at line %1, column %2: %3")
                        .arg(parseResult.errorLine).arg(parseResult.errorColumn).arg(parseResult.errorMessage),
                        "E57_ERROR_XML_PARSE");
        return false;
    }

    qDebug() << "XML parsed successfully";

    // Task 1.1.2: Navigate DOM to find /e57Root/data3D/0/points element
    QDomElement root = doc.documentElement();
    if (root.tagName() != "e57Root") {
        setDetailedError(root,
                        QString("Expected 'e57Root' but found '%1'").arg(root.tagName()),
                        "E57_ERROR_BAD_ROOT");
        return false;
    }

    QDomElement data3DElement = root.firstChildElement("data3D");
    if (data3DElement.isNull()) {
        setDetailedError(root, "Missing required 'data3D' element", "E57_ERROR_MISSING_DATA3D");
        return false;
    }

    // For this sprint, assume the first point cloud (index 0)
    QDomElement vectorChild = data3DElement.firstChildElement("vectorChild");
    if (vectorChild.isNull()) {
        setDetailedError(data3DElement, "Missing required 'vectorChild' element", "E57_ERROR_MISSING_VECTORCHILD");
        return false;
    }

    QDomElement pointsElement = vectorChild.firstChildElement("points");
    if (pointsElement.isNull()) {
        setDetailedError(vectorChild, "Missing required 'points' element", "E57_ERROR_MISSING_POINTS");
        return false;
    }

    qDebug() << "Found points element in XML structure";

    // Task 1.1.2: Extract attributes from points element
    return parseData3D(pointsElement);
}

bool E57Parser::parseData3D(const QDomElement& pointsElement)
{
    qDebug() << "=== E57Parser::parseData3D (Enhanced) ===";

    // Sprint 1.2 & 2.1: Check if this is a CompressedVector type
    QString pointsType = pointsElement.attribute("type");
    if (pointsType == "CompressedVector") {
        qDebug() << "Detected CompressedVector type, using Sprint 2.1 enhanced codec parsing";

        // Sprint 2.1: Use enhanced codec parsing
        CompressedVectorInfo vectorInfo;
        if (parseCompressedVectorWithCodec(pointsElement, vectorInfo)) {
            // Store the vector info for later use in binary extraction
            m_compressedVectors.clear();
            m_compressedVectors.push_back(vectorInfo);

            // Set legacy member variables for compatibility
            m_recordCount = vectorInfo.recordCount;
            m_binaryDataOffset = vectorInfo.binaryStartOffset;
            m_hasXYZ = !vectorInfo.fields.empty();

            return true;
        } else {
            // Fall back to Sprint 1.2 parsing if codec parsing fails
            qDebug() << "Codec parsing failed, falling back to Sprint 1.2 parsing";
            return parseCompressedVector(pointsElement);
        }
    }

    // Task 1.1.2: Parse the prototype element within points to confirm XYZ presence
    QDomElement prototypeElement = pointsElement.firstChildElement("prototype");
    if (prototypeElement.isNull()) {
        setDetailedError(pointsElement, "Missing required 'prototype' element", "E57_ERROR_MISSING_PROTOTYPE");
        return false;
    }

    // Task 1.1.2: Confirm presence of cartesianX, cartesianY, cartesianZ
    QDomElement cartesianX = prototypeElement.firstChildElement("cartesianX");
    QDomElement cartesianY = prototypeElement.firstChildElement("cartesianY");
    QDomElement cartesianZ = prototypeElement.firstChildElement("cartesianZ");

    if (cartesianX.isNull() || cartesianY.isNull() || cartesianZ.isNull()) {
        QStringList missing;
        if (cartesianX.isNull()) missing << "cartesianX";
        if (cartesianY.isNull()) missing << "cartesianY";
        if (cartesianZ.isNull()) missing << "cartesianZ";
        setDetailedError(prototypeElement,
                        QString("Missing required coordinate elements: %1").arg(missing.join(", ")),
                        "E57_ERROR_MISSING_COORDINATES");
        return false;
    }

    // Task 1.1.2: Extract data types (assume Float precision="single" for this sprint)
    QString xType = cartesianX.attribute("type", "Float");
    QString yType = cartesianY.attribute("type", "Float");
    QString zType = cartesianZ.attribute("type", "Float");

    QString xPrecision = cartesianX.attribute("precision", "single");
    QString yPrecision = cartesianY.attribute("precision", "single");
    QString zPrecision = cartesianZ.attribute("precision", "single");

    qDebug() << "Coordinate types - X:" << xType << "(" << xPrecision << ")";
    qDebug() << "                   Y:" << yType << "(" << yPrecision << ")";
    qDebug() << "                   Z:" << zType << "(" << zPrecision << ")";

    // For this sprint, we assume single-precision floats
    if (xType != "Float" || yType != "Float" || zType != "Float") {
        setError("Unsupported coordinate data types (expected Float)");
        return false;
    }

    m_hasXYZ = true;
    m_pointDataType = xPrecision; // Store precision for later use

    // Check for optional fields
    m_hasColor = !prototypeElement.firstChildElement("colorRed").isNull() &&
                 !prototypeElement.firstChildElement("colorGreen").isNull() &&
                 !prototypeElement.firstChildElement("colorBlue").isNull();

    m_hasIntensity = !prototypeElement.firstChildElement("intensity").isNull();

    qDebug() << "Optional fields - Color:" << m_hasColor << "Intensity:" << m_hasIntensity;

    // Task 1.1.2: Extract attributes from points element (fileOffset, recordCount)
    QString fileOffsetStr = pointsElement.attribute("fileOffset");
    if (!fileOffsetStr.isEmpty()) {
        m_binaryDataOffset = fileOffsetStr.toLongLong();
        qDebug() << "Found fileOffset attribute:" << m_binaryDataOffset;
    }

    QString recordCountStr = pointsElement.attribute("recordCount");
    if (!recordCountStr.isEmpty()) {
        m_recordCount = recordCountStr.toLongLong();
        qDebug() << "Found recordCount attribute:" << m_recordCount;
    }

    // Task 1.1.2: Parse the codecs section to identify if data is uncompressed
    QDomElement codecsElement = pointsElement.firstChildElement("codecs");
    if (codecsElement.isNull()) {
        setError("No codecs element found in points");
        return false;
    }

    // Look for CompressedVectorNode or similar structure
    QDomElement vectorElement = codecsElement.firstChildElement("CompressedVectorNode");
    if (vectorElement.isNull()) {
        // Try alternative structure names
        vectorElement = codecsElement.firstChildElement("VectorNode");
        if (vectorElement.isNull()) {
            setError("No vector node found in codecs");
            return false;
        }
    }

    // Extract record count if not found in attributes
    if (m_recordCount == 0) {
        QDomElement recordCountElement = vectorElement.firstChildElement("recordCount");
        if (!recordCountElement.isNull()) {
            m_recordCount = recordCountElement.text().toLongLong();
            qDebug() << "Found recordCount in vector node:" << m_recordCount;
        }
    }

    // Extract binary section reference if fileOffset not found in attributes
    if (m_binaryDataOffset == 0) {
        QDomElement binaryElement = vectorElement.firstChildElement("binarySection");
        if (!binaryElement.isNull()) {
            QString binaryRef = binaryElement.text();
            qDebug() << "Binary section reference:" << binaryRef;

            // For simplicity in this sprint, assume binary data starts after XML section
            // In a real implementation, we would parse the binary section references properly
            m_binaryDataOffset = m_xmlOffset + m_xmlLength;
            qDebug() << "Estimated binary data offset:" << m_binaryDataOffset;
        }
    }

    // Task 1.1.3: Validate that we have the required metadata
    if (m_recordCount <= 0) {
        setError("Invalid or missing record count");
        return false;
    }

    if (m_binaryDataOffset <= 0) {
        setError("Invalid or missing binary data offset");
        return false;
    }

    qDebug() << "Successfully parsed point cloud metadata:";
    qDebug() << "  Record count:" << m_recordCount;
    qDebug() << "  Binary data offset:" << m_binaryDataOffset;
    qDebug() << "  Data type:" << m_pointDataType;
    qDebug() << "  Has XYZ:" << m_hasXYZ;
    qDebug() << "  Has Color:" << m_hasColor;
    qDebug() << "  Has Intensity:" << m_hasIntensity;

    return true;
}

std::vector<float> E57Parser::extractPointsFromBinarySection(QFile& file, qint64 binaryOffset, qint64 recordCount)
{
    qDebug() << "=== E57Parser::extractPointsFromBinarySection (Enhanced) ===";
    qDebug() << "Binary offset:" << binaryOffset << "Record count:" << recordCount;

    std::vector<float> points;

    // Sprint 2.1: Check if we have compressed vector info with codec
    if (!m_compressedVectors.empty()) {
        qDebug() << "Using Sprint 2.1 codec decompression";

        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);

        for (const auto& vectorInfo : m_compressedVectors) {
            // Check codec support
            if (!vectorInfo.codec.isSupported) {
                reportCodecError(vectorInfo.codec.type);
                return points;
            }

            std::vector<float> vectorPoints;

            if (vectorInfo.codec.type == "bitPackCodec") {
                if (!decompressWithBitPack(stream, vectorInfo, vectorPoints)) {
                    qDebug() << "BitPack decompression failed, falling back to uncompressed reading";
                    // Fall through to legacy extraction
                    break;
                }
            } else {
                // This should not happen due to earlier validation, but defensive coding
                reportCodecError(vectorInfo.codec.type);
                return points;
            }

            // Merge points from this vector
            points.insert(points.end(), vectorPoints.begin(), vectorPoints.end());
        }

        // If we successfully decompressed, return the results
        if (!points.empty()) {
            qDebug() << "Successfully decompressed" << (points.size() / 3) << "points using codec";
            return points;
        }
    }

    // Task 1.2.1: Validate parameters derived from XML parsing
    if (recordCount <= 0 || binaryOffset <= 0) {
        qWarning() << "Invalid parameters for binary extraction";
        return points;
    }

    // Task 1.2.2: Use fileOffset to seek to correct position in E57 file
    if (!file.seek(binaryOffset)) {
        qWarning() << "Failed to seek to binary data section at offset" << binaryOffset;
        return points;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    // Task 1.2.3: Set precision based on point data type from XML
    if (m_pointDataType == "single") {
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    } else {
        stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    }

    // Task 1.2.4: Reserve space for efficiency - populate vector with XYZ coordinates
    points.reserve(static_cast<size_t>(recordCount * 3));

    qDebug() << "Starting point extraction with data type:" << m_pointDataType;

    // Task 1.2.3: Read recordCount number of points (uncompressed XYZ float data)
    // For this sprint, assume data is stored as contiguous single-precision floats (X, Y, Z)
    for (qint64 i = 0; i < recordCount; ++i) {
        float x, y, z;

        // Task 1.2.3: Read three single-precision float values for X, Y, and Z
        stream >> x >> y >> z;

        // Task 1.2.5: Implement robust I/O error handling during binary data reading
        if (stream.status() != QDataStream::Ok) {
            qWarning() << "Stream error at point" << i << "- status:" << stream.status();
            if (stream.status() == QDataStream::ReadPastEnd) {
                qWarning() << "Unexpected end-of-file while reading point data";
            }
            break;
        }

        // Task 1.2.4: Validate coordinate values (finite numbers)
        if (std::isfinite(x) && std::isfinite(y) && std::isfinite(z)) {
            points.push_back(x);
            points.push_back(y);
            points.push_back(z);
        } else {
            qWarning() << "Invalid coordinates at point" << i << ":" << x << y << z;
            // Continue processing other points instead of stopping
        }

        // Update progress every 1000 points
        if (i % 1000 == 0) {
            int progress = static_cast<int>((i * 90) / recordCount); // 90% max for extraction
            emit progressUpdated(progress, QString("Reading point data: %1/%2 points").arg(i).arg(recordCount));
        }

        // Safety check to prevent infinite loops
        if (i > 10000000) { // 10 million points max
            qWarning() << "Reached maximum point limit, stopping extraction";
            break;
        }
    }

    qDebug() << "Extracted" << (points.size() / 3) << "valid points from" << recordCount << "records";

    // Log sample points for debugging
    if (!points.empty() && points.size() >= 9) {
        qDebug() << "Sample extracted coordinates - First point:" << points[0] << points[1] << points[2];
        if (points.size() >= 6) {
            size_t midIndex = (points.size() / 6) * 3; // Middle point
            if (midIndex + 2 < points.size()) {
                qDebug() << "Sample extracted coordinates - Middle point:" << points[midIndex] << points[midIndex + 1] << points[midIndex + 2];
            }
        }
        size_t lastIndex = points.size() - 3;
        qDebug() << "Sample extracted coordinates - Last point:" << points[lastIndex] << points[lastIndex + 1] << points[lastIndex + 2];
    }

    return points;
}

// Sprint 1.2: CompressedVector parsing implementation
bool E57Parser::parseCompressedVector(const QDomElement& pointsElement)
{
    qDebug() << "=== E57Parser::parseCompressedVector ===";

    // Validate that this is indeed a CompressedVector
    if (pointsElement.attribute("type") != "CompressedVector") {
        setDetailedError(pointsElement,
                        QString("Expected CompressedVector type, got '%1'").arg(pointsElement.attribute("type")),
                        "E57_ERROR_BAD_PROTOTYPE");
        return false;
    }

    // Navigate to codecs structure
    QDomElement codecsElement = pointsElement.firstChildElement("codecs");
    if (codecsElement.isNull()) {
        setDetailedError(pointsElement, "Missing required 'codecs' element in CompressedVector",
                        "E57_ERROR_BAD_CODECS");
        return false;
    }

    // Process CompressedVectorNode elements
    QDomNodeList cvNodes = codecsElement.elementsByTagName("CompressedVectorNode");
    if (cvNodes.isEmpty()) {
        // Try alternative naming
        cvNodes = codecsElement.elementsByTagName("VectorNode");
        if (cvNodes.isEmpty()) {
            setDetailedError(codecsElement, "No CompressedVectorNode elements found in codecs",
                            "E57_ERROR_MISSING_VECTORNODE");
            return false;
        }
    }

    qDebug() << "Found" << cvNodes.count() << "CompressedVectorNode elements";

    // For this sprint, process the first relevant CompressedVectorNode
    for (int i = 0; i < cvNodes.count(); ++i) {
        QDomElement vectorNode = cvNodes.at(i).toElement();
        if (parseCompressedVectorNode(vectorNode)) {
            qDebug() << "Successfully parsed CompressedVectorNode" << i;
            return true;
        }
        // If parsing failed, the error is already set by parseCompressedVectorNode
        // For now, just log and continue to next node (if any)
        qDebug() << "Failed to parse CompressedVectorNode" << i << "- error already set";
    }

    // If we get here, all CompressedVectorNode parsing attempts failed
    // The error from the last parseCompressedVectorNode call is already set
    // Don't call setDetailedError again to avoid potential DOM access issues
    qDebug() << "All CompressedVectorNode parsing attempts failed";
    return false;
}

bool E57Parser::parseCompressedVectorNode(const QDomElement& vectorNode)
{
    qDebug() << "=== E57Parser::parseCompressedVectorNode ===";

    // Extract recordCount from CompressedVectorNode
    QString recordCountStr = vectorNode.attribute("recordCount");
    if (recordCountStr.isEmpty()) {
        setDetailedError(vectorNode, "Missing required 'recordCount' attribute",
                        "E57_ERROR_MISSING_RECORDCOUNT");
        return false;
    }

    bool ok;
    m_recordCount = recordCountStr.toLongLong(&ok);
    if (!ok || m_recordCount < 0) {  // Allow 0 for empty point clouds
        setDetailedError(vectorNode,
                        QString("Invalid recordCount value: '%1'").arg(recordCountStr),
                        "E57_ERROR_INVALID_RECORDCOUNT");
        return false;
    }

    qDebug() << "CompressedVectorNode recordCount:" << m_recordCount;

    // Extract binarySection reference or fileOffset
    QString fileOffsetStr = vectorNode.attribute("fileOffset");
    if (!fileOffsetStr.isEmpty()) {
        m_binaryDataOffset = fileOffsetStr.toLongLong(&ok);
        if (!ok) {
            setDetailedError(vectorNode,
                            QString("Invalid fileOffset value: '%1'").arg(fileOffsetStr),
                            "E57_ERROR_INVALID_FILEOFFSET");
            return false;
        }
        qDebug() << "Found fileOffset attribute:" << m_binaryDataOffset;
    } else {
        // Look for binarySection element
        QDomElement binaryElement = vectorNode.firstChildElement("binarySection");
        if (!binaryElement.isNull()) {
            QString binaryRef = binaryElement.text();
            qDebug() << "Binary section reference:" << binaryRef;

            // For simplicity in this sprint, assume binary data starts after XML section
            // In a real implementation, we would parse the binary section references properly
            m_binaryDataOffset = m_xmlOffset + m_xmlLength;
            qDebug() << "Estimated binary data offset:" << m_binaryDataOffset;
        } else {
            setDetailedError(vectorNode, "Missing both 'fileOffset' attribute and 'binarySection' element",
                            "E57_ERROR_MISSING_BINARY_REFERENCE");
            return false;
        }
    }

    // Validate prototype for XYZ coordinates (similar to regular parsing)
    QDomElement prototypeElement = vectorNode.firstChildElement("prototype");
    if (prototypeElement.isNull()) {
        // Try to find prototype in parent or use default assumption
        qDebug() << "No prototype in CompressedVectorNode, assuming XYZ float data";
        m_pointDataType = "single";
        m_hasXYZ = true;
    } else {
        // DEFENSIVE DOM ACCESS: Extract all coordinate data immediately to avoid heap corruption
        // This prevents multiple DOM traversals that can cause SEH exceptions
        bool hasCartesianX = false;
        bool hasCartesianY = false;
        bool hasCartesianZ = false;
        QString xPrecision = "single"; // Default precision

        try {
            // Check for coordinate elements existence in a single pass
            QDomElement cartesianX = prototypeElement.firstChildElement("cartesianX");
            QDomElement cartesianY = prototypeElement.firstChildElement("cartesianY");
            QDomElement cartesianZ = prototypeElement.firstChildElement("cartesianZ");

            hasCartesianX = !cartesianX.isNull();
            hasCartesianY = !cartesianY.isNull();
            hasCartesianZ = !cartesianZ.isNull();

            // Extract precision immediately if X coordinate exists
            if (hasCartesianX) {
                xPrecision = cartesianX.attribute("precision", "single");
            }

        } catch (const std::exception& e) {
            qWarning() << "E57Parser: Exception during coordinate validation:" << e.what();
            setDetailedError("CompressedVectorNode prototype", "DOM access failed during coordinate validation",
                            "E57_ERROR_DOM_ACCESS_FAILED");
            return false;
        } catch (...) {
            qWarning() << "E57Parser: Unknown exception during coordinate validation";
            setDetailedError("CompressedVectorNode prototype", "Unknown error during coordinate validation",
                            "E57_ERROR_UNKNOWN_DOM_ERROR");
            return false;
        }

        // Check coordinate completeness using local boolean flags
        if (!hasCartesianX || !hasCartesianY || !hasCartesianZ) {
            QStringList missing;
            if (!hasCartesianX) missing << "cartesianX";
            if (!hasCartesianY) missing << "cartesianY";
            if (!hasCartesianZ) missing << "cartesianZ";

            setDetailedError("CompressedVectorNode prototype",
                            QString("Missing required coordinate elements: %1").arg(missing.join(", ")),
                            "E57_ERROR_MISSING_COORDINATES");
            return false;
        }

        // Set extracted data
        m_pointDataType = xPrecision;
        m_hasXYZ = true;

        qDebug() << "CompressedVectorNode coordinate precision:" << m_pointDataType;
    }

    // Validate that we have the required metadata
    if (m_recordCount < 0) {  // Allow 0 for empty point clouds
        setDetailedError(vectorNode, "Invalid or missing record count", "E57_ERROR_INVALID_RECORDCOUNT");
        return false;
    }

    if (m_binaryDataOffset < 0) {  // Allow 0 for binary data at start of file
        setDetailedError(vectorNode, "Invalid or missing binary data offset", "E57_ERROR_INVALID_BINARY_OFFSET");
        return false;
    }

    qDebug() << "CompressedVectorNode parsing successful:";
    qDebug() << "  Record count:" << m_recordCount;
    qDebug() << "  Binary offset:" << m_binaryDataOffset;
    qDebug() << "  Data type:" << m_pointDataType;

    return true;
}

// Sprint 2.1: Enhanced codec parsing methods
bool E57Parser::parseCompressedVectorWithCodec(const QDomElement& pointsElement, CompressedVectorInfo& vectorInfo)
{
    qDebug() << "=== E57Parser::parseCompressedVectorWithCodec ===";

    // Initialize with defaults
    vectorInfo.codec.type = "bitPackCodec";  // ASTM E57 default
    vectorInfo.codec.isSupported = true;

    // Navigate to codecs structure
    QDomElement codecsElement = pointsElement.firstChildElement("codecs");
    if (codecsElement.isNull()) {
        setDetailedError(pointsElement, "Missing required 'codecs' element in CompressedVector",
                        "E57_ERROR_BAD_CODECS");
        return false;
    }

    // Parse codec information
    if (!parseCodecsSection(codecsElement, vectorInfo.codec)) {
        return false;
    }

    // Parse prototype section for field descriptors
    QDomElement prototypeElement = pointsElement.firstChildElement("prototype");
    if (prototypeElement.isNull()) {
        setDetailedError(pointsElement, "Missing required 'prototype' element", "E57_ERROR_MISSING_PROTOTYPE");
        return false;
    }

    if (!parsePrototypeSection(prototypeElement, vectorInfo.fields)) {
        return false;
    }

    // Extract record count
    QString recordCountStr = pointsElement.attribute("recordCount");
    if (!recordCountStr.isEmpty()) {
        vectorInfo.recordCount = recordCountStr.toLongLong();
    } else {
        // Look for recordCount in codecs section
        QDomElement vectorElement = codecsElement.firstChildElement("vector");
        if (!vectorElement.isNull()) {
            QDomElement recordCountElement = vectorElement.firstChildElement("recordCount");
            if (!recordCountElement.isNull()) {
                vectorInfo.recordCount = recordCountElement.text().toLongLong();
            }
        }
    }

    // Extract binary section information
    QString fileOffsetStr = pointsElement.attribute("fileOffset");
    if (!fileOffsetStr.isEmpty()) {
        vectorInfo.binaryStartOffset = fileOffsetStr.toLongLong();
    }

    // Validate codec support (Task 2.1.2.4 requirement)
    if (vectorInfo.codec.type != "bitPackCodec") {
        m_hasUnsupportedCodec = true;
        m_unsupportedCodecName = vectorInfo.codec.type;
        vectorInfo.codec.isSupported = false;
        return false;  // Early exit for unsupported codecs
    }

    qDebug() << "Successfully parsed CompressedVector with codec:" << vectorInfo.codec.type;
    qDebug() << "  Record count:" << vectorInfo.recordCount;
    qDebug() << "  Binary offset:" << vectorInfo.binaryStartOffset;
    qDebug() << "  Field count:" << vectorInfo.fields.size();

    return true;
}

bool E57Parser::parseCodecsSection(const QDomElement& codecsElement, CodecParams& codec)
{
    qDebug() << "=== E57Parser::parseCodecsSection ===";

    QDomElement vectorElement = codecsElement.firstChildElement("vector");
    if (vectorElement.isNull()) {
        // No explicit codec found, default to bitPackCodec per ASTM E57
        codec.type = "bitPackCodec";
        codec.isSupported = true;
        qDebug() << "No explicit codec found, defaulting to bitPackCodec";
        return true;
    }

    // Look for codec specifications within vector
    QDomNodeList codecNodes = vectorElement.childNodes();
    for (int i = 0; i < codecNodes.count(); ++i) {
        QDomNode node = codecNodes.at(i);
        if (node.isElement()) {
            QDomElement codecElement = node.toElement();
            QString codecName = codecElement.tagName();

            if (codecName == "bitPackCodec") {
                codec.type = "bitPackCodec";
                codec.isSupported = true;

                // Extract any parameters (bitPackCodec typically has none)
                QDomNamedNodeMap attrs = codecElement.attributes();
                for (int j = 0; j < attrs.count(); ++j) {
                    QDomNode attr = attrs.item(j);
                    codec.parameters[attr.nodeName()] = attr.nodeValue();
                }

                qDebug() << "Found bitPackCodec with" << codec.parameters.size() << "parameters";
                return true;

            } else {
                // Unknown/unsupported codec
                codec.type = codecName;
                codec.isSupported = false;
                qDebug() << "Found unsupported codec:" << codecName;
                return false;
            }
        }
    }

    // If no explicit codec found, default to bitPackCodec per ASTM E57
    if (codec.type.isEmpty()) {
        codec.type = "bitPackCodec";
        codec.isSupported = true;
        qDebug() << "No codec specification found, defaulting to bitPackCodec";
    }

    return true;
}

bool E57Parser::parsePrototypeSection(const QDomElement& prototypeElement, std::vector<FieldDescriptor>& fields)
{
    qDebug() << "=== E57Parser::parsePrototypeSection ===";

    fields.clear();

    // Focus on coordinate fields for Sprint 2.1
    QStringList coordinateFields = {"cartesianX", "cartesianY", "cartesianZ"};

    for (const QString& fieldName : coordinateFields) {
        QDomElement fieldElement = prototypeElement.firstChildElement(fieldName);
        if (!fieldElement.isNull()) {
            FieldDescriptor field;
            field.name = fieldName;

            if (!parseFieldDescriptor(fieldElement, field)) {
                return false;
            }

            fields.push_back(field);
            qDebug() << "Added field:" << field.name << "type:" << field.dataType << "precision:" << field.precision;
        }
    }

    if (fields.empty()) {
        setDetailedError(prototypeElement, "No coordinate fields found in prototype", "E57_ERROR_NO_COORDINATES");
        return false;
    }

    qDebug() << "Successfully parsed" << fields.size() << "coordinate fields";
    return true;
}

bool E57Parser::parseFieldDescriptor(const QDomElement& fieldElement, FieldDescriptor& field)
{
    qDebug() << "=== E57Parser::parseFieldDescriptor for" << field.name << "===";

    // Determine field type from XML element structure
    QString elementType = fieldElement.attribute("type", "Float");
    QString precision = fieldElement.attribute("precision", "double");

    if (elementType == "Float") {
        field.dataType = "Float";
        if (precision == "single") {
            field.precision = 32;
        } else if (precision == "double") {
            field.precision = 64;
        } else {
            field.precision = precision.toInt();
            if (field.precision <= 0) {
                field.precision = 64;  // Default to double precision
            }
        }
    } else if (elementType == "ScaledInteger") {
        field.dataType = "ScaledInteger";
        field.precision = fieldElement.attribute("precision", "32").toInt();
        field.scale = fieldElement.attribute("scale", "1.0").toDouble();
        field.offset = fieldElement.attribute("offset", "0.0").toDouble();
    } else if (elementType == "Integer") {
        field.dataType = "Integer";
        field.precision = fieldElement.attribute("precision", "32").toInt();
    } else {
        field.dataType = "Float";
        field.precision = 64;  // Default fallback
    }

    // Extract range information
    QString minStr = fieldElement.attribute("minimum");
    if (!minStr.isEmpty()) {
        field.minimum = minStr.toDouble();
    }

    QString maxStr = fieldElement.attribute("maximum");
    if (!maxStr.isEmpty()) {
        field.maximum = maxStr.toDouble();
    }

    qDebug() << "Field" << field.name << "parsed:";
    qDebug() << "  Type:" << field.dataType;
    qDebug() << "  Precision:" << field.precision << "bits";
    qDebug() << "  Range:" << field.minimum << "to" << field.maximum;
    if (field.dataType == "ScaledInteger") {
        qDebug() << "  Scale:" << field.scale << "Offset:" << field.offset;
    }

    return true;
}

// Sprint 2.1: Decompression methods
bool E57Parser::decompressWithBitPack(QDataStream& stream,
                                     const CompressedVectorInfo& vectorInfo,
                                     std::vector<float>& outPoints)
{
    qDebug() << "=== E57Parser::decompressWithBitPack ===";
    qDebug() << "Record count:" << vectorInfo.recordCount;
    qDebug() << "Field count:" << vectorInfo.fields.size();

    try {
        outPoints.clear();

        // Calculate expected output size
        size_t expectedSize = static_cast<size_t>(vectorInfo.recordCount * vectorInfo.fields.size());
        outPoints.reserve(expectedSize);

        // Seek to binary data start
        if (!stream.device()->seek(vectorInfo.binaryStartOffset)) {
            reportDecompressionError("Failed to seek to binary data start");
            return false;
        }

        // BitPack decompression: Process in chunks for memory efficiency
        const qint64 chunkSize = 1000;  // Process 1000 records at a time

        for (qint64 recordStart = 0; recordStart < vectorInfo.recordCount; recordStart += chunkSize) {
            qint64 recordsInChunk = qMin(chunkSize, vectorInfo.recordCount - recordStart);

            // Process each record in the chunk
            for (qint64 record = 0; record < recordsInChunk; ++record) {
                // Process each field in the record
                for (const auto& field : vectorInfo.fields) {
                    // Read packed bits for this field
                    quint64 packedValue = readPackedBits(stream, field.precision);

                    // Convert to float value
                    double floatValue = unpackFieldValue(packedValue, field);
                    outPoints.push_back(static_cast<float>(floatValue));
                }
            }

            // Update progress
            int progress = static_cast<int>((recordStart * 90) / vectorInfo.recordCount);
            emit progressUpdated(progress, QString("Decompressing data: %1/%2 records").arg(recordStart).arg(vectorInfo.recordCount));
        }

        // Validate output size
        if (outPoints.size() != expectedSize) {
            reportDecompressionError(QString("Size mismatch: expected %1, got %2")
                                   .arg(expectedSize).arg(outPoints.size()));
            return false;
        }

        qDebug() << "Successfully decompressed" << (outPoints.size() / 3) << "points";
        return true;

    } catch (const std::exception &e) {
        reportDecompressionError(QString("Exception during decompression: %1").arg(e.what()));
        return false;
    } catch (...) {
        reportDecompressionError("Unknown error during decompression");
        return false;
    }
}

// Sprint 2.1: Utility methods for bit manipulation
quint64 E57Parser::readPackedBits(QDataStream& stream, int bitCount)
{
    static quint64 bitBuffer = 0;
    static int bitsInBuffer = 0;

    quint64 result = 0;
    int bitsNeeded = bitCount;

    while (bitsNeeded > 0) {
        // Refill buffer if needed
        if (bitsInBuffer == 0) {
            quint8 byte;
            stream >> byte;

            if (stream.status() != QDataStream::Ok) {
                throw std::runtime_error("Stream read error during bit unpacking");
            }

            bitBuffer = byte;
            bitsInBuffer = 8;
        }

        // Extract bits from buffer
        int bitsToTake = qMin(bitsNeeded, bitsInBuffer);
        quint64 mask = (1ULL << bitsToTake) - 1;
        quint64 extractedBits = bitBuffer & mask;

        result |= (extractedBits << (bitCount - bitsNeeded));
        bitBuffer >>= bitsToTake;
        bitsInBuffer -= bitsToTake;
        bitsNeeded -= bitsToTake;
    }

    return result;
}

double E57Parser::unpackFieldValue(quint64 packedValue, const FieldDescriptor& field)
{
    if (field.dataType == "Float") {
        if (field.precision == 32) {
            // Interpret as 32-bit float
            uint32_t int32Value = static_cast<uint32_t>(packedValue);
            float floatValue;
            memcpy(&floatValue, &int32Value, sizeof(float));
            return static_cast<double>(floatValue);

        } else if (field.precision == 64) {
            // Interpret as 64-bit double
            double doubleValue;
            memcpy(&doubleValue, &packedValue, sizeof(double));
            return doubleValue;
        }

    } else if (field.dataType == "ScaledInteger") {
        // Apply scaling: result = (rawValue * scale) + offset
        return (static_cast<double>(packedValue) * field.scale) + field.offset;

    } else if (field.dataType == "Integer") {
        return static_cast<double>(packedValue);
    }

    // Fallback
    return static_cast<double>(packedValue);
}

// Sprint 2.1: Error handling for codecs
void E57Parser::reportCodecError(const QString& codecName)
{
    QString errorMsg = QString("Unsupported E57 compression codec: %1. "
                              "Currently supported: bitPackCodec")
                      .arg(codecName);

    setError(errorMsg);
    emit parsingFinished(false, errorMsg, std::vector<float>());
}

void E57Parser::reportDecompressionError(const QString& details)
{
    QString errorMsg = QString("E57 decompression failed: %1").arg(details);
    setError(errorMsg);
    emit parsingFinished(false, errorMsg, std::vector<float>());
}
