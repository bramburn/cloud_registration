#include "FeatureBasedRegistration.h"
#include <QDebug>
#include <QElapsedTimer>
#include <algorithm>
#include <cmath>

namespace Registration {

FeatureBasedRegistration::FeatureBasedRegistration(QObject* parent) 
    : QObject(parent), m_featureExtractor(new Features::FeatureExtractor(this)) {
}

FeatureBasedRegistration::Result FeatureBasedRegistration::registerPointClouds(
    const std::vector<Point3D>& sourcePoints,
    const std::vector<Point3D>& targetPoints,
    const Parameters& params) {
    
    QElapsedTimer timer;
    timer.start();
    
    Result result;
    
    try {
        emit registrationProgress(0);
        
        // Extract planes from source point cloud
        qDebug() << "Extracting planes from source cloud (" << sourcePoints.size() << "points)";
        QList<Features::Plane> sourcePlanes = m_featureExtractor->extractPlanes(
            sourcePoints, params.extractionParams);
        result.sourcePlanesFound = sourcePlanes.size();
        
        emit registrationProgress(25);
        
        // Extract planes from target point cloud
        qDebug() << "Extracting planes from target cloud (" << targetPoints.size() << "points)";
        QList<Features::Plane> targetPlanes = m_featureExtractor->extractPlanes(
            targetPoints, params.extractionParams);
        result.targetPlanesFound = targetPlanes.size();
        
        emit registrationProgress(50);
        emit planesExtracted(result.sourcePlanesFound, result.targetPlanesFound);
        
        if (sourcePlanes.size() < params.minCorrespondences || 
            targetPlanes.size() < params.minCorrespondences) {
            result.errorMessage = QString("Insufficient planes found (source: %1, target: %2, min: %3)")
                .arg(sourcePlanes.size()).arg(targetPlanes.size()).arg(params.minCorrespondences);
            qWarning() << result.errorMessage;
            return result;
        }
        
        // Find plane correspondences
        qDebug() << "Finding plane correspondences";
        result.correspondences = findPlaneCorrespondences(sourcePlanes, targetPlanes, params);
        result.correspondencesFound = result.correspondences.size();
        
        emit registrationProgress(75);
        emit correspondencesFound(result.correspondencesFound);
        
        if (result.correspondences.size() < params.minCorrespondences) {
            result.errorMessage = QString("Insufficient correspondences found: %1 (min: %2)")
                .arg(result.correspondences.size()).arg(params.minCorrespondences);
            qWarning() << result.errorMessage;
            return result;
        }
        
        // Compute transformation
        qDebug() << "Computing transformation from" << result.correspondences.size() << "correspondences";
        result.transformation = computeTransformFromPlanes(result.correspondences);
        
        emit registrationProgress(90);
        
        // Validate result if requested
        if (params.validateResult) {
            result.quality = validateRegistrationQuality(result, sourcePoints, targetPoints);
            
            if (result.quality < params.minRegistrationQuality) {
                result.errorMessage = QString("Registration quality too low: %1 (min: %2)")
                    .arg(result.quality).arg(params.minRegistrationQuality);
                qWarning() << result.errorMessage;
                return result;
            }
        } else {
            result.quality = 1.0f; // Assume good quality if not validated
        }
        
        result.success = true;
        
        qDebug() << "Feature-based registration completed successfully in" 
                 << timer.elapsed() << "ms";
        qDebug() << "Quality:" << result.quality 
                 << "Correspondences:" << result.correspondencesFound;
        
        emit registrationProgress(100);
        emit registrationCompleted(result);
        
    } catch (const std::exception& e) {
        result.errorMessage = QString("Registration failed: %1").arg(e.what());
        qCritical() << result.errorMessage;
    }
    
    return result;
}

QList<QPair<Features::Plane, Features::Plane>> FeatureBasedRegistration::findPlaneCorrespondences(
    const QList<Features::Plane>& sourcePlanes,
    const QList<Features::Plane>& targetPlanes,
    const Parameters& params) {
    
    QList<QPair<Features::Plane, Features::Plane>> correspondences;
    QSet<int> usedTargetIndices;
    
    // For each source plane, find the best matching target plane
    for (int i = 0; i < sourcePlanes.size(); ++i) {
        const Features::Plane& sourcePlane = sourcePlanes[i];
        
        int bestTargetIndex = -1;
        float bestSimilarity = 0.0f;
        
        for (int j = 0; j < targetPlanes.size(); ++j) {
            if (usedTargetIndices.contains(j)) {
                continue; // Already used
            }
            
            const Features::Plane& targetPlane = targetPlanes[j];
            float similarity = calculatePlaneSimilarity(sourcePlane, targetPlane, params);
            
            if (similarity > bestSimilarity) {
                bestSimilarity = similarity;
                bestTargetIndex = j;
            }
        }
        
        // Add correspondence if similarity is above threshold
        if (bestTargetIndex >= 0 && bestSimilarity > 0.5f) {
            correspondences.append({sourcePlane, targetPlanes[bestTargetIndex]});
            usedTargetIndices.insert(bestTargetIndex);
            
            qDebug() << "Found correspondence" << i << "->" << bestTargetIndex 
                     << "similarity:" << bestSimilarity;
        }
    }
    
    return correspondences;
}

QMatrix4x4 FeatureBasedRegistration::computeTransformFromPlanes(
    const QList<QPair<Features::Plane, Features::Plane>>& correspondences) {
    
    if (correspondences.size() < 2) {
        qWarning() << "Insufficient correspondences for transformation computation";
        return QMatrix4x4();
    }
    
    // Extract normals and centroids
    QList<QVector3D> sourceNormals, targetNormals;
    QList<QVector3D> sourceCentroids, targetCentroids;
    
    for (const auto& correspondence : correspondences) {
        sourceNormals.append(correspondence.first.normal);
        targetNormals.append(correspondence.second.normal);
        sourceCentroids.append(correspondence.first.centroid);
        targetCentroids.append(correspondence.second.centroid);
    }
    
    // Step 1: Solve for rotation using normal vectors
    QMatrix4x4 rotation = solveForRotation(sourceNormals, targetNormals);
    
    // Step 2: Solve for translation using centroids
    QVector3D translation = solveForTranslation(sourceCentroids, targetCentroids, rotation);
    
    // Combine rotation and translation
    QMatrix4x4 transformation = rotation;
    transformation(0, 3) = translation.x();
    transformation(1, 3) = translation.y();
    transformation(2, 3) = translation.z();
    
    return transformation;
}

float FeatureBasedRegistration::validateRegistrationQuality(
    const Result& result,
    const std::vector<Point3D>& sourcePoints,
    const std::vector<Point3D>& targetPoints) const {
    
    if (!result.success || result.correspondences.isEmpty()) {
        return 0.0f;
    }
    
    // Calculate registration error
    float registrationError = calculateRegistrationError(result.correspondences, result.transformation);
    
    // Quality based on error (lower error = higher quality)
    float errorQuality = std::exp(-registrationError * 10.0f);
    
    // Quality based on number of correspondences
    float correspondenceQuality = std::min(1.0f, result.correspondences.size() / 5.0f);
    
    // Combined quality score
    float quality = (errorQuality + correspondenceQuality) / 2.0f;
    
    return std::max(0.0f, std::min(1.0f, quality));
}

FeatureBasedRegistration::Parameters FeatureBasedRegistration::getRecommendedParameters(
    const std::vector<Point3D>& sourcePoints,
    const std::vector<Point3D>& targetPoints) const {
    
    Parameters params;
    
    // Get recommended extraction parameters
    params.extractionParams = m_featureExtractor->getRecommendedParameters(sourcePoints);
    
    // Adjust based on point cloud sizes
    size_t totalPoints = sourcePoints.size() + targetPoints.size();
    
    if (totalPoints > 2000000) {
        // Large point clouds - be more selective
        params.maxAngleDifference = 0.052f; // ~3 degrees
        params.maxDistanceDifference = 0.3f; // 30cm
        params.minCorrespondences = 4;
    } else if (totalPoints > 500000) {
        // Medium point clouds
        params.maxAngleDifference = 0.087f; // ~5 degrees
        params.maxDistanceDifference = 0.5f; // 50cm
        params.minCorrespondences = 3;
    }
    
    return params;
}

float FeatureBasedRegistration::calculatePlaneSimilarity(
    const Features::Plane& plane1,
    const Features::Plane& plane2,
    const Parameters& params) const {
    
    // Check if planes are similar enough to be considered
    if (!plane1.isSimilarTo(plane2, params.maxAngleDifference, params.maxDistanceDifference)) {
        return 0.0f;
    }
    
    // Calculate similarity score based on normal alignment and distance
    float normalDot = std::abs(QVector3D::dotProduct(plane1.normal, plane2.normal));
    float normalSimilarity = normalDot; // [0,1]
    
    float distanceDiff = std::abs(plane1.distance - plane2.distance);
    float distanceSimilarity = std::exp(-distanceDiff / params.maxDistanceDifference);
    
    // Combine similarities
    return (normalSimilarity + distanceSimilarity) / 2.0f;
}

QMatrix4x4 FeatureBasedRegistration::solveForRotation(
    const QList<QVector3D>& sourceNormals,
    const QList<QVector3D>& targetNormals) const {

    if (sourceNormals.size() != targetNormals.size() || sourceNormals.isEmpty()) {
        qWarning() << "Invalid normal vectors for rotation computation";
        return QMatrix4x4();
    }

    // Calculate centroids
    QVector3D sourceCentroid(0, 0, 0);
    QVector3D targetCentroid(0, 0, 0);

    for (int i = 0; i < sourceNormals.size(); ++i) {
        sourceCentroid += sourceNormals[i];
        targetCentroid += targetNormals[i];
    }
    sourceCentroid /= sourceNormals.size();
    targetCentroid /= targetNormals.size();

    // Center the vectors
    QList<QVector3D> centeredSource, centeredTarget;
    for (int i = 0; i < sourceNormals.size(); ++i) {
        centeredSource.append(sourceNormals[i] - sourceCentroid);
        centeredTarget.append(targetNormals[i] - targetCentroid);
    }

    // Compute cross-covariance matrix
    QMatrix3x3 H = computeCrossCovariance(centeredSource, centeredTarget);

    // Compute rotation matrix using SVD
    QMatrix3x3 R = svdRotation(H);

    // Convert to 4x4 matrix
    QMatrix4x4 rotation;
    rotation.setToIdentity();
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            rotation(i, j) = R(i, j);
        }
    }

    return rotation;
}

QVector3D FeatureBasedRegistration::solveForTranslation(
    const QList<QVector3D>& sourceCentroids,
    const QList<QVector3D>& targetCentroids,
    const QMatrix4x4& rotation) const {

    if (sourceCentroids.size() != targetCentroids.size() || sourceCentroids.isEmpty()) {
        qWarning() << "Invalid centroids for translation computation";
        return QVector3D();
    }

    // Calculate average centroids
    QVector3D avgSourceCentroid(0, 0, 0);
    QVector3D avgTargetCentroid(0, 0, 0);

    for (int i = 0; i < sourceCentroids.size(); ++i) {
        avgSourceCentroid += sourceCentroids[i];
        avgTargetCentroid += targetCentroids[i];
    }
    avgSourceCentroid /= sourceCentroids.size();
    avgTargetCentroid /= targetCentroids.size();

    // Apply rotation to source centroid
    QVector3D rotatedSourceCentroid = rotation.map(avgSourceCentroid);

    // Translation is the difference
    return avgTargetCentroid - rotatedSourceCentroid;
}

QMatrix3x3 FeatureBasedRegistration::computeCrossCovariance(
    const QList<QVector3D>& sourceVectors,
    const QList<QVector3D>& targetVectors) const {

    QMatrix3x3 H;
    H.fill(0.0f);

    for (int i = 0; i < sourceVectors.size(); ++i) {
        const QVector3D& s = sourceVectors[i];
        const QVector3D& t = targetVectors[i];

        // H += s * t^T
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                H(row, col) += s[row] * t[col];
            }
        }
    }

    return H;
}

QMatrix3x3 FeatureBasedRegistration::svdRotation(const QMatrix3x3& matrix) const {
    // Simplified SVD implementation for 3x3 matrices
    // This is a basic implementation - in production, use a proper SVD library

    // For now, return identity matrix as placeholder
    // In a full implementation, this would perform proper SVD decomposition
    QMatrix3x3 result;
    result.setToIdentity();

    // Basic approach: if the matrix is already close to orthogonal, use it directly
    float det = matrix.determinant();
    if (std::abs(det - 1.0f) < 0.1f) {
        // Matrix is close to orthogonal, use it as rotation
        result = matrix;

        // Ensure proper rotation matrix (det = 1)
        if (det < 0) {
            // Flip one column to ensure positive determinant
            for (int i = 0; i < 3; ++i) {
                result(i, 2) = -result(i, 2);
            }
        }
    }

    return result;
}

float FeatureBasedRegistration::calculateRegistrationError(
    const QList<QPair<Features::Plane, Features::Plane>>& correspondences,
    const QMatrix4x4& transformation) const {

    if (correspondences.isEmpty()) {
        return std::numeric_limits<float>::max();
    }

    float totalError = 0.0f;

    for (const auto& correspondence : correspondences) {
        const Features::Plane& sourcePlane = correspondence.first;
        const Features::Plane& targetPlane = correspondence.second;

        // Transform source plane
        QVector3D transformedNormal = transformation.mapVector(sourcePlane.normal);
        QVector3D transformedCentroid = transformation.map(sourcePlane.centroid);

        // Calculate errors
        float normalError = (transformedNormal - targetPlane.normal).length();
        float centroidError = (transformedCentroid - targetPlane.centroid).length();

        totalError += normalError + centroidError;
    }

    return totalError / correspondences.size();
}

} // namespace Registration
