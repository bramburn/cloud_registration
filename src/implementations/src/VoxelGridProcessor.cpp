#include "implementations/VoxelGridProcessor.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

// VoxelGridParams implementation
VoxelGridProcessor::VoxelGridParams VoxelGridProcessor::VoxelGridParams::fromJson(const QString& jsonStr) {
    VoxelGridParams params;
    
    if (jsonStr.isEmpty()) {
        return params; // Return defaults
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        return params; // Return defaults on parse error
    }
    
    QJsonObject obj = doc.object();
    
    if (obj.contains("voxelSize") && obj["voxelSize"].isDouble()) {
        params.voxelSize = static_cast<float>(obj["voxelSize"].toDouble());
    }
    
    if (obj.contains("preserveIntensity") && obj["preserveIntensity"].isBool()) {
        params.preserveIntensity = obj["preserveIntensity"].toBool();
    }
    
    if (obj.contains("useAveraging") && obj["useAveraging"].isBool()) {
        params.useAveraging = obj["useAveraging"].toBool();
    }
    
    return params;
}

QString VoxelGridProcessor::VoxelGridParams::toJson() const {
    QJsonObject obj;
    obj["voxelSize"] = static_cast<double>(voxelSize);
    obj["preserveIntensity"] = preserveIntensity;
    obj["useAveraging"] = useAveraging;
    
    QJsonDocument doc(obj);
    return doc.toJson(QJsonDocument::Compact);
}

// VoxelGridProcessor implementation
IPointCloudProcessor::ProcessingResult VoxelGridProcessor::processPointCloud(
    const std::vector<Point3D>& points,
    const QString& parameters) {
    
    QElapsedTimer timer;
    timer.start();
    
    ProcessingResult result;
    result.originalPointCount = points.size();
    
    try {
        // Parse parameters
        VoxelGridParams params = VoxelGridParams::fromJson(parameters);
        
        if (params.voxelSize <= 0.0f) {
            result.success = false;
            result.errorMessage = "Invalid voxel size: must be positive";
            return result;
        }
        
        // Create voxel grid
        std::unordered_map<VoxelKey, std::vector<Point3D>, VoxelKeyHash> voxelGrid;
        
        // Assign points to voxels
        for (const auto& point : points) {
            VoxelKey key = getVoxelKey(point, params.voxelSize);
            voxelGrid[key].push_back(point);
        }
        
        // Process each voxel
        result.processedPoints.reserve(voxelGrid.size());
        
        for (const auto& voxelPair : voxelGrid) {
            const std::vector<Point3D>& voxelPoints = voxelPair.second;
            
            if (voxelPoints.empty()) continue;
            
            Point3D processedPoint = params.useAveraging ? 
                averagePoints(voxelPoints) : voxelPoints[0];
            
            result.processedPoints.push_back(processedPoint);
        }
        
        result.processedPointCount = result.processedPoints.size();
        result.success = true;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.errorMessage = QString("Processing error: %1").arg(e.what());
    }
    
    result.processingTimeSeconds = timer.elapsed() / 1000.0;
    m_lastProcessingTime = result.processingTimeSeconds;
    
    updateStatistics(result);
    
    return result;
}

QString VoxelGridProcessor::getRecommendedParameters() const {
    VoxelGridParams defaultParams;
    return defaultParams.toJson();
}

bool VoxelGridProcessor::validateParameters(const QString& parameters) const {
    if (parameters.isEmpty()) {
        return true; // Empty parameters use defaults
    }
    
    VoxelGridParams params = VoxelGridParams::fromJson(parameters);
    return params.voxelSize > 0.0f;
}

// Helper methods
VoxelGridProcessor::VoxelKey VoxelGridProcessor::getVoxelKey(const Point3D& point, float voxelSize) const {
    return {
        static_cast<int>(std::floor(point.x / voxelSize)),
        static_cast<int>(std::floor(point.y / voxelSize)),
        static_cast<int>(std::floor(point.z / voxelSize))
    };
}

IPointCloudProcessor::Point3D VoxelGridProcessor::averagePoints(const std::vector<Point3D>& points) const {
    if (points.empty()) {
        return Point3D(0, 0, 0);
    }
    
    float sumX = 0, sumY = 0, sumZ = 0, sumIntensity = 0;
    int intensityCount = 0;
    
    for (const auto& point : points) {
        sumX += point.x;
        sumY += point.y;
        sumZ += point.z;
        
        if (point.hasIntensity) {
            sumIntensity += point.intensity;
            intensityCount++;
        }
    }
    
    float avgX = sumX / points.size();
    float avgY = sumY / points.size();
    float avgZ = sumZ / points.size();
    
    if (intensityCount > 0) {
        float avgIntensity = sumIntensity / intensityCount;
        return Point3D(avgX, avgY, avgZ, avgIntensity);
    } else {
        return Point3D(avgX, avgY, avgZ);
    }
}

void VoxelGridProcessor::updateStatistics(const ProcessingResult& result) const {
    m_lastStats.clear();
    
    m_lastStats.emplace_back("Original Point Count", QString::number(result.originalPointCount));
    m_lastStats.emplace_back("Processed Point Count", QString::number(result.processedPointCount));
    m_lastStats.emplace_back("Processing Time (s)", QString::number(result.processingTimeSeconds, 'f', 3));
    
    if (result.originalPointCount > 0) {
        double reductionRatio = 1.0 - (static_cast<double>(result.processedPointCount) / result.originalPointCount);
        m_lastStats.emplace_back("Reduction Ratio", QString::number(reductionRatio * 100, 'f', 1) + "%");
    }
    
    m_lastStats.emplace_back("Success", result.success ? "Yes" : "No");
    
    if (!result.success && !result.errorMessage.isEmpty()) {
        m_lastStats.emplace_back("Error", result.errorMessage);
    }
}
