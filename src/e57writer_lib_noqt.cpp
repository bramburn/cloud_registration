#include "e57writer_lib_noqt.h"
#include <E57Format.h>
#include <iostream>
#include <cfloat>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

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
