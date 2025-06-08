#ifndef POINTTOPLANEICP_H
#define POINTTOPLANEICP_H

#include "ICPRegistration.h"

/**
 * @brief Point-to-Plane ICP Algorithm
 * 
 * Implements the point-to-plane variant of ICP which minimizes the distance
 * from source points to the planes defined by target points and their normals.
 * This variant typically converges faster and more accurately for structured
 * environments with planar surfaces.
 */
class PointToPlaneICP : public ICPRegistration {
    Q_OBJECT

public:
    explicit PointToPlaneICP(QObject* parent = nullptr);
    virtual ~PointToPlaneICP() = default;

    /**
     * @brief Compute transformation using point-to-plane ICP
     * @param source Source point cloud to be transformed
     * @param target Target point cloud with normals (reference)
     * @param initialGuess Initial transformation estimate
     * @param params ICP algorithm parameters
     * @return Final transformation matrix
     */
    QMatrix4x4 compute(const PointCloud& source, const PointCloud& target,
                      const QMatrix4x4& initialGuess, const ICPParams& params) override;

protected:
    /**
     * @brief Find correspondences with normal information
     * @param source Source point cloud
     * @param target Target point cloud with normals
     * @param maxDistance Maximum correspondence distance
     * @return List of correspondences with normal information
     */
    std::vector<Correspondence> findCorrespondences(
        const PointCloud& source, const PointCloud& target, float maxDistance) override;

    /**
     * @brief Compute transformation using point-to-plane error metric
     * @param correspondences List of point correspondences with normals
     * @return Transformation matrix
     */
    QMatrix4x4 computeTransformation(const std::vector<Correspondence>& correspondences) override;

    /**
     * @brief Calculate point-to-plane RMS error
     * @param correspondences List of point correspondences with normals
     * @return Point-to-plane RMS error value
     */
    float calculateRMSError(const std::vector<Correspondence>& correspondences) override;

private:
    /**
     * @brief Estimate normals for point cloud if not present
     * @param cloud Point cloud to estimate normals for
     * @param searchRadius Radius for normal estimation
     */
    void estimateNormals(PointCloud& cloud, float searchRadius = 0.1f);

    /**
     * @brief Solve point-to-plane linear system using least squares
     * @param correspondences List of correspondences with normals
     * @return 6-DOF transformation parameters [tx, ty, tz, rx, ry, rz]
     */
    std::vector<float> solveLinearSystem(const std::vector<Correspondence>& correspondences);

    /**
     * @brief Convert 6-DOF parameters to transformation matrix
     * @param params 6-DOF parameters [tx, ty, tz, rx, ry, rz]
     * @return 4x4 transformation matrix
     */
    QMatrix4x4 parametersToMatrix(const std::vector<float>& params);
};

#endif // POINTTOPLANEICP_H
