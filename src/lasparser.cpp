#include "lasparser.h"
#include "loadingsettings.h"
#include "lasheadermetadata.h"
#include "performance_profiler.h"
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
    , m_versionMajor(0)
    , m_versionMinor(0)
    , m_pointDataRecordLength(0)
    , m_headerSize(0)
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
    PROFILE_FUNCTION();

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
        emit progressUpdated(1, "Initializing...");
        QFile file(filePath);
        {
            PROFILE_SECTION("LAS::FileOpen");
            if (!file.open(QIODevice::ReadOnly)) {
                throw LasParseException(QString("Failed to open file: %1").arg(file.errorString()));
            }
            m_fileSize = file.size();
            qDebug() << "File size:" << m_fileSize << "bytes";
        }

        // Read and validate header
        emit progressUpdated(5, "Reading LAS header...");
        LasHeader header;
        {
            PROFILE_SECTION("LAS::HeaderRead");
            if (!readHeader(file, header)) {
                throw LasParseException("Failed to read LAS header");
            }
        }

        emit progressUpdated(10, "Validating header data...");
        {
            PROFILE_SECTION("LAS::HeaderValidation");
            if (!validateHeader(header)) {
                throw LasParseException("Invalid LAS header");
            }
        }
        emit progressUpdated(15, "Header validated");

        qDebug() << "Header parsed successfully - Point count:" << header.numberOfPointRecords;
        qDebug() << "Point data format:" << header.pointDataFormat;
        qDebug() << "Header bounding box: Min(" << header.minX << "," << header.minY << "," << header.minZ
                 << ") Max(" << header.maxX << "," << header.maxY << "," << header.maxZ << ")";
        qDebug() << "Scale factors: X=" << header.xScaleFactor << " Y=" << header.yScaleFactor << " Z=" << header.zScaleFactor;
        qDebug() << "Offsets: X=" << header.xOffset << " Y=" << header.yOffset << " Z=" << header.zOffset;

        // Store header information (Sprint 1.3: Enhanced storage)
        m_pointCount = header.numberOfPointRecords;
        m_pointFormat = header.pointDataFormat;
        m_versionMajor = header.versionMajor;
        m_versionMinor = header.versionMinor;
        m_pointDataRecordLength = header.pointDataRecordLength;
        m_headerSize = header.headerSize;
        m_xScale = header.xScaleFactor;
        m_yScale = header.yScaleFactor;
        m_zScale = header.zScaleFactor;
        m_xOffset = header.xOffset;
        m_yOffset = header.yOffset;
        m_zOffset = header.zOffset;

        // Store bounding box information
        m_boundingBoxMin = {header.minX, header.minY, header.minZ};
        m_boundingBoxMax = {header.maxX, header.maxY, header.maxZ};

        // Emit enhanced header metadata (Sprint 1.3)
        LasHeaderMetadata metadata;
        metadata.numberOfPointRecords = header.numberOfPointRecords;
        metadata.minBounds = m_boundingBoxMin;
        metadata.maxBounds = m_boundingBoxMax;
        metadata.filePath = filePath;
        metadata.versionMajor = header.versionMajor;
        metadata.versionMinor = header.versionMinor;
        metadata.pointDataFormat = header.pointDataFormat;
        metadata.systemIdentifier = QString::fromLatin1(header.systemIdentifier, 32).trimmed();
        metadata.generatingSoftware = QString::fromLatin1(header.generatingSoftware, 32).trimmed();
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
            emit progressUpdated(50, "Reading point data for filtering...");
            std::vector<float> rawPoints;
            {
                PROFILE_SECTION("LAS::PointDataRead");
                rawPoints = readPointData(file, header);
            }
            qDebug() << "Read" << (rawPoints.size() / 3) << "points before filtering";

            emit progressUpdated(75, "Applying voxel grid filter...");
            {
                PROFILE_SECTION("LAS::VoxelGridFilter");
                VoxelGridFilter filter;
                points = filter.filter(rawPoints, settings);
            }
            qDebug() << "After voxel grid filtering:" << (points.size() / 3) << "points remain";

            // Clear raw points to free memory
            std::vector<float>().swap(rawPoints);

            emit parsingFinished(true, QString("Successfully loaded %1 points (filtered from %2)")
                               .arg(points.size() / 3).arg(header.numberOfPointRecords), points);
        } else {
            // Read point data for full load
            qDebug() << "Full load mode - reading all point data...";
            emit progressUpdated(20, "Reading point cloud data...");
            {
                PROFILE_SECTION("LAS::PointDataRead");
                points = readPointData(file, header);
            }
            qDebug() << "Successfully read" << (points.size() / 3) << "points";
            emit progressUpdated(100, "Loading complete");
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
    // Task 1.3.1.1: Enhanced LAS header validation for Sprint 1.3

    // Check signature
    if (std::memcmp(header.signature, LAS_FILE_SIGNATURE, 4) != 0) {
        setError("Invalid LAS file signature. Expected 'LASF'.");
        return false;
    }

    // Task 1.3.2.1: Support LAS 1.2, 1.3, 1.4
    if (!isVersionSupported(header.versionMajor, header.versionMinor)) {
        setError(QString("Unsupported LAS version %1.%2. Supported versions: 1.2, 1.3, 1.4")
                .arg(header.versionMajor).arg(header.versionMinor));
        return false;
    }

    // Task 1.3.1.2: Validate PDRF 0-3 support
    if (header.pointDataFormat > MAX_SUPPORTED_POINT_FORMAT) {
        setError(QString("LAS %1.%2 PDRF %3: Unsupported Point Data Record Format. Supported: 0-3")
                .arg(header.versionMajor)
                .arg(header.versionMinor)
                .arg(header.pointDataFormat));
        return false;
    }

    // Task 1.3.1.3: Validate record length for each PDRF
    if (!validateRecordLength(header)) {
        return false; // Error message set in validateRecordLength
    }

    // Task 1.3.1.3: Validate scale factors
    if (!validateScaleFactors(header)) {
        return false; // Error message set in validateScaleFactors
    }

    // Check if we have points
    if (header.numberOfPointRecords == 0) {
        setError(QString("LAS %1.%2: No point records found in file")
                .arg(header.versionMajor).arg(header.versionMinor));
        return false;
    }

    // Validate header size for version
    uint16_t expectedHeaderSize = getExpectedHeaderSize(header.versionMinor);
    if (header.headerSize < expectedHeaderSize) {
        setError(QString("LAS %1.%2: Invalid header size %3. Expected minimum %4")
                .arg(header.versionMajor)
                .arg(header.versionMinor)
                .arg(header.headerSize)
                .arg(expectedHeaderSize));
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

    // Calculate how many bytes to skip after reading XYZ (handle vendor extensions)
    int bytesToSkip = header.pointDataRecordLength - 12; // 12 bytes for XYZ coordinates

    for (uint32_t i = 0; i < header.numberOfPointRecords; ++i) {
        int32_t x, y, z;

        if (!readValue(file, x) || !readValue(file, y) || !readValue(file, z)) {
            throw LasParseException(QString("Failed to read point %1").arg(i));
        }

        // Skip remaining fields (standard PDRF 0 fields + any vendor extensions)
        // Standard PDRF 0: intensity, return info, classification, scan angle, user data, point source ID = 8 bytes
        // Plus any vendor-specific extensions
        if (file.read(bytesToSkip).size() != bytesToSkip) {
            throw LasParseException(QString("Failed to skip point data for point %1 (expected %2 bytes)").arg(i).arg(bytesToSkip));
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

    // Calculate how many bytes to skip after reading XYZ (handle vendor extensions)
    int bytesToSkip = header.pointDataRecordLength - 12; // 12 bytes for XYZ coordinates

    for (uint32_t i = 0; i < header.numberOfPointRecords; ++i) {
        int32_t x, y, z;

        if (!readValue(file, x) || !readValue(file, y) || !readValue(file, z)) {
            throw LasParseException(QString("Failed to read point %1").arg(i));
        }

        // Skip remaining fields (standard PDRF 1 fields + any vendor extensions)
        // Standard PDRF 1: intensity, return info, classification, scan angle, user data, point source ID, GPS time = 16 bytes
        // Plus any vendor-specific extensions
        if (file.read(bytesToSkip).size() != bytesToSkip) {
            throw LasParseException(QString("Failed to skip point data for point %1 (expected %2 bytes)").arg(i).arg(bytesToSkip));
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

    // Calculate how many bytes to skip after reading XYZ (handle vendor extensions)
    int bytesToSkip = header.pointDataRecordLength - 12; // 12 bytes for XYZ coordinates

    for (uint32_t i = 0; i < header.numberOfPointRecords; ++i) {
        int32_t x, y, z;

        if (!readValue(file, x) || !readValue(file, y) || !readValue(file, z)) {
            throw LasParseException(QString("Failed to read point %1").arg(i));
        }

        // Skip remaining fields (standard PDRF 2 fields + any vendor extensions)
        // Standard PDRF 2: intensity, return info, classification, scan angle, user data, point source ID, RGB = 14 bytes
        // Plus any vendor-specific extensions
        if (file.read(bytesToSkip).size() != bytesToSkip) {
            throw LasParseException(QString("Failed to skip point data for point %1 (expected %2 bytes)").arg(i).arg(bytesToSkip));
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

    // Calculate how many bytes to skip after reading XYZ (handle vendor extensions)
    int bytesToSkip = header.pointDataRecordLength - 12; // 12 bytes for XYZ coordinates

    for (uint32_t i = 0; i < header.numberOfPointRecords; ++i) {
        int32_t x, y, z;

        if (!readValue(file, x) || !readValue(file, y) || !readValue(file, z)) {
            throw LasParseException(QString("Failed to read point %1").arg(i));
        }

        // Skip remaining fields (standard PDRF 3 fields + any vendor extensions)
        // Standard PDRF 3: intensity, return info, classification, scan angle, user data, point source ID, GPS time, RGB = 22 bytes
        // Plus any vendor-specific extensions
        if (file.read(bytesToSkip).size() != bytesToSkip) {
            throw LasParseException(QString("Failed to skip point data for point %1 (expected %2 bytes)").arg(i).arg(bytesToSkip));
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
        emit progressUpdated(progress, QString("Reading points: %1/%2").arg(currentPoint).arg(totalPoints));
    }
}

void LasParser::setError(const QString& error)
{
    m_hasError = true;
    m_lastError = error;
    qDebug() << "LasParser Error:" << error;
}

// Sprint 1.3: Enhanced validation helper methods

bool LasParser::isVersionSupported(uint8_t major, uint8_t minor) const
{
    return (major == SUPPORTED_VERSION_MAJOR &&
            minor >= MIN_VERSION_MINOR &&
            minor <= MAX_VERSION_MINOR);
}

uint16_t LasParser::getExpectedRecordLength(uint8_t pointDataFormat) const
{
    // Task 1.3.1.3: Expected record lengths for PDRFs 0-3
    switch(pointDataFormat) {
        case 0: return 20;  // XYZ, Intensity, Return info, Classification, Scan angle, User data, Point source ID
        case 1: return 28;  // PDRF 0 + GPS Time (8 bytes)
        case 2: return 26;  // PDRF 0 + RGB (6 bytes)
        case 3: return 34;  // PDRF 0 + GPS Time + RGB (14 bytes)
        default: return 0;  // Unsupported format
    }
}

uint16_t LasParser::getExpectedHeaderSize(uint8_t versionMinor) const
{
    // Task 1.3.2.2: Header sizes for different LAS versions
    switch(versionMinor) {
        case 2: return 227;  // LAS 1.2 minimum header size
        case 3: return 235;  // LAS 1.3 minimum header size (adds waveform data start)
        case 4: return 375;  // LAS 1.4 minimum header size (adds EVLR and extended point counts)
        default: return 227; // Default to LAS 1.2 size
    }
}

bool LasParser::validateRecordLength(const LasHeader& header) const
{
    uint16_t minimumLength = getExpectedRecordLength(header.pointDataFormat);
    if (minimumLength == 0) {
        const_cast<LasParser*>(this)->setError(QString("LAS %1.%2: Unsupported point data format %3")
                .arg(header.versionMajor)
                .arg(header.versionMinor)
                .arg(header.pointDataFormat));
        return false;
    }

    // Sprint 1.3: Allow record lengths >= minimum (LAS spec allows vendor extensions)
    if (header.pointDataRecordLength < minimumLength) {
        const_cast<LasParser*>(this)->setError(QString("LAS %1.%2 PDRF %3: Point data record length too short. Minimum %4, got %5")
                .arg(header.versionMajor)
                .arg(header.versionMinor)
                .arg(header.pointDataFormat)
                .arg(minimumLength)
                .arg(header.pointDataRecordLength));
        return false;
    }

    // Log if we have extended record length (vendor-specific data)
    if (header.pointDataRecordLength > minimumLength) {
        qDebug() << QString("LAS %1.%2 PDRF %3: Extended record length detected. Standard: %4, Actual: %5 (+%6 vendor bytes)")
                .arg(header.versionMajor)
                .arg(header.versionMinor)
                .arg(header.pointDataFormat)
                .arg(minimumLength)
                .arg(header.pointDataRecordLength)
                .arg(header.pointDataRecordLength - minimumLength);
    }

    return true;
}

bool LasParser::validateScaleFactors(const LasHeader& header) const
{
    if (header.xScaleFactor == 0.0) {
        const_cast<LasParser*>(this)->setError(QString("LAS %1.%2: Scale factor for X axis is zero, data may be invalid")
                .arg(header.versionMajor).arg(header.versionMinor));
        return false;
    }

    if (header.yScaleFactor == 0.0) {
        const_cast<LasParser*>(this)->setError(QString("LAS %1.%2: Scale factor for Y axis is zero, data may be invalid")
                .arg(header.versionMajor).arg(header.versionMinor));
        return false;
    }

    if (header.zScaleFactor == 0.0) {
        const_cast<LasParser*>(this)->setError(QString("LAS %1.%2: Scale factor for Z axis is zero, data may be invalid")
                .arg(header.versionMajor).arg(header.versionMinor));
        return false;
    }

    return true;
}
