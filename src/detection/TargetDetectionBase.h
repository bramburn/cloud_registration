#ifndef TARGETDETECTIONBASE_H
#define TARGETDETECTIONBASE_H

#include <QObject>
#include <QVariantMap>
#include <QList>
#include <memory>
#include "../registration/Target.h"
#include "../pointdata.h"

/**
 * @brief Abstract base class for all target detection algorithms
 */
class TargetDetectionBase : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Detection parameters structure
     */
    struct DetectionParams {
        // Common parameters
        float distanceThreshold = 0.01f;
        int maxIterations = 1000;
        float minQuality = 0.5f;
        bool enablePreprocessing = true;
        
        // Sphere-specific parameters
        float minRadius = 0.05f;
        float maxRadius = 0.5f;
        int minInliers = 50;
        
        // Natural point-specific parameters
        float neighborhoodRadius = 0.1f;
        float curvatureThreshold = 0.1f;
        
        QVariantMap toVariantMap() const;
        void fromVariantMap(const QVariantMap& map);
    };

    /**
     * @brief Detection result structure
     */
    struct DetectionResult {
        QList<std::shared_ptr<Target>> targets;
        int processedPoints = 0;
        double processingTime = 0.0;
        bool success = false;
        QString errorMessage;
        
        template<typename T>
        QList<std::shared_ptr<T>> getTargetsOfType() const {
            QList<std::shared_ptr<T>> result;
            for (const auto& target : targets) {
                auto typedTarget = std::dynamic_pointer_cast<T>(target);
                if (typedTarget) {
                    result.append(typedTarget);
                }
            }
            return result;
        }
    };

public:
    explicit TargetDetectionBase(QObject* parent = nullptr);
    virtual ~TargetDetectionBase() = default;
    
    virtual DetectionResult detect(const std::vector<PointFullData>& points, 
                                 const DetectionParams& params) = 0;
    
    virtual QString getAlgorithmName() const = 0;
    virtual QStringList getSupportedTargetTypes() const = 0;
    virtual bool validateParameters(const DetectionParams& params) const;
    virtual DetectionParams getDefaultParameters() const;
    virtual bool canHandlePointCount(size_t pointCount) const;

signals:
    void detectionProgress(int percentage, const QString& stage);
    void detectionCompleted(const DetectionResult& result);
    void detectionError(const QString& error);

protected:
    virtual std::vector<PointFullData> preprocessPoints(
        const std::vector<PointFullData>& points,
        const DetectionParams& params) const;
    
    void calculateNormals(std::vector<PointFullData>& points, float radius) const;
    void removeOutliers(std::vector<PointFullData>& points, 
                       int meanK = 50, float stddevMulThresh = 1.0f) const;
    void downsamplePoints(std::vector<PointFullData>& points, float voxelSize) const;
    QString generateTargetId(const QString& prefix) const;
    void emitProgress(int percentage, const QString& stage);

private:
    static int s_targetIdCounter;
};

#endif // TARGETDETECTIONBASE_H
