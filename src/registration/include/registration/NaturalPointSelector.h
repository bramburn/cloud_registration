#ifndef NATURALPOINTSELECTOR_H
#define NATURALPOINTSELECTOR_H

#include "TargetDetectionBase.h"
#include <QObject>
#include <QVector3D>
#include <QMatrix4x4>
#include <QPoint>
#include <QSize>
#include <vector>

/**
 * @brief Manual natural point selection system
 * 
 * This class provides tools for manually selecting natural feature points
 * in point cloud data. It includes raycasting for 3D point selection,
 * feature analysis for point quality assessment, and correspondence
 * suggestion algorithms.
 */
class NaturalPointSelector : public TargetDetectionBase
{
    Q_OBJECT

public:
    /**
     * @brief Ray structure for 3D picking
     */
    struct Ray {
        QVector3D origin;     ///< Ray origin point
        QVector3D direction;  ///< Ray direction (normalized)
        
        Ray() = default;
        Ray(const QVector3D& orig, const QVector3D& dir) 
            : origin(orig), direction(dir.normalized()) {}
    };

    /**
     * @brief Point selection result
     */
    struct SelectionResult {
        bool success = false;           ///< Whether selection was successful
        QVector3D selectedPoint;        ///< Coordinates of selected point
        int pointIndex = -1;            ///< Index in original point cloud
        float confidence = 0.0f;        ///< Confidence in selection quality
        QVector3D featureVector;        ///< Geometric feature descriptor
        QString description;            ///< Auto-generated description
        
        bool isValid() const {
            return success && pointIndex >= 0 && confidence > 0.0f;
        }
    };

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     */
    explicit NaturalPointSelector(QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~NaturalPointSelector() override = default;
    
    // TargetDetectionBase interface implementation
    DetectionResult detect(const std::vector<PointFullData>& points, 
                          const DetectionParams& params) override;
    
    QString getAlgorithmName() const override { return "Natural Point Selector"; }
    
    QStringList getSupportedTargetTypes() const override { 
        return QStringList() << "Natural Point"; 
    }
    
    /**
     * @brief Select a point using screen coordinates (raycasting)
     * @param points Point cloud data
     * @param viewMatrix Camera view matrix
     * @param projectionMatrix Camera projection matrix
     * @param screenPos Screen coordinates of mouse click
     * @param viewportSize Size of the viewport
     * @param selectionRadius Radius for point selection (pixels)
     * @return Selection result
     */
    SelectionResult selectPoint(const std::vector<PointFullData>& points,
                               const QMatrix4x4& viewMatrix,
                               const QMatrix4x4& projectionMatrix,
                               const QPoint& screenPos,
                               const QSize& viewportSize,
                               float selectionRadius = 5.0f);
    
    /**
     * @brief Select the closest point to a 3D position
     * @param points Point cloud data
     * @param targetPosition 3D position to find closest point to
     * @param maxDistance Maximum search distance
     * @return Selection result
     */
    SelectionResult selectClosestPoint(const std::vector<PointFullData>& points,
                                      const QVector3D& targetPosition,
                                      float maxDistance = 0.1f);
    
    /**
     * @brief Suggest corresponding points in another point cloud
     * @param sourcePoints Source point cloud
     * @param targetPoints Target point cloud  
     * @param selectedPoint Point selected in source cloud
     * @param searchRadius Search radius for correspondences
     * @return List of suggested corresponding points
     */
    std::vector<SelectionResult> suggestCorrespondences(
        const std::vector<PointFullData>& sourcePoints,
        const std::vector<PointFullData>& targetPoints,
        const SelectionResult& selectedPoint,
        float searchRadius = 0.5f);

public slots:
    /**
     * @brief Handle mouse click for point selection
     * @param points Point cloud data
     * @param viewMatrix Camera view matrix
     * @param projectionMatrix Camera projection matrix
     * @param screenPos Screen coordinates
     * @param viewportSize Viewport size
     */
    void onMouseClick(const std::vector<PointFullData>& points,
                     const QMatrix4x4& viewMatrix,
                     const QMatrix4x4& projectionMatrix,
                     const QPoint& screenPos,
                     const QSize& viewportSize);

signals:
    /**
     * @brief Emitted when a point is successfully selected
     * @param result Selection result
     */
    void pointSelected(const SelectionResult& result);
    
    /**
     * @brief Emitted when point selection fails
     * @param reason Failure reason
     */
    void selectionFailed(const QString& reason);
    
    /**
     * @brief Emitted when correspondences are suggested
     * @param correspondences List of suggested points
     */
    void correspondencesSuggested(const std::vector<SelectionResult>& correspondences);

private:
    /**
     * @brief Create ray from screen coordinates
     * @param screenPos Screen position
     * @param viewportSize Viewport size
     * @param viewMatrix View matrix
     * @param projectionMatrix Projection matrix
     * @return Ray in world coordinates
     */
    Ray createRayFromScreen(const QPoint& screenPos,
                           const QSize& viewportSize,
                           const QMatrix4x4& viewMatrix,
                           const QMatrix4x4& projectionMatrix) const;
    
    /**
     * @brief Find closest point to ray
     * @param points Point cloud data
     * @param ray Ray to test against
     * @param maxDistance Maximum distance from ray
     * @return Index of closest point, or -1 if none found
     */
    int findClosestPointToRay(const std::vector<PointFullData>& points,
                             const Ray& ray,
                             float maxDistance = 0.1f) const;
    
    /**
     * @brief Calculate distance from point to ray
     * @param point Point to test
     * @param ray Ray
     * @return Distance from point to ray
     */
    float distancePointToRay(const QVector3D& point, const Ray& ray) const;
    
    /**
     * @brief Analyze geometric features around a point
     * @param points Point cloud data
     * @param pointIndex Index of center point
     * @param radius Analysis radius
     * @return Feature vector describing local geometry
     */
    QVector3D analyzeLocalFeatures(const std::vector<PointFullData>& points,
                                  int pointIndex,
                                  float radius = 0.1f) const;
    
    /**
     * @brief Calculate point selection confidence
     * @param points Point cloud data
     * @param pointIndex Selected point index
     * @param featureVector Local feature descriptor
     * @return Confidence score (0.0 - 1.0)
     */
    float calculateSelectionConfidence(const std::vector<PointFullData>& points,
                                      int pointIndex,
                                      const QVector3D& featureVector) const;
    
    /**
     * @brief Generate description for selected point
     * @param points Point cloud data
     * @param pointIndex Selected point index
     * @param featureVector Feature descriptor
     * @return Human-readable description
     */
    QString generatePointDescription(const std::vector<PointFullData>& points,
                                    int pointIndex,
                                    const QVector3D& featureVector) const;
    
    /**
     * @brief Calculate feature similarity between two points
     * @param feature1 First feature vector
     * @param feature2 Second feature vector
     * @return Similarity score (0.0 - 1.0)
     */
    float calculateFeatureSimilarity(const QVector3D& feature1,
                                    const QVector3D& feature2) const;
    
    /**
     * @brief Find neighbors within radius
     * @param points Point cloud data
     * @param centerIndex Center point index
     * @param radius Search radius
     * @return List of neighbor indices
     */
    std::vector<int> findNeighbors(const std::vector<PointFullData>& points,
                                  int centerIndex,
                                  float radius) const;

private:
    float m_defaultSelectionRadius;    ///< Default selection radius in pixels
    float m_defaultSearchRadius;      ///< Default search radius for features
    float m_minConfidenceThreshold;   ///< Minimum confidence for valid selection
};

#endif // NATURALPOINTSELECTOR_H
