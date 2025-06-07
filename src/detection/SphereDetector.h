#ifndef SPHEREDETECTOR_H
#define SPHEREDETECTOR_H

#include "TargetDetectionBase.h"
#include <QVector3D>
#include <vector>
#include <random>
#include <map>

/**
 * @brief RANSAC-based sphere detection algorithm
 * 
 * This class implements a robust sphere detection algorithm using RANSAC
 * (Random Sample Consensus) to identify spherical targets in point cloud data.
 * The algorithm is designed to handle noisy data and can detect multiple
 * spheres in a single point cloud.
 */
class SphereDetector : public TargetDetectionBase
{
    Q_OBJECT

public:
    /**
     * @brief Sphere model parameters
     */
    struct SphereModel {
        QVector3D center;    ///< Center point of the sphere
        float radius;        ///< Radius of the sphere
        float rmsError;      ///< RMS fitting error
        int inlierCount;     ///< Number of inlier points
        float quality;       ///< Quality score (0.0 - 1.0)
        
        SphereModel() : radius(0.0f), rmsError(0.0f), inlierCount(0), quality(0.0f) {}
        
        /**
         * @brief Check if this sphere model is valid
         */
        bool isValid() const {
            return radius > 0.0f && inlierCount > 0 && quality > 0.0f;
        }
    };

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     */
    explicit SphereDetector(QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~SphereDetector() override = default;
    
    // TargetDetectionBase interface implementation
    DetectionResult detect(const std::vector<PointFullData>& points, 
                          const DetectionParams& params) override;
    
    QString getAlgorithmName() const override { return "RANSAC Sphere Detector"; }
    
    QStringList getSupportedTargetTypes() const override { 
        return QStringList() << "Sphere"; 
    }
    
    bool validateParameters(const DetectionParams& params) const override;
    
    DetectionParams getDefaultParameters() const override;

public slots:
    /**
     * @brief Detect spheres asynchronously
     * @param points Point cloud data
     * @param params Detection parameters
     */
    void detectAsync(const std::vector<PointFullData>& points, 
                    const DetectionParams& params);

private:
    /**
     * @brief Detect a single sphere using RANSAC
     * @param points Point cloud data
     * @param params Detection parameters
     * @param usedPoints Points already used by previous detections
     * @return Detected sphere model
     */
    SphereModel detectSingleSphere(const std::vector<PointFullData>& points,
                                  const DetectionParams& params,
                                  const std::vector<bool>& usedPoints) const;
    
    /**
     * @brief Fit sphere to 4 points (minimum required)
     * @param p1, p2, p3, p4 Four points to fit sphere to
     * @return Fitted sphere model
     */
    SphereModel fitSphereToPoints(const QVector3D& p1, const QVector3D& p2,
                                 const QVector3D& p3, const QVector3D& p4) const;
    
    /**
     * @brief Calculate distance from point to sphere surface
     * @param point Point to test
     * @param sphere Sphere model
     * @return Distance to sphere surface (negative if inside)
     */
    float distanceToSphere(const QVector3D& point, const SphereModel& sphere) const;
    
    /**
     * @brief Find inlier points for a sphere model
     * @param points Point cloud data
     * @param sphere Sphere model
     * @param threshold Distance threshold for inliers
     * @param usedPoints Points already used by previous detections
     * @return List of inlier point indices
     */
    std::vector<int> findInliers(const std::vector<PointFullData>& points,
                                const SphereModel& sphere,
                                float threshold,
                                const std::vector<bool>& usedPoints) const;
    
    /**
     * @brief Refine sphere model using least squares fitting
     * @param points Point cloud data
     * @param inlierIndices Indices of inlier points
     * @param initialSphere Initial sphere estimate
     * @return Refined sphere model
     */
    SphereModel refineSphereModel(const std::vector<PointFullData>& points,
                                 const std::vector<int>& inlierIndices,
                                 const SphereModel& initialSphere) const;
    
    /**
     * @brief Calculate quality score for a sphere
     * @param sphere Sphere model
     * @param totalPoints Total number of points in cloud
     * @param params Detection parameters
     * @return Quality score (0.0 - 1.0)
     */
    float calculateQuality(const SphereModel& sphere, 
                          int totalPoints,
                          const DetectionParams& params) const;
    
    /**
     * @brief Validate sphere model against parameters
     * @param sphere Sphere model to validate
     * @param params Detection parameters
     * @return true if sphere is valid
     */
    bool validateSphere(const SphereModel& sphere, 
                       const DetectionParams& params) const;
    
    /**
     * @brief Remove overlapping spheres (keep best quality)
     * @param spheres List of detected spheres
     * @param overlapThreshold Overlap threshold (0.0 - 1.0)
     * @return Filtered list of spheres
     */
    std::vector<SphereModel> removeOverlappingSpheres(
        const std::vector<SphereModel>& spheres,
        float overlapThreshold = 0.5f) const;
    
    /**
     * @brief Calculate RMS error for sphere fit
     * @param points Point cloud data
     * @param inlierIndices Indices of inlier points
     * @param sphere Sphere model
     * @return RMS error
     */
    float calculateRMSError(const std::vector<PointFullData>& points,
                           const std::vector<int>& inlierIndices,
                           const SphereModel& sphere) const;
    
    /**
     * @brief Generate random sample of 4 points
     * @param points Point cloud data
     * @param usedPoints Points already used by previous detections
     * @param generator Random number generator
     * @return Indices of 4 random points, or empty vector if not enough points
     */
    std::vector<int> generateRandomSample(const std::vector<PointFullData>& points,
                                         const std::vector<bool>& usedPoints,
                                         std::mt19937& generator) const;

private:
    mutable std::mt19937 m_randomGenerator;  ///< Random number generator for RANSAC
    
    // Algorithm parameters
    static constexpr int MIN_POINTS_FOR_SPHERE = 4;     ///< Minimum points to fit sphere
    static constexpr float MIN_SPHERE_RADIUS = 0.01f;   ///< Minimum valid sphere radius
    static constexpr float MAX_SPHERE_RADIUS = 10.0f;   ///< Maximum valid sphere radius
    static constexpr int MAX_SPHERES_PER_CLOUD = 50;    ///< Maximum spheres to detect
};

#endif // SPHEREDETECTOR_H
