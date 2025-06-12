#include "parsers/E57ParserCore.h"
#include <E57Format/E57Format.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_set>
#include <cmath>

E57ParserCore::E57ParserCore() 
    : m_imageFile(nullptr) {
}

E57ParserCore::~E57ParserCore() {
    closeFile();
}

bool E57ParserCore::openFile(const std::string& filePath) {
    try {
        closeFile();
        clearError();
        
        if (filePath.empty()) {
            setError("Empty file path provided");
            return false;
        }
        
        if (!isValidE57File(filePath)) {
            setError("Invalid E57 file format: " + filePath);
            return false;
        }
        
        return openE57FileInternal(filePath);
        
    } catch (const std::exception& ex) {
        handleE57Exception(ex, "opening file");
        return false;
    }
}

void E57ParserCore::closeFile() {
    closeE57FileInternal();
}

bool E57ParserCore::isOpen() const {
    return m_imageFile && m_imageFile->isOpen();
}

bool E57ParserCore::isValidE57File(const std::string& filePath) {
    try {
        // Check if file exists and is readable
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Check file size (E57 files should be at least a few KB)
        file.seekg(0, std::ios::end);
        std::streamsize fileSize = file.tellg();
        file.close();
        
        if (fileSize < 1024) { // Less than 1KB is suspicious
            return false;
        }
        
        // Try to open with libE57Format
        e57::ImageFile testFile(filePath, "r");
        bool isValid = testFile.isOpen();
        testFile.close();
        
        return isValid;
        
    } catch (...) {
        return false;
    }
}

std::string E57ParserCore::getGuid() const {
    if (!isOpen()) {
        return "";
    }
    
    try {
        e57::StructureNode root = m_imageFile->root();
        if (root.isDefined("guid")) {
            e57::StringNode guidNode(root.get("guid"));
            return guidNode.value();
        }
    } catch (const e57::E57Exception&) {
        // Ignore and return empty string
    }
    
    return "";
}

std::pair<int, int> E57ParserCore::getVersion() const {
    if (!isOpen()) {
        return {0, 0};
    }
    
    try {
        return {m_imageFile->majorVersion(), m_imageFile->minorVersion()};
    } catch (const e57::E57Exception&) {
        return {0, 0};
    }
}

int E57ParserCore::getScanCount() const {
    if (!isOpen()) {
        return 0;
    }
    
    try {
        e57::StructureNode root = m_imageFile->root();
        if (root.isDefined("data3D")) {
            e57::VectorNode data3D(root.get("data3D"));
            return static_cast<int>(data3D.childCount());
        }
    } catch (const e57::E57Exception&) {
        // Ignore and return 0
    }
    
    return 0;
}

CoreScanMetadata E57ParserCore::getScanMetadata(int scanIndex) const {
    if (!isOpen() || scanIndex < 0 || scanIndex >= getScanCount()) {
        return CoreScanMetadata();
    }
    
    try {
        return extractScanMetadataInternal(scanIndex);
    } catch (const std::exception& ex) {
        // Return empty metadata on error
        return CoreScanMetadata();
    }
}

std::vector<float> E57ParserCore::extractXYZData(int scanIndex, const CoreLoadingSettings& settings) {
    std::vector<CorePointData> pointData = extractPointData(scanIndex, settings);
    
    std::vector<float> xyzData;
    xyzData.reserve(pointData.size() * 3);
    
    for (const auto& point : pointData) {
        xyzData.push_back(point.x);
        xyzData.push_back(point.y);
        xyzData.push_back(point.z);
    }
    
    return xyzData;
}

std::vector<CorePointData> E57ParserCore::extractPointData(int scanIndex, const CoreLoadingSettings& settings) {
    if (!isOpen()) {
        setError("No E57 file is currently open");
        return {};
    }
    
    if (scanIndex < 0 || scanIndex >= getScanCount()) {
        setError("Invalid scan index: " + std::to_string(scanIndex));
        return {};
    }
    
    try {
        reportProgress(0, "Starting point data extraction");
        
        std::vector<CorePointData> points = extractPointDataFromScan(scanIndex, settings);
        
        if (points.empty()) {
            setError("No valid points extracted from scan " + std::to_string(scanIndex));
            return {};
        }
        
        reportProgress(100, "Point data extraction complete");
        return points;
        
    } catch (const std::exception& ex) {
        handleE57Exception(ex, "extracting point data");
        return {};
    }
}

int64_t E57ParserCore::getPointCount(int scanIndex) const {
    if (!isOpen() || scanIndex < 0 || scanIndex >= getScanCount()) {
        return 0;
    }
    
    try {
        e57::StructureNode root = m_imageFile->root();
        e57::VectorNode data3D(root.get("data3D"));
        e57::StructureNode scanHeader(data3D.get(scanIndex));
        
        if (scanHeader.isDefined("points")) {
            e57::CompressedVectorNode points(scanHeader.get("points"));
            return points.childCount();
        }
    } catch (const e57::E57Exception&) {
        // Ignore and return 0
    }
    
    return 0;
}

void E57ParserCore::setProgressCallback(ProgressCallback callback) {
    m_progressCallback = callback;
}

void E57ParserCore::clearProgressCallback() {
    m_progressCallback = nullptr;
}

std::string E57ParserCore::getLastError() const {
    return m_lastError;
}

void E57ParserCore::clearError() {
    m_lastError.clear();
}

// Private implementation methods
bool E57ParserCore::openE57FileInternal(const std::string& filePath) {
    try {
        m_imageFile = std::make_unique<e57::ImageFile>(filePath, "r");
        
        if (!m_imageFile->isOpen()) {
            setError("Failed to open E57 file handle");
            return false;
        }
        
        m_currentFilePath = filePath;
        return true;
        
    } catch (const e57::E57Exception& ex) {
        setError("E57 Exception: " + std::string(ex.what()));
        return false;
    } catch (const std::exception& ex) {
        setError("Standard exception: " + std::string(ex.what()));
        return false;
    }
}

void E57ParserCore::closeE57FileInternal() {
    if (m_imageFile) {
        try {
            m_imageFile->close();
        } catch (...) {
            // Ignore exceptions during close
        }
        m_imageFile.reset();
    }
    
    m_currentFilePath.clear();
    m_prototypeInfo.reset();
    m_dataLimits.reset();
}

CoreScanMetadata E57ParserCore::extractScanMetadataInternal(int scanIndex) const {
    CoreScanMetadata metadata;
    
    try {
        e57::StructureNode root = m_imageFile->root();
        e57::VectorNode data3D(root.get("data3D"));
        e57::StructureNode scanHeader(data3D.get(scanIndex));
        
        // Extract basic metadata
        if (scanHeader.isDefined("name")) {
            e57::StringNode nameNode(scanHeader.get("name"));
            metadata.name = nameNode.value();
        } else {
            metadata.name = "Scan " + std::to_string(scanIndex);
        }
        
        if (scanHeader.isDefined("guid")) {
            e57::StringNode guidNode(scanHeader.get("guid"));
            metadata.guid = guidNode.value();
        }
        
        if (scanHeader.isDefined("description")) {
            e57::StringNode descNode(scanHeader.get("description"));
            metadata.description = descNode.value();
        }
        
        if (scanHeader.isDefined("acquisitionStart")) {
            // Handle acquisition date/time
            e57::StructureNode acqStart(scanHeader.get("acquisitionStart"));
            if (acqStart.isDefined("dateTimeValue")) {
                e57::FloatNode dateTimeNode(acqStart.get("dateTimeValue"));
                // Convert GPS time to readable format (simplified)
                metadata.acquisitionDateTime = std::to_string(dateTimeNode.value());
            }
        }
        
        // Get point count
        if (scanHeader.isDefined("points")) {
            e57::CompressedVectorNode points(scanHeader.get("points"));
            metadata.pointCount = points.childCount();
        }
        
        // Extract bounding box if available
        extractBoundingBox(scanHeader, metadata);
        
    } catch (const e57::E57Exception& ex) {
        // Return partial metadata on error
    }
    
    return metadata;
}

void E57ParserCore::reportProgress(int percentage, const std::string& stage) {
    if (m_progressCallback) {
        m_progressCallback(percentage, stage);
    }
}

void E57ParserCore::setError(const std::string& error) {
    m_lastError = error;
}

void E57ParserCore::handleE57Exception(const std::exception& ex, const std::string& context) {
    std::string errorMsg = "Error " + context + ": " + ex.what();
    setError(errorMsg);
}

bool E57ParserCore::extractBoundingBox(const e57::StructureNode& scanHeader, CoreScanMetadata& metadata) const {
    try {
        if (scanHeader.isDefined("cartesianBounds")) {
            e57::StructureNode bounds(scanHeader.get("cartesianBounds"));

            if (bounds.isDefined("xMinimum")) {
                e57::FloatNode xMin(bounds.get("xMinimum"));
                metadata.minX = xMin.value();
            }
            if (bounds.isDefined("xMaximum")) {
                e57::FloatNode xMax(bounds.get("xMaximum"));
                metadata.maxX = xMax.value();
            }
            if (bounds.isDefined("yMinimum")) {
                e57::FloatNode yMin(bounds.get("yMinimum"));
                metadata.minY = yMin.value();
            }
            if (bounds.isDefined("yMaximum")) {
                e57::FloatNode yMax(bounds.get("yMaximum"));
                metadata.maxY = yMax.value();
            }
            if (bounds.isDefined("zMinimum")) {
                e57::FloatNode zMin(bounds.get("zMinimum"));
                metadata.minZ = zMin.value();
            }
            if (bounds.isDefined("zMaximum")) {
                e57::FloatNode zMax(bounds.get("zMaximum"));
                metadata.maxZ = zMax.value();
            }

            return true;
        }
    } catch (const e57::E57Exception&) {
        // Ignore and return false
    }

    return false;
}

std::vector<CorePointData> E57ParserCore::extractPointDataFromScan(int scanIndex, const CoreLoadingSettings& settings) {
    try {
        e57::StructureNode root = m_imageFile->root();
        e57::VectorNode data3D(root.get("data3D"));
        e57::StructureNode scanHeader(data3D.get(scanIndex));

        reportProgress(10, "Analyzing scan structure");

        if (!inspectPointPrototype(scanHeader)) {
            setError("Failed to analyze point data structure");
            return {};
        }

        reportProgress(20, "Extracting point data");

        if (!scanHeader.isDefined("points")) {
            setError("Scan does not contain point data");
            return {};
        }

        e57::CompressedVectorNode cvNode(scanHeader.get("points"));
        std::vector<CorePointData> points;

        if (!extractPointDataFromCompressedVector(cvNode, points, settings)) {
            setError("Failed to extract point data from compressed vector");
            return {};
        }

        reportProgress(80, "Applying filters");

        // Apply spatial filtering if enabled
        if (settings.enableSpatialFilter) {
            applySpatialFilter(points, settings);
        }

        // Apply voxel filtering if enabled
        if (settings.voxelSize > 0.0) {
            applyVoxelFilter(points, settings.voxelSize);
        }

        // Limit point count if specified
        if (settings.maxPoints > 0 && static_cast<int64_t>(points.size()) > settings.maxPoints) {
            points.resize(static_cast<size_t>(settings.maxPoints));
        }

        reportProgress(90, "Finalizing point data");

        return points;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "extracting point data from scan");
        return {};
    }
}

bool E57ParserCore::inspectPointPrototype(const e57::StructureNode& scanHeader) {
    try {
        m_prototypeInfo.reset();

        if (!scanHeader.isDefined("points")) {
            return false;
        }

        e57::CompressedVectorNode cvNode(scanHeader.get("points"));
        e57::StructureNode prototype(cvNode.prototype());

        // Check for required XYZ fields
        if (!prototype.isDefined("cartesianX") ||
            !prototype.isDefined("cartesianY") ||
            !prototype.isDefined("cartesianZ")) {
            setError("Point prototype missing required XYZ coordinates");
            return false;
        }

        // Check for intensity
        if (prototype.isDefined("intensity")) {
            m_prototypeInfo.hasIntensity = true;
            e57::Node intensityNode = prototype.get("intensity");
            m_prototypeInfo.intensityDataType = (intensityNode.type() == e57::E57_FLOAT) ? "float" : "integer";
        }

        // Check for color fields
        if (prototype.isDefined("colorRed")) {
            m_prototypeInfo.hasColorRed = true;
        }
        if (prototype.isDefined("colorGreen")) {
            m_prototypeInfo.hasColorGreen = true;
        }
        if (prototype.isDefined("colorBlue")) {
            m_prototypeInfo.hasColorBlue = true;
        }

        if (m_prototypeInfo.hasColorRed || m_prototypeInfo.hasColorGreen || m_prototypeInfo.hasColorBlue) {
            // Determine color data type from red channel (assuming all channels have same type)
            if (prototype.isDefined("colorRed")) {
                e57::Node colorNode = prototype.get("colorRed");
                m_prototypeInfo.colorDataType = (colorNode.type() == e57::E57_FLOAT) ? "float" : "integer";
            }
        }

        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "inspecting point prototype");
        return false;
    }
}

bool E57ParserCore::extractPointDataFromCompressedVector(const e57::CompressedVectorNode& cvNode,
                                                        std::vector<CorePointData>& points,
                                                        const CoreLoadingSettings& settings) {
    try {
        int64_t pointCount = cvNode.childCount();
        if (pointCount == 0) {
            return true; // Empty scan is valid
        }

        // Limit the number of points to read
        int64_t pointsToRead = pointCount;
        if (settings.maxPoints > 0 && pointsToRead > settings.maxPoints) {
            pointsToRead = settings.maxPoints;
        }

        points.reserve(static_cast<size_t>(pointsToRead));

        // Set up data buffers for reading
        std::vector<double> xData(static_cast<size_t>(pointsToRead));
        std::vector<double> yData(static_cast<size_t>(pointsToRead));
        std::vector<double> zData(static_cast<size_t>(pointsToRead));

        std::vector<double> intensityData;
        std::vector<uint16_t> redData, greenData, blueData;

        if (m_prototypeInfo.hasIntensity && settings.loadIntensity) {
            intensityData.resize(static_cast<size_t>(pointsToRead));
        }

        if ((m_prototypeInfo.hasColorRed || m_prototypeInfo.hasColorGreen || m_prototypeInfo.hasColorBlue) && settings.loadColor) {
            if (m_prototypeInfo.hasColorRed) redData.resize(static_cast<size_t>(pointsToRead));
            if (m_prototypeInfo.hasColorGreen) greenData.resize(static_cast<size_t>(pointsToRead));
            if (m_prototypeInfo.hasColorBlue) blueData.resize(static_cast<size_t>(pointsToRead));
        }

        // Create CompressedVectorReader
        e57::CompressedVectorReader reader = cvNode.reader();

        // Set up source buffers
        reader.read(e57::SourceDestBuffer(m_imageFile.get(), "cartesianX", xData.data(), static_cast<size_t>(pointsToRead), true));
        reader.read(e57::SourceDestBuffer(m_imageFile.get(), "cartesianY", yData.data(), static_cast<size_t>(pointsToRead), true));
        reader.read(e57::SourceDestBuffer(m_imageFile.get(), "cartesianZ", zData.data(), static_cast<size_t>(pointsToRead), true));

        if (!intensityData.empty()) {
            reader.read(e57::SourceDestBuffer(m_imageFile.get(), "intensity", intensityData.data(), static_cast<size_t>(pointsToRead), true));
        }

        if (!redData.empty()) {
            reader.read(e57::SourceDestBuffer(m_imageFile.get(), "colorRed", redData.data(), static_cast<size_t>(pointsToRead), true));
        }
        if (!greenData.empty()) {
            reader.read(e57::SourceDestBuffer(m_imageFile.get(), "colorGreen", greenData.data(), static_cast<size_t>(pointsToRead), true));
        }
        if (!blueData.empty()) {
            reader.read(e57::SourceDestBuffer(m_imageFile.get(), "colorBlue", blueData.data(), static_cast<size_t>(pointsToRead), true));
        }

        // Read the data
        uint64_t pointsRead = 0;
        reader.read(pointsRead);
        reader.close();

        // Convert to CorePointData structures
        for (uint64_t i = 0; i < pointsRead; ++i) {
            CorePointData point;
            point.x = static_cast<float>(xData[i]);
            point.y = static_cast<float>(yData[i]);
            point.z = static_cast<float>(zData[i]);

            // Add intensity if available
            if (!intensityData.empty()) {
                point.intensity = static_cast<float>(intensityData[i]);
                point.hasIntensity = true;
            }

            // Add color if available
            if (!redData.empty() || !greenData.empty() || !blueData.empty()) {
                point.red = !redData.empty() ? static_cast<uint8_t>(std::min(255.0, redData[i])) : 0;
                point.green = !greenData.empty() ? static_cast<uint8_t>(std::min(255.0, greenData[i])) : 0;
                point.blue = !blueData.empty() ? static_cast<uint8_t>(std::min(255.0, blueData[i])) : 0;
                point.hasColor = true;
            }

            // Validate point data
            if (validatePointData(point, settings)) {
                points.push_back(point);
            }
        }

        return true;

    } catch (const e57::E57Exception& ex) {
        handleE57Exception(ex, "extracting point data from compressed vector");
        return false;
    }
}

bool E57ParserCore::validatePointData(const CorePointData& point, const CoreLoadingSettings& settings) const {
    // Check for NaN or infinite values
    if (!std::isfinite(point.x) || !std::isfinite(point.y) || !std::isfinite(point.z)) {
        return false;
    }

    // Apply spatial filter if enabled
    if (settings.enableSpatialFilter) {
        if (point.x < settings.filterMinX || point.x > settings.filterMaxX ||
            point.y < settings.filterMinY || point.y > settings.filterMaxY ||
            point.z < settings.filterMinZ || point.z > settings.filterMaxZ) {
            return false;
        }
    }

    return true;
}

void E57ParserCore::applySpatialFilter(std::vector<CorePointData>& points, const CoreLoadingSettings& settings) const {
    if (!settings.enableSpatialFilter) {
        return;
    }

    points.erase(std::remove_if(points.begin(), points.end(),
        [&settings](const CorePointData& point) {
            return point.x < settings.filterMinX || point.x > settings.filterMaxX ||
                   point.y < settings.filterMinY || point.y > settings.filterMaxY ||
                   point.z < settings.filterMinZ || point.z > settings.filterMaxZ;
        }), points.end());
}

void E57ParserCore::applyVoxelFilter(std::vector<CorePointData>& points, double voxelSize) const {
    if (voxelSize <= 0.0 || points.empty()) {
        return;
    }

    // Simple voxel grid implementation
    std::unordered_set<std::string> occupiedVoxels;
    std::vector<CorePointData> filteredPoints;
    filteredPoints.reserve(points.size());

    for (const auto& point : points) {
        // Calculate voxel coordinates
        int voxelX = static_cast<int>(std::floor(point.x / voxelSize));
        int voxelY = static_cast<int>(std::floor(point.y / voxelSize));
        int voxelZ = static_cast<int>(std::floor(point.z / voxelSize));

        // Create voxel key
        std::string voxelKey = std::to_string(voxelX) + "," +
                              std::to_string(voxelY) + "," +
                              std::to_string(voxelZ);

        // Check if voxel is already occupied
        if (occupiedVoxels.find(voxelKey) == occupiedVoxels.end()) {
            occupiedVoxels.insert(voxelKey);
            filteredPoints.push_back(point);
        }
    }

    points = std::move(filteredPoints);
}
