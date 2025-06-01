#include "e57parserlib.h"
#include <E57Format.h>
#include <sstream>
#include <QString>
#include <QDebug>

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

// Sprint 4: Multi-scan support enhancement
E57ParserLib::ScanMetadata E57ParserLib::getScanMetadata(int scanIndex) const {
    ScanMetadata metadata;

    if (!m_imageFile || !m_imageFile->isOpen()) {
        setError("No E57 file is open");
        return metadata;
    }

    try {
        e57::StructureNode root = m_imageFile->root();

        if (!root.isDefined("data3D")) {
            return metadata;
        }

        e57::VectorNode data3DVector(root.get("data3D"));
        int64_t scanCount = data3DVector.childCount();

        if (scanIndex < 0 || scanIndex >= scanCount) {
            return metadata;
        }

        e57::StructureNode scanHeader(data3DVector.get(scanIndex));

        // Fill metadata
        metadata.index = scanIndex;

        // Get scan name if available
        if (scanHeader.isDefined("name")) {
            e57::StringNode nameNode(scanHeader.get("name"));
            metadata.name = nameNode.value();
        } else {
            metadata.name = QString("Scan %1").arg(scanIndex).toStdString();
        }

        // Get scan GUID if available
        if (scanHeader.isDefined("guid")) {
            e57::StringNode guidNode(scanHeader.get("guid"));
            metadata.guid = guidNode.value();
        }

        // Get point count
        if (scanHeader.isDefined("points")) {
            e57::CompressedVectorNode points(scanHeader.get("points"));
            metadata.pointCount = points.childCount();
        }

        // Check for intensity and color data
        if (scanHeader.isDefined("points")) {
            e57::CompressedVectorNode points(scanHeader.get("points"));
            e57::StructureNode prototype = points.prototype();

            metadata.hasIntensity = prototype.isDefined("intensity");
            metadata.hasColor = prototype.isDefined("colorRed") &&
                               prototype.isDefined("colorGreen") &&
                               prototype.isDefined("colorBlue");
        }

        metadata.isLoaded = false; // Not tracking loaded state in this implementation

        return metadata;

    } catch (const e57::E57Exception& ex) {
        setError(std::string("E57 Exception during scan metadata retrieval: ") + ex.what());
        return metadata;
    } catch (const std::exception& ex) {
        setError(std::string("Standard exception during scan metadata retrieval: ") + ex.what());
        return metadata;
    }
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

// Sprint 3: Enhanced point data extraction with intensity and color

std::vector<E57ParserLib::PointData> E57ParserLib::extractEnhancedPointData(int scanIndex) {
    try {
        clearError();
        std::vector<PointData> points;

        if (!isOpen()) {
            setError("No E57 file is open");
            emit parsingFinished(false, QString::fromStdString(getLastError()), std::vector<float>());
            return points;
        }

        if (scanIndex < 0 || scanIndex >= getScanCount()) {
            setError("Invalid scan index: " + std::to_string(scanIndex));
            emit parsingFinished(false, QString::fromStdString(getLastError()), std::vector<float>());
            return points;
        }

        emit progressUpdated(10, "Accessing scan data...");

        // Access the scan data
        e57::StructureNode root = m_imageFile->root();
        e57::VectorNode data3DVectorNode = static_cast<e57::VectorNode>(root.get("/data3D"));
        e57::StructureNode scanHeaderNode = static_cast<e57::StructureNode>(data3DVectorNode.get(scanIndex));

        emit progressUpdated(20, "Inspecting enhanced prototype...");

        // Task 3.1.1 & 3.2.1: Enhanced prototype inspection for intensity and color
        if (!inspectEnhancedPrototype(scanHeaderNode)) {
            emit parsingFinished(false, QString::fromStdString(getLastError()), std::vector<float>());
            return points;
        }

        emit progressUpdated(25, "Extracting data limits...");

        // Task 3.1.4 & 3.2.4: Extract intensity and color limits for normalization
        if (!extractDataLimits(scanHeaderNode)) {
            // Non-fatal error - continue with default limits
            qDebug() << "Warning: Could not extract data limits, using defaults";
        }

        emit progressUpdated(30, "Extracting enhanced point data...");

        // Task 3.3: Extract point data with all available attributes
        if (!extractEnhancedPointData(scanHeaderNode, points)) {
            emit parsingFinished(false, QString::fromStdString(getLastError()), std::vector<float>());
            return points;
        }

        emit progressUpdated(100, "Enhanced point extraction complete");
        emit parsingFinished(true, QString("Successfully extracted %1 enhanced points").arg(points.size()), std::vector<float>());

        return points;

    } catch (const e57::E57Exception& ex) {
        setError(std::string("E57 Exception during enhanced point extraction: ") + ex.what());
        emit parsingFinished(false, QString::fromStdString(getLastError()), std::vector<float>());
        return std::vector<PointData>();
    } catch (const std::exception& ex) {
        setError(std::string("Standard exception during enhanced point extraction: ") + ex.what());
        emit parsingFinished(false, QString::fromStdString(getLastError()), std::vector<float>());
        return std::vector<PointData>();
    }
}

// Sprint 3: Enhanced prototype inspection for intensity and color fields

bool E57ParserLib::inspectEnhancedPrototype(const e57::StructureNode& scanHeaderNode) {
    try {
        // First, do the basic XYZ prototype inspection
        if (!inspectPointPrototype(scanHeaderNode)) {
            return false;
        }

        // Get the CompressedVectorNode for points
        e57::CompressedVectorNode cvNode = static_cast<e57::CompressedVectorNode>(scanHeaderNode.get("points"));
        e57::StructureNode prototype = cvNode.prototype();

        // Task 3.1.1: Check for intensity field
        if (prototype.isDefined("intensity")) {
            m_prototypeInfo.hasIntensity = true;
            e57::Node intensityNode = prototype.get("intensity");

            switch (intensityNode.type()) {
                case e57::E57_FLOAT:
                    m_prototypeInfo.intensityDataType = "float";
                    break;
                case e57::E57_INTEGER:
                    m_prototypeInfo.intensityDataType = "integer";
                    break;
                case e57::E57_SCALED_INTEGER:
                    m_prototypeInfo.intensityDataType = "scaledInteger";
                    break;
                default:
                    qDebug() << "Warning: Unsupported intensity data type, treating as float";
                    m_prototypeInfo.intensityDataType = "float";
                    break;
            }

            qDebug() << "Found intensity field with type:" << QString::fromStdString(m_prototypeInfo.intensityDataType);
        } else {
            m_prototypeInfo.hasIntensity = false;
            qDebug() << "No intensity field found in prototype";
        }

        // Task 3.2.1: Check for color fields
        m_prototypeInfo.hasColorRed = prototype.isDefined("colorRed");
        m_prototypeInfo.hasColorGreen = prototype.isDefined("colorGreen");
        m_prototypeInfo.hasColorBlue = prototype.isDefined("colorBlue");

        if (m_prototypeInfo.hasColorRed || m_prototypeInfo.hasColorGreen || m_prototypeInfo.hasColorBlue) {
            // Determine color data type from the first available color channel
            e57::Node colorNode;
            if (m_prototypeInfo.hasColorRed) {
                colorNode = prototype.get("colorRed");
            } else if (m_prototypeInfo.hasColorGreen) {
                colorNode = prototype.get("colorGreen");
            } else {
                colorNode = prototype.get("colorBlue");
            }

            switch (colorNode.type()) {
                case e57::E57_INTEGER:
                    m_prototypeInfo.colorDataType = "integer";
                    break;
                case e57::E57_SCALED_INTEGER:
                    m_prototypeInfo.colorDataType = "scaledInteger";
                    break;
                default:
                    qDebug() << "Warning: Unsupported color data type, treating as integer";
                    m_prototypeInfo.colorDataType = "integer";
                    break;
            }

            qDebug() << "Found color fields - Red:" << m_prototypeInfo.hasColorRed
                     << "Green:" << m_prototypeInfo.hasColorGreen
                     << "Blue:" << m_prototypeInfo.hasColorBlue
                     << "Type:" << QString::fromStdString(m_prototypeInfo.colorDataType);
        } else {
            qDebug() << "No color fields found in prototype";
        }

        return true;

    } catch (const e57::E57Exception& ex) {
        setError(std::string("E57 Exception during enhanced prototype inspection: ") + ex.what());
        return false;
    }
}

// Sprint 3: Extract intensity and color limits for normalization

bool E57ParserLib::extractDataLimits(const e57::StructureNode& scanHeaderNode) {
    try {
        // Reset limits to defaults
        m_dataLimits = DataLimits();

        // Task 3.1.4: Extract intensity limits
        if (m_prototypeInfo.hasIntensity && scanHeaderNode.isDefined("intensityLimits")) {
            e57::StructureNode intensityLimits = static_cast<e57::StructureNode>(scanHeaderNode.get("intensityLimits"));

            if (intensityLimits.isDefined("intensityMinimum")) {
                e57::Node minNode = intensityLimits.get("intensityMinimum");
                if (minNode.type() == e57::E57_FLOAT) {
                    m_dataLimits.intensityMin = static_cast<e57::FloatNode>(minNode).value();
                } else if (minNode.type() == e57::E57_INTEGER) {
                    m_dataLimits.intensityMin = static_cast<double>(static_cast<e57::IntegerNode>(minNode).value());
                } else if (minNode.type() == e57::E57_SCALED_INTEGER) {
                    m_dataLimits.intensityMin = static_cast<e57::ScaledIntegerNode>(minNode).scaledValue();
                }
            }

            if (intensityLimits.isDefined("intensityMaximum")) {
                e57::Node maxNode = intensityLimits.get("intensityMaximum");
                if (maxNode.type() == e57::E57_FLOAT) {
                    m_dataLimits.intensityMax = static_cast<e57::FloatNode>(maxNode).value();
                } else if (maxNode.type() == e57::E57_INTEGER) {
                    m_dataLimits.intensityMax = static_cast<double>(static_cast<e57::IntegerNode>(maxNode).value());
                } else if (maxNode.type() == e57::E57_SCALED_INTEGER) {
                    m_dataLimits.intensityMax = static_cast<e57::ScaledIntegerNode>(maxNode).scaledValue();
                }
            }

            m_dataLimits.hasIntensityLimits = true;
            qDebug() << "Extracted intensity limits: min=" << m_dataLimits.intensityMin
                     << "max=" << m_dataLimits.intensityMax;
        }

        // Task 3.2.4: Extract color limits
        if ((m_prototypeInfo.hasColorRed || m_prototypeInfo.hasColorGreen || m_prototypeInfo.hasColorBlue)
            && scanHeaderNode.isDefined("colorLimits")) {

            e57::StructureNode colorLimits = static_cast<e57::StructureNode>(scanHeaderNode.get("colorLimits"));

            // Extract red limits
            if (colorLimits.isDefined("colorRedMinimum") && colorLimits.isDefined("colorRedMaximum")) {
                e57::Node redMinNode = colorLimits.get("colorRedMinimum");
                e57::Node redMaxNode = colorLimits.get("colorRedMaximum");

                if (redMinNode.type() == e57::E57_INTEGER) {
                    m_dataLimits.colorRedMin = static_cast<double>(static_cast<e57::IntegerNode>(redMinNode).value());
                }
                if (redMaxNode.type() == e57::E57_INTEGER) {
                    m_dataLimits.colorRedMax = static_cast<double>(static_cast<e57::IntegerNode>(redMaxNode).value());
                }
            }

            // Extract green limits
            if (colorLimits.isDefined("colorGreenMinimum") && colorLimits.isDefined("colorGreenMaximum")) {
                e57::Node greenMinNode = colorLimits.get("colorGreenMinimum");
                e57::Node greenMaxNode = colorLimits.get("colorGreenMaximum");

                if (greenMinNode.type() == e57::E57_INTEGER) {
                    m_dataLimits.colorGreenMin = static_cast<double>(static_cast<e57::IntegerNode>(greenMinNode).value());
                }
                if (greenMaxNode.type() == e57::E57_INTEGER) {
                    m_dataLimits.colorGreenMax = static_cast<double>(static_cast<e57::IntegerNode>(greenMaxNode).value());
                }
            }

            // Extract blue limits
            if (colorLimits.isDefined("colorBlueMinimum") && colorLimits.isDefined("colorBlueMaximum")) {
                e57::Node blueMinNode = colorLimits.get("colorBlueMinimum");
                e57::Node blueMaxNode = colorLimits.get("colorBlueMaximum");

                if (blueMinNode.type() == e57::E57_INTEGER) {
                    m_dataLimits.colorBlueMin = static_cast<double>(static_cast<e57::IntegerNode>(blueMinNode).value());
                }
                if (blueMaxNode.type() == e57::E57_INTEGER) {
                    m_dataLimits.colorBlueMax = static_cast<double>(static_cast<e57::IntegerNode>(blueMaxNode).value());
                }
            }

            m_dataLimits.hasColorLimits = true;
            qDebug() << "Extracted color limits: R(" << m_dataLimits.colorRedMin << "-" << m_dataLimits.colorRedMax << ")"
                     << "G(" << m_dataLimits.colorGreenMin << "-" << m_dataLimits.colorGreenMax << ")"
                     << "B(" << m_dataLimits.colorBlueMin << "-" << m_dataLimits.colorBlueMax << ")";
        }

        return true;

    } catch (const e57::E57Exception& ex) {
        setError(std::string("E57 Exception during data limits extraction: ") + ex.what());
        return false;
    }
}

// Sprint 3: Normalization helper methods

float E57ParserLib::normalizeIntensity(float rawValue) const {
    if (!m_dataLimits.hasIntensityLimits) {
        // No limits available, assume already normalized or use as-is
        return std::max(0.0f, std::min(1.0f, rawValue));
    }

    // Check for degenerate case where min == max
    if (std::abs(m_dataLimits.intensityMax - m_dataLimits.intensityMin) < 1e-6) {
        qDebug() << "Warning: Intensity min equals max, returning 0.5";
        return 0.5f;
    }

    // Apply normalization formula: (rawValue - min) / (max - min)
    float normalized = static_cast<float>((rawValue - m_dataLimits.intensityMin) /
                                         (m_dataLimits.intensityMax - m_dataLimits.intensityMin));

    // Clamp to [0.0, 1.0] range
    return std::max(0.0f, std::min(1.0f, normalized));
}

uint8_t E57ParserLib::normalizeColorChannel(float rawValue, double minVal, double maxVal) const {
    // Check for degenerate case where min == max
    if (std::abs(maxVal - minVal) < 1e-6) {
        qDebug() << "Warning: Color channel min equals max, returning 128";
        return 128;
    }

    // Apply normalization formula and scale to 0-255 range
    double normalized = (rawValue - minVal) / (maxVal - minVal);
    int result = static_cast<int>(normalized * 255.0 + 0.5); // +0.5 for rounding

    // Clamp to [0, 255] range
    return static_cast<uint8_t>(std::max(0, std::min(255, result)));
}

// Sprint 3: Enhanced point data extraction with intensity and color

bool E57ParserLib::extractEnhancedPointData(const e57::StructureNode& scanHeaderNode, std::vector<PointData>& points) {
    try {
        // Get the CompressedVectorNode for points
        e57::CompressedVectorNode cvNode = static_cast<e57::CompressedVectorNode>(scanHeaderNode.get("points"));
        int64_t totalPoints = cvNode.childCount();

        if (totalPoints == 0) {
            setError("No points found in scan");
            return false;
        }

        // Task 3.3.1: Prepare SourceDestBuffer objects for all available attributes
        const int64_t POINTS_PER_READ_BLOCK = 65536; // Read in blocks
        int64_t bufferSize = std::min(totalPoints, POINTS_PER_READ_BLOCK);

        // Prepare buffers for XYZ coordinates (required)
        std::vector<double> xBuffer_d(bufferSize);
        std::vector<double> yBuffer_d(bufferSize);
        std::vector<double> zBuffer_d(bufferSize);

        // Prepare buffers for intensity (optional)
        std::vector<float> intensityBuffer_f(bufferSize);

        // Prepare buffers for color channels (optional)
        std::vector<uint8_t> rBuffer_u8(bufferSize);
        std::vector<uint8_t> gBuffer_u8(bufferSize);
        std::vector<uint8_t> bBuffer_u8(bufferSize);

        // Task 3.1.2 & 3.2.2: Setup SourceDestBuffer vector
        std::vector<e57::SourceDestBuffer> sdbufs;

        // Always add XYZ buffers
        sdbufs.emplace_back(*m_imageFile, "cartesianX", xBuffer_d.data(), bufferSize, true, false, sizeof(double));
        sdbufs.emplace_back(*m_imageFile, "cartesianY", yBuffer_d.data(), bufferSize, true, false, sizeof(double));
        sdbufs.emplace_back(*m_imageFile, "cartesianZ", zBuffer_d.data(), bufferSize, true, false, sizeof(double));

        // Add intensity buffer if available
        if (m_prototypeInfo.hasIntensity) {
            sdbufs.emplace_back(*m_imageFile, "intensity", intensityBuffer_f.data(), bufferSize, true, true, sizeof(float));
            qDebug() << "Added intensity buffer to SourceDestBuffer vector";
        }

        // Add color buffers if available
        if (m_prototypeInfo.hasColorRed) {
            sdbufs.emplace_back(*m_imageFile, "colorRed", rBuffer_u8.data(), bufferSize, true, true, sizeof(uint8_t));
            qDebug() << "Added colorRed buffer to SourceDestBuffer vector";
        }
        if (m_prototypeInfo.hasColorGreen) {
            sdbufs.emplace_back(*m_imageFile, "colorGreen", gBuffer_u8.data(), bufferSize, true, true, sizeof(uint8_t));
            qDebug() << "Added colorGreen buffer to SourceDestBuffer vector";
        }
        if (m_prototypeInfo.hasColorBlue) {
            sdbufs.emplace_back(*m_imageFile, "colorBlue", bBuffer_u8.data(), bufferSize, true, true, sizeof(uint8_t));
            qDebug() << "Added colorBlue buffer to SourceDestBuffer vector";
        }

        // Task 3.3.2: Create CompressedVectorReader
        e57::CompressedVectorReader reader = cvNode.reader(sdbufs);

        // Reserve space for the final point data
        points.reserve(totalPoints);

        int64_t pointsRead = 0;
        int lastProgressPercent = 30;

        // Task 3.3.3: Read point data in blocks
        try {
            while (pointsRead < totalPoints) {
                // Read a block of points
                uint64_t actualPointsRead = reader.read();

                if (actualPointsRead == 0) {
                    break; // No more data
                }

                // Task 3.1.3, 3.2.3: Process all attributes for each point
                for (uint64_t i = 0; i < actualPointsRead; ++i) {
                    PointData point;

                    // XYZ coordinates (always present)
                    point.x = static_cast<float>(xBuffer_d[i]);
                    point.y = static_cast<float>(yBuffer_d[i]);
                    point.z = static_cast<float>(zBuffer_d[i]);

                    // Task 3.1.4: Process intensity if available
                    if (m_prototypeInfo.hasIntensity) {
                        float rawIntensity = intensityBuffer_f[i];
                        point.intensity = normalizeIntensity(rawIntensity);
                        point.hasIntensity = true;
                    }

                    // Task 3.2.4: Process color if available
                    if (m_prototypeInfo.hasColorRed || m_prototypeInfo.hasColorGreen || m_prototypeInfo.hasColorBlue) {
                        if (m_prototypeInfo.hasColorRed) {
                            point.r = rBuffer_u8[i];
                        }
                        if (m_prototypeInfo.hasColorGreen) {
                            point.g = gBuffer_u8[i];
                        }
                        if (m_prototypeInfo.hasColorBlue) {
                            point.b = bBuffer_u8[i];
                        }
                        point.hasColor = true;
                    }

                    points.push_back(point);
                }

                pointsRead += actualPointsRead;

                // Emit progress updates
                int progressPercent = 30 + static_cast<int>((pointsRead * 70) / totalPoints);
                if (progressPercent > lastProgressPercent + 5) { // Update every 5%
                    emit progressUpdated(progressPercent, QString("Reading enhanced points... %1/%2").arg(pointsRead).arg(totalPoints));
                    lastProgressPercent = progressPercent;
                }
            }

            // Close the reader
            reader.close();

        } catch (const e57::E57Exception& ex) {
            // Handle reader errors
            reader.close();
            setError(std::string("E57 Exception during enhanced point reading: ") + ex.what());
            return false;
        }

        if (pointsRead != totalPoints) {
            setError("Warning: Read " + std::to_string(pointsRead) + " points, expected " + std::to_string(totalPoints));
            // Don't return false - partial data might still be useful
        }

        qDebug() << "Successfully extracted" << points.size() << "enhanced points with attributes:";
        qDebug() << "  XYZ: always present";
        qDebug() << "  Intensity:" << (m_prototypeInfo.hasIntensity ? "present" : "not present");
        qDebug() << "  Color:" << (m_prototypeInfo.hasColorRed || m_prototypeInfo.hasColorGreen || m_prototypeInfo.hasColorBlue ? "present" : "not present");

        return true;

    } catch (const e57::E57Exception& ex) {
        setError(std::string("E57 Exception during enhanced point data extraction: ") + ex.what());
        return false;
    }
}
