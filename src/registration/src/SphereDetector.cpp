#include "SphereDetector.h"
#include "../registration/Target.h"
#include <QDebug>
#include <QThread>
#include <algorithm>
#include <cmath>
#include <chrono>

SphereDetector::SphereDetector(QObject* parent)
    : TargetDetectionBase(parent)
    , m_randomGenerator(std::chrono::steady_clock::now().time_since_epoch().count())
{
}

TargetDetectionBase::DetectionResult SphereDetector::detect(
    const std::vector<PointFullData>& points, 
    const DetectionParams& params)
{
    DetectionResult result;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    qDebug() << "SphereDetector: Starting detection with" << points.size() << "points";
    
    if (points.size() < MIN_POINTS_FOR_SPHERE) {
        result.errorMessage = QString("Insufficient points for sphere detection. Need at least %1 points.")
                             .arg(MIN_POINTS_FOR_SPHERE);
        return result;
    }
    
    if (!validateParameters(params)) {
        result.errorMessage = "Invalid detection parameters";
        return result;
    }
    
    emitProgress(0, "Preprocessing point cloud");
    
    // Preprocess points
    std::vector<PointFullData> processedPoints = preprocessPoints(points, params);
    result.processedPoints = static_cast<int>(processedPoints.size());
    
    emitProgress(20, "Detecting spheres");
    
    // Track which points have been used
    std::vector<bool> usedPoints(processedPoints.size(), false);
    std::vector<SphereModel> detectedSpheres;
    
    // Detect multiple spheres
    for (int sphereIndex = 0; sphereIndex < MAX_SPHERES_PER_CLOUD; ++sphereIndex) {
        emitProgress(20 + (sphereIndex * 60) / MAX_SPHERES_PER_CLOUD, 
                    QString("Detecting sphere %1").arg(sphereIndex + 1));
        
        SphereModel sphere = detectSingleSphere(processedPoints, params, usedPoints);
        
        if (!sphere.isValid() || !validateSphere(sphere, params)) {
            break;  // No more valid spheres found
        }
        
        detectedSpheres.push_back(sphere);
        
        // Mark inlier points as used
        std::vector<int> inliers = findInliers(processedPoints, sphere, 
                                              params.distanceThreshold, usedPoints);
        for (int idx : inliers) {
            usedPoints[idx] = true;
        }
        
        qDebug() << "Detected sphere" << sphereIndex + 1 << "- Center:" 
                 << sphere.center << "Radius:" << sphere.radius 
                 << "Quality:" << sphere.quality;
    }
    
    emitProgress(85, "Removing overlapping spheres");
    
    // Remove overlapping spheres
    detectedSpheres = removeOverlappingSpheres(detectedSpheres);
    
    emitProgress(95, "Creating target objects");
    
    // Convert sphere models to target objects
    for (size_t i = 0; i < detectedSpheres.size(); ++i) {
        const auto& sphere = detectedSpheres[i];
        
        QString targetId = generateTargetId("sphere");
        auto sphereTarget = std::make_shared<SphereTarget>(targetId, sphere.center, sphere.radius);
        
        sphereTarget->setQuality(sphere.quality);
        sphereTarget->setRMSError(sphere.rmsError);
        sphereTarget->setInlierCount(sphere.inlierCount);
        
        result.targets.append(sphereTarget);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    result.processingTime = std::chrono::duration<double>(endTime - startTime).count();
    result.success = true;
    
    emitProgress(100, "Detection completed");
    
    qDebug() << "SphereDetector: Detected" << result.targets.size() << "spheres in" 
             << result.processingTime << "seconds";
    
    return result;
}

bool SphereDetector::validateParameters(const DetectionParams& params) const
{
    if (!TargetDetectionBase::validateParameters(params)) {
        return false;
    }
    
    // Additional sphere-specific validation
    if (params.minRadius >= params.maxRadius) {
        return false;
    }
    
    if (params.minRadius < MIN_SPHERE_RADIUS || params.maxRadius > MAX_SPHERE_RADIUS) {
        return false;
    }
    
    return true;
}

TargetDetectionBase::DetectionParams SphereDetector::getDefaultParameters() const
{
    DetectionParams params = TargetDetectionBase::getDefaultParameters();
    
    // Sphere-specific defaults
    params.minRadius = 0.05f;      // 5cm minimum
    params.maxRadius = 0.5f;       // 50cm maximum
    params.minInliers = 100;       // Minimum points for reliable detection
    params.distanceThreshold = 0.005f;  // 5mm tolerance
    params.maxIterations = 2000;   // More iterations for better results
    
    return params;
}

void SphereDetector::detectAsync(const std::vector<PointFullData>& points, 
                                const DetectionParams& params)
{
    // Run detection in a separate thread to avoid blocking UI
    QThread* workerThread = QThread::create([this, points, params]() {
        DetectionResult result = detect(points, params);
        emit detectionCompleted(result);
    });
    
    connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
    workerThread->start();
}

SphereDetector::SphereModel SphereDetector::detectSingleSphere(
    const std::vector<PointFullData>& points,
    const DetectionParams& params,
    const std::vector<bool>& usedPoints) const
{
    SphereModel bestSphere;
    int bestInlierCount = 0;
    
    // Count available points
    int availablePoints = 0;
    for (bool used : usedPoints) {
        if (!used) availablePoints++;
    }
    
    if (availablePoints < MIN_POINTS_FOR_SPHERE) {
        return bestSphere;  // Not enough points available
    }
    
    // RANSAC iterations
    for (int iteration = 0; iteration < params.maxIterations; ++iteration) {
        // Generate random sample of 4 points
        std::vector<int> sampleIndices = generateRandomSample(points, usedPoints, m_randomGenerator);
        if (sampleIndices.size() != 4) {
            continue;  // Couldn't get 4 valid points
        }
        
        // Fit sphere to sample points
        QVector3D p1(points[sampleIndices[0]].x, points[sampleIndices[0]].y, points[sampleIndices[0]].z);
        QVector3D p2(points[sampleIndices[1]].x, points[sampleIndices[1]].y, points[sampleIndices[1]].z);
        QVector3D p3(points[sampleIndices[2]].x, points[sampleIndices[2]].y, points[sampleIndices[2]].z);
        QVector3D p4(points[sampleIndices[3]].x, points[sampleIndices[3]].y, points[sampleIndices[3]].z);
        
        SphereModel candidateSphere = fitSphereToPoints(p1, p2, p3, p4);
        
        if (!candidateSphere.isValid() || !validateSphere(candidateSphere, params)) {
            continue;
        }
        
        // Find inliers
        std::vector<int> inliers = findInliers(points, candidateSphere, 
                                              params.distanceThreshold, usedPoints);
        
        if (static_cast<int>(inliers.size()) > bestInlierCount && 
            static_cast<int>(inliers.size()) >= params.minInliers) {
            
            // Refine sphere using all inliers
            SphereModel refinedSphere = refineSphereModel(points, inliers, candidateSphere);
            
            if (refinedSphere.isValid() && validateSphere(refinedSphere, params)) {
                bestSphere = refinedSphere;
                bestInlierCount = static_cast<int>(inliers.size());
                bestSphere.inlierCount = bestInlierCount;
                bestSphere.quality = calculateQuality(bestSphere, availablePoints, params);
            }
        }
    }
    
    return bestSphere;
}

SphereDetector::SphereModel SphereDetector::fitSphereToPoints(
    const QVector3D& p1, const QVector3D& p2, const QVector3D& p3, const QVector3D& p4) const
{
    SphereModel sphere;
    
    // Solve for sphere center and radius using 4 points
    // This is a simplified implementation - in practice, you might want to use
    // a more robust numerical method
    
    // Calculate sphere center using the fact that it's equidistant from all 4 points
    // Set up system of equations: |center - pi|^2 = r^2 for i = 1,2,3,4
    
    // For simplicity, use centroid as initial guess and calculate average distance
    QVector3D center = (p1 + p2 + p3 + p4) * 0.25f;
    
    float r1 = (center - p1).length();
    float r2 = (center - p2).length();
    float r3 = (center - p3).length();
    float r4 = (center - p4).length();
    
    float avgRadius = (r1 + r2 + r3 + r4) * 0.25f;
    
    // Check if points are roughly equidistant (simple validation)
    float maxDeviation = std::max({std::abs(r1 - avgRadius), std::abs(r2 - avgRadius),
                                  std::abs(r3 - avgRadius), std::abs(r4 - avgRadius)});
    
    if (maxDeviation < avgRadius * 0.2f) {  // Allow 20% deviation
        sphere.center = center;
        sphere.radius = avgRadius;
        sphere.quality = 1.0f - (maxDeviation / avgRadius);
    }
    
    return sphere;
}

float SphereDetector::distanceToSphere(const QVector3D& point, const SphereModel& sphere) const
{
    float distanceToCenter = (point - sphere.center).length();
    return std::abs(distanceToCenter - sphere.radius);
}

std::vector<int> SphereDetector::findInliers(const std::vector<PointFullData>& points,
                                            const SphereModel& sphere,
                                            float threshold,
                                            const std::vector<bool>& usedPoints) const
{
    std::vector<int> inliers;
    inliers.reserve(points.size() / 10);  // Reserve some space

    for (size_t i = 0; i < points.size(); ++i) {
        if (usedPoints[i]) {
            continue;  // Skip already used points
        }

        QVector3D point(points[i].x, points[i].y, points[i].z);
        float distance = distanceToSphere(point, sphere);

        if (distance <= threshold) {
            inliers.push_back(static_cast<int>(i));
        }
    }

    return inliers;
}

SphereDetector::SphereModel SphereDetector::refineSphereModel(
    const std::vector<PointFullData>& points,
    const std::vector<int>& inlierIndices,
    const SphereModel& initialSphere) const
{
    if (inlierIndices.size() < 4) {
        return initialSphere;
    }

    SphereModel refinedSphere = initialSphere;

    // Use least squares to refine the sphere parameters
    // For simplicity, we'll use an iterative approach

    for (int iteration = 0; iteration < 10; ++iteration) {
        QVector3D centerSum(0, 0, 0);
        float radiusSum = 0.0f;

        // Calculate new center as average of inlier points
        for (int idx : inlierIndices) {
            QVector3D point(points[idx].x, points[idx].y, points[idx].z);
            centerSum += point;
        }

        QVector3D newCenter = centerSum / static_cast<float>(inlierIndices.size());

        // Calculate new radius as average distance to new center
        for (int idx : inlierIndices) {
            QVector3D point(points[idx].x, points[idx].y, points[idx].z);
            radiusSum += (point - newCenter).length();
        }

        float newRadius = radiusSum / static_cast<float>(inlierIndices.size());

        // Check for convergence
        float centerChange = (newCenter - refinedSphere.center).length();
        float radiusChange = std::abs(newRadius - refinedSphere.radius);

        refinedSphere.center = newCenter;
        refinedSphere.radius = newRadius;

        if (centerChange < 0.001f && radiusChange < 0.001f) {
            break;  // Converged
        }
    }

    // Calculate RMS error
    refinedSphere.rmsError = calculateRMSError(points, inlierIndices, refinedSphere);

    return refinedSphere;
}

float SphereDetector::calculateQuality(const SphereModel& sphere,
                                      int totalPoints,
                                      const DetectionParams& params) const
{
    if (totalPoints == 0 || sphere.inlierCount == 0) {
        return 0.0f;
    }

    // Quality based on multiple factors:
    // 1. Inlier ratio (more inliers = better)
    float inlierRatio = static_cast<float>(sphere.inlierCount) / static_cast<float>(totalPoints);

    // 2. RMS error (lower error = better)
    float errorFactor = std::exp(-sphere.rmsError / params.distanceThreshold);

    // 3. Radius validity (closer to expected range = better)
    float radiusFactor = 1.0f;
    float expectedRadius = (params.minRadius + params.maxRadius) * 0.5f;
    float radiusDeviation = std::abs(sphere.radius - expectedRadius) / expectedRadius;
    radiusFactor = std::exp(-radiusDeviation);

    // Combine factors
    float quality = inlierRatio * errorFactor * radiusFactor;

    return std::min(1.0f, quality);
}

bool SphereDetector::validateSphere(const SphereModel& sphere,
                                   const DetectionParams& params) const
{
    if (!sphere.isValid()) {
        return false;
    }

    if (sphere.radius < params.minRadius || sphere.radius > params.maxRadius) {
        return false;
    }

    if (sphere.inlierCount < params.minInliers) {
        return false;
    }

    if (sphere.quality < params.minQuality) {
        return false;
    }

    return true;
}

std::vector<SphereDetector::SphereModel> SphereDetector::removeOverlappingSpheres(
    const std::vector<SphereModel>& spheres,
    float overlapThreshold) const
{
    if (spheres.size() <= 1) {
        return spheres;
    }

    std::vector<SphereModel> filteredSpheres;
    std::vector<bool> removed(spheres.size(), false);

    // Sort spheres by quality (best first)
    std::vector<size_t> indices(spheres.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(),
              [&spheres](size_t a, size_t b) {
                  return spheres[a].quality > spheres[b].quality;
              });

    for (size_t i = 0; i < indices.size(); ++i) {
        if (removed[indices[i]]) {
            continue;
        }

        const auto& sphere1 = spheres[indices[i]];
        filteredSpheres.push_back(sphere1);

        // Mark overlapping spheres for removal
        for (size_t j = i + 1; j < indices.size(); ++j) {
            if (removed[indices[j]]) {
                continue;
            }

            const auto& sphere2 = spheres[indices[j]];

            // Calculate overlap
            float centerDistance = (sphere1.center - sphere2.center).length();
            float radiusSum = sphere1.radius + sphere2.radius;

            if (centerDistance < radiusSum * overlapThreshold) {
                removed[indices[j]] = true;  // Remove lower quality sphere
            }
        }
    }

    return filteredSpheres;
}

float SphereDetector::calculateRMSError(const std::vector<PointFullData>& points,
                                       const std::vector<int>& inlierIndices,
                                       const SphereModel& sphere) const
{
    if (inlierIndices.empty()) {
        return std::numeric_limits<float>::max();
    }

    float sumSquaredErrors = 0.0f;

    for (int idx : inlierIndices) {
        QVector3D point(points[idx].x, points[idx].y, points[idx].z);
        float distance = distanceToSphere(point, sphere);
        sumSquaredErrors += distance * distance;
    }

    return std::sqrt(sumSquaredErrors / static_cast<float>(inlierIndices.size()));
}

std::vector<int> SphereDetector::generateRandomSample(
    const std::vector<PointFullData>& points,
    const std::vector<bool>& usedPoints,
    std::mt19937& generator) const
{
    std::vector<int> availableIndices;
    availableIndices.reserve(points.size());

    // Collect available point indices
    for (size_t i = 0; i < points.size(); ++i) {
        if (!usedPoints[i]) {
            availableIndices.push_back(static_cast<int>(i));
        }
    }

    if (availableIndices.size() < 4) {
        return {};  // Not enough points
    }

    // Randomly select 4 points
    std::shuffle(availableIndices.begin(), availableIndices.end(), generator);

    return std::vector<int>(availableIndices.begin(), availableIndices.begin() + 4);
}
