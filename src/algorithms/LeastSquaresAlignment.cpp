#include "LeastSquaresAlignment.h"
#include <Eigen/SVD>
#include <Eigen/Dense>
#include <QDebug>
#include <cmath>

QMatrix4x4 LeastSquaresAlignment::computeTransformation(
    const QList<QPair<QVector3D, QVector3D>>& correspondences)
{
    // Validate input
    if (!validateCorrespondences(correspondences)) {
        qWarning() << "Invalid correspondences for transformation computation";
        return QMatrix4x4(); // Return identity matrix
    }
    
    qDebug() << "Computing transformation for" << correspondences.size() << "correspondences";
    
    // Extract source and target points
    QList<QVector3D> sourcePoints, targetPoints;
    for (const auto& pair : correspondences) {
        sourcePoints.append(pair.first);
        targetPoints.append(pair.second);
    }
    
    // Check for degenerate cases
    if (arePointsCollinear(sourcePoints) || arePointsCollinear(targetPoints)) {
        qWarning() << "Collinear points detected - transformation may be unstable";
    }
    
    // Calculate centroids
    QVector3D sourceCentroid = calculateCentroid(sourcePoints);
    QVector3D targetCentroid = calculateCentroid(targetPoints);
    
    qDebug() << "Source centroid:" << sourceCentroid;
    qDebug() << "Target centroid:" << targetCentroid;
    
    // Compute covariance matrix H
    Eigen::Matrix3f H = Eigen::Matrix3f::Zero();
    
    for (const auto& pair : correspondences) {
        // Center points by subtracting centroids
        QVector3D centeredSource = pair.first - sourceCentroid;
        QVector3D centeredTarget = pair.second - targetCentroid;
        
        // Convert to Eigen vectors
        Eigen::Vector3f srcVec(centeredSource.x(), centeredSource.y(), centeredSource.z());
        Eigen::Vector3f tgtVec(centeredTarget.x(), centeredTarget.y(), centeredTarget.z());
        
        // Accumulate outer product: H += src * tgt^T
        H += srcVec * tgtVec.transpose();
    }
    
    qDebug() << "Covariance matrix H computed";
    
    // Perform SVD decomposition: H = U * S * V^T
    Eigen::JacobiSVD<Eigen::Matrix3f> svd(H, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Eigen::Matrix3f U = svd.matrixU();
    Eigen::Matrix3f V = svd.matrixV();
    
    qDebug() << "SVD decomposition completed";
    
    // Calculate rotation matrix R = V * U^T
    Eigen::Matrix3f R = V * U.transpose();
    
    // Handle reflection case: if det(R) < 0, correct by negating last column of V
    if (R.determinant() < 0) {
        qDebug() << "Reflection case detected - correcting rotation matrix";
        V.col(2) *= -1;
        R = V * U.transpose();
    }
    
    // Calculate translation vector t = centroid_target - R * centroid_source
    Eigen::Vector3f srcCentroidEigen(sourceCentroid.x(), sourceCentroid.y(), sourceCentroid.z());
    Eigen::Vector3f tgtCentroidEigen(targetCentroid.x(), targetCentroid.y(), targetCentroid.z());
    Eigen::Vector3f t = tgtCentroidEigen - R * srcCentroidEigen;
    
    qDebug() << "Rotation and translation computed";

    // Convert Eigen matrices to arrays for assembleTransformationMatrix
    float rotationArray[9];
    float translationArray[3];

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            rotationArray[i * 3 + j] = R(i, j);
        }
        translationArray[i] = t(i);
    }

    // Assemble and return transformation matrix
    return assembleTransformationMatrix(rotationArray, translationArray);
}

QVector3D LeastSquaresAlignment::calculateCentroid(const QList<QVector3D>& points)
{
    if (points.isEmpty()) {
        return QVector3D(0, 0, 0);
    }
    
    QVector3D centroid(0, 0, 0);
    for (const QVector3D& point : points) {
        centroid += point;
    }
    
    return centroid / static_cast<float>(points.size());
}

bool LeastSquaresAlignment::validateCorrespondences(
    const QList<QPair<QVector3D, QVector3D>>& correspondences)
{
    // Need at least 3 correspondences for 3D transformation
    if (correspondences.size() < 3) {
        qWarning() << "Insufficient correspondences:" << correspondences.size() << "< 3";
        return false;
    }
    
    // Check for duplicate points
    for (int i = 0; i < correspondences.size(); ++i) {
        for (int j = i + 1; j < correspondences.size(); ++j) {
            float srcDist = correspondences[i].first.distanceToPoint(correspondences[j].first);
            float tgtDist = correspondences[i].second.distanceToPoint(correspondences[j].second);
            
            if (srcDist < MINIMUM_POINT_SEPARATION || tgtDist < MINIMUM_POINT_SEPARATION) {
                qWarning() << "Duplicate or very close points detected at indices" << i << "and" << j;
                return false;
            }
        }
    }
    
    return true;
}

bool LeastSquaresAlignment::arePointsCollinear(const QList<QVector3D>& points)
{
    if (points.size() < 3) {
        return true; // Less than 3 points are always collinear
    }
    
    // Check if all points lie on the same line
    QVector3D v1 = points[1] - points[0];
    v1.normalize();
    
    for (int i = 2; i < points.size(); ++i) {
        QVector3D v2 = points[i] - points[0];
        v2.normalize();
        
        // Cross product magnitude indicates deviation from collinearity
        QVector3D cross = QVector3D::crossProduct(v1, v2);
        if (cross.length() > COLLINEARITY_THRESHOLD) {
            return false; // Found non-collinear point
        }
    }
    
    return true; // All points are collinear
}

QMatrix4x4 LeastSquaresAlignment::assembleTransformationMatrix(
    const float rotation[9],
    const float translation[3])
{
    QMatrix4x4 transform;
    transform.setToIdentity();
    
    // Set rotation part (top-left 3x3)
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            transform(row, col) = rotation[row * 3 + col];
        }
    }

    // Set translation part (top-right 3x1)
    transform(0, 3) = translation[0];
    transform(1, 3) = translation[1];
    transform(2, 3) = translation[2];
    
    // Bottom row is already [0, 0, 0, 1] from setToIdentity()
    
    qDebug() << "Transformation matrix assembled successfully";
    return transform;
}
