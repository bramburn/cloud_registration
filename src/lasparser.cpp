#include "lasparser.h"
#include "loadingsettings.h"
#include "lasheadermetadata.h"
#include <QDebug>
#include <QFileInfo>
#include <QtEndian>
#include <cstring>

// LAS file format constants
const char LasParser::LAS_FILE_SIGNATURE[4] = {'L', 'A', 'S', 'F'};
const uint8_t LasParser::SUPPORTED_VERSION_MAJOR = 1;
const uint8_t LasParser::MIN_VERSION_MINOR = 2;
const uint8_t LasParser::MAX_VERSION_MINOR = 4;
const uint8_t LasParser::MAX_SUPPORTED_POINT_FORMAT = 3;

LasParser::LasParser(QObject *parent)
    : QObject(parent)
    , m_hasError(false)
    , m_fileSize(0)
    , m_currentPosition(0)
    , m_headerParsed(false)
    , m_pointCount(0)
    , m_pointFormat(0)
    , m_xScale(1.0)
    , m_yScale(1.0)
    , m_zScale(1.0)
    , m_xOffset(0.0)
    , m_yOffset(0.0)
    , m_zOffset(0.0)
{
}

LasParser::~LasParser()
{
}

std::vector<float> LasParser::parse(const QString& filePath)
{
    // Default to full load for backward compatibility
    LoadingSettings defaultSettings;
    defaultSettings.method = LoadingMethod::FullLoad;
    return parse(filePath, defaultSettings);
}

std::vector<float> LasParser::parse(const QString& filePath, const LoadingSettings& settings)
{
    m_hasError = false;
    m_lastError.clear();

    // Debug logging for parsing process (User Story 1)
    qDebug() << "=== LasParser::parse ===";
    qDebug() << "File path:" << filePath;
    qDebug() << "Loading method:" << static_cast<int>(settings.method);
    if (settings.method == LoadingMethod::VoxelGrid) {
        qDebug() << "Voxel grid parameters:" << settings.parameters;
    }

    try {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            throw LasParseException(QString("Failed to open file: %1").arg(file.errorString()));
        }

        m_fileSize = file.size();
        qDebug() << "File size:" << m_fileSize << "bytes";

        // Read and validate header
        LasHeader header;
        if (!readHeader(file, header)) {
            throw LasParseException("Failed to read LAS header");
        }

        if (!validateHeader(header)) {
            throw LasParseException("Invalid LAS header");
        }

        qDebug() << "Header parsed successfully - Point count:" << header.numberOfPointRecords;
        qDebug() << "Point data format:" << header.pointDataFormat;
        qDebug() << "Header bounding box: Min(" << header.minX << "," << header.minY << "," << header.minZ
                 << ") Max(" << header.maxX << "," << header.maxY << "," << header.maxZ << ")";
        qDebug() << "Scale factors: X=" << header.xScaleFactor << " Y=" << header.yScaleFactor << " Z=" << header.zScaleFactor;
        qDebug() << "Offsets: X=" << header.xOffset << " Y=" << header.yOffset << " Z=" << header.zOffset;

        // Store header information
        m_pointCount = header.numberOfPointRecords;
        m_pointFormat = header.pointDataFormat;
        m_xScale = header.xScaleFactor;
        m_yScale = header.yScaleFactor;
        m_zScale = header.zScaleFactor;
        m_xOffset = header.xOffset;
        m_yOffset = header.yOffset;
        m_zOffset = header.zOffset;

        // Store bounding box information
        m_boundingBoxMin = {header.minX, header.minY, header.minZ};
        m_boundingBoxMax = {header.maxX, header.maxY, header.maxZ};

        // Emit header metadata
        LasHeaderMetadata metadata;
        metadata.numberOfPointRecords = header.numberOfPointRecords;
        metadata.minBounds = m_boundingBoxMin;
        metadata.maxBounds = m_boundingBoxMax;
        metadata.filePath = filePath;
        emit headerParsed(metadata);

        // Conditional parsing based on loading method
        std::vector<float> points;
        if (settings.method == LoadingMethod::HeaderOnly) {
            // Return empty vector for header-only mode
            qDebug() << "Header-only mode selected - returning empty points vector";
            emit parsingFinished(true, QString("Header loaded: %1 points").arg(header.numberOfPointRecords), points);
        } else if (settings.method == LoadingMethod::VoxelGrid) {
            // Read point data and apply voxel grid filtering
            qDebug() << "Reading all points for voxel grid filtering...";
            emit progressUpdated(50);
            std::vector<float> rawPoints = readPointData(file, header);
            qDebug() << "Read" << (rawPoints.size() / 3) << "points before filtering";

            emit progressUpdated(75);
            VoxelGridFilter filter;
            points = filter.filter(rawPoints, settings);
            qDebug() << "After voxel grid filtering:" << (points.size() / 3) << "points remain";

            // Clear raw points to free memory
            std::vector<float>().swap(rawPoints);

            emit parsingFinished(true, QString("Successfully loaded %1 points (filtered from %2)")
                               .arg(points.size() / 3).arg(header.numberOfPointRecords), points);
        } else {
            // Read point data for full load
            qDebug() << "Full load mode - reading all point data...";
            points = readPointData(file, header);
            qDebug() << "Successfully read" << (points.size() / 3) << "points";
            emit parsingFinished(true, QString("Successfully loaded %1 points").arg(points.size() / 3), points);
        }

        // Log sample coordinates if we have data
        if (!points.empty() && points.size() >= 9) {
            qDebug() << "Sample coordinates - First point:" << points[0] << points[1] << points[2];
            if (points.size() >= 6) {
                size_t midIndex = (points.size() / 6) * 3; // Middle point
                if (midIndex + 2 < points.size()) {
                    qDebug() << "Sample coordinates - Middle point:" << points[midIndex] << points[midIndex + 1] << points[midIndex + 2];
                }
            }
            size_t lastIndex = points.size() - 3;
            qDebug() << "Sample coordinates - Last point:" << points[lastIndex] << points[lastIndex + 1] << points[lastIndex + 2];
        }

        return points;

    } catch (const LasParseException& e) {
        setError(QString::fromStdString(e.what()));
        qDebug() << "LAS parsing failed with LasParseException:" << m_lastError;
        emit parsingFinished(false, m_lastError, std::vector<float>());
        return std::vector<float>();
    } catch (const std::exception& e) {
        setError(QString("Unexpected error: %1").arg(QString::fromStdString(e.what())));
        qDebug() << "LAS parsing failed with unexpected error:" << m_lastError;
        emit parsingFinished(false, m_lastError, std::vector<float>());
        return std::vector<float>();
    }
}

void LasParser::startParsing(const QString& filePath)
{
    // Default to full load for backward compatibility
    LoadingSettings defaultSettings;
    defaultSettings.method = LoadingMethod::FullLoad;
    startParsing(filePath, defaultSettings);
}

void LasParser::startParsing(const QString& filePath, const LoadingSettings& settings)
{
    // This slot is called from the worker thread
    try {
        std::vector<float> points = parse(filePath, settings);
        // The parse() method already emits the parsingFinished signal
    } catch (const std::exception& e) {
        emit parsingFinished(false, QString("Error in startParsing: %1").arg(e.what()), std::vector<float>());
    }
}

bool LasParser::isValidLasFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // Check file signature
    char signature[4];
    if (file.read(signature, 4) != 4) {
        return false;
    }

    return std::memcmp(signature, LAS_FILE_SIGNATURE, 4) == 0;
}

QString LasParser::getLastError() const
{
    return m_lastError;
}

bool LasParser::readHeader(QFile& file, LasHeader& header)
{
    file.seek(0);

    // Read signature directly
    if (file.read(header.signature, 4) != 4) {
        return false;
    }

    // Read header fields with proper endianness handling
    if (!readValue(file, header.fileSourceId) ||
        !readValue(file, header.globalEncoding)) {
        return false;
    }

    // Skip GUID (16 bytes)
    if (file.read(16).size() != 16) {
        return false;
    }

    if (!readValue(file, header.versionMajor) ||
        !readValue(file, header.versionMinor)) {
        return false;
    }

    // Skip system identifier and generating software (64 bytes total)
    if (file.read(64).size() != 64) {
        return false;
    }

    if (!readValue(file, header.creationDayOfYear) ||
        !readValue(file, header.creationYear) ||
        !readValue(file, header.headerSize) ||
        !readValue(file, header.pointDataOffset) ||
        !readValue(file, header.numberOfVLRs) ||
        !readValue(file, header.pointDataFormat) ||
        !readValue(file, header.pointDataRecordLength) ||
        !readValue(file, header.numberOfPointRecords)) {
        return false;
    }

    // Skip number of points by return (20 bytes)
    if (file.read(20).size() != 20) {
        return false;
    }

    if (!readValue(file, header.xScaleFactor) ||
        !readValue(file, header.yScaleFactor) ||
        !readValue(file, header.zScaleFactor) ||
        !readValue(file, header.xOffset) ||
        !readValue(file, header.yOffset) ||
        !readValue(file, header.zOffset) ||
        !readValue(file, header.maxX) ||
        !readValue(file, header.minX) ||
        !readValue(file, header.maxY) ||
        !readValue(file, header.minY) ||
        !readValue(file, header.maxZ) ||
        !readValue(file, header.minZ)) {
        return false;
    }

    return true;
}

bool LasParser::validateHeader(const LasHeader& header)
{
    // Check signature
    if (std::memcmp(header.signature, LAS_FILE_SIGNATURE, 4) != 0) {
        setError("Invalid LAS file signature");
        return false;
    }

    // Check version
    if (header.versionMajor != SUPPORTED_VERSION_MAJOR ||
        header.versionMinor < MIN_VERSION_MINOR ||
        header.versionMinor > MAX_VERSION_MINOR) {
        setError(QString("Unsupported LAS version: %1.%2")
                .arg(header.versionMajor).arg(header.versionMinor));
        return false;
    }

    // Check point data format
    if (header.pointDataFormat > MAX_SUPPORTED_POINT_FORMAT) {
        setError(QString("Unsupported point data format: %1").arg(header.pointDataFormat));
        return false;
    }

    // Check if we have points
    if (header.numberOfPointRecords == 0) {
        setError("No point records found in file");
        return false;
    }

    // Check scale factors
    if (header.xScaleFactor == 0.0 || header.yScaleFactor == 0.0 || header.zScaleFactor == 0.0) {
        setError("Invalid scale factors in header");
        return false;
    }

    return true;
}

std::vector<float> LasParser::readPointData(QFile& file, const LasHeader& header)
{
    // Seek to point data
    if (!file.seek(header.pointDataOffset)) {
        throw LasParseException("Failed to seek to point data offset");
    }

    // Dispatch to appropriate point format reader
    switch (header.pointDataFormat) {
        case 0:
            return readPointFormat0(file, header);
        case 1:
            return readPointFormat1(file, header);
        case 2:
            return readPointFormat2(file, header);
        case 3:
            return readPointFormat3(file, header);
        default:
            throw LasParseException(QString("Unsupported point format: %1").arg(header.pointDataFormat));
    }
}

template<typename T>
bool LasParser::readValue(QFile& file, T& value)
{
    QByteArray data = file.read(sizeof(T));
    if (data.size() != sizeof(T)) {
        return false;
    }

    // LAS files use little-endian format
    if constexpr (sizeof(T) == 1) {
        value = *reinterpret_cast<const T*>(data.constData());
    } else if constexpr (sizeof(T) == 2) {
        value = qFromLittleEndian<T>(data.constData());
    } else if constexpr (sizeof(T) == 4) {
        value = qFromLittleEndian<T>(data.constData());
    } else if constexpr (sizeof(T) == 8) {
        value = qFromLittleEndian<T>(data.constData());
    } else {
        std::memcpy(&value, data.constData(), sizeof(T));
    }

    return true;
}

std::vector<float> LasParser::readPointFormat0(QFile& file, const LasHeader& header)
{
    std::vector<float> points;
    points.reserve(header.numberOfPointRecords * 3);

    for (uint32_t i = 0; i < header.numberOfPointRecords; ++i) {
        int32_t x, y, z;

        if (!readValue(file, x) || !readValue(file, y) || !readValue(file, z)) {
            throw LasParseException(QString("Failed to read point %1").arg(i));
        }

        // Skip remaining fields (intensity, return info, classification, etc.)
        // Point format 0 has 20 bytes total, we've read 12 (3 * 4 bytes for XYZ)
        if (file.read(8).size() != 8) {
            throw LasParseException(QString("Failed to skip point data for point %1").arg(i));
        }

        // Transform coordinates and add to points vector
        transformAndAddPoint(points, x, y, z, header);

        // Update progress if needed
        updateProgressIfNeeded(i, header.numberOfPointRecords);
    }

    return points;
}

std::vector<float> LasParser::readPointFormat1(QFile& file, const LasHeader& header)
{
    std::vector<float> points;
    points.reserve(header.numberOfPointRecords * 3);

    for (uint32_t i = 0; i < header.numberOfPointRecords; ++i) {
        int32_t x, y, z;

        if (!readValue(file, x) || !readValue(file, y) || !readValue(file, z)) {
            throw LasParseException(QString("Failed to read point %1").arg(i));
        }

        // Skip remaining fields (intensity, return info, classification, scan angle, user data, point source ID, GPS time)
        // Point format 1 has 28 bytes total, we've read 12 (3 * 4 bytes for XYZ)
        if (file.read(16).size() != 16) {
            throw LasParseException(QString("Failed to skip point data for point %1").arg(i));
        }

        // Transform coordinates and add to points vector
        transformAndAddPoint(points, x, y, z, header);

        // Update progress if needed
        updateProgressIfNeeded(i, header.numberOfPointRecords);
    }

    return points;
}

std::vector<float> LasParser::readPointFormat2(QFile& file, const LasHeader& header)
{
    std::vector<float> points;
    points.reserve(header.numberOfPointRecords * 3);

    for (uint32_t i = 0; i < header.numberOfPointRecords; ++i) {
        int32_t x, y, z;

        if (!readValue(file, x) || !readValue(file, y) || !readValue(file, z)) {
            throw LasParseException(QString("Failed to read point %1").arg(i));
        }

        // Skip remaining fields (intensity, return info, classification, scan angle, user data, point source ID, RGB)
        // Point format 2 has 26 bytes total, we've read 12 (3 * 4 bytes for XYZ)
        if (file.read(14).size() != 14) {
            throw LasParseException(QString("Failed to skip point data for point %1").arg(i));
        }

        // Transform coordinates and add to points vector
        transformAndAddPoint(points, x, y, z, header);

        // Update progress if needed
        updateProgressIfNeeded(i, header.numberOfPointRecords);
    }

    return points;
}

std::vector<float> LasParser::readPointFormat3(QFile& file, const LasHeader& header)
{
    std::vector<float> points;
    points.reserve(header.numberOfPointRecords * 3);

    for (uint32_t i = 0; i < header.numberOfPointRecords; ++i) {
        int32_t x, y, z;

        if (!readValue(file, x) || !readValue(file, y) || !readValue(file, z)) {
            throw LasParseException(QString("Failed to read point %1").arg(i));
        }

        // Skip remaining fields (intensity, return info, classification, scan angle, user data, point source ID, GPS time, RGB)
        // Point format 3 has 34 bytes total, we've read 12 (3 * 4 bytes for XYZ)
        if (file.read(22).size() != 22) {
            throw LasParseException(QString("Failed to skip point data for point %1").arg(i));
        }

        // Transform coordinates and add to points vector
        transformAndAddPoint(points, x, y, z, header);

        // Update progress if needed
        updateProgressIfNeeded(i, header.numberOfPointRecords);
    }

    return points;
}

void LasParser::transformAndAddPoint(std::vector<float>& points, int32_t x, int32_t y, int32_t z, const LasHeader& header)
{
    // Apply scale and offset to get actual coordinates
    float actualX = static_cast<float>(x * header.xScaleFactor + header.xOffset);
    float actualY = static_cast<float>(y * header.yScaleFactor + header.yOffset);
    float actualZ = static_cast<float>(z * header.zScaleFactor + header.zOffset);

    points.push_back(actualX);
    points.push_back(actualY);
    points.push_back(actualZ);
}

void LasParser::updateProgressIfNeeded(uint32_t currentPoint, uint32_t totalPoints)
{
    // Update progress every 10000 points
    if (currentPoint % 10000 == 0) {
        int progress = static_cast<int>((currentPoint * 100) / totalPoints);
        emit progressUpdated(progress);
    }
}

void LasParser::setError(const QString& error)
{
    m_hasError = true;
    m_lastError = error;
    qDebug() << "LasParser Error:" << error;
}
