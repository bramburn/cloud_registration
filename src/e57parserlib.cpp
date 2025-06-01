#include "e57parserlib.h"
#include <E57Format.h>
#include <sstream>
#include <QString>

E57ParserLib::E57ParserLib(QObject *parent)
    : QObject(parent), m_imageFile(nullptr)
{
}

E57ParserLib::~E57ParserLib() {
    closeFile();
}

bool E57ParserLib::openFile(const std::string& filePath) {
    try {
        closeFile();
        clearError();
        
        // Create new ImageFile instance
        m_imageFile = std::make_unique<e57::ImageFile>(filePath, "r");
        
        if (!m_imageFile->isOpen()) {
            setError("Failed to open file handle");
            return false;
        }
        
        return true;
        
    } catch (const e57::E57Exception& ex) {
        setError(std::string("E57 Exception: ") + ex.what());
        return false;
    } catch (const std::exception& ex) {
        setError(std::string("Standard exception: ") + ex.what());
        return false;
    }
}

void E57ParserLib::closeFile() {
    if (m_imageFile) {
        try {
            if (m_imageFile->isOpen()) {
                m_imageFile->close();
            }
        } catch (const e57::E57Exception& ex) {
            // Log error but don't throw in destructor path
            setError(std::string("E57 Exception during close: ") + ex.what());
        }
        m_imageFile.reset();
    }
}

std::string E57ParserLib::getGuid() const {
    if (!m_imageFile || !m_imageFile->isOpen()) {
        return "";
    }
    
    try {
        e57::StructureNode root = m_imageFile->root();
        if (root.isDefined("guid")) {
            e57::StringNode guidNode = static_cast<e57::StringNode>(root.get("guid"));
            return guidNode.value();
        }
    } catch (const e57::E57Exception&) {
        // Return empty string on error
    }
    
    return "";
}

std::pair<int, int> E57ParserLib::getVersion() const {
    if (!m_imageFile || !m_imageFile->isOpen()) {
        return {0, 0};
    }

    try {
        // Get version from root structure
        e57::StructureNode root = m_imageFile->root();
        if (root.isDefined("formatName")) {
            // For now, return a default version since the API doesn't expose version directly
            return {1, 0}; // Default E57 version
        }
    } catch (const e57::E57Exception&) {
        return {0, 0};
    }

    return {0, 0};
}

int E57ParserLib::getScanCount() const {
    if (!m_imageFile || !m_imageFile->isOpen()) {
        return 0;
    }
    
    try {
        e57::StructureNode root = m_imageFile->root();
        if (root.isDefined("/data3D")) {
            e57::VectorNode data3D = static_cast<e57::VectorNode>(root.get("/data3D"));
            return static_cast<int>(data3D.childCount());
        }
    } catch (const e57::E57Exception&) {
        // Return 0 on error
    }
    
    return 0;
}

std::string E57ParserLib::getLastError() const {
    return m_lastError;
}

bool E57ParserLib::isOpen() const {
    return m_imageFile && m_imageFile->isOpen();
}

void E57ParserLib::clearError() {
    m_lastError.clear();
}

void E57ParserLib::setError(const std::string& error) {
    m_lastError = error;
}

// Sprint 2: Point data extraction methods

std::vector<float> E57ParserLib::extractPointData() {
    return extractPointData(0);
}

std::vector<float> E57ParserLib::extractPointData(int scanIndex) {
    try {
        clearError();
        m_points.clear();

        if (!isOpen()) {
            setError("No E57 file is open");
            emit parsingFinished(false, QString::fromStdString(getLastError()), std::vector<float>());
            return std::vector<float>();
        }

        if (scanIndex < 0 || scanIndex >= getScanCount()) {
            setError("Invalid scan index: " + std::to_string(scanIndex));
            emit parsingFinished(false, QString::fromStdString(getLastError()), std::vector<float>());
            return std::vector<float>();
        }

        emit progressUpdated(10, "Accessing scan data...");

        // Task 2.1.1: Access the first Data3D StructureNode
        e57::StructureNode rootNode = m_imageFile->root();
        e57::VectorNode data3DVectorNode = static_cast<e57::VectorNode>(rootNode.get("/data3D"));

        if (data3DVectorNode.childCount() <= scanIndex) {
            setError("Scan index out of range");
            emit parsingFinished(false, QString::fromStdString(getLastError()), std::vector<float>());
            return std::vector<float>();
        }

        e57::StructureNode scanHeaderNode = static_cast<e57::StructureNode>(data3DVectorNode.get(scanIndex));

        emit progressUpdated(20, "Inspecting point prototype...");

        // Task 2.1.2 & 2.1.3: Inspect the point prototype
        if (!inspectPointPrototype(scanHeaderNode)) {
            emit parsingFinished(false, QString::fromStdString(getLastError()), std::vector<float>());
            return std::vector<float>();
        }

        emit progressUpdated(30, "Extracting point data...");

        // Task 2.2: Extract the actual point data
        if (!extractUncompressedXYZData(scanHeaderNode)) {
            emit parsingFinished(false, QString::fromStdString(getLastError()), std::vector<float>());
            return std::vector<float>();
        }

        emit progressUpdated(100, "Point extraction complete");
        emit parsingFinished(true, QString("Successfully extracted %1 points").arg(m_points.size() / 3), m_points);

        return m_points;

    } catch (const e57::E57Exception& ex) {
        setError(std::string("E57 Exception during point extraction: ") + ex.what());
        emit parsingFinished(false, QString::fromStdString(getLastError()), std::vector<float>());
        return std::vector<float>();
    } catch (const std::exception& ex) {
        setError(std::string("Standard exception during point extraction: ") + ex.what());
        emit parsingFinished(false, QString::fromStdString(getLastError()), std::vector<float>());
        return std::vector<float>();
    }
}

int64_t E57ParserLib::getPointCount(int scanIndex) const {
    try {
        if (!isOpen() || scanIndex < 0 || scanIndex >= getScanCount()) {
            return 0;
        }

        e57::StructureNode rootNode = m_imageFile->root();
        e57::VectorNode data3DVectorNode = static_cast<e57::VectorNode>(rootNode.get("/data3D"));
        e57::StructureNode scanHeaderNode = static_cast<e57::StructureNode>(data3DVectorNode.get(scanIndex));

        if (scanHeaderNode.isDefined("points")) {
            e57::Node pointsNode = scanHeaderNode.get("points");
            if (pointsNode.type() == e57::TypeCompressedVector) {
                e57::CompressedVectorNode cvNode = static_cast<e57::CompressedVectorNode>(pointsNode);
                return cvNode.childCount();
            }
        }

    } catch (const e57::E57Exception&) {
        // Return 0 on error
    }

    return 0;
}

// Sprint 2: Helper methods for point data extraction

bool E57ParserLib::inspectPointPrototype(const e57::StructureNode& scanHeaderNode) {
    try {
        // Task 2.1.2: Locate the points child node and validate it's a CompressedVectorNode
        if (!scanHeaderNode.isDefined("points")) {
            setError("Scan header does not contain 'points' node");
            return false;
        }

        e57::Node pointsNode = scanHeaderNode.get("points");
        if (pointsNode.type() != e57::TypeCompressedVector) {
            setError("Points node is not a CompressedVectorNode");
            return false;
        }

        e57::CompressedVectorNode cvNode = static_cast<e57::CompressedVectorNode>(pointsNode);
        e57::StructureNode pointPrototype = static_cast<e57::StructureNode>(cvNode.prototype());

        // Task 2.1.3: Inspect the prototype to identify cartesianX, Y, Z fields
        validatePrototypeFields(pointPrototype);

        // Task 2.1.4: Handle cases where cartesianX/Y/Z are missing
        if (!m_prototypeInfo.hasCartesianX || !m_prototypeInfo.hasCartesianY || !m_prototypeInfo.hasCartesianZ) {
            std::string missingFields;
            if (!m_prototypeInfo.hasCartesianX) missingFields += "cartesianX ";
            if (!m_prototypeInfo.hasCartesianY) missingFields += "cartesianY ";
            if (!m_prototypeInfo.hasCartesianZ) missingFields += "cartesianZ ";

            setError("Missing required coordinate fields: " + missingFields);
            return false;
        }

        return true;

    } catch (const e57::E57Exception& ex) {
        setError(std::string("E57 Exception during prototype inspection: ") + ex.what());
        return false;
    }
}

void E57ParserLib::validatePrototypeFields(const e57::StructureNode& prototype) {
    // Reset prototype info
    m_prototypeInfo = PrototypeInfo();

    try {
        // Iterate through all children in the prototype
        for (int64_t i = 0; i < prototype.childCount(); ++i) {
            e57::Node fieldNode = prototype.get(i);
            std::string fieldName = fieldNode.elementName();
            e57::NodeType fieldType = fieldNode.type();

            // Check for cartesian coordinate fields
            if (fieldName == "cartesianX" && fieldType == e57::TypeFloat) {
                m_prototypeInfo.hasCartesianX = true;
                e57::FloatNode xNode = static_cast<e57::FloatNode>(fieldNode);
                m_prototypeInfo.isDoublePrec = (xNode.precision() == e57::PrecisionDouble);
            }
            else if (fieldName == "cartesianY" && fieldType == e57::TypeFloat) {
                m_prototypeInfo.hasCartesianY = true;
            }
            else if (fieldName == "cartesianZ" && fieldType == e57::TypeFloat) {
                m_prototypeInfo.hasCartesianZ = true;
            }
        }

    } catch (const e57::E57Exception& ex) {
        setError(std::string("E57 Exception during field validation: ") + ex.what());
    }
}

bool E57ParserLib::extractUncompressedXYZData(const e57::StructureNode& scanHeaderNode) {
    try {
        // Get the CompressedVectorNode for points
        e57::CompressedVectorNode cvNode = static_cast<e57::CompressedVectorNode>(scanHeaderNode.get("points"));
        int64_t totalPoints = cvNode.childCount();

        if (totalPoints == 0) {
            setError("No points found in scan");
            return false;
        }

        // Task 2.2.1: Prepare SourceDestBuffer objects
        const int64_t POINTS_PER_READ_BLOCK = 65536; // Read in blocks
        int64_t bufferSize = std::min(totalPoints, POINTS_PER_READ_BLOCK);

        // Prepare buffers for reading (use double precision as libE57Format prefers)
        std::vector<double> xBuffer_d(bufferSize);
        std::vector<double> yBuffer_d(bufferSize);
        std::vector<double> zBuffer_d(bufferSize);

        std::vector<e57::SourceDestBuffer> sdbufs;
        sdbufs.emplace_back(*m_imageFile, "cartesianX", xBuffer_d.data(), bufferSize, true, false, sizeof(double));
        sdbufs.emplace_back(*m_imageFile, "cartesianY", yBuffer_d.data(), bufferSize, true, false, sizeof(double));
        sdbufs.emplace_back(*m_imageFile, "cartesianZ", zBuffer_d.data(), bufferSize, true, false, sizeof(double));

        // Task 2.2.2: Create CompressedVectorReader
        e57::CompressedVectorReader reader = cvNode.reader(sdbufs);

        // Reserve space for the final point data (interleaved X,Y,Z)
        m_points.reserve(totalPoints * 3);

        int64_t pointsRead = 0;
        int lastProgressPercent = 30;

        // Task 2.2.3: Read point data in blocks
        try {
            while (pointsRead < totalPoints) {
                // Read a block of points
                uint64_t actualPointsRead = reader.read();

                if (actualPointsRead == 0) {
                    break; // No more data
                }

                // Task 2.2.4: Convert double to float and append to main vector
                for (uint64_t i = 0; i < actualPointsRead; ++i) {
                    m_points.push_back(static_cast<float>(xBuffer_d[i]));
                    m_points.push_back(static_cast<float>(yBuffer_d[i]));
                    m_points.push_back(static_cast<float>(zBuffer_d[i]));
                }

                pointsRead += actualPointsRead;

                // Emit progress updates
                int progressPercent = 30 + static_cast<int>((pointsRead * 70) / totalPoints);
                if (progressPercent > lastProgressPercent + 5) { // Update every 5%
                    emit progressUpdated(progressPercent, QString("Reading points... %1/%2").arg(pointsRead).arg(totalPoints));
                    lastProgressPercent = progressPercent;
                }
            }

            // Task 2.2.5: Close the reader
            reader.close();

        } catch (const e57::E57Exception& ex) {
            // Task 2.2.6: Handle reader errors
            reader.close();
            setError(std::string("E57 Exception during point reading: ") + ex.what());
            return false;
        }

        if (pointsRead != totalPoints) {
            setError("Warning: Read " + std::to_string(pointsRead) + " points, expected " + std::to_string(totalPoints));
            // Don't return false - partial data might still be useful
        }

        return true;

    } catch (const e57::E57Exception& ex) {
        setError(std::string("E57 Exception during point data extraction: ") + ex.what());
        return false;
    }
}
