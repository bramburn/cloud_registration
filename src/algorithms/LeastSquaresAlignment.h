#pragma once

#include <QList>
#include <QVector3D>
#include <QMatrix4x4>
#include <QPair>

/**
 * @brief LeastSquaresAlignment - Implements least-squares transformation algorithms
 * 
 * This class provides static methods for computing optimal rigid body transformations
 * from correspondence point pairs using least-squares optimization. The implementation
 * uses Singular Value Decomposition (SVD) to find the rotation and translation that
 * minimizes the sum of squared distances between corresponding points.
 * 
 * Sprint 4 Requirements:
 * - Implements Horn's method for point-to-point alignment
 * - Uses Eigen library for robust SVD computation
 * - Handles reflection case detection and correction
 * - Provides survey-grade accuracy for professional applications
 * - Supports 3+ correspondence points with least-squares fitting
 */
class LeastSquaresAlignment
{
public:
    /**
     * @brief Compute optimal rigid body transformation from correspondence pairs
     * 
     * Uses the Horn's method with SVD to compute the optimal rotation and translation
     * that minimizes the sum of squared distances between corresponding points.
     * 
     * Algorithm steps:
     * 1. Calculate centroids of source and target point sets
     * 2. Center points by subtracting respective centroids
     * 3. Compute 3x3 covariance matrix H = sum(centered_source_i * centered_target_i^T)
     * 4. Perform SVD decomposition: H = U * S * V^T
     * 5. Calculate rotation matrix R = V * U^T
     * 6. Handle reflection case: if det(R) < 0, negate last column of V
     * 7. Calculate translation t = centroid_target - R * centroid_source
     * 8. Assemble 4x4 transformation matrix
     * 
     * @param correspondences List of point pairs (source, target)
     * @return 4x4 transformation matrix that transforms source points to target points
     * 
     * @note Requires at least 3 non-collinear correspondence pairs
     * @note For exactly 3 points, provides perfect alignment (within floating-point precision)
     * @note For >3 points, provides least-squares best-fit transformation
     */
    static QMatrix4x4 computeTransformation(
        const QList<QPair<QVector3D, QVector3D>>& correspondences);

private:
    /**
     * @brief Calculate centroid (average position) of a point set
     * @param points List of 3D points
     * @return Centroid position
     */
    static QVector3D calculateCentroid(const QList<QVector3D>& points);
    
    /**
     * @brief Validate input correspondences for transformation computation
     * @param correspondences List of point pairs to validate
     * @return true if correspondences are valid for transformation computation
     */
    static bool validateCorrespondences(
        const QList<QPair<QVector3D, QVector3D>>& correspondences);
    
    /**
     * @brief Check if points are collinear (degenerate case)
     * @param points List of 3D points to check
     * @return true if points are collinear within tolerance
     */
    static bool arePointsCollinear(const QList<QVector3D>& points);
    
    /**
     * @brief Convert Eigen matrix to QMatrix4x4
     * @param rotation 3x3 rotation matrix from Eigen
     * @param translation 3x1 translation vector from Eigen
     * @return 4x4 transformation matrix in Qt format
     */
    static QMatrix4x4 assembleTransformationMatrix(
        const float rotation[9],
        const float translation[3]);

    // Constants for numerical stability
    static constexpr float COLLINEARITY_THRESHOLD = 1e-6f;
    static constexpr float MINIMUM_POINT_SEPARATION = 1e-3f; // 1mm minimum separation
};
