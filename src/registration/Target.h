#ifndef TARGET_H
#define TARGET_H

#include <QString>
#include <QVector3D>
#include <QList>
#include <QVariantMap>
#include <QMetaType>
#include <memory>

/**
 * @brief Base class for all registration targets
 * 
 * This abstract base class defines the common interface for all types of
 * registration targets used in point cloud alignment. Targets represent
 * identifiable features that can be matched between different scans.
 * 
 * Sprint 2 Implementation: Foundational target data models
 */
class Target
{
public:
    Target(const QString& id, const QVector3D& pos);
    virtual ~Target() = default;

    // Basic properties
    QString targetId() const { return targetId_; }
    void setTargetId(const QString& id) { targetId_ = id; }
    
    QVector3D position() const { return position_; }
    void setPosition(const QVector3D& pos) { position_ = pos; }
    
    // Quality metrics
    float confidence() const { return confidence_; }
    void setConfidence(float confidence) { confidence_ = confidence; }
    
    bool isValid() const { return isValid_; }
    void setValid(bool valid) { isValid_ = valid; }
    
    // Metadata
    QString description() const { return description_; }
    void setDescription(const QString& desc) { description_ = desc; }
    
    QString scanId() const { return scanId_; }
    void setScanId(const QString& id) { scanId_ = id; }
    
    // Virtual interface
    virtual QString getType() const = 0;
    virtual QVariantMap serialize() const;
    virtual bool deserialize(const QVariantMap& data);
    virtual std::unique_ptr<Target> clone() const = 0;
    
    // Validation
    virtual bool validate() const;
    virtual QString getValidationError() const;

protected:
    QString targetId_;
    QVector3D position_;
    float confidence_;
    bool isValid_;
    QString description_;
    QString scanId_;
    
    // Helper for derived classes
    QVariantMap serializeBase() const;
    bool deserializeBase(const QVariantMap& data);
};

/**
 * @brief Sphere target for registration
 */
class SphereTarget : public Target
{
public:
    SphereTarget(const QString& id, const QVector3D& pos, float radius);
    
    // Sphere-specific properties
    float radius() const { return radius_; }
    void setRadius(float radius) { radius_ = radius; }
    
    float rmsError() const { return rmsError_; }
    void setRmsError(float error) { rmsError_ = error; }
    
    int inlierCount() const { return inlierCount_; }
    void setInlierCount(int count) { inlierCount_ = count; }
    
    float coverage() const { return coverage_; }
    void setCoverage(float coverage) { coverage_ = coverage; }
    
    // Virtual interface implementation
    QString getType() const override { return "Sphere"; }
    QVariantMap serialize() const override;
    bool deserialize(const QVariantMap& data) override;
    std::unique_ptr<Target> clone() const override;
    
    // Validation
    bool validate() const override;
    QString getValidationError() const override;

private:
    float radius_;
    float rmsError_;
    int inlierCount_;
    float coverage_;
};

/**
 * @brief Checkerboard target for registration
 */
class CheckerboardTarget : public Target
{
public:
    CheckerboardTarget(const QString& id, const QVector3D& pos, const QList<QVector3D>& corners);
    
    // Checkerboard-specific properties
    QList<QVector3D> cornerPoints() const { return cornerPoints_; }
    void setCornerPoints(const QList<QVector3D>& corners) { cornerPoints_ = corners; }
    
    QVector3D normal() const { return normal_; }
    void setNormal(const QVector3D& normal) { normal_ = normal; }
    
    float planeError() const { return planeError_; }
    void setPlaneError(float error) { planeError_ = error; }
    
    // Derived properties
    int cornerCount() const { return cornerPoints_.size(); }
    QVector3D centroid() const;
    float area() const;
    
    // Virtual interface implementation
    QString getType() const override { return "Checkerboard"; }
    QVariantMap serialize() const override;
    bool deserialize(const QVariantMap& data) override;
    std::unique_ptr<Target> clone() const override;
    
    // Validation
    bool validate() const override;
    QString getValidationError() const override;

private:
    QList<QVector3D> cornerPoints_;
    QVector3D normal_;
    float planeError_;
    
    void calculateDerivedProperties();
};

/**
 * @brief Natural point target for registration
 */
class NaturalPointTarget : public Target
{
public:
    NaturalPointTarget(const QString& id, const QVector3D& pos, const QString& desc = QString());
    
    // Natural point specific properties
    QVector3D normal() const { return normal_; }
    void setNormal(const QVector3D& normal) { normal_ = normal; }
    
    float curvature() const { return curvature_; }
    void setCurvature(float curvature) { curvature_ = curvature; }
    
    float distinctiveness() const { return distinctiveness_; }
    void setDistinctiveness(float distinctiveness) { distinctiveness_ = distinctiveness; }
    
    int neighborCount() const { return neighborCount_; }
    void setNeighborCount(int count) { neighborCount_ = count; }
    
    // Feature descriptor (for matching)
    QList<float> featureDescriptor() const { return featureDescriptor_; }
    void setFeatureDescriptor(const QList<float>& descriptor) { featureDescriptor_ = descriptor; }
    
    // Virtual interface implementation
    QString getType() const override { return "NaturalPoint"; }
    QVariantMap serialize() const override;
    bool deserialize(const QVariantMap& data) override;
    std::unique_ptr<Target> clone() const override;
    
    // Validation
    bool validate() const override;
    QString getValidationError() const override;

private:
    QVector3D normal_;
    float curvature_;
    float distinctiveness_;
    int neighborCount_;
    QList<float> featureDescriptor_;
};

// Factory function for creating targets from serialized data
std::unique_ptr<Target> createTargetFromData(const QVariantMap& data);

// Utility functions
QString targetTypeToString(const Target* target);
QVector3D calculateCentroid(const QList<QVector3D>& points);
float calculateDistance(const QVector3D& p1, const QVector3D& p2);

// Register metatypes for Qt's type system
Q_DECLARE_METATYPE(Target*)
Q_DECLARE_METATYPE(SphereTarget*)
Q_DECLARE_METATYPE(CheckerboardTarget*)
Q_DECLARE_METATYPE(NaturalPointTarget*)

#endif // TARGET_H
