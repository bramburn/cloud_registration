#include "e57writer_lib_noqt.h"
#include <E57Format.h>
#include <iostream>
#include <cfloat>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <limits>
#include <algorithm>

E57WriterLibNoQt::E57WriterLibNoQt()
    : m_fileOpen(false)
    , m_scanCount(0)
{
}

E57WriterLibNoQt::~E57WriterLibNoQt()
{
    if (m_fileOpen) {
        closeFile();
    }
}

bool E57WriterLibNoQt::createFile(const std::string& filePath)
{
    try {
        // Close any existing file
        if (m_fileOpen) {
            closeFile();
        }

        // Create ImageFile in write mode
        m_imageFile = std::make_unique<e57::ImageFile>(filePath, "w");
        
        if (!m_imageFile->isOpen()) {
            setError("Failed to open file handle");
            return false;
        }

        m_currentFilePath = filePath;
        m_fileOpen = true;
        m_scanCount = 0;

        // Setup basic E57Root elements
        if (!initializeE57Root()) {
            closeFile();
            return false;
        }

        std::cout << "E57WriterLibNoQt: Successfully created E57 file: " << filePath << std::endl;
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "createFile");
        return false;
    } catch (const std::exception& ex) {
        setError(std::string("Standard exception in createFile: ") + ex.what());
        return false;
    }
}

bool E57WriterLibNoQt::initializeE57Root()
{
    try {
        // Setup basic E57Root elements like formatName, guid
        e57::StructureNode rootNode = m_imageFile->root();

        // Required E57Root elements according to ASTM E2807 standard
        e57::StringNode formatNameNode(*m_imageFile, "ASTM E57 3D Imaging Data File");
        rootNode.set("formatName", formatNameNode);

        e57::StringNode guidNode(*m_imageFile, generateGUID());
        rootNode.set("guid", guidNode);

        // Add version information (required for valid E57 files)
        e57::IntegerNode versionMajorNode(*m_imageFile, 1, 0, 255);
        rootNode.set("versionMajor", versionMajorNode);

        e57::IntegerNode versionMinorNode(*m_imageFile, 0, 0, 255);
        rootNode.set("versionMinor", versionMinorNode);

        // Add creation date/time (ISO format)
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;

#ifdef _WIN32
        struct tm tm_buf;
        gmtime_s(&tm_buf, &time_t);
        ss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%SZ");
#else
        ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
#endif

        e57::StringNode dateTimeNode(*m_imageFile, ss.str());
        rootNode.set("creationDateTime", dateTimeNode);

        // Add coordinate metadata (required for proper E57 structure)
        e57::StringNode coordinateMetadataNode(*m_imageFile, "");
        rootNode.set("coordinateMetadata", coordinateMetadataNode);

        // Create the data3D vector immediately to ensure proper file structure
        e57::VectorNode data3DVector(*m_imageFile, false); // allowHeteroChildren = false for Data3D
        rootNode.set("data3D", data3DVector);

        std::cout << "E57WriterLibNoQt: Initialized E57Root with required elements and data3D vector" << std::endl;
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "initializeE57Root");
        return false;
    }
}

bool E57WriterLibNoQt::addScan(const std::string& scanName)
{
    if (!m_fileOpen) {
        setError("No file is currently open for writing");
        return false;
    }

    try {
        // Create /data3D VectorNode if it doesn't exist
        if (!createData3DVectorNode()) {
            return false;
        }

        // Create a new StructureNode for this scan
        if (!createScanStructureNode(scanName)) {
            return false;
        }

        m_scanCount++;
        std::cout << "E57WriterLibNoQt: Successfully added scan: " << scanName << std::endl;
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "addScan");
        return false;
    } catch (const std::exception& ex) {
        setError(std::string("Standard exception in addScan: ") + ex.what());
        return false;
    }
}

bool E57WriterLibNoQt::createData3DVectorNode()
{
    try {
        e57::StructureNode rootNode = m_imageFile->root();

        // The data3D vector should already exist from initializeE57Root()
        if (rootNode.isDefined("data3D")) {
            // Get existing data3D node - use constructor-based conversion instead of static_cast
            e57::VectorNode existingNode(rootNode.get("data3D"));
            m_data3DNode = std::make_shared<e57::VectorNode>(existingNode);
            std::cout << "E57WriterLibNoQt: Using existing /data3D VectorNode" << std::endl;
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

bool E57WriterLibNoQt::createScanStructureNode(const std::string& scanName)
{
    try {
        // Create a new StructureNode for the scan header
        e57::StructureNode scanHeaderNode(*m_imageFile);

        // Populate the Data3D StructureNode with mandatory header elements
        e57::StringNode guidNode(*m_imageFile, generateGUID());
        scanHeaderNode.set("guid", guidNode);

        e57::StringNode nameNode(*m_imageFile, scanName);
        scanHeaderNode.set("name", nameNode);

        // Add the scan to the /data3D vector
        m_data3DNode->append(scanHeaderNode);

        // Store reference for later use (e.g., adding points)
        m_currentScanNode = std::make_shared<e57::StructureNode>(scanHeaderNode);

        std::cout << "E57WriterLibNoQt: Created scan structure node with name: " << scanName << std::endl;
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "createScanStructureNode");
        return false;
    }
}

std::string E57WriterLibNoQt::generateGUID() const
{
    // Generate a simple UUID-like string
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "{";
    for (int i = 0; i < 8; i++) ss << std::hex << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << std::hex << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << std::hex << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; i++) ss << std::hex << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; i++) ss << std::hex << dis(gen);
    ss << "}";
    
    return ss.str();
}

bool E57WriterLibNoQt::defineXYZPrototype()
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
        // Create a StructureNode to serve as the prototype
        e57::StructureNode prototypeStructure(*m_imageFile);

        // Add three FloatNode children for XYZ coordinates
        e57::FloatNode xNode(*m_imageFile, 0.0, e57::PrecisionDouble, -DBL_MAX, DBL_MAX);
        prototypeStructure.set("cartesianX", xNode);

        e57::FloatNode yNode(*m_imageFile, 0.0, e57::PrecisionDouble, -DBL_MAX, DBL_MAX);
        prototypeStructure.set("cartesianY", yNode);

        e57::FloatNode zNode(*m_imageFile, 0.0, e57::PrecisionDouble, -DBL_MAX, DBL_MAX);
        prototypeStructure.set("cartesianZ", zNode);

        // Create the required 'codecs' VectorNode for the CompressedVectorNode
        e57::VectorNode codecsNode(*m_imageFile, true); // allowHeterogeneousChildren = true for codecs

        // Create a proper CompressedVectorNode with the prototype and codecs
        e57::CompressedVectorNode pointsNode(*m_imageFile, prototypeStructure, codecsNode);

        // Add the points CompressedVectorNode to the scan
        m_currentScanNode->set("points", pointsNode);

        std::cout << "E57WriterLibNoQt: Successfully defined XYZ prototype and created CompressedVectorNode for current scan" << std::endl;
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "defineXYZPrototype");
        return false;
    } catch (const std::exception& ex) {
        setError(std::string("Standard exception in defineXYZPrototype: ") + ex.what());
        return false;
    }
}

// Sprint W2: Point writing implementation
bool E57WriterLibNoQt::writePoints(const std::vector<Point3D>& points)
{
    if (!m_fileOpen) {
        setError("Cannot write points: No file is open");
        return false;
    }

    if (!m_currentScanNode) {
        setError("Cannot write points: No current scan available. Call addScan() first");
        return false;
    }

    return writePointsToScan(*m_currentScanNode, points);
}

bool E57WriterLibNoQt::writePoints(int scanIndex, const std::vector<Point3D>& points)
{
    if (!m_fileOpen) {
        setError("Cannot write points: No file is open");
        return false;
    }

    e57::StructureNode* scanNode = getScanNode(scanIndex);
    if (!scanNode) {
        setError("Cannot write points: Invalid scan index " + std::to_string(scanIndex));
        return false;
    }

    return writePointsToScan(*scanNode, points);
}

bool E57WriterLibNoQt::writePointsToScan(e57::StructureNode& scanNode, const std::vector<Point3D>& points)
{
    try {
        // Task W2.1.2: Retrieve the points CompressedVectorNode for the scan
        if (!scanNode.isDefined("points")) {
            setError("Scan does not have a points CompressedVectorNode. Call defineXYZPrototype() first");
            return false;
        }

        e57::CompressedVectorNode pointsNode(scanNode.get("points"));

        // Task W2.2.1: Calculate cartesian bounds before writing points
        if (!calculateAndWriteCartesianBounds(scanNode, points)) {
            return false; // Error already set
        }

        // Handle empty point set
        if (points.empty()) {
            std::cout << "E57WriterLibNoQt: Writing 0 points to scan" << std::endl;
            return true;
        }

        // Task W2.1.3: Prepare SourceDestBuffer for XYZ coordinates
        const int64_t POINTS_PER_WRITE_BLOCK = 10000; // Block size for memory management
        std::vector<double> xAppBlockBuffer(POINTS_PER_WRITE_BLOCK);
        std::vector<double> yAppBlockBuffer(POINTS_PER_WRITE_BLOCK);
        std::vector<double> zAppBlockBuffer(POINTS_PER_WRITE_BLOCK);

        std::vector<e57::SourceDestBuffer> sdbufs;
        sdbufs.emplace_back(m_imageFile.get(), "cartesianX", xAppBlockBuffer.data(), POINTS_PER_WRITE_BLOCK, true, false, sizeof(double));
        sdbufs.emplace_back(m_imageFile.get(), "cartesianY", yAppBlockBuffer.data(), POINTS_PER_WRITE_BLOCK, true, false, sizeof(double));
        sdbufs.emplace_back(m_imageFile.get(), "cartesianZ", zAppBlockBuffer.data(), POINTS_PER_WRITE_BLOCK, true, false, sizeof(double));

        // Task W2.1.4: Create CompressedVectorWriter
        e57::CompressedVectorWriter writer = pointsNode.writer(sdbufs);

        // Task W2.1.5: Write points in blocks
        size_t totalPoints = points.size();
        size_t pointsWritten = 0;

        while (pointsWritten < totalPoints) {
            // Determine points in current block
            size_t pointsInBlock = std::min(static_cast<size_t>(POINTS_PER_WRITE_BLOCK), totalPoints - pointsWritten);

            // Copy data to block buffers
            for (size_t i = 0; i < pointsInBlock; ++i) {
                const Point3D& point = points[pointsWritten + i];
                xAppBlockBuffer[i] = point.x;
                yAppBlockBuffer[i] = point.y;
                zAppBlockBuffer[i] = point.z;
            }

            // Write current block
            writer.write(pointsInBlock);
            pointsWritten += pointsInBlock;
        }

        // Task W2.1.6: Close writer to finalize
        writer.close();

        std::cout << "E57WriterLibNoQt: Successfully wrote " << totalPoints << " points to scan" << std::endl;
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "writePointsToScan");
        return false;
    } catch (const std::exception& ex) {
        setError(std::string("Standard exception in writePointsToScan: ") + ex.what());
        return false;
    }
}

bool E57WriterLibNoQt::closeFile()
{
    if (!m_fileOpen) {
        return true; // Already closed
    }

    try {
        if (m_imageFile) {
            // Close the file - this triggers the actual write to disk
            m_imageFile->close();
            m_imageFile.reset();
        }

        m_fileOpen = false;
        m_currentScanNode.reset();
        m_data3DNode.reset();

        std::cout << "E57WriterLibNoQt: Successfully closed E57 file: " << m_currentFilePath << std::endl;
        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "closeFile");
        return false;
    } catch (const std::exception& ex) {
        setError(std::string("Standard exception in closeFile: ") + ex.what());
        return false;
    }
}

std::string E57WriterLibNoQt::getLastError() const
{
    return m_lastError;
}

bool E57WriterLibNoQt::isFileOpen() const
{
    return m_fileOpen;
}

std::string E57WriterLibNoQt::getCurrentFilePath() const
{
    return m_currentFilePath;
}

void E57WriterLibNoQt::setError(const std::string& errorMessage)
{
    m_lastError = errorMessage;
    std::cerr << "E57WriterLibNoQt Error: " << errorMessage << std::endl;
}

void E57WriterLibNoQt::handleE57Exception(const std::exception& ex, const std::string& context)
{
    std::string errorMsg = "E57 Exception in " + context + ": " + ex.what();
    setError(errorMsg);
}

// Sprint W2: Helper method implementations
bool E57WriterLibNoQt::calculateAndWriteCartesianBounds(e57::StructureNode& scanNode, const std::vector<Point3D>& points)
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

        std::cout << "E57WriterLibNoQt: Calculated cartesian bounds: "
                  << "X[" << minX << "," << maxX << "] "
                  << "Y[" << minY << "," << maxY << "] "
                  << "Z[" << minZ << "," << maxZ << "]" << std::endl;

        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "calculateAndWriteCartesianBounds");
        return false;
    } catch (const std::exception& ex) {
        setError(std::string("Standard exception in calculateAndWriteCartesianBounds: ") + ex.what());
        return false;
    }
}

e57::StructureNode* E57WriterLibNoQt::getScanNode(int scanIndex)
{
    try {
        if (!m_data3DNode) {
            setError("Data3D vector not available");
            return nullptr;
        }

        if (scanIndex < 0 || scanIndex >= static_cast<int>(m_data3DNode->childCount())) {
            setError("Scan index " + std::to_string(scanIndex) + " out of range [0, " + std::to_string(m_data3DNode->childCount() - 1) + "]");
            return nullptr;
        }

        // Return pointer to scan node - caller must handle lifetime
        static e57::StructureNode scanNode;
        scanNode = e57::StructureNode(m_data3DNode->get(scanIndex));
        return &scanNode;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "getScanNode");
        return nullptr;
    } catch (const std::exception& ex) {
        setError(std::string("Standard exception in getScanNode: ") + ex.what());
        return nullptr;
    }
}
