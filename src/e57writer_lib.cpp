#include "e57writer_lib.h"
#include <E57Format.h>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <cfloat>
#include <limits>
#include <algorithm>

E57WriterLib::E57WriterLib(QObject *parent)
    : QObject(parent)
    , m_fileOpen(false)
    , m_scanCount(0)
{
}

E57WriterLib::~E57WriterLib()
{
    if (m_fileOpen) {
        closeFile();
    }
}

bool E57WriterLib::createFile(const QString& filePath)
{
    try {
        // Close any existing file
        if (m_fileOpen) {
            closeFile();
        }

        // Validate file path
        QFileInfo fileInfo(filePath);
        QDir parentDir = fileInfo.dir();
        if (!parentDir.exists()) {
            setError(QString("Directory does not exist: %1").arg(parentDir.absolutePath()));
            return false;
        }

        // Task W1.1.1: Create ImageFile in write mode
        m_imageFile = std::make_unique<e57::ImageFile>(filePath.toStdString(), "w");
        
        if (!m_imageFile->isOpen()) {
            setError("Failed to open file handle");
            return false;
        }

        m_currentFilePath = filePath;
        m_fileOpen = true;
        m_scanCount = 0;

        // Task W1.1.1: Setup basic E57Root elements
        if (!initializeE57Root()) {
            closeFile();
            return false;
        }

        qDebug() << "E57WriterLib: Successfully created E57 file:" << filePath;
        emit fileCreated(true, filePath);
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "createFile");
        return false;
    } catch (const std::exception& ex) {
        setError(QString("Standard exception in createFile: %1").arg(ex.what()));
        return false;
    }
}

bool E57WriterLib::initializeE57Root()
{
    try {
        // Task W1.1.1: Setup basic E57Root elements like formatName, guid
        e57::StructureNode rootNode = m_imageFile->root();

        // Required E57Root elements according to ASTM E2807 standard
        e57::StringNode formatNameNode(*m_imageFile, "ASTM E57 3D Imaging Data File");
        rootNode.set("formatName", formatNameNode);

        e57::StringNode guidNode(*m_imageFile, generateGUID().toStdString());
        rootNode.set("guid", guidNode);

        // Add version information (required for valid E57 files)
        e57::IntegerNode versionMajorNode(*m_imageFile, 1, 0, 255);
        rootNode.set("versionMajor", versionMajorNode);

        e57::IntegerNode versionMinorNode(*m_imageFile, 0, 0, 255);
        rootNode.set("versionMinor", versionMinorNode);

        // Add creation date/time
        QString creationDateTime = QDateTime::currentDateTime().toString(Qt::ISODate);
        e57::StringNode dateTimeNode(*m_imageFile, creationDateTime.toStdString());
        rootNode.set("creationDateTime", dateTimeNode);

        // Add coordinate metadata (required for proper E57 structure)
        e57::StringNode coordinateMetadataNode(*m_imageFile, "");
        rootNode.set("coordinateMetadata", coordinateMetadataNode);

        // Create the data3D vector immediately to ensure proper file structure
        // This is critical for libE57Format to recognize the file as valid
        // Use false for allowHeteroChildren as per E57 standard (all children should be Data3D StructureNodes)
        e57::VectorNode data3DVector(*m_imageFile, false); // allowHeteroChildren = false for Data3D
        rootNode.set("data3D", data3DVector);

        qDebug() << "E57WriterLib: Initialized E57Root with required elements and data3D vector";
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "initializeE57Root");
        return false;
    }
}

bool E57WriterLib::addScan(const QString& scanName)
{
    if (!m_fileOpen) {
        setError("No file is currently open for writing");
        return false;
    }

    try {
        // Task W1.2.1: Get the E57Root node
        e57::StructureNode rootNode = m_imageFile->root();

        // Task W1.2.2: Create /data3D VectorNode if it doesn't exist
        if (!createData3DVectorNode()) {
            return false;
        }

        // Task W1.2.3: Create a new StructureNode for this scan
        if (!createScanStructureNode(scanName)) {
            return false;
        }

        m_scanCount++;
        qDebug() << "E57WriterLib: Successfully added scan:" << scanName;
        emit scanAdded(true, scanName);
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "addScan");
        return false;
    } catch (const std::exception& ex) {
        setError(QString("Standard exception in addScan: %1").arg(ex.what()));
        return false;
    }
}

bool E57WriterLib::createData3DVectorNode()
{
    try {
        e57::StructureNode rootNode = m_imageFile->root();

        // The data3D vector should already exist from initializeE57Root()
        if (rootNode.isDefined("data3D")) {
            // Get existing data3D node - use constructor-based conversion instead of static_cast
            e57::VectorNode existingNode(rootNode.get("data3D"));
            m_data3DNode = std::make_shared<e57::VectorNode>(existingNode);
            qDebug() << "E57WriterLib: Using existing /data3D VectorNode";
        } else {
            // This should not happen if initializeE57Root() was called correctly
            setError("data3D vector not found - initializeE57Root() may have failed");
            return false;
        }

        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "createData3DVectorNode");
        return false;
    }
}

bool E57WriterLib::createScanStructureNode(const QString& scanName)
{
    try {
        // Task W1.2.3: Create a new StructureNode for the scan header
        e57::StructureNode scanHeaderNode(*m_imageFile);

        // Task W1.2.4: Populate the Data3D StructureNode with mandatory header elements
        e57::StringNode guidNode(*m_imageFile, generateGUID().toStdString());
        scanHeaderNode.set("guid", guidNode);

        e57::StringNode nameNode(*m_imageFile, scanName.toStdString());
        scanHeaderNode.set("name", nameNode);

        // Add the scan to the /data3D vector
        m_data3DNode->append(scanHeaderNode);

        // Store reference for later use (e.g., adding points)
        m_currentScanNode = std::make_shared<e57::StructureNode>(scanHeaderNode);

        qDebug() << "E57WriterLib: Created scan structure node with name:" << scanName;
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "createScanStructureNode");
        return false;
    }
}

QString E57WriterLib::generateGUID() const
{
    // Generate a new UUID in the format expected by E57
    QUuid uuid = QUuid::createUuid();
    return uuid.toString(); // Returns format like "{12345678-1234-1234-1234-123456789abc}"
}

// Sprint W3: Enhanced prototype definition with optional intensity and color
bool E57WriterLib::definePointPrototype(const ExportOptions& options)
{
    if (!m_fileOpen) {
        setError("No file is currently open for writing");
        return false;
    }

    if (!m_currentScanNode) {
        setError("No scan has been added yet. Call addScan() first.");
        return false;
    }

    try {
        // Task W3.1.1 & W3.2.1: Create a StructureNode to serve as the prototype
        e57::StructureNode prototypeStructure(*m_imageFile);

        // Task W1.3.3: Add three FloatNode children for XYZ coordinates (always required)
        e57::FloatNode xNode(*m_imageFile, 0.0, e57::PrecisionDouble, -DBL_MAX, DBL_MAX);
        prototypeStructure.set("cartesianX", xNode);

        e57::FloatNode yNode(*m_imageFile, 0.0, e57::PrecisionDouble, -DBL_MAX, DBL_MAX);
        prototypeStructure.set("cartesianY", yNode);

        e57::FloatNode zNode(*m_imageFile, 0.0, e57::PrecisionDouble, -DBL_MAX, DBL_MAX);
        prototypeStructure.set("cartesianZ", zNode);

        // Task W3.1.2: Add intensity field if requested
        if (options.includeIntensity) {
            // Using FloatNode for normalized intensity values (0.0-1.0)
            e57::FloatNode intensityNode(*m_imageFile, 0.0, e57::PrecisionSingle, 0.0, 1.0);
            prototypeStructure.set("intensity", intensityNode);
            qDebug() << "E57WriterLib: Added intensity field to prototype (FloatNode, 0.0-1.0)";
        }

        // Task W3.2.2: Add color fields if requested
        if (options.includeColor) {
            // Using IntegerNode for 8-bit color channels (0-255)
            e57::IntegerNode redNode(*m_imageFile, 0, 0, 255);
            prototypeStructure.set("colorRed", redNode);

            e57::IntegerNode greenNode(*m_imageFile, 0, 0, 255);
            prototypeStructure.set("colorGreen", greenNode);

            e57::IntegerNode blueNode(*m_imageFile, 0, 0, 255);
            prototypeStructure.set("colorBlue", blueNode);
            qDebug() << "E57WriterLib: Added color fields to prototype (IntegerNode, 0-255)";
        }

        // Task W1.3.1: Create the required 'codecs' VectorNode for the CompressedVectorNode
        e57::VectorNode codecsNode(*m_imageFile, true); // allowHeterogeneousChildren = true for codecs

        // Task W1.3.1: Create a proper CompressedVectorNode with the prototype and codecs
        e57::CompressedVectorNode pointsNode(*m_imageFile, prototypeStructure, codecsNode);

        // Add the points CompressedVectorNode to the scan
        m_currentScanNode->set("points", pointsNode);

        qDebug() << "E57WriterLib: Successfully defined point prototype with"
                 << "XYZ=" << "true"
                 << "intensity=" << options.includeIntensity
                 << "color=" << options.includeColor;
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "definePointPrototype");
        return false;
    } catch (const std::exception& ex) {
        setError(QString("Standard exception in definePointPrototype: %1").arg(ex.what()));
        return false;
    }
}

bool E57WriterLib::defineXYZPrototype()
{
    // Sprint W3: Use the new enhanced method with XYZ-only options for backward compatibility
    ExportOptions xyzOnlyOptions(false, false); // No intensity, no color
    return definePointPrototype(xyzOnlyOptions);
}

// Sprint W3: Enhanced point writing implementation with export options
bool E57WriterLib::writePoints(const std::vector<Point3D>& points, const ExportOptions& options)
{
    if (!m_fileOpen) {
        setError("Cannot write points: No file is open");
        return false;
    }

    if (!m_currentScanNode) {
        setError("Cannot write points: No current scan available. Call addScan() first");
        return false;
    }

    return writePointsToScan(*m_currentScanNode, points, options);
}

bool E57WriterLib::writePoints(int scanIndex, const std::vector<Point3D>& points, const ExportOptions& options)
{
    if (!m_fileOpen) {
        setError("Cannot write points: No file is open");
        return false;
    }

    e57::StructureNode* scanNode = getScanNode(scanIndex);
    if (!scanNode) {
        setError(QString("Cannot write points: Invalid scan index %1").arg(scanIndex));
        return false;
    }

    return writePointsToScan(*scanNode, points, options);
}

// Sprint W2: Legacy point writing implementation for backward compatibility
bool E57WriterLib::writePoints(const std::vector<Point3D>& points)
{
    // Use the enhanced method with XYZ-only options for backward compatibility
    ExportOptions xyzOnlyOptions(false, false); // No intensity, no color
    return writePoints(points, xyzOnlyOptions);
}

bool E57WriterLib::writePoints(int scanIndex, const std::vector<Point3D>& points)
{
    // Use the enhanced method with XYZ-only options for backward compatibility
    ExportOptions xyzOnlyOptions(false, false); // No intensity, no color
    return writePoints(scanIndex, points, xyzOnlyOptions);
}

// Sprint W2: Legacy writePointsToScan for backward compatibility
bool E57WriterLib::writePointsToScan(e57::StructureNode& scanNode, const std::vector<Point3D>& points)
{
    // Use the enhanced method with XYZ-only options for backward compatibility
    ExportOptions xyzOnlyOptions(false, false); // No intensity, no color
    return writePointsToScan(scanNode, points, xyzOnlyOptions);
}

// Sprint W3: Enhanced writePointsToScan with intensity and color support
bool E57WriterLib::writePointsToScan(e57::StructureNode& scanNode, const std::vector<Point3D>& points, const ExportOptions& options)
{
    try {
        // Task W2.1.2: Retrieve the points CompressedVectorNode for the scan
        if (!scanNode.isDefined("points")) {
            setError("Scan does not have a points CompressedVectorNode. Call definePointPrototype() first");
            return false;
        }

        e57::CompressedVectorNode pointsNode(scanNode.get("points"));

        // Task W2.2.1: Calculate cartesian bounds before writing points
        if (!calculateAndWriteCartesianBounds(scanNode, points)) {
            return false; // Error already set
        }

        // Task W3.4.1 & W3.4.2: Calculate and write intensity/color limits if enabled
        if (options.includeIntensity && hasValidIntensityData(points)) {
            if (!calculateAndWriteIntensityLimits(scanNode, points)) {
                return false; // Error already set
            }
        }

        if (options.includeColor && hasValidColorData(points)) {
            if (!calculateAndWriteColorLimits(scanNode, points)) {
                return false; // Error already set
            }
        }

        // Handle empty point set
        if (points.empty()) {
            qDebug() << "E57WriterLib: Writing 0 points to scan";
            return true;
        }

        // Task W3.3.2: Prepare SourceDestBuffer for all enabled attributes
        const int64_t POINTS_PER_WRITE_BLOCK = 10000; // Block size for memory management

        // XYZ buffers (always required)
        std::vector<double> xAppBlockBuffer(POINTS_PER_WRITE_BLOCK);
        std::vector<double> yAppBlockBuffer(POINTS_PER_WRITE_BLOCK);
        std::vector<double> zAppBlockBuffer(POINTS_PER_WRITE_BLOCK);

        // Optional intensity buffer
        std::vector<float> intensityAppBlockBuffer;
        if (options.includeIntensity) {
            intensityAppBlockBuffer.resize(POINTS_PER_WRITE_BLOCK);
        }

        // Optional color buffers
        std::vector<uint8_t> redAppBlockBuffer, greenAppBlockBuffer, blueAppBlockBuffer;
        if (options.includeColor) {
            redAppBlockBuffer.resize(POINTS_PER_WRITE_BLOCK);
            greenAppBlockBuffer.resize(POINTS_PER_WRITE_BLOCK);
            blueAppBlockBuffer.resize(POINTS_PER_WRITE_BLOCK);
        }

        // Build SourceDestBuffer vector
        std::vector<e57::SourceDestBuffer> sdbufs;
        sdbufs.emplace_back(*m_imageFile, "cartesianX", xAppBlockBuffer.data(), POINTS_PER_WRITE_BLOCK, true, false, sizeof(double));
        sdbufs.emplace_back(*m_imageFile, "cartesianY", yAppBlockBuffer.data(), POINTS_PER_WRITE_BLOCK, true, false, sizeof(double));
        sdbufs.emplace_back(*m_imageFile, "cartesianZ", zAppBlockBuffer.data(), POINTS_PER_WRITE_BLOCK, true, false, sizeof(double));

        if (options.includeIntensity) {
            sdbufs.emplace_back(*m_imageFile, "intensity", intensityAppBlockBuffer.data(), POINTS_PER_WRITE_BLOCK, true, false, sizeof(float));
        }

        if (options.includeColor) {
            sdbufs.emplace_back(*m_imageFile, "colorRed", redAppBlockBuffer.data(), POINTS_PER_WRITE_BLOCK, true, false, sizeof(uint8_t));
            sdbufs.emplace_back(*m_imageFile, "colorGreen", greenAppBlockBuffer.data(), POINTS_PER_WRITE_BLOCK, true, false, sizeof(uint8_t));
            sdbufs.emplace_back(*m_imageFile, "colorBlue", blueAppBlockBuffer.data(), POINTS_PER_WRITE_BLOCK, true, false, sizeof(uint8_t));
        }

        // Task W2.1.4: Create CompressedVectorWriter
        e57::CompressedVectorWriter writer = pointsNode.writer(sdbufs);

        // Task W3.3.3: Write points in blocks with all enabled attributes
        size_t totalPoints = points.size();
        size_t pointsWritten = 0;

        while (pointsWritten < totalPoints) {
            // Determine points in current block
            size_t pointsInBlock = std::min(static_cast<size_t>(POINTS_PER_WRITE_BLOCK), totalPoints - pointsWritten);

            // Copy data to block buffers
            for (size_t i = 0; i < pointsInBlock; ++i) {
                const Point3D& point = points[pointsWritten + i];

                // XYZ coordinates (always required)
                xAppBlockBuffer[i] = point.x;
                yAppBlockBuffer[i] = point.y;
                zAppBlockBuffer[i] = point.z;

                // Optional intensity
                if (options.includeIntensity) {
                    intensityAppBlockBuffer[i] = point.hasIntensity ? point.intensity : 0.0f;
                }

                // Optional color
                if (options.includeColor) {
                    redAppBlockBuffer[i] = point.hasColor ? point.colorRed : 0;
                    greenAppBlockBuffer[i] = point.hasColor ? point.colorGreen : 0;
                    blueAppBlockBuffer[i] = point.hasColor ? point.colorBlue : 0;
                }
            }

            // Write current block
            writer.write(pointsInBlock);
            pointsWritten += pointsInBlock;
        }

        // Task W2.1.6: Close writer to finalize
        writer.close();

        qDebug() << "E57WriterLib: Successfully wrote" << totalPoints << "points to scan with"
                 << "intensity=" << options.includeIntensity
                 << "color=" << options.includeColor;
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "writePointsToScan");
        return false;
    } catch (const std::exception& ex) {
        setError(QString("Standard exception in writePointsToScan: %1").arg(ex.what()));
        return false;
    }
}

bool E57WriterLib::closeFile()
{
    if (!m_fileOpen) {
        return true; // Already closed
    }

    try {
        if (m_imageFile) {
            // Ensure the file structure is properly finalized before closing
            // This is critical for libE57Format to write data to disk

            // The data3D vector should already exist from initializeE57Root()
            // Just close the file - this triggers the actual write to disk
            m_imageFile->close();
            m_imageFile.reset();
        }

        m_fileOpen = false;
        m_currentScanNode.reset();
        m_data3DNode.reset();

        qDebug() << "E57WriterLib: Successfully closed E57 file:" << m_currentFilePath;
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "closeFile");
        return false;
    } catch (const std::exception& ex) {
        setError(QString("Standard exception in closeFile: %1").arg(ex.what()));
        return false;
    }
}

QString E57WriterLib::getLastError() const
{
    return m_lastError;
}

bool E57WriterLib::isFileOpen() const
{
    return m_fileOpen;
}

QString E57WriterLib::getCurrentFilePath() const
{
    return m_currentFilePath;
}

void E57WriterLib::setError(const QString& errorMessage)
{
    m_lastError = errorMessage;
    qWarning() << "E57WriterLib Error:" << errorMessage;
    emit errorOccurred(errorMessage);
}

void E57WriterLib::handleE57Exception(const std::exception& ex, const QString& context)
{
    QString errorMsg = QString("E57 Exception in %1: %2").arg(context, ex.what());
    setError(errorMsg);
}

// Sprint W2: Helper method implementations
bool E57WriterLib::calculateAndWriteCartesianBounds(e57::StructureNode& scanNode, const std::vector<Point3D>& points)
{
    try {
        // Task W2.2.1: Calculate min/max values for X, Y, Z coordinates
        double minX = std::numeric_limits<double>::infinity();
        double maxX = -std::numeric_limits<double>::infinity();
        double minY = std::numeric_limits<double>::infinity();
        double maxY = -std::numeric_limits<double>::infinity();
        double minZ = std::numeric_limits<double>::infinity();
        double maxZ = -std::numeric_limits<double>::infinity();

        // Handle empty point set - Task W2.2.5
        if (points.empty()) {
            minX = maxX = minY = maxY = minZ = maxZ = 0.0;
        } else {
            for (const Point3D& point : points) {
                minX = std::min(minX, point.x);
                maxX = std::max(maxX, point.x);
                minY = std::min(minY, point.y);
                maxY = std::max(maxY, point.y);
                minZ = std::min(minZ, point.z);
                maxZ = std::max(maxZ, point.z);
            }
        }

        // Task W2.2.3: Create cartesianBounds StructureNode
        e57::StructureNode cartesianBoundsNode(*m_imageFile);
        scanNode.set("cartesianBounds", cartesianBoundsNode);

        // Task W2.2.4: Populate with six FloatNode children
        cartesianBoundsNode.set("xMinimum", e57::FloatNode(*m_imageFile, minX, e57::PrecisionDouble));
        cartesianBoundsNode.set("xMaximum", e57::FloatNode(*m_imageFile, maxX, e57::PrecisionDouble));
        cartesianBoundsNode.set("yMinimum", e57::FloatNode(*m_imageFile, minY, e57::PrecisionDouble));
        cartesianBoundsNode.set("yMaximum", e57::FloatNode(*m_imageFile, maxY, e57::PrecisionDouble));
        cartesianBoundsNode.set("zMinimum", e57::FloatNode(*m_imageFile, minZ, e57::PrecisionDouble));
        cartesianBoundsNode.set("zMaximum", e57::FloatNode(*m_imageFile, maxZ, e57::PrecisionDouble));

        qDebug() << "E57WriterLib: Calculated cartesian bounds:"
                 << "X[" << minX << "," << maxX << "]"
                 << "Y[" << minY << "," << maxY << "]"
                 << "Z[" << minZ << "," << maxZ << "]";

        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "calculateAndWriteCartesianBounds");
        return false;
    } catch (const std::exception& ex) {
        setError(QString("Standard exception in calculateAndWriteCartesianBounds: %1").arg(ex.what()));
        return false;
    }
}

e57::StructureNode* E57WriterLib::getScanNode(int scanIndex)
{
    try {
        if (!m_data3DNode) {
            setError("Data3D vector not available");
            return nullptr;
        }

        if (scanIndex < 0 || scanIndex >= static_cast<int>(m_data3DNode->childCount())) {
            setError(QString("Scan index %1 out of range [0, %2]").arg(scanIndex).arg(m_data3DNode->childCount() - 1));
            return nullptr;
        }

        // Create a static shared_ptr to manage the lifetime properly
        static std::shared_ptr<e57::StructureNode> scanNodePtr;
        scanNodePtr = std::make_shared<e57::StructureNode>(e57::StructureNode(m_data3DNode->get(scanIndex)));
        return scanNodePtr.get();

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "getScanNode");
        return nullptr;
    } catch (const std::exception& ex) {
        setError(QString("Standard exception in getScanNode: %1").arg(ex.what()));
        return nullptr;
    }
}

// Sprint W3: Helper method implementations for intensity and color support

bool E57WriterLib::calculateAndWriteIntensityLimits(e57::StructureNode& scanNode, const std::vector<Point3D>& points)
{
    try {
        // Task W3.4.1: Calculate min/max intensity values from actual point data
        float minIntensity = std::numeric_limits<float>::infinity();
        float maxIntensity = -std::numeric_limits<float>::infinity();
        bool hasAnyIntensity = false;

        for (const Point3D& point : points) {
            if (point.hasIntensity) {
                minIntensity = std::min(minIntensity, point.intensity);
                maxIntensity = std::max(maxIntensity, point.intensity);
                hasAnyIntensity = true;
            }
        }

        // Task W3.4.6: Handle case where no valid intensity data exists
        if (!hasAnyIntensity) {
            minIntensity = maxIntensity = 0.0f;
        }

        // Task W3.4.4: Create intensityLimits StructureNode
        e57::StructureNode intensityLimitsNode(*m_imageFile);
        scanNode.set("intensityLimits", intensityLimitsNode);

        // Populate with FloatNode children for normalized intensity (0.0-1.0)
        intensityLimitsNode.set("intensityMinimum", e57::FloatNode(*m_imageFile, minIntensity, e57::PrecisionSingle));
        intensityLimitsNode.set("intensityMaximum", e57::FloatNode(*m_imageFile, maxIntensity, e57::PrecisionSingle));

        qDebug() << "E57WriterLib: Calculated intensity limits:"
                 << "min=" << minIntensity << "max=" << maxIntensity;

        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "calculateAndWriteIntensityLimits");
        return false;
    } catch (const std::exception& ex) {
        setError(QString("Standard exception in calculateAndWriteIntensityLimits: %1").arg(ex.what()));
        return false;
    }
}

bool E57WriterLib::calculateAndWriteColorLimits(e57::StructureNode& scanNode, const std::vector<Point3D>& points)
{
    try {
        // Task W3.4.2: Calculate min/max color values from actual point data
        uint8_t minRed = 255, maxRed = 0;
        uint8_t minGreen = 255, maxGreen = 0;
        uint8_t minBlue = 255, maxBlue = 0;
        bool hasAnyColor = false;

        for (const Point3D& point : points) {
            if (point.hasColor) {
                minRed = std::min(minRed, point.colorRed);
                maxRed = std::max(maxRed, point.colorRed);
                minGreen = std::min(minGreen, point.colorGreen);
                maxGreen = std::max(maxGreen, point.colorGreen);
                minBlue = std::min(minBlue, point.colorBlue);
                maxBlue = std::max(maxBlue, point.colorBlue);
                hasAnyColor = true;
            }
        }

        // Task W3.4.6: Handle case where no valid color data exists
        if (!hasAnyColor) {
            minRed = maxRed = minGreen = maxGreen = minBlue = maxBlue = 0;
        }

        // Task W3.4.5: Create colorLimits StructureNode
        e57::StructureNode colorLimitsNode(*m_imageFile);
        scanNode.set("colorLimits", colorLimitsNode);

        // Populate with IntegerNode children for 8-bit color (0-255)
        colorLimitsNode.set("colorRedMinimum", e57::IntegerNode(*m_imageFile, minRed, 0, 255));
        colorLimitsNode.set("colorRedMaximum", e57::IntegerNode(*m_imageFile, maxRed, 0, 255));
        colorLimitsNode.set("colorGreenMinimum", e57::IntegerNode(*m_imageFile, minGreen, 0, 255));
        colorLimitsNode.set("colorGreenMaximum", e57::IntegerNode(*m_imageFile, maxGreen, 0, 255));
        colorLimitsNode.set("colorBlueMinimum", e57::IntegerNode(*m_imageFile, minBlue, 0, 255));
        colorLimitsNode.set("colorBlueMaximum", e57::IntegerNode(*m_imageFile, maxBlue, 0, 255));

        qDebug() << "E57WriterLib: Calculated color limits:"
                 << "R[" << minRed << "," << maxRed << "]"
                 << "G[" << minGreen << "," << maxGreen << "]"
                 << "B[" << minBlue << "," << maxBlue << "]";

        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "calculateAndWriteColorLimits");
        return false;
    } catch (const std::exception& ex) {
        setError(QString("Standard exception in calculateAndWriteColorLimits: %1").arg(ex.what()));
        return false;
    }
}

bool E57WriterLib::hasValidIntensityData(const std::vector<Point3D>& points) const
{
    for (const Point3D& point : points) {
        if (point.hasIntensity) {
            return true;
        }
    }
    return false;
}

bool E57WriterLib::hasValidColorData(const std::vector<Point3D>& points) const
{
    for (const Point3D& point : points) {
        if (point.hasColor) {
            return true;
        }
    }
    return false;
}
