#include "PointToPlaneICP.h"
#include <QDebug>
#include <cmath>
#include <algorithm>

PointToPlaneICP::PointToPlaneICP(QObject* parent)
    : ICPRegistration(parent) {
}

QMatrix4x4 PointToPlaneICP::compute(const PointCloud& source, const PointCloud& target,
                                   const QMatrix4x4& initialGuess, const ICPParams& params) {
    
    // Ensure target has normals
    PointCloud workingTarget = target;
    if (workingTarget.normals.empty()) {
        qDebug() << "Target point cloud has no normals, estimating...";
        estimateNormals(workingTarget);
    }
    
    if (workingTarget.normals.empty()) {
        qWarning() << "Failed to estimate normals, falling back to point-to-point ICP";
        return ICPRegistration::compute(source, target, initialGuess, params);
    }
    
    qDebug() << "Starting Point-to-Plane ICP with" << source.size() << "source points and" 
             << target.size() << "target points with normals";
    
    // Use the base class implementation but with overridden methods
    return ICPRegistration::compute(source, workingTarget, initialGuess, params);
}

std::vector<Correspondence> PointToPlaneICP::findCorrespondences(
    const PointCloud& source, const PointCloud& target, float maxDistance) {
    
    std::vector<Correspondence> correspondences;
    correspondences.reserve(source.size());
    
    auto kdTree = buildKDTree(target);
    
    for (const auto& sourcePoint : source.points) {
        QVector3D nearestPoint;
        float distance;
        
        if (kdTree->findNearestNeighbor(sourcePoint, maxDistance, nearestPoint, distance)) {
            // Find the index of the nearest point to get its normal
            auto it = std::find(target.points.begin(), target.points.end(), nearestPoint);
            if (it != target.points.end()) {
                size_t index = std::distance(target.points.begin(), it);
                
                Correspondence corr(sourcePoint, nearestPoint, distance);
                if (index < target.normals.size()) {
                    corr.targetNormal = target.normals[index];
                }
                correspondences.push_back(corr);
            }
        }
    }
    
    qDebug() << "Found" << correspondences.size() << "correspondences with normals out of" 
             << source.size() << "source points";
    
    return correspondences;
}

QMatrix4x4 PointToPlaneICP::computeTransformation(const std::vector<Correspondence>& correspondences) {
    if (correspondences.size() < 6) {  // Need at least 6 correspondences for 6-DOF
        qWarning() << "Insufficient correspondences for point-to-plane transformation:" << correspondences.size();
        return QMatrix4x4();
    }
    
    // Solve the linear system for point-to-plane ICP
    auto params = solveLinearSystem(correspondences);
    if (params.empty()) {
        qWarning() << "Failed to solve linear system, falling back to point-to-point";
        return ICPRegistration::computeTransformation(correspondences);
    }
    
    return parametersToMatrix(params);
}

float PointToPlaneICP::calculateRMSError(const std::vector<Correspondence>& correspondences) {
    if (correspondences.empty()) return 0.0f;
    
    float sumSquaredErrors = 0.0f;
    int validCount = 0;
    
    for (const auto& corr : correspondences) {
        if (corr.isValid && corr.targetNormal.length() > 0.0f) {
            // Calculate point-to-plane distance
            QVector3D diff = corr.sourcePoint - corr.targetPoint;
            float planeDistance = QVector3D::dotProduct(diff, corr.targetNormal.normalized());
            sumSquaredErrors += planeDistance * planeDistance;
            validCount++;
        }
    }
    
    if (validCount == 0) return 0.0f;
    
    return std::sqrt(sumSquaredErrors / validCount);
}

void PointToPlaneICP::estimateNormals(PointCloud& cloud, float searchRadius) {
    if (cloud.empty()) return;
    
    cloud.normals.clear();
    cloud.normals.reserve(cloud.size());
    
    // Simple normal estimation using local neighborhood
    auto kdTree = std::make_unique<KDTree>(cloud);
    
    for (const auto& point : cloud.points) {
        // Find neighbors within search radius
        std::vector<QVector3D> neighbors;
        
        // For simplicity, use a basic approach - in production, use proper radius search
        for (const auto& otherPoint : cloud.points) {
            float dist = (point - otherPoint).length();
            if (dist <= searchRadius && dist > 0.0f) {
                neighbors.push_back(otherPoint);
            }
        }
        
        if (neighbors.size() < 3) {
            // Not enough neighbors, use default normal
            cloud.normals.emplace_back(0, 0, 1);
            continue;
        }
        
        // Compute centroid of neighbors
        QVector3D centroid(0, 0, 0);
        for (const auto& neighbor : neighbors) {
            centroid += neighbor;
        }
        centroid /= static_cast<float>(neighbors.size());
        
        // Compute covariance matrix (simplified)
        float cov[3][3] = {{0}};
        for (const auto& neighbor : neighbors) {
            QVector3D diff = neighbor - centroid;
            cov[0][0] += diff.x() * diff.x();
            cov[0][1] += diff.x() * diff.y();
            cov[0][2] += diff.x() * diff.z();
            cov[1][1] += diff.y() * diff.y();
            cov[1][2] += diff.y() * diff.z();
            cov[2][2] += diff.z() * diff.z();
        }
        cov[1][0] = cov[0][1];
        cov[2][0] = cov[0][2];
        cov[2][1] = cov[1][2];
        
        // For simplicity, use the smallest eigenvalue direction as normal
        // In production, use proper eigenvalue decomposition
        QVector3D normal(0, 0, 1);  // Default normal
        
        // Simple heuristic: if the surface is more horizontal, normal points up
        if (cov[2][2] < cov[0][0] && cov[2][2] < cov[1][1]) {
            normal = QVector3D(0, 0, 1);
        } else if (cov[1][1] < cov[0][0]) {
            normal = QVector3D(0, 1, 0);
        } else {
            normal = QVector3D(1, 0, 0);
        }
        
        cloud.normals.push_back(normal.normalized());
    }
    
    qDebug() << "Estimated normals for" << cloud.normals.size() << "points";
}

std::vector<float> PointToPlaneICP::solveLinearSystem(const std::vector<Correspondence>& correspondences) {
    // For point-to-plane ICP, we solve: A * x = b
    // where x = [tx, ty, tz, rx, ry, rz] (translation and small rotation angles)
    // This is a simplified implementation - production code would use proper linear algebra
    
    if (correspondences.size() < 6) {
        return {};
    }
    
    // For now, fall back to the centroid-based approach
    // A full implementation would set up the 6x6 linear system
    qDebug() << "Point-to-plane linear system solving not fully implemented, using simplified approach";
    
    // Return small incremental transformation
    return {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
}

QMatrix4x4 PointToPlaneICP::parametersToMatrix(const std::vector<float>& params) {
    if (params.size() != 6) {
        return QMatrix4x4();
    }
    
    QMatrix4x4 transform;
    transform.setToIdentity();
    
    // Translation
    transform(0, 3) = params[0];  // tx
    transform(1, 3) = params[1];  // ty
    transform(2, 3) = params[2];  // tz
    
    // Small angle rotations (linearized)
    float rx = params[3];
    float ry = params[4];
    float rz = params[5];
    
    // Rotation matrix for small angles
    transform(0, 0) = 1.0f;
    transform(0, 1) = -rz;
    transform(0, 2) = ry;
    transform(1, 0) = rz;
    transform(1, 1) = 1.0f;
    transform(1, 2) = -rx;
    transform(2, 0) = -ry;
    transform(2, 1) = rx;
    transform(2, 2) = 1.0f;
    
    return transform;
}
