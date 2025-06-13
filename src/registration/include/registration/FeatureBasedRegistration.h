#pragma once

#include "features/FeatureExtractor.h"
#include <QObject>
#include <QMatrix4x4>
#include <QList>
#include <QPair>

namespace Registration {

/**
 * @brief Feature-based registration using geometric features like planes
 */
class FeatureBasedRegistration : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief Parameters for feature-based registration
     */
    struct Parameters {
        // Plane matching parameters
        float maxAngleDifference = 0.087f;    // ~5 degrees in radians
        float maxDistanceDifference = 0.5f;   // 50cm
        int minCorrespondences = 3;           // Minimum plane correspondences
        
        // Feature extraction parameters
        Features::FeatureExtractor::PlaneExtractionParams extractionParams;
        
        // Registration quality
        float minRegistrationQuality = 0.5f;
        bool validateResult = true;
    };
    
    /**
     * @brief Result of feature-based registration
     */
    struct Result {
        bool success = false;
        QMatrix4x4 transformation;
        QList<QPair<Features::Plane, Features::Plane>> correspondences;
        float quality = 0.0f;
        QString errorMessage;
        
        // Statistics
        int sourcePlanesFound = 0;
        int targetPlanesFound = 0;
        int correspondencesFound = 0;
    };
    
    explicit FeatureBasedRegistration(QObject* parent = nullptr);
    
    /**
     * @brief Register two point clouds using plane features
     * @param sourcePoints Source point cloud
     * @param targetPoints Target point cloud
     * @param params Registration parameters
     * @return Registration result
     */
    Result registerPointClouds(const std::vector<Point3D>& sourcePoints,
                              const std::vector<Point3D>& targetPoints,
                              const Parameters& params = Parameters());
    
    /**
     * @brief Find plane correspondences between two sets of planes
     * @param sourcePlanes Planes from source scan
     * @param targetPlanes Planes from target scan
     * @param params Registration parameters
     * @return List of plane correspondences
     */
    QList<QPair<Features::Plane, Features::Plane>> findPlaneCorrespondences(
        const QList<Features::Plane>& sourcePlanes,
        const QList<Features::Plane>& targetPlanes,
        const Parameters& params = Parameters());
    
    /**
     * @brief Compute transformation from plane correspondences
     * @param correspondences Plane correspondences
     * @return Transformation matrix
     */
    QMatrix4x4 computeTransformFromPlanes(
        const QList<QPair<Features::Plane, Features::Plane>>& correspondences);
    
    /**
     * @brief Validate registration result quality
     * @param result Registration result to validate
     * @param sourcePoints Source point cloud
     * @param targetPoints Target point cloud
     * @return Quality score [0,1]
     */
    float validateRegistrationQuality(const Result& result,
                                     const std::vector<Point3D>& sourcePoints,
                                     const std::vector<Point3D>& targetPoints) const;
    
    /**
     * @brief Get recommended parameters based on point cloud characteristics
     * @param sourcePoints Source point cloud
     * @param targetPoints Target point cloud
     * @return Recommended parameters
     */
    Parameters getRecommendedParameters(const std::vector<Point3D>& sourcePoints,
                                       const std::vector<Point3D>& targetPoints) const;
    
signals:
    void registrationProgress(int percentage);
    void planesExtracted(int sourcePlanes, int targetPlanes);
    void correspondencesFound(int count);
    void registrationCompleted(const Result& result);
    
private:
    /**
     * @brief Calculate similarity score between two planes
     * @param plane1 First plane
     * @param plane2 Second plane
     * @param params Registration parameters
     * @return Similarity score [0,1], 0 = no match
     */
    float calculatePlaneSimilarity(const Features::Plane& plane1,
                                  const Features::Plane& plane2,
                                  const Parameters& params) const;
    
    /**
     * @brief Solve for rotation matrix from normal correspondences
     * @param sourceNormals Source plane normals
     * @param targetNormals Target plane normals
     * @return Rotation matrix
     */
    QMatrix4x4 solveForRotation(const QList<QVector3D>& sourceNormals,
                               const QList<QVector3D>& targetNormals) const;
    
    /**
     * @brief Solve for translation from centroid correspondences
     * @param sourceCentroids Source plane centroids
     * @param targetCentroids Target plane centroids
     * @param rotation Rotation matrix
     * @return Translation vector
     */
    QVector3D solveForTranslation(const QList<QVector3D>& sourceCentroids,
                                 const QList<QVector3D>& targetCentroids,
                                 const QMatrix4x4& rotation) const;
    
    /**
     * @brief Compute cross-covariance matrix for rotation estimation
     * @param sourceVectors Source vectors (centered)
     * @param targetVectors Target vectors (centered)
     * @return 3x3 cross-covariance matrix
     */
    QMatrix3x3 computeCrossCovariance(const QList<QVector3D>& sourceVectors,
                                     const QList<QVector3D>& targetVectors) const;
    
    /**
     * @brief Perform SVD decomposition for rotation matrix
     * @param matrix 3x3 matrix to decompose
     * @return Rotation matrix from SVD
     */
    QMatrix3x3 svdRotation(const QMatrix3x3& matrix) const;
    
    /**
     * @brief Calculate registration error for validation
     * @param correspondences Plane correspondences
     * @param transformation Applied transformation
     * @return RMS error
     */
    float calculateRegistrationError(
        const QList<QPair<Features::Plane, Features::Plane>>& correspondences,
        const QMatrix4x4& transformation) const;
    
    Features::FeatureExtractor* m_featureExtractor;
};

} // namespace Registration
