#include "parsers/e57parserlib.h"

#include <QDebug>
#include <QFileInfo>
#include <QString>
#include <QThread>
#include <QTimer>

#include <random>
#include <sstream>

#include <E57Format/E57Format.h>  // Add proper E57Format include

#include "core/performance_profiler.h"  // Sprint 2.2: Performance profiling
#include "core/profiling_macros.h"      // Sprint 7.3: Profiling macros

// Constructor
E57ParserLib::E57ParserLib(QObject* parent)
    : IE57Parser(parent),
      m_parserCore(std::make_unique<E57ParserCore>()),
      m_progressTimer(nullptr),
      m_cancelRequested(false)
{
    setupForThreading();
}

// Destructor
E57ParserLib::~E57ParserLib()
{
    if (m_imageFile && m_imageFile->isOpen())
    {
        m_imageFile->close();
    }
}

// MainWindow-compatible interface methods
void E57ParserLib::startParsing(const QString& filePath, const IE57Parser::LoadingSettings& settings)
{
    m_currentFilePath = filePath;
    m_currentSettings = settings;
    m_cancelRequested = false;

    // Start parsing in the current thread (simplified for now)
    performParsing();
}

bool E57ParserLib::isOpen() const
{
    return m_parserCore && m_parserCore->isOpen();
}

void E57ParserLib::closeFile()
{
    if (m_parserCore)
    {
        m_parserCore->closeFile();
    }
    if (m_imageFile)
    {
        m_imageFile->close();
        m_imageFile.reset();
    }
}

int E57ParserLib::getScanCount() const
{
    if (!m_parserCore || !m_parserCore->isOpen())
    {
        return 0;
    }
    return m_parserCore->getScanCount();
}

void E57ParserLib::cancelParsing()
{
    m_cancelRequested = true;
}

QString E57ParserLib::getLastError() const
{
    return m_lastError;
}

// Additional interface methods
bool E57ParserLib::isValidE57File(const QString& filePath)
{
    try
    {
        e57::ImageFile testFile(filePath.toStdString(), "r");
        bool isValid = testFile.isOpen();
        testFile.close();
        return isValid;
    }
    catch (...)
    {
        return false;
    }
}

int E57ParserLib::getScanCount(const QString& filePath)
{
    try
    {
        e57::ImageFile testFile(filePath.toStdString(), "r");
        if (!testFile.isOpen())
        {
            return 0;
        }

        e57::StructureNode root = testFile.root();
        if (!root.isDefined("data3D"))
        {
            testFile.close();
            return 0;
        }

        e57::VectorNode data3D(root.get("data3D"));
        int count = static_cast<int>(data3D.childCount());
        testFile.close();
        return count;
    }
    catch (...)
    {
        return 0;
    }
}

bool E57ParserLib::openFile(const std::string& filePath)
{
    try
    {
        closeFile();
        m_imageFile = std::make_unique<e57::ImageFile>(filePath, "r");

        if (!m_imageFile->isOpen())
        {
            setError("Failed to open E57 file");
            return false;
        }

        // Also open with core parser
        if (!m_parserCore->openFile(filePath))
        {
            setError(m_parserCore->getLastError());
            return false;
        }

        return true;
    }
    catch (const std::exception& ex)
    {
        setError(std::string("Exception opening file: ") + ex.what());
        return false;
    }
}

std::string E57ParserLib::getGuid() const
{
    if (!m_imageFile || !m_imageFile->isOpen())
    {
        return "";
    }

    try
    {
        e57::StructureNode root = m_imageFile->root();
        if (root.isDefined("guid"))
        {
            e57::StringNode guidNode(root.get("guid"));
            return guidNode.value();
        }
    }
    catch (...)
    {
        // Return empty string on error
    }

    return "";
}

std::pair<int, int> E57ParserLib::getVersion() const
{
    if (!m_imageFile || !m_imageFile->isOpen())
    {
        return {0, 0};
    }

    try
    {
        e57::StructureNode root = m_imageFile->root();
        int major = 0, minor = 0;

        if (root.isDefined("versionMajor"))
        {
            e57::IntegerNode majorNode(root.get("versionMajor"));
            major = static_cast<int>(majorNode.value());
        }

        if (root.isDefined("versionMinor"))
        {
            e57::IntegerNode minorNode(root.get("versionMinor"));
            minor = static_cast<int>(minorNode.value());
        }

        return {major, minor};
    }
    catch (...)
    {
        return {0, 0};
    }
}

std::vector<float> E57ParserLib::extractPointData()
{
    return extractPointData(0);
}

// Sprint 4: Multi-scan support enhancement
IE57Parser::ScanMetadata E57ParserLib::getScanMetadata(int scanIndex) const
{
    IE57Parser::ScanMetadata metadata;

    if (!m_parserCore || !m_parserCore->isOpen())
    {
        setError("No E57 file is open");
        return metadata;
    }

    try
    {
        CoreScanMetadata coreMetadata = m_parserCore->getScanMetadata(scanIndex);

        // Convert core metadata to interface metadata
        metadata.index = scanIndex;
        metadata.name = coreMetadata.name;
        metadata.guid = coreMetadata.guid;
        metadata.pointCount = coreMetadata.pointCount;
        metadata.isLoaded = false;
        metadata.hasIntensity = true;  // Assume available for now
        metadata.hasColor = true;      // Assume available for now

        return metadata;
    }
    catch (const std::exception& ex)
    {
        setError(std::string("Exception during scan metadata retrieval: ") + ex.what());
        return metadata;
    }
}

std::vector<float> E57ParserLib::extractPointData(int scanIndex)
{
    PROFILE_FUNCTION();  // Sprint 2.2: Performance profiling

    try
    {
        clearError();

        if (!m_parserCore || !m_parserCore->isOpen())
        {
            setError("No E57 file is open");
            emit parsingFinished(false, getLastError(), std::vector<float>());
            return std::vector<float>();
        }

        if (scanIndex < 0 || scanIndex >= getScanCount())
        {
            setError("Invalid scan index: " + std::to_string(scanIndex));
            emit parsingFinished(false, getLastError(), std::vector<float>());
            return std::vector<float>();
        }

        emit progressUpdated(10, "Extracting point data...");

        // Use the core parser to extract XYZ data
        CoreLoadingSettings coreSettings = convertLoadingSettings(m_currentSettings);
        std::vector<float> points = m_parserCore->extractXYZData(scanIndex, coreSettings);

        if (points.empty())
        {
            setError(m_parserCore->getLastError());
            emit parsingFinished(false, getLastError(), std::vector<float>());
            return std::vector<float>();
        }

        emit progressUpdated(100, "Point extraction complete");
        emit parsingFinished(true, QString("Successfully extracted %1 points").arg(points.size() / 3), points);

        return points;
    }
    catch (const std::exception& ex)
    {
        setError(std::string("Exception during point extraction: ") + ex.what());
        emit parsingFinished(false, getLastError(), std::vector<float>());
        return std::vector<float>();
    }
}

int64_t E57ParserLib::getPointCount(int scanIndex) const
{
    if (!m_parserCore || !m_parserCore->isOpen())
    {
        return 0;
    }

    return m_parserCore->getPointCount(scanIndex);
}

// Data conversion helpers
IE57Parser::PointData E57ParserLib::convertCorePointData(const CorePointData& corePoint)
{
    IE57Parser::PointData qtPoint;
    qtPoint.x = corePoint.x;
    qtPoint.y = corePoint.y;
    qtPoint.z = corePoint.z;
    qtPoint.intensity = corePoint.intensity;
    qtPoint.hasIntensity = corePoint.hasIntensity;
    qtPoint.r = corePoint.red;
    qtPoint.g = corePoint.green;
    qtPoint.b = corePoint.blue;
    qtPoint.hasColor = corePoint.hasColor;
    return qtPoint;
}

CoreLoadingSettings E57ParserLib::convertLoadingSettings(const IE57Parser::LoadingSettings& qtSettings)
{
    CoreLoadingSettings coreSettings;
    coreSettings.maxPoints = (qtSettings.maxPointsPerScan > 0) ? qtSettings.maxPointsPerScan : 1000000;
    coreSettings.loadIntensity = qtSettings.loadIntensity;
    coreSettings.loadColor = qtSettings.loadColor;
    coreSettings.voxelSize = 0.0;              // Not supported in IE57Parser interface
    coreSettings.enableSpatialFilter = false;  // Not supported in IE57Parser interface
    return coreSettings;
}

std::vector<IE57Parser::PointData> E57ParserLib::extractEnhancedPointData(int scanIndex)
{
    try
    {
        clearError();
        std::vector<IE57Parser::PointData> points;

        if (!m_parserCore || !m_parserCore->isOpen())
        {
            setError("No E57 file is open");
            emit parsingFinished(false, getLastError(), std::vector<float>());
            return points;
        }

        if (scanIndex < 0 || scanIndex >= getScanCount())
        {
            setError("Invalid scan index: " + std::to_string(scanIndex));
            emit parsingFinished(false, getLastError(), std::vector<float>());
            return points;
        }

        emit progressUpdated(10, "Extracting enhanced point data...");

        // Use the core parser to extract enhanced point data
        CoreLoadingSettings coreSettings = convertLoadingSettings(m_currentSettings);
        std::vector<CorePointData> corePoints = m_parserCore->extractPointData(scanIndex, coreSettings);

        if (corePoints.empty())
        {
            setError(m_parserCore->getLastError());
            emit parsingFinished(false, getLastError(), std::vector<float>());
            return points;
        }

        emit progressUpdated(50, "Converting point data...");

        // Convert core point data to interface point data
        points.reserve(corePoints.size());
        for (const auto& corePoint : corePoints)
        {
            points.push_back(convertCorePointData(corePoint));
        }

        emit progressUpdated(100, "Enhanced point extraction complete");
        emit parsingFinished(
            true, QString("Successfully extracted %1 enhanced points").arg(points.size()), std::vector<float>());

        return points;
    }
    catch (const std::exception& ex)
    {
        setError(std::string("Exception during enhanced point extraction: ") + ex.what());
        emit parsingFinished(false, getLastError(), std::vector<float>());
        return std::vector<IE57Parser::PointData>();
    }
}

// Simplified implementation methods - delegate to core parser
void E57ParserLib::performParsing()
{
    PROFILE_FUNCTION();  // Sprint 7.3: Performance profiling

    emit progressUpdated(0, "Initializing E57 parser...");

    try
    {
        {
            PROFILE_SECTION("E57::OpenFile");
            // Use the core parser for all operations
            if (!m_parserCore->openFile(m_currentFilePath.toStdString()))
            {
                setError(m_parserCore->getLastError());
                emit parsingFinished(false, getLastError(), std::vector<float>());
                return;
            }
        }

        emit progressUpdated(20, "Extracting point data...");

        std::vector<float> points;
        {
            PROFILE_SECTION("E57::ExtractPointData");
            // Extract point data using core parser
            CoreLoadingSettings coreSettings = convertLoadingSettings(m_currentSettings);
            points = m_parserCore->extractXYZData(0, coreSettings);
        }

        if (points.empty())
        {
            setError(m_parserCore->getLastError());
            emit parsingFinished(false, getLastError(), std::vector<float>());
            return;
        }

        emit progressUpdated(100, "Parsing complete");
        emit parsingFinished(true, QString("Successfully extracted %1 points").arg(points.size() / 3), points);
    }
    catch (const std::exception& ex)
    {
        setError(std::string("Exception during parsing: ") + ex.what());
        emit parsingFinished(false, getLastError(), std::vector<float>());
    }
}

void E57ParserLib::setupForThreading()
{
    // Setup progress timer for thread-safe progress updates
    m_progressTimer = new QTimer(this);
    m_progressTimer->setSingleShot(true);
}

void E57ParserLib::updateProgress(int percentage, const QString& stage)
{
    emit progressUpdated(percentage, stage);
}

std::vector<float> E57ParserLib::convertToXYZVector(const std::vector<IE57Parser::PointData>& pointData)
{
    std::vector<float> xyzVector;
    xyzVector.reserve(pointData.size() * 3);

    for (const auto& point : pointData)
    {
        xyzVector.push_back(static_cast<float>(point.x));
        xyzVector.push_back(static_cast<float>(point.y));
        xyzVector.push_back(static_cast<float>(point.z));
    }

    return xyzVector;
}

void E57ParserLib::handleE57Exception(const std::exception& ex, const QString& context)
{
    setError(context.toStdString() + ": " + ex.what());
}

QString E57ParserLib::translateE57Error(const QString& technicalError)
{
    return technicalError;  // Simple pass-through for now
}

// Helper methods
void E57ParserLib::clearError() const
{
    m_lastError.clear();
}

void E57ParserLib::setError(const std::string& error) const
{
    m_lastError = QString::fromStdString(error);
}

// Stub implementations for legacy methods that are no longer needed
bool E57ParserLib::inspectPointPrototype(const e57::StructureNode& /*scanHeaderNode*/)
{
    return true;  // Stub - not used in simplified implementation
}

void E57ParserLib::validatePrototypeFields(const e57::StructureNode& /*prototype*/)
{
    // Stub - not used in simplified implementation
}

bool E57ParserLib::extractUncompressedXYZData(const e57::StructureNode& /*scanHeaderNode*/)
{
    return true;  // Stub - not used in simplified implementation
}

bool E57ParserLib::inspectEnhancedPrototype(const e57::StructureNode& /*scanHeaderNode*/)
{
    return true;  // Stub - not used in simplified implementation
}

bool E57ParserLib::extractDataLimits(const e57::StructureNode& /*scanHeaderNode*/)
{
    return true;  // Stub - not used in simplified implementation
}

bool E57ParserLib::extractEnhancedPointData(const e57::StructureNode& /*scanHeaderNode*/,
                                            std::vector<IE57Parser::PointData>& /*points*/)
{
    return true;  // Stub - not used in simplified implementation
}

float E57ParserLib::normalizeIntensity(float rawValue) const
{
    return std::max(0.0f, std::min(1.0f, rawValue));  // Simple clamp
}

uint8_t E57ParserLib::normalizeColorChannel(float rawValue, double /*minVal*/, double /*maxVal*/) const
{
    return static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, rawValue)));  // Simple clamp
}
