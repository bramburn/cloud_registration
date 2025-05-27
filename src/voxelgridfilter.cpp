#include "voxelgridfilter.h"
#include "loadingsettings.h"
#include <QDebug>
#include <algorithm>
#include <limits>
#include <cmath>

VoxelGridFilter::VoxelGridFilter()
    : m_minBound{std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max(),
                 std::numeric_limits<float>::max()}
    , m_maxBound{std::numeric_limits<float>::lowest(),
                 std::numeric_limits<float>::lowest(),
                 std::numeric_limits<float>::lowest()}
{
}

VoxelGridFilter::~VoxelGridFilter()
{
}

std::vector<float> VoxelGridFilter::filter(const std::vector<float>& input, const LoadingSettings& settings)
{
    // Validate input
    if (input.empty() || input.size() % 3 != 0) {
        qWarning() << "VoxelGridFilter: Invalid input - empty or not divisible by 3";
        return std::vector<float>();
    }

    // Extract parameters from settings
    float leafSize = settings.parameters.value("leafSize", 0.1).toFloat();
    int minPointsPerVoxel = settings.parameters.value("minPointsPerVoxel", 1).toInt();

    // Validate parameters
    if (leafSize <= 0.0f) {
        qWarning() << "VoxelGridFilter: Invalid leafSize" << leafSize << "- using default 0.1";
        leafSize = 0.1f;
    }
    if (minPointsPerVoxel < 1) {
        qWarning() << "VoxelGridFilter: Invalid minPointsPerVoxel" << minPointsPerVoxel << "- using default 1";
        minPointsPerVoxel = 1;
    }

    // Compute bounding box
    computeBoundingBox(input);

    // Create voxel map to group points
    std::unordered_map<VoxelKey, std::vector<Vector3D>, VoxelKeyHasher> voxelMap;

    // Group points into voxels
    const size_t pointCount = input.size() / 3;
    for (size_t i = 0; i < pointCount; ++i) {
        float x = input[i * 3];
        float y = input[i * 3 + 1];
        float z = input[i * 3 + 2];

        VoxelKey key = worldToVoxelKey(x, y, z, leafSize);
        voxelMap[key].emplace_back(Vector3D{x, y, z});
    }

    // Generate output points (centroids of voxels with sufficient points)
    std::vector<float> output;
    output.reserve(voxelMap.size() * 3); // Reserve space for efficiency

    for (const auto& [key, points] : voxelMap) {
        if (static_cast<int>(points.size()) >= minPointsPerVoxel) {
            Vector3D centroid = calculateVoxelCentroid(points);
            output.push_back(centroid.x);
            output.push_back(centroid.y);
            output.push_back(centroid.z);
        }
    }

    qDebug() << "VoxelGridFilter: Reduced" << pointCount << "points to" << (output.size() / 3)
             << "points using leafSize" << leafSize << "and minPointsPerVoxel" << minPointsPerVoxel;

    return output;
}

void VoxelGridFilter::computeBoundingBox(const std::vector<float>& points)
{
    // Reset bounds
    m_minBound = Vector3D{std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max(),
                          std::numeric_limits<float>::max()};
    m_maxBound = Vector3D{std::numeric_limits<float>::lowest(),
                          std::numeric_limits<float>::lowest(),
                          std::numeric_limits<float>::lowest()};

    // Find min and max coordinates
    const size_t pointCount = points.size() / 3;
    for (size_t i = 0; i < pointCount; ++i) {
        float x = points[i * 3];
        float y = points[i * 3 + 1];
        float z = points[i * 3 + 2];

        m_minBound.x = std::min(m_minBound.x, x);
        m_minBound.y = std::min(m_minBound.y, y);
        m_minBound.z = std::min(m_minBound.z, z);

        m_maxBound.x = std::max(m_maxBound.x, x);
        m_maxBound.y = std::max(m_maxBound.y, y);
        m_maxBound.z = std::max(m_maxBound.z, z);
    }

    qDebug() << "VoxelGridFilter: Bounding box - Min:(" << m_minBound.x << "," << m_minBound.y << "," << m_minBound.z
             << ") Max:(" << m_maxBound.x << "," << m_maxBound.y << "," << m_maxBound.z << ")";
}

VoxelGridFilter::Vector3D VoxelGridFilter::calculateVoxelCentroid(const std::vector<Vector3D>& points) const
{
    if (points.empty()) {
        return Vector3D{0, 0, 0};
    }

    Vector3D sum{0, 0, 0};
    for (const Vector3D& point : points) {
        sum.x += point.x;
        sum.y += point.y;
        sum.z += point.z;
    }

    float count = static_cast<float>(points.size());
    return Vector3D{sum.x / count, sum.y / count, sum.z / count};
}

VoxelGridFilter::VoxelKey VoxelGridFilter::worldToVoxelKey(float x, float y, float z, float leafSize) const
{
    return VoxelKey{
        static_cast<int>(std::floor((x - m_minBound.x) / leafSize)),
        static_cast<int>(std::floor((y - m_minBound.y) / leafSize)),
        static_cast<int>(std::floor((z - m_minBound.z) / leafSize))
    };
}
