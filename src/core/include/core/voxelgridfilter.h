#ifndef VOXELGRIDFILTER_H
#define VOXELGRIDFILTER_H

// #include <QVector3D>  // Commented out for testing compatibility
#include <vector>
#include <unordered_map>

// Forward declaration
struct LoadingSettings;

/**
 * @brief VoxelGridFilter implements voxel grid subsampling for point clouds
 *
 * This class provides efficient point cloud downsampling by dividing the 3D space
 * into a regular grid of voxels (3D cubes) and representing each occupied voxel
 * with a single representative point (typically the centroid of all points within
 * that voxel). This approach maintains uniform spatial distribution while
 * significantly reducing point count.
 */
class VoxelGridFilter
{
public:
    /**
     * @brief Default constructor
     */
    VoxelGridFilter();

    /**
     * @brief Destructor
     */
    ~VoxelGridFilter();

    /**
     * @brief Apply voxel grid filtering to input point cloud
     * @param input Input point cloud as vector of floats (XYZXYZXYZ...)
     * @param settings Loading settings containing leafSize and minPointsPerVoxel parameters
     * @return Filtered point cloud as vector of floats
     */
    std::vector<float> filter(const std::vector<float>& input, const LoadingSettings& settings);

private:
    /**
     * @brief Structure representing a 3D voxel key for spatial hashing
     */
    struct VoxelKey {
        int x, y, z;

        bool operator==(const VoxelKey& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    /**
     * @brief Hash function for VoxelKey to enable use in unordered_map
     */
    struct VoxelKeyHasher {
        size_t operator()(const VoxelKey& k) const {
            // Use prime numbers to reduce hash collisions
            return ((static_cast<size_t>(k.x) * 73856093) ^
                    (static_cast<size_t>(k.y) * 19349663) ^
                    (static_cast<size_t>(k.z) * 83492791));
        }
    };

    // Simple 3D vector structure for testing compatibility
    struct Vector3D { float x, y, z; };

    // Member variables for bounding box calculation
    Vector3D m_minBound;
    Vector3D m_maxBound;

    /**
     * @brief Compute the bounding box of the input point cloud
     * @param points Input point cloud as vector of floats
     */
    void computeBoundingBox(const std::vector<float>& points);

    /**
     * @brief Calculate the centroid of a collection of 3D points
     * @param points Vector of Vector3D points
     * @return Centroid as Vector3D
     */
    Vector3D calculateVoxelCentroid(const std::vector<Vector3D>& points) const;

    /**
     * @brief Convert world coordinates to voxel key
     * @param x World X coordinate
     * @param y World Y coordinate
     * @param z World Z coordinate
     * @param leafSize Size of each voxel cube
     * @return VoxelKey for the given coordinates
     */
    VoxelKey worldToVoxelKey(float x, float y, float z, float leafSize) const;
};

#endif // VOXELGRIDFILTER_H
