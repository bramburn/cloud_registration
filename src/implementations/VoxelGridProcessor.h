#ifndef VOXELGRIDPROCESSOR_H
#define VOXELGRIDPROCESSOR_H

#include "../interfaces/IPointCloudProcessor.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <unordered_map>

/**
 * @brief Concrete implementation of IPointCloudProcessor for voxel grid filtering
 * 
 * This class demonstrates how to properly implement the modern interface pattern:
 * - Inherits from abstract interface
 * - Implements all pure virtual functions
 * - Maintains internal state while respecting interface contract
 * - Provides specific functionality (voxel grid downsampling)
 */
class VoxelGridProcessor : public IPointCloudProcessor {
private:
    // Internal state
    mutable std::vector<std::pair<QString, QString>> m_lastStats;
    double m_lastProcessingTime = 0.0;
    
    // Voxel grid parameters
    struct VoxelGridParams {
        float voxelSize = 0.1f;
        bool preserveIntensity = true;
        bool useAveraging = true;
        
        static VoxelGridParams fromJson(const QString& jsonStr);
        QString toJson() const;
    };

    // Voxel key for spatial hashing
    struct VoxelKey {
        int x, y, z;
        
        bool operator==(const VoxelKey& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    // Hash function for VoxelKey
    struct VoxelKeyHash {
        std::size_t operator()(const VoxelKey& key) const {
            return std::hash<int>()(key.x) ^ 
                   (std::hash<int>()(key.y) << 1) ^ 
                   (std::hash<int>()(key.z) << 2);
        }
    };

    // Helper methods
    VoxelKey getVoxelKey(const Point3D& point, float voxelSize) const;
    Point3D averagePoints(const std::vector<Point3D>& points) const;
    void updateStatistics(const ProcessingResult& result) const;

public:
    /**
     * @brief Constructor with default parameters
     */
    VoxelGridProcessor() = default;

    /**
     * @brief Virtual destructor
     */
    ~VoxelGridProcessor() override = default;

    // IPointCloudProcessor interface implementation
    ProcessingResult processPointCloud(
        const std::vector<Point3D>& points,
        const QString& parameters = QString()) override;

    QString getProcessorName() const override {
        return "VoxelGridProcessor";
    }

    bool supportsPointCount(size_t pointCount) const override {
        // Can handle up to 10 million points efficiently
        return pointCount <= 10000000;
    }

    QString getRecommendedParameters() const override;

    bool validateParameters(const QString& parameters) const override;

    std::vector<std::pair<QString, QString>> getLastProcessingStats() const override {
        return m_lastStats;
    }
};

/**
 * @brief Factory implementation for creating VoxelGridProcessor instances
 */
class VoxelGridProcessorFactory : public IPointCloudProcessorFactory {
public:
    ~VoxelGridProcessorFactory() override = default;

    std::unique_ptr<IPointCloudProcessor> createProcessor(
        const QString& processorType) override {
        if (processorType == "VoxelGrid" || processorType == "voxel_grid") {
            return std::make_unique<VoxelGridProcessor>();
        }
        return nullptr;
    }

    std::vector<QString> getSupportedProcessorTypes() const override {
        return {"VoxelGrid", "voxel_grid"};
    }
};

#endif // VOXELGRIDPROCESSOR_H
