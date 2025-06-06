#include "E57XmlParser.h"
#include <E57Format/E57Format.h>
#include <iostream>
#include <stdexcept>

namespace E57Parser {

E57XmlParser::E57XmlParser(const std::string& filePath) 
    : m_filePath(filePath) {
    openFile();
}

E57XmlParser::~E57XmlParser() {
    closeFile();
}

void E57XmlParser::openFile() {
    try {
        m_imageFile = std::make_unique<e57::ImageFile>(m_filePath, "r");
        if (!m_imageFile->isOpen()) {
            throw std::runtime_error("Failed to open E57 file: " + m_filePath);
        }
    } catch (const e57::E57Exception& e) {
        throw std::runtime_error("E57 Exception opening file '" + m_filePath + "': " + e.what());
    } catch (const std::exception& e) {
        throw std::runtime_error("Exception opening E57 file '" + m_filePath + "': " + e.what());
    }
}

void E57XmlParser::closeFile() {
    if (m_imageFile) {
        try {
            m_imageFile->close();
        } catch (const e57::E57Exception& e) {
            std::cerr << "Warning: E57 Exception closing file: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Warning: Exception closing file: " << e.what() << std::endl;
        }
        m_imageFile.reset();
    }
}

bool E57XmlParser::isValidE57File() {
    try {
        if (!m_imageFile || !m_imageFile->isOpen()) {
            return false;
        }
        
        // Try to access the root node
        e57::StructureNode root(m_imageFile->root());
        return true;
        
    } catch (const e57::E57Exception&) {
        return false;
    } catch (const std::exception&) {
        return false;
    }
}

E57FileMetadata E57XmlParser::parseFile() {
    E57FileMetadata metadata;
    
    if (!m_imageFile || !m_imageFile->isOpen()) {
        throw std::runtime_error("E57 file not open for parsing");
    }
    
    try {
        e57::StructureNode root(m_imageFile->root());
        
        // Parse file-level metadata
        if (root.isDefined("guid")) {
            e57::StringNode guidNode(root.get("guid"));
            metadata.fileGuid = guidNode.value();
        }
        
        if (root.isDefined("creationDateTime")) {
            e57::StructureNode dateTimeNode(root.get("creationDateTime"));
            if (dateTimeNode.isDefined("dateTimeValue")) {
                e57::StringNode dateTimeValue(dateTimeNode.get("dateTimeValue"));
                metadata.creationDateTime = dateTimeValue.value();
            }
        }
        
        if (root.isDefined("coordinateMetadata")) {
            e57::StringNode coordMetaNode(root.get("coordinateMetadata"));
            metadata.coordinateMetadata = coordMetaNode.value();
        }
        
        // Parse 3D data sections
        metadata.scans = parseData3DSections();
        
        // Parse 2D images
        metadata.images2D = parseImages2D();
        
    } catch (const e57::E57Exception& e) {
        throw std::runtime_error("E57 Exception parsing file metadata: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("Exception parsing file metadata: " + std::string(e.what()));
    }
    
    return metadata;
}

std::vector<ScanMetadata> E57XmlParser::parseData3DSections() {
    std::vector<ScanMetadata> scans;
    
    if (!m_imageFile || !m_imageFile->isOpen()) {
        throw std::runtime_error("E57 file not open for parsing data3D sections");
    }
    
    try {
        e57::StructureNode root(m_imageFile->root());
        
        if (!root.isDefined("data3D")) {
            return scans; // No 3D data sections
        }
        
        e57::VectorNode data3D(root.get("data3D"));
        int64_t scanCount = data3D.childCount();
        
        for (int64_t i = 0; i < scanCount; ++i) {
            e57::StructureNode scanNode(data3D.get(i));
            ScanMetadata scan = parseScanNode(scanNode, static_cast<int>(i));
            scans.push_back(scan);
            
            // Cache the scan metadata
            if (!scan.guid.empty()) {
                m_scanCache[scan.guid] = scan;
            }
        }
        
    } catch (const e57::E57Exception& e) {
        throw std::runtime_error("E57 Exception parsing data3D sections: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("Exception parsing data3D sections: " + std::string(e.what()));
    }
    
    return scans;
}

ScanMetadata E57XmlParser::parseScanNode(const e57::Node& scanNode, int scanIndex) {
    ScanMetadata scan;

    try {
        // Verify the node is a structure node before casting
        if (scanNode.type() != e57::TypeStructure) {
            throw std::runtime_error("Scan node " + std::to_string(scanIndex) + " is not a structure node");
        }
        e57::StructureNode structNode(scanNode);
        
        // Validate scan node has required elements
        if (!validateScanNode(structNode, scanIndex)) {
            throw std::runtime_error("Scan " + std::to_string(scanIndex) + " missing required elements");
        }
        
        // Parse GUID
        if (structNode.isDefined("guid")) {
            e57::StringNode guidNode(structNode.get("guid"));
            scan.guid = guidNode.value();
        } else {
            scan.guid = "scan_" + std::to_string(scanIndex);
        }
        
        // Parse name
        if (structNode.isDefined("name")) {
            e57::StringNode nameNode(structNode.get("name"));
            scan.name = nameNode.value();
        } else {
            scan.name = "Scan " + std::to_string(scanIndex);
        }
        
        // Parse description
        if (structNode.isDefined("description")) {
            e57::StringNode descNode(structNode.get("description"));
            scan.description = descNode.value();
        }
        
        // Parse points section
        if (structNode.isDefined("points")) {
            e57::CompressedVectorNode points(structNode.get("points"));
            
            // Get point count
            scan.pointCount = static_cast<uint64_t>(points.childCount());
            
            // Parse point prototype
            e57::StructureNode prototype(points.prototype());
            scan.pointAttributes = parsePointPrototype(prototype);
            
            // Extract binary section information
            try {
                BinarySection binaryInfo = extractBinarySectionInfo(points, scan.guid);
                scan.binaryOffset = binaryInfo.offset;
                scan.binaryLength = binaryInfo.length;
            } catch (const std::exception& e) {
                std::cerr << "Warning: Could not extract binary section info for scan " 
                         << scan.guid << ": " << e.what() << std::endl;
            }
        }
        
        // Parse coordinate metadata
        if (structNode.isDefined("coordinateMetadata")) {
            scan.coordinates = parseCoordinateMetadata(structNode.get("coordinateMetadata"));
        }
        
    } catch (const e57::E57Exception& e) {
        throw std::runtime_error("E57 Exception parsing scan node " + std::to_string(scanIndex) + 
                                ": " + std::string(e.what()));
    }
    
    return scan;
}

std::vector<PointAttribute> E57XmlParser::parsePointPrototype(const e57::StructureNode& prototype) {
    std::vector<PointAttribute> attributes;
    
    try {
        // Standard E57 point attributes to look for
        std::vector<std::string> standardAttrs = {
            "cartesianX", "cartesianY", "cartesianZ",
            "sphericalRange", "sphericalAzimuth", "sphericalElevation",
            "colorRed", "colorGreen", "colorBlue",
            "intensity", "timeStamp", "rowIndex", "columnIndex"
        };
        
        for (const auto& attrName : standardAttrs) {
            if (prototype.isDefined(attrName)) {
                e57::Node attrNode = prototype.get(attrName);
                PointAttribute attr = parseAttributeNode(attrNode);
                attr.name = attrName;
                attributes.push_back(attr);
            }
        }
        
    } catch (const e57::E57Exception& e) {
        throw std::runtime_error("E57 Exception parsing point prototype: " + std::string(e.what()));
    }
    
    return attributes;
}

PointAttribute E57XmlParser::parseAttributeNode(const e57::Node& attrNode) {
    PointAttribute attr;
    
    try {
        attr.elementType = static_cast<int>(attrNode.type());
        
        // Parse limits based on node type
        if (attrNode.type() == e57::TypeScaledInteger) {
            e57::ScaledIntegerNode scaledNode(attrNode);
            attr.minimum = scaledNode.minimum();
            attr.maximum = scaledNode.maximum();
            attr.hasLimits = true;
        } else if (attrNode.type() == e57::TypeFloat) {
            e57::FloatNode floatNode(attrNode);
            attr.minimum = floatNode.minimum();
            attr.maximum = floatNode.maximum();
            attr.hasLimits = true;
        } else if (attrNode.type() == e57::TypeInteger) {
            e57::IntegerNode intNode(attrNode);
            attr.minimum = static_cast<double>(intNode.minimum());
            attr.maximum = static_cast<double>(intNode.maximum());
            attr.hasLimits = true;
        }
        
    } catch (const e57::E57Exception& e) {
        throw std::runtime_error("E57 Exception parsing attribute node: " + std::string(e.what()));
    }
    
    return attr;
}

std::vector<std::string> E57XmlParser::parseImages2D() {
    std::vector<std::string> images;
    
    try {
        e57::StructureNode root(m_imageFile->root());
        
        if (!root.isDefined("images2D")) {
            return images; // No 2D images
        }
        
        e57::VectorNode images2D(root.get("images2D"));
        int64_t imageCount = images2D.childCount();
        
        for (int64_t i = 0; i < imageCount; ++i) {
            e57::Node imageNodeBase = images2D.get(i);
            if (imageNodeBase.type() != e57::TypeStructure) {
                continue; // Skip non-structure nodes
            }
            e57::StructureNode imageNode(imageNodeBase);
            
            if (imageNode.isDefined("guid")) {
                e57::StringNode guidNode(imageNode.get("guid"));
                images.push_back(guidNode.value());
            }
        }
        
    } catch (const e57::E57Exception& e) {
        throw std::runtime_error("E57 Exception parsing images2D: " + std::string(e.what()));
    }
    
    return images;
}

int E57XmlParser::getScanCount() {
    try {
        if (!m_imageFile || !m_imageFile->isOpen()) {
            return 0;
        }
        
        e57::StructureNode root(m_imageFile->root());
        
        if (!root.isDefined("data3D")) {
            return 0;
        }
        
        e57::VectorNode data3D(root.get("data3D"));
        return static_cast<int>(data3D.childCount());
        
    } catch (const e57::E57Exception&) {
        return 0;
    } catch (const std::exception&) {
        return 0;
    }
}

BinarySection E57XmlParser::getBinarySectionInfo(const std::string& scanGuid) {
    // Check cache first
    auto it = m_scanCache.find(scanGuid);
    if (it != m_scanCache.end()) {
        return BinarySection(it->second.binaryOffset, it->second.binaryLength, scanGuid, "points");
    }

    // Parse scans if not cached
    std::vector<ScanMetadata> scans = parseData3DSections();

    for (const auto& scan : scans) {
        if (scan.guid == scanGuid) {
            return BinarySection(scan.binaryOffset, scan.binaryLength, scanGuid, "points");
        }
    }

    throw std::runtime_error("Scan with GUID '" + scanGuid + "' not found in E57 file");
}

BinarySection E57XmlParser::extractBinarySectionInfo(const e57::CompressedVectorNode& pointsNode,
                                                    const std::string& scanGuid) {
    BinarySection section;
    section.guid = scanGuid;
    section.sectionType = "points";

    // Note: libE57Format doesn't directly expose binary section offsets and lengths
    // This is a simplified implementation that would need to be enhanced with
    // access to the internal E57 file structure or by parsing the file manually

    try {
        // For now, we'll set placeholder values
        // In a complete implementation, this would require:
        // 1. Access to the E57 file's internal binary section table
        // 2. Mapping from CompressedVectorNode to its binary representation
        // 3. Calculation of actual file offsets and lengths

        section.offset = 0;  // Would be determined from E57 internal structures
        section.length = 0;  // Would be determined from E57 internal structures

        // Get point count for validation
        int64_t pointCount = pointsNode.childCount();
        if (pointCount > 0) {
            // Estimate binary size based on point count and prototype
            // This is a rough estimate - actual implementation would need precise calculation
            e57::StructureNode prototype(pointsNode.prototype());
            size_t estimatedPointSize = 12; // Minimum for XYZ coordinates

            if (prototype.isDefined("intensity")) estimatedPointSize += 4;
            if (prototype.isDefined("colorRed")) estimatedPointSize += 1;
            if (prototype.isDefined("colorGreen")) estimatedPointSize += 1;
            if (prototype.isDefined("colorBlue")) estimatedPointSize += 1;

            section.length = static_cast<uint64_t>(pointCount * estimatedPointSize);
        }

    } catch (const e57::E57Exception& e) {
        throw std::runtime_error("E57 Exception extracting binary section info: " + std::string(e.what()));
    }

    return section;
}

bool E57XmlParser::validateScanNode(const e57::StructureNode& scanNode, int scanIndex) {
    try {
        // Check for required points section
        if (!scanNode.isDefined("points")) {
            std::cerr << "Warning: Scan " << scanIndex << " missing 'points' section" << std::endl;
            return false;
        }

        // Validate points section is a CompressedVectorNode
        e57::Node pointsNode = scanNode.get("points");
        if (pointsNode.type() != e57::TypeCompressedVector) {
            std::cerr << "Warning: Scan " << scanIndex << " 'points' is not a CompressedVector" << std::endl;
            return false;
        }

        // Check prototype has required coordinate fields
        e57::CompressedVectorNode points(pointsNode);
        e57::StructureNode prototype(points.prototype());

        bool hasCartesianCoords = prototype.isDefined("cartesianX") &&
                                 prototype.isDefined("cartesianY") &&
                                 prototype.isDefined("cartesianZ");

        bool hasSphericalCoords = prototype.isDefined("sphericalRange") &&
                                 prototype.isDefined("sphericalAzimuth") &&
                                 prototype.isDefined("sphericalElevation");

        if (!hasCartesianCoords && !hasSphericalCoords) {
            std::cerr << "Warning: Scan " << scanIndex << " missing coordinate fields" << std::endl;
            return false;
        }

        return true;

    } catch (const e57::E57Exception& e) {
        std::cerr << "E57 Exception validating scan node " << scanIndex << ": " << e.what() << std::endl;
        return false;
    }
}

CoordinateMetadata E57XmlParser::parseCoordinateMetadata(const e57::Node& coordNode) {
    CoordinateMetadata coord;

    try {
        if (coordNode.type() == e57::TypeString) {
            e57::StringNode stringNode(coordNode);
            coord.coordinateSystemName = stringNode.value();
        } else if (coordNode.type() == e57::TypeStructure) {
            e57::StructureNode structNode(coordNode);

            if (structNode.isDefined("coordinateSystemName")) {
                e57::StringNode nameNode(structNode.get("coordinateSystemName"));
                coord.coordinateSystemName = nameNode.value();
            }

            if (structNode.isDefined("datum")) {
                e57::StringNode datumNode(structNode.get("datum"));
                coord.datum = datumNode.value();
            }

            if (structNode.isDefined("projection")) {
                e57::StringNode projNode(structNode.get("projection"));
                coord.projection = projNode.value();
            }
        }

    } catch (const e57::E57Exception& e) {
        throw std::runtime_error("E57 Exception parsing coordinate metadata: " + std::string(e.what()));
    }

    return coord;
}

std::string E57XmlParser::elementTypeToString(int elementType) {
    switch (elementType) {
        case e57::TypeStructure: return "Structure";
        case e57::TypeVector: return "Vector";
        case e57::TypeCompressedVector: return "CompressedVector";
        case e57::TypeInteger: return "Integer";
        case e57::TypeScaledInteger: return "ScaledInteger";
        case e57::TypeFloat: return "Float";
        case e57::TypeString: return "String";
        case e57::TypeBlob: return "Blob";
        default: return "Unknown(" + std::to_string(elementType) + ")";
    }
}

} // namespace E57Parser
