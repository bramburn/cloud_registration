#include "TargetDetectionBase.h"
#include <QDebug>
#include <QDateTime>
#include <algorithm>
#include <cmath>
#include <random>

// Initialize static counter
int TargetDetectionBase::s_targetIdCounter = 0;

TargetDetectionBase::TargetDetectionBase(QObject* parent)
    : QObject(parent)
{
}

bool TargetDetectionBase::validateParameters(const DetectionParams& params) const
{
    if (params.distanceThreshold <= 0.0f || params.distanceThreshold > 1.0f) return false;
    if (params.maxIterations <= 0 || params.maxIterations > 10000) return false;
    if (params.minQuality < 0.0f || params.minQuality > 1.0f) return false;
    if (params.minRadius <= 0.0f || params.maxRadius <= params.minRadius) return false;
    if (params.minInliers <= 0) return false;
    if (params.neighborhoodRadius <= 0.0f) return false;
    if (params.curvatureThreshold < 0.0f) return false;
    
    return true;
}

TargetDetectionBase::DetectionParams TargetDetectionBase::getDefaultParameters() const
{
    return DetectionParams();
}

bool TargetDetectionBase::canHandlePointCount(size_t pointCount) const
{
    return pointCount <= 10000000;
}

std::vector<PointFullData> TargetDetectionBase::preprocessPoints(
    const std::vector<PointFullData>& points,
    const DetectionParams& params) const
{
    if (!params.enablePreprocessing) {
        return points;
    }
    
    std::vector<PointFullData> processedPoints = points;
    
    emitProgress(10, "Removing outliers");
    removeOutliers(processedPoints);
    
    emitProgress(30, "Calculating normals");
    calculateNormals(processedPoints, params.neighborhoodRadius);
    
    emitProgress(50, "Downsampling points");
    downsamplePoints(processedPoints, params.distanceThreshold * 0.5f);
    
    return processedPoints;
}

void TargetDetectionBase::calculateNormals(std::vector<PointFullData>& points, float radius) const
{
    for (size_t i = 0; i < points.size(); ++i) {
        const auto& centerPoint = points[i];
        
        std::vector<QVector3D> neighbors;
        for (size_t j = 0; j < points.size(); ++j) {
            if (i == j) continue;
            
            QVector3D diff = QVector3D(points[j].x, points[j].y, points[j].z) - 
                           QVector3D(centerPoint.x, centerPoint.y, centerPoint.z);
            
            if (diff.length() <= radius) {
                neighbors.push_back(QVector3D(points[j].x, points[j].y, points[j].z));
            }
        }
        
        if (neighbors.size() >= 3) {
            QVector3D centroid(0, 0, 0);
            for (const auto& neighbor : neighbors) {
                centroid += neighbor;
            }
            centroid /= static_cast<float>(neighbors.size());
            
            if (neighbors.size() >= 2) {
                QVector3D v1 = neighbors[0] - centroid;
                QVector3D v2 = neighbors[1] - centroid;
                QVector3D normal = QVector3D::crossProduct(v1, v2).normalized();
                
                points[i].nx = normal.x();
                points[i].ny = normal.y();
                points[i].nz = normal.z();
                points[i].hasNormal = true;
            }
        }
    }
}

void TargetDetectionBase::removeOutliers(std::vector<PointFullData>& points, 
                                       int meanK, float stddevMulThresh) const
{
    if (points.size() < static_cast<size_t>(meanK)) return;
    
    std::vector<float> distances;
    distances.reserve(points.size());
    
    for (size_t i = 0; i < points.size(); ++i) {
        const auto& point = points[i];
        QVector3D pos(point.x, point.y, point.z);
        
        std::vector<float> neighborDistances;
        for (size_t j = 0; j < points.size(); ++j) {
            if (i == j) continue;
            
            QVector3D neighborPos(points[j].x, points[j].y, points[j].z);
            float dist = (pos - neighborPos).length();
            neighborDistances.push_back(dist);
        }
        
        std::sort(neighborDistances.begin(), neighborDistances.end());
        float meanDist = 0.0f;
        int count = std::min(meanK, static_cast<int>(neighborDistances.size()));
        for (int k = 0; k < count; ++k) {
            meanDist += neighborDistances[k];
        }
        meanDist /= count;
        distances.push_back(meanDist);
    }
    
    float mean = 0.0f;
    for (float dist : distances) {
        mean += dist;
    }
    mean /= distances.size();
    
    float variance = 0.0f;
    for (float dist : distances) {
        variance += (dist - mean) * (dist - mean);
    }
    float stddev = std::sqrt(variance / distances.size());
    
    float threshold = mean + stddevMulThresh * stddev;
    std::vector<PointFullData> filteredPoints;
    filteredPoints.reserve(points.size());
    
    for (size_t i = 0; i < points.size(); ++i) {
        if (distances[i] <= threshold) {
            filteredPoints.push_back(points[i]);
        }
    }
    
    points = std::move(filteredPoints);
}

void TargetDetectionBase::downsamplePoints(std::vector<PointFullData>& points, float voxelSize) const
{
    if (voxelSize <= 0.0f || points.empty()) return;
    
    std::map<std::tuple<int, int, int>, std::vector<PointFullData>> voxelMap;
    
    for (const auto& point : points) {
        int vx = static_cast<int>(std::floor(point.x / voxelSize));
        int vy = static_cast<int>(std::floor(point.y / voxelSize));
        int vz = static_cast<int>(std::floor(point.z / voxelSize));
        
        voxelMap[std::make_tuple(vx, vy, vz)].push_back(point);
    }
    
    std::vector<PointFullData> downsampledPoints;
    downsampledPoints.reserve(voxelMap.size());
    
    for (const auto& voxel : voxelMap) {
        const auto& voxelPoints = voxel.second;
        PointFullData avgPoint = voxelPoints[0];
        
        if (voxelPoints.size() > 1) {
            float x = 0, y = 0, z = 0, intensity = 0;
            for (const auto& p : voxelPoints) {
                x += p.x; y += p.y; z += p.z; intensity += p.intensity;
            }
            float count = static_cast<float>(voxelPoints.size());
            avgPoint.x = x / count;
            avgPoint.y = y / count;
            avgPoint.z = z / count;
            avgPoint.intensity = intensity / count;
        }
        
        downsampledPoints.push_back(avgPoint);
    }
    
    points = std::move(downsampledPoints);
}

QString TargetDetectionBase::generateTargetId(const QString& prefix) const
{
    return QString("%1_%2_%3")
        .arg(prefix)
        .arg(QDateTime::currentMSecsSinceEpoch())
        .arg(++s_targetIdCounter);
}

void TargetDetectionBase::emitProgress(int percentage, const QString& stage) const
{
    emit const_cast<TargetDetectionBase*>(this)->detectionProgress(percentage, stage);
}

// DetectionParams implementation
QVariantMap TargetDetectionBase::DetectionParams::toVariantMap() const
{
    QVariantMap map;
    map["distanceThreshold"] = distanceThreshold;
    map["maxIterations"] = maxIterations;
    map["minQuality"] = minQuality;
    map["enablePreprocessing"] = enablePreprocessing;
    map["minRadius"] = minRadius;
    map["maxRadius"] = maxRadius;
    map["minInliers"] = minInliers;
    map["neighborhoodRadius"] = neighborhoodRadius;
    map["curvatureThreshold"] = curvatureThreshold;
    return map;
}

void TargetDetectionBase::DetectionParams::fromVariantMap(const QVariantMap& map)
{
    if (map.contains("distanceThreshold")) distanceThreshold = map["distanceThreshold"].toFloat();
    if (map.contains("maxIterations")) maxIterations = map["maxIterations"].toInt();
    if (map.contains("minQuality")) minQuality = map["minQuality"].toFloat();
    if (map.contains("enablePreprocessing")) enablePreprocessing = map["enablePreprocessing"].toBool();
    if (map.contains("minRadius")) minRadius = map["minRadius"].toFloat();
    if (map.contains("maxRadius")) maxRadius = map["maxRadius"].toFloat();
    if (map.contains("minInliers")) minInliers = map["minInliers"].toInt();
    if (map.contains("neighborhoodRadius")) neighborhoodRadius = map["neighborhoodRadius"].toFloat();
    if (map.contains("curvatureThreshold")) curvatureThreshold = map["curvatureThreshold"].toFloat();
}
