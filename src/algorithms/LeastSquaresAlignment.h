#ifndef LEASTSQUARESALIGNMENT_H
#define LEASTSQUARESALIGNMENT_H

#include <QMatrix4x4>
#include <QVector3D>
#include <QList>
#include <QPair>

/**
 * @brief Least Squares Alignment Algorithm
 * 
 * Implements various least-squares algorithms for computing rigid transformations
 * from point correspondences. This class provides the core transformation
 * computation used by ICP and manual alignment systems.
 */
class LeastSquaresAlignment {
public:
    /**
     * @brief Compute rigid transformation using Horn's method (quaternion-based)
     * @param correspondences List of point correspondences (source, target)
     * @return 4x4 transformation matrix
     */
    static QMatrix4x4 computeTransformation(const QList<QPair<QVector3D, QVector3D>>& correspondences);
    
    /**
     * @brief Compute rigid transformation using SVD-based method
     * @param correspondences List of point correspondences (source, target)
     * @return 4x4 transformation matrix
     */
    static QMatrix4x4 computeTransformationSVD(const QList<QPair<QVector3D, QVector3D>>& correspondences);
    
    /**
     * @brief Compute weighted rigid transformation
     * @param correspondences List of point correspondences (source, target)
     * @param weights Weight for each correspondence
     * @return 4x4 transformation matrix
     */
    static QMatrix4x4 computeWeightedTransformation(
        const QList<QPair<QVector3D, QVector3D>>& correspondences,
        const QList<float>& weights);

private:
    // Helper methods for transformation computation
    static QMatrix4x4 hornMethod(const QList<QPair<QVector3D, QVector3D>>& correspondences);
    static QMatrix4x4 svdMethod(const QList<QPair<QVector3D, QVector3D>>& correspondences);
    
    // Utility methods
    static QPair<QVector3D, QVector3D> computeCentroids(
        const QList<QPair<QVector3D, QVector3D>>& correspondences);
    static QPair<QVector3D, QVector3D> computeWeightedCentroids(
        const QList<QPair<QVector3D, QVector3D>>& correspondences,
        const QList<float>& weights);
};

#endif // LEASTSQUARESALIGNMENT_H
